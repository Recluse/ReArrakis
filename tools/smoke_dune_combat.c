/* Harkonnen first mission: choose the house through original pad input,
   select a friendly unit and attack through the SDL mouse event adapter.
   Require the enemy to die and the simulation to continue.
   Build after tools/build_dune.py; no ROM interpreter or game-state shortcuts. */
#define GENESIS_NO_MAIN
#define GENESIS_SDL2
#include "../build/dune.c"
static CPU c;
static unsigned calls;
static SDLHost host;
static void frames(unsigned n){
 uint64_t end=c.vdp.frames+n;
 while(!c.fault && c.vdp.frames<end){
  if(c.pc==0x44f36) {calls++;fprintf(stderr,"callback frame=%llu unit=%06x\n",(unsigned long long)c.vdp.frames,dune_long(&c,0xdea0));}
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

static void unit_click(unsigned u,int button){
 unsigned camera=dune_long(&c,0xe3ec);
 int x=dune_word(&c,u+12)/8-(camera>>16)+16;
 int y=dune_word(&c,u+10)/8-(camera&65535)+16;
 fprintf(stderr,"click unit=%04x screen=%d,%d hp=%u\n",u,x,y,dune_word(&c,u+18));
 click(x,y,button);
}
int main(int argc,char **argv){
  c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;
  c.audio_mode=argc>1 && !strcmp(argv[1],"on")?AUDIO_ON:AUDIO_MUTE;
  if(c.audio_mode==AUDIO_ON && !audio_init(&c,NULL))return 3;
  c.io_tx[0]=c.io_tx[1]=0xff;c.io_tx[2]=0xfb;
  c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
  int house=0,chosen=0,game=0;
  while(!c.fault && c.vdp.frames<4944){
   unsigned f=c.vdp.frames;
   if(c.pc==0x26be4)house=1;
   if(c.pc==0x1f394){if(c.d[4]!=0)return 4;house=0;chosen=1;}
   if(chosen && (dune_long(&c,0xe002)==0x6092 || dune_long(&c,0xe002)==0x6d10))game=1;
   c.pad_buttons[0]=f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;
   if(chosen)c.pad_buttons[0]=game?0:(f%180>=90 && f%180<98?PAD_C:0);
   if(house)c.pad_buttons[0]=dune_word(&c,0xbef8)<208?PAD_RIGHT:PAD_C;
   machine_step(&c);
  }
  if(c.fault || !game || dune_word(&c,0xc274)!=0){report("cold Harkonnen boot");fprintf(stderr,"house=%u game=%u callback=%06x\n",dune_word(&c,0xc274),game,dune_long(&c,0xe002));return 5;}
 host.dune_mouse.enabled=1;
 unsigned enemy=0x26f8,old_hp=dune_word(&c,enemy+18);
 if(!old_hp || !dune_word(&c,enemy+4))return 6;
 unit_click(0x2068,SDL_BUTTON_LEFT);frames(30);
 if(dune_long(&c,0xc25c)!=0xff2068)return 7;
 SDL_Event motion={.type=SDL_MOUSEMOTION};motion.motion.x=160;motion.motion.y=112;
 dune_mouse_event(&host,&c,&motion);frames(4);
 for(unsigned n=0;n<40;n++){
  unsigned camera=dune_long(&c,0xe3ec);
  int x=dune_word(&c,enemy+12)/8-(camera>>16)+16;
  int y=dune_word(&c,enemy+10)/8-(camera&65535)+16;
  unsigned direction=y>190?PAD_DOWN:y<32?PAD_UP:x>240?PAD_RIGHT:x<32?PAD_LEFT:0;
  if(!direction)break;
  pad(direction,8);
 }
 unit_click(enemy,SDL_BUTTON_RIGHT);frames(3000);
 fprintf(stderr,"callbacks=%u enemy hp=%u -> %u flags=%04x\n",calls,old_hp,dune_word(&c,enemy+18),dune_word(&c,enemy+4));
 int result=c.fault?1:(dune_word(&c,enemy+18)!=0 || dune_word(&c,enemy+4)!=0 || host.dune_mouse.clicks!=2)?8:0;
 if(result && !c.fault)fprintf(stderr,"FAIL: enemy death or both mouse clicks not observed\n");
 report("Harkonnen early combat");
 audio_finish(&c);
 return result;
}
