/* Native menu dispatcher fixtures: substitute each descriptor on a loaded
   building, without claiming a played campaign or construction of every type. */
#define GENESIS_NO_MAIN
#define GENESIS_SDL2
#include "../build/dune.c"
static CPU c,baseline;
static SDLHost host;
static void frames(unsigned n){
 uint64_t end=c.vdp.frames+n;
 while(!c.fault && c.vdp.frames<end){dune_mouse_observe(&host.dune_mouse,&c);machine_step(&c);}
}
static void click(int x,int y,int button){
 SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};e.button.button=button;e.button.x=x;e.button.y=y;
 dune_mouse_event(&host,&c,&e);
}
int main(void){
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_MUTE;
 c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
 c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 while(!c.fault && c.vdp.frames<4944){
  unsigned f=c.vdp.frames;
  c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;
  machine_step(&c);
 }
 if(c.fault)return 1;baseline=c;
 for(unsigned type=2;type<=18;++type){
  if(type==6 || type==14)continue; /* IX and wall have no native command menu. */
  c=baseline;host=(SDLHost){.dune_mouse={.enabled=1}};
  c.ram[0x4eba]=type;write_mem(&c,0xffc578,4,0xff4eb8);
  uint32_t camera=dune_long(&c,0xe3ec);
  int x=(int)dune_word(&c,0x4ec4)/8-(int)(camera>>16)+16;
  int y=(int)dune_word(&c,0x4ec2)/8-(int)(camera&65535)+16;
  click(x,y,SDL_BUTTON_LEFT);click(x,y,SDL_BUTTON_LEFT);frames(60);
  unsigned grid=type==11?0xbfcc:0xbf8e,cols=type==11?0xbfc8:0xbf8a;
  int context=dune_mouse_context(&c);
  if(c.fault || context!=(type==11?3:2))goto fail;
  /* Visit each enabled cell using native arrows, without activating a purchase. */
  for(unsigned k=0;k<(type==11?12u:18u);++k){
   if(c.ram[grid+k]==(type==11?255:128))continue;
   SDL_Event e={.type=SDL_MOUSEMOTION};e.motion.x=48+(k%3)*32;e.motion.y=60+(k/3)*24;
   dune_mouse_event(&host,&c,&e);frames(80);
   if(c.fault || dune_word(&c,cols)!=k%3 || dune_word(&c,cols+2)!=k/3){fprintf(stderr,"target cell=%u value=%02x moves=%llu dirty=%d context=%d mousexy=%d,%d\n",k,c.ram[grid+k],(unsigned long long)host.dune_mouse.moves,host.dune_mouse.dirty,host.dune_mouse.context,host.dune_mouse.x,host.dune_mouse.y);goto fail;}
  }
  if(type%2)click(48,60,SDL_BUTTON_LEFT);
  else click(300,210,SDL_BUTTON_RIGHT);
  frames(100);
  if(c.fault || dune_mouse_context(&c)!=1 || host.dune_mouse.clicks!=3)goto fail;
  printf("PASS native building menu type=%u all enabled hover cells + %s exit\n",type,type%2?"left-click":"right-click");fflush(stdout);
  continue;
 fail:
  fprintf(stderr,"FAIL type=%u pc=%06x context=%d callback=%06x grid=%u,%u fault=%s\n",type,c.pc,dune_mouse_context(&c),dune_long(&c,0xe002),dune_word(&c,cols),dune_word(&c,cols+2),c.reason?c.reason:"none");return 2;
 }
 return 0;
}
