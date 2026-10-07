/* Native first-mission mouse combinations, using the SDL event adapter.
   Build after tools/build_dune.py; no ROM interpreter or game-state shortcuts. */
#define GENESIS_NO_MAIN
#define GENESIS_SDL2
#include "../build/dune.c"
static CPU c,baseline;
static SDLHost host;
static void frames(unsigned n){
 uint64_t end=c.vdp.frames+n;
 while(!c.fault && c.vdp.frames<end){
  dune_mouse_observe(&host.dune_mouse,&c);machine_step(&c);
 }
}
static void click(int x,int y,int button){
 SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};
 e.button.button=button;e.button.x=x;e.button.y=y;
 dune_mouse_event(&host,&c,&e);
}
static void pad(unsigned button,unsigned n){
 SDL_Event e={.type=SDL_KEYDOWN};
 e.key.keysym.sym=button==PAD_A?SDLK_z:button==PAD_B?SDLK_x:button==PAD_C?SDLK_c:
  button==PAD_UP?SDLK_UP:button==PAD_DOWN?SDLK_DOWN:button==PAD_LEFT?SDLK_LEFT:SDLK_RIGHT;
 dune_mouse_event(&host,&c,&e);
 c.pad_buttons[0]=button;frames(n);c.pad_buttons[0]=0;frames(12);
}
static int report(const char *label){
 printf("%s %s pc=%06x frames=%llu clicks=%llu selected=%06x building=%06x page=%02x table=%06x fault=%s\n",
  c.fault?"FAIL":"PASS",label,c.pc,(unsigned long long)c.vdp.frames,
  (unsigned long long)host.dune_mouse.clicks,dune_long(&c,0xc25c),dune_long(&c,0xc578),c.ram[0xc618],dune_long(&c,0xc61c),c.reason?c.reason:"none");
 fflush(stdout);return !!c.fault;
}
static void restore(void){c=baseline;memset(&host,0,sizeof host);host.dune_mouse.enabled=1;}
int main(int argc,char **argv){
 int failed=0,cases=0;
 int directions=argc>2 && !strcmp(argv[2],"--directions");
 const char *cache="build/mouse-baseline.bin";
 if(argc>1 && !strcmp(argv[1],"--cached")){
  FILE *f=fopen(cache,"rb");if(!f || fread(&baseline,sizeof baseline,1,f)!=1)return 3;fclose(f);
  baseline.rom=rom_data;baseline.rom_size=sizeof rom_data;baseline.reason=NULL;
 }else{
  c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_MUTE;
  c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
  c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
  while(!c.fault && c.vdp.frames<4944){
   unsigned f=c.vdp.frames;
   c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;
   machine_step(&c);
  }
  if(report("cold-start"))return 1;
  baseline=c;FILE *f=fopen(cache,"wb");if(!f)return 3;fwrite(&baseline,sizeof baseline,1,f);fclose(f);
 }
 restore();
 FILE *ppm=fopen("build/mouse-baseline.ppm","wb");if(!ppm)return 3;
 fprintf(ppm,"P6\n%u %u\n255\n",c.vdp.frame_width,c.vdp.frame_height);
 fwrite(c.vdp.frame,3,c.vdp.frame_width*c.vdp.frame_height,ppm);fclose(ppm);
 const struct {const char *name;int x,y;} points[]={
  {"base",135,108},{"infantry",72,130},{"bottom-unit",105,204},
  {"right-unit",233,135},{"upper-unit",204,72},{"sand",160,176},{"minimap-area",270,180}
 };
 const int buttons[]={SDL_BUTTON_LEFT,SDL_BUTTON_RIGHT,SDL_BUTTON_MIDDLE};
 for(unsigned p=0;p<sizeof points/sizeof points[0] && !directions;++p){
  for(unsigned a=0;a<3;++a)for(unsigned b=0;b<3;++b){
   restore();click(points[p].x,points[p].y,buttons[a]);frames(20);
   uint32_t first_unit=dune_long(&c,0xc25c),first_building=dune_long(&c,0xc578);
   click(points[p].x,points[p].y,buttons[b]);frames(100);
   char label[100];snprintf(label,sizeof label,"%s-%d-%d",points[p].name,buttons[a],buttons[b]);
   printf("first unit=%06x building=%06x ",first_unit,first_building);
   failed+=report(label);++cases;
   if(p==0 && a==0 && b==0 && !c.fault){
    FILE *f=fopen("build/mouse-base-double.ppm","wb");if(!f)return 3;
    fprintf(f,"P6\n%u %u\n255\n",c.vdp.frame_width,c.vdp.frame_height);
    fwrite(c.vdp.frame,3,c.vdp.frame_width*c.vdp.frame_height,f);fclose(f);
    if(!first_building || host.dune_mouse.clicks!=2 || c.ram[0xc618]!=0x13){
     printf("FAIL production menu did not open\n");++failed;
    }
   }
  }
 }
 /* Queued double clicks, cancellations and returning to the map. */
 for(int variant=directions?12:0;variant<13;++variant){
  restore();char label[100];snprintf(label,sizeof label,"sequence-%d",variant);
  if(variant<3){
   click(135,108,SDL_BUTTON_LEFT);click(135,108,SDL_BUTTON_LEFT);frames(60);
   if(variant==0)pad(PAD_B,3);
   if(variant==1){pad(PAD_DOWN,3);pad(PAD_A,3);}
   if(variant==2){pad(PAD_C,3);pad(PAD_B,3);}
  }else if(variant==3){
   click(72,130,SDL_BUTTON_LEFT);frames(20);click(160,176,SDL_BUTTON_RIGHT);frames(60);
   click(160,176,SDL_BUTTON_LEFT);frames(20);click(70,170,SDL_BUTTON_RIGHT);
  }else if(variant==4){
   click(135,108,SDL_BUTTON_LEFT);frames(20);click(72,130,SDL_BUTTON_LEFT);frames(20);
   click(160,176,SDL_BUTTON_RIGHT);frames(60);click(135,108,SDL_BUTTON_LEFT);
  }else if(variant==5){
   for(int i=0;i<8;++i)click(135,108,SDL_BUTTON_LEFT);frames(80);pad(PAD_B,3);
  }else if(variant==6){
   click(72,130,SDL_BUTTON_LEFT);frames(20);click(160,176,SDL_BUTTON_MIDDLE);frames(20);
   click(135,108,SDL_BUTTON_LEFT);frames(20);click(135,108,SDL_BUTTON_LEFT);frames(60);pad(PAD_B,3);
  }else if(variant==7){
   pad(PAD_RIGHT,40);click(160,110,SDL_BUTTON_LEFT);frames(20);
   pad(PAD_LEFT,40);click(135,108,SDL_BUTTON_LEFT);frames(20);click(135,108,SDL_BUTTON_LEFT);
  }else if(variant==12){
   click(135,108,SDL_BUTTON_LEFT);click(135,108,SDL_BUTTON_LEFT);frames(60);
   pad(PAD_DOWN,3);pad(PAD_RIGHT,3);pad(PAD_LEFT,3);pad(PAD_UP,3);pad(PAD_A,3);
  }else{
   click(135,108,SDL_BUTTON_LEFT);click(135,108,SDL_BUTTON_LEFT);frames(60);
   if(variant==11){pad(PAD_RIGHT,3);pad(PAD_A,3);}else{
    pad(PAD_DOWN,3);if(variant==9)pad(PAD_RIGHT,3);pad(PAD_A,3);
    click(variant==10?60:192,variant==10?60:112,SDL_BUTTON_LEFT);frames(60);
    click(192,176,SDL_BUTTON_MIDDLE);frames(20);
   }
  }
  frames(120);failed+=report(label);++cases;
 }
 printf("RESULT cases=%d failures=%d\n",cases,failed);return failed?1:0;
}
