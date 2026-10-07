/* Native Windtrap -> Refinery -> Harvester -> spice unloading -> Victory. */
#define GENESIS_NO_MAIN
#define GENESIS_SDL2
#include "../build/dune.c"
static CPU c;
static unsigned refinery_calls,unloads,completion_frames;
static SDLHost host;
static void frames(unsigned n){
 uint64_t end=c.vdp.frames+n;
 while(!c.fault && c.vdp.frames<end){if(c.pc==0xb544)completion_frames++;if(c.pc==0xd06c)refinery_calls++;if(c.pc==0xd1b6)unloads++;dune_mouse_observe(&host.dune_mouse,&c);machine_step(&c);}
}
static void click(int x,int y,int button){
 SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};e.button.button=button;e.button.x=x;e.button.y=y;
 dune_mouse_event(&host,&c,&e);
}
static void base_click(void){
 uint32_t camera=dune_long(&c,0xe3ec);
 int x=(int)dune_word(&c,0x4ec4)/8-(int)(camera>>16)+16;
 int y=(int)dune_word(&c,0x4ec2)/8-(int)(camera&65535)+16;
 click(x,y,SDL_BUTTON_LEFT);
}
static void status(const char *tag){
 printf("%s frame=%llu pc=%06x callback=%06x producer=%06x clicks=%llu fault=%s Zpc=%04x 2CF=%02x\n",
  tag,(unsigned long long)c.vdp.frames,c.pc,dune_long(&c,0xe002),dune_long(&c,0xc578),
  (unsigned long long)host.dune_mouse.clicks,c.reason?c.reason:"none",c.z80_cpu.pc,c.z80_bus.ram[0x2cf]);fflush(stdout);
 if(c.fault){FILE *f=fopen("build/construction-z80.bin","wb");fwrite(c.z80_bus.ram,1,8192,f);fclose(f);}
}
static int require(int condition,const char *label){
 if(c.fault || !condition){status(label);return 0;}return 1;
}
int main(int argc,char **argv){
 int item=1;
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;
 c.audio_mode=argc>2 && !strcmp(argv[2],"on")?AUDIO_ON:AUDIO_MUTE;
 if(c.audio_mode==AUDIO_ON && !audio_init(&c,NULL))return 3;
 c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
 c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<4944){
  unsigned f=c.vdp.frames;
  c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;
  machine_step(&c);
 }
 host.dune_mouse.enabled=1;if(!require(!c.fault,"boot"))return 1;
 if(item==1){
  click(72,130,SDL_BUTTON_LEFT);frames(20);
  if(!require(dune_long(&c,0xc25c)>=0xff0000,"select-blocking-unit"))return 1;
  uint32_t unit=dune_long(&c,0xc25c),position=dune_long(&c,(unit&65535)+10);
  click(55,160,SDL_BUTTON_RIGHT);frames(300);
  if(!require(position!=dune_long(&c,(unit&65535)+10),"blocking-unit-moved"))return 1;
 }
 base_click();base_click();frames(60);
 if(!require(dune_mouse_context(&c)==2,"open-menu"))return 1;
 /* Test hover before purchase: the visible native highlight must follow. */
 SDL_Event move={.type=SDL_MOUSEMOTION};move.motion.x=48+item*32;move.motion.y=84;
 dune_mouse_event(&host,&c,&move);frames(12);
 if(!require(dune_word(&c,0xbf8a)==(unsigned)item && dune_word(&c,0xbf8c)==1,"hover"))return 1;
 click(48+item*32,84,SDL_BUTTON_LEFT);frames(100);status("purchase");
 uint32_t producer=dune_long(&c,0xc578);
 if(!require(producer>=0xff0000 && producer<=0xffffa0,"producer"))return 1;
 unsigned offset=producer&65535;
 for(unsigned n=0;n<100 && !c.fault && !(dune_word(&c,offset+6)&0x2000);++n)frames(60);
 if(!require((dune_word(&c,offset+6)&0x2000) && c.ram[offset+3]!=255,"ready"))return 1;
 status("ready");
 uint32_t camera=dune_long(&c,0xe3ec);
 int destination_x=(int)dune_word(&c,offset+12)/8-(int)(camera>>16)-(item==1?64:32)+8;
 int destination_y=(int)dune_word(&c,offset+10)/8-(int)(camera&65535)+8;
 move.motion.x=destination_x;move.motion.y=destination_y;dune_mouse_event(&host,&c,&move);frames(12);
 unsigned tile=dune_word(&c,0xc240);
 uint32_t map=read_mem(&c,0x4aa24,4),before=read_mem(&c,map+tile*4,4);
 click(destination_x,destination_y,SDL_BUTTON_LEFT);frames(120);status("placement");
 if(!require(!(dune_word(&c,offset+6)&0x2000) && c.ram[offset+3]==255,"placement-consumed"))return 1;
 if(!require(read_mem(&c,map+tile*4,4)!=before,"map-changed"))return 1;
 frames(600);status("after-placement");
 if(!require(!c.fault,"sound-after-placement"))return 1;
 /* Return to production and leave it entirely with the mouse. */
 base_click();frames(20);base_click();frames(60);
 if(!require(dune_mouse_context(&c)==2,"reopen-menu"))return 1;
 click(112,84,SDL_BUTTON_LEFT);frames(100);status("refinery-purchase");
 for(unsigned n=0;n<100 && !c.fault && !(dune_word(&c,offset+6)&0x2000);++n)frames(60);
 if(!require((dune_word(&c,offset+6)&0x2000) && c.ram[offset+3]!=255,"refinery-ready"))return 1;
 status("refinery-ready");
 /* Scroll to leave room below the construction yard for the 3x2 footprint. */
 move.motion.x=160;move.motion.y=112;dune_mouse_event(&host,&c,&move);
 c.pad_buttons[0]=PAD_DOWN;frames(60);c.pad_buttons[0]=0;frames(12);
 camera=dune_long(&c,0xe3ec);
 destination_x=(int)dune_word(&c,offset+12)/8-(int)(camera>>16)+8;
 destination_y=(int)dune_word(&c,offset+10)/8-(int)(camera&65535)+64+8;
 fprintf(stderr,"refinery destination=%d,%d\n",destination_x,destination_y);
 click(destination_x,destination_y,SDL_BUTTON_LEFT);frames(120);status("refinery-placement");
 if(!require(!(dune_word(&c,offset+6)&0x2000) && c.ram[offset+3]==255,"refinery-installed"))return 1;
 frames(12000);status("refinery-operation");
 unsigned harvesters=0;
 for(unsigned u=0x1000;u<0x47cc;u+=140)
  if((dune_word(&c,u+4)&3)==3 && c.ram[u+2]==16 && c.ram[u+8]==dune_word(&c,0xc274))harvesters++;
 fprintf(stderr,"refinery calls=%u unloads=%u harvesters=%u\n",refinery_calls,unloads,harvesters);
 if(!require(refinery_calls && unloads && harvesters,"refinery-harvester"))return 1;
 /* Continue harvesting until the mission objective triggers the original
    completion transition. No credits or victory flags are edited. */
 for(unsigned n=0;n<400 && !c.fault && !completion_frames;n++)frames(60);
 if(!require(completion_frames,"mission-completion-entered"))return 1;
 frames(1800);status("after-mission-completion");
 if(!require(!c.fault,"mission-completion-continued"))return 1;
 fprintf(stderr,"completion callback calls=%u\n",completion_frames);
 FILE *out=fopen("build/mission-completion.ppm","wb");if(!out)return 3;
 fprintf(out,"P6\n%u %u\n255\n",c.vdp.frame_width,c.vdp.frame_height);
 fwrite(c.vdp.frame,3,c.vdp.frame_width*c.vdp.frame_height,out);fclose(out);
 audio_finish(&c);puts("PASS refinery harvesting and mission completion");return 0;

}
