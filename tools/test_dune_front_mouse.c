/* Cold boot and original menus/cutscenes driven only by SDL mouse events. */
#define GENESIS_NO_MAIN
#define GENESIS_SDL2
#include "../build/dune.c"
static CPU c,title;
static SDLHost host;
static void click(int x,int y){SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};e.button.button=SDL_BUTTON_LEFT;e.button.x=x;e.button.y=y;dune_mouse_event(&host,&c,&e);}
static void move(int x,int y){SDL_Event e={.type=SDL_MOUSEMOTION};e.motion.x=x;e.motion.y=y;dune_mouse_event(&host,&c,&e);}
static void snap(const char *name){char p[100];snprintf(p,sizeof p,"build/front-%s.ppm",name);FILE *o=fopen(p,"wb");fprintf(o,"P6\n%u %u\n255\n",c.vdp.frame_width,c.vdp.frame_height);fwrite(c.vdp.frame,3,c.vdp.frame_width*c.vdp.frame_height,o);fclose(o);}
static int front_fail(const char *stage){fprintf(stderr,"FAIL %s frame=%llu pc=%06x callback=%06x front=%d context=%d clicks=%llu fault=%s\n",stage,(unsigned long long)c.vdp.frames,c.pc,dune_long(&c,0xe002),host.dune_mouse.front_kind,dune_mouse_active(&host.dune_mouse,&c),(unsigned long long)host.dune_mouse.clicks,c.reason?c.reason:"none");return 1;}
static void step(void){dune_mouse_observe(&host.dune_mouse,&c);machine_step(&c);}
int main(void){
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_MUTE;
 c.io_tx[0]=c.io_tx[1]=255;c.io_tx[2]=251;c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 host.dune_mouse.enabled=1;
 while(!c.fault && c.vdp.frames<6000){
  dune_mouse_observe(&host.dune_mouse,&c);
  int front=dune_mouse_active(&host.dune_mouse,&c);
  if(front==5){title=c;snap("title");break;}
  if(front==4 && !host.dune_mouse.count && c.vdp.frames>=host.dune_mouse.cooldown)click(160,100);
  machine_step(&c);
 }
 if(c.fault || dune_mouse_active(&host.dune_mouse,&c)!=5 || !host.dune_mouse.clicks)return front_fail("cold mouse intro -> title");
 printf("PASS cold intro skipped by mouse: title frame=%llu clicks=%llu\n",(unsigned long long)c.vdp.frames,(unsigned long long)host.dune_mouse.clicks);fflush(stdout);
 for(unsigned item=0;item<3;++item){
  c=title;host=(SDLHost){.dune_mouse={.enabled=1}};dune_mouse_observe(&host.dune_mouse,&c);
  move(160,160+item*8);uint64_t end=c.vdp.frames+100;
  while(!c.fault && c.vdp.frames<end)step();
  if(dune_word(&c,0xd70e)!=160+item*8)return front_fail("title hover");
  click(160,160+item*8);end=c.vdp.frames+300;
  while(!c.fault && c.pc!=0x17a12 && c.vdp.frames<end)step();
  if(c.fault || c.pc!=0x17a12 || c.d[6]!=item || host.dune_mouse.clicks!=1)return front_fail("title item activation");
  printf("PASS title item=%u native result=%u\n",item,c.d[6]);fflush(stdout);
  if(item==1){
   uint64_t limit=c.vdp.frames+400;
   while(!c.fault && c.vdp.frames<limit)step();
   if(c.fault)return front_fail("options screen");snap("options");
  }
 }
 for(unsigned house=0;house<3;++house){
  c=title;host=(SDLHost){.dune_mouse={.enabled=1}};dune_mouse_observe(&host.dune_mouse,&c);click(160,160);
  int selected=0,sent=0;uint64_t end=c.vdp.frames+7000;
  while(!c.fault && c.vdp.frames<end){
   dune_mouse_observe(&host.dune_mouse,&c);int front=dune_mouse_active(&host.dune_mouse,&c);
   if(front==6 && !sent){snap("houses");click(48+88*house,112);sent=1;}
   else if((front==4 || front==7) && !host.dune_mouse.count && c.vdp.frames>=host.dune_mouse.cooldown)click(160,front==7?168:100);
   if(c.pc==0x1f394){unsigned expected=house==0?1:house==1?2:0;if(c.d[4]!=expected)return front_fail("house native result");selected=1;}
   if(selected && dune_mouse_context(&c)==1)break;
   machine_step(&c);
  }
  if(c.fault || !sent || !selected || dune_mouse_context(&c)!=1)return front_fail("house -> mission by mouse");
  printf("PASS house=%u player=%u mouse-only mission frame=%llu\n",house,dune_word(&c,0xc274),(unsigned long long)c.vdp.frames);fflush(stdout);
  uint64_t settle=c.vdp.frames+180;while(!c.fault && c.vdp.frames<settle)step();
  uint32_t camera=dune_long(&c,0xe3ec);
  int x=(int)dune_word(&c,0x4ec4)/8-(int)(camera>>16)+16;
  int y=(int)dune_word(&c,0x4ec2)/8-(int)(camera&65535)+16;
  click(x,y);click(x,y);settle=c.vdp.frames+100;
  while(!c.fault && c.vdp.frames<settle)step();
  if(c.fault || dune_mouse_context(&c)!=2)return front_fail("map -> construction mouse regression");
  printf("PASS house=%u construction menu after intro mouse controls\n",house);fflush(stdout);
 }
 return 0;
}
