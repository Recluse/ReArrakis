/* Fast adapter checks; full native selection/movement: DUNE_MOUSE_SMOKE. */
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define GENESIS_DUNE_MOUSE
#include "../genesis_recompiler/controller.h"
typedef struct {
 uint8_t ram[65536],pad_buttons[2];uint32_t pc,d[8],a[8];int fault;
 struct {unsigned rendered_frames,frame_width,frame_height;uint64_t frames;} vdp;
} CPU;
#include "../genesis_recompiler/dune_mouse.h"
#ifdef DUNE_MOUSE_SDL_TEST
#define GENESIS_SDL2
#include <SDL2/SDL.h>
typedef struct {int paused,stopped;DuneMouse dune_mouse;struct {int pointer_valid,pointer_x,pointer_y;} dune_view;} SDLHost;
static uint8_t sdl_pad_key(SDL_Keycode key){return key==SDLK_z?PAD_A:0;}
static int dune_view_mouse(SDLHost *h,const CPU *c,int mx,int my,int *x,int *y,int *world){(void)h;(void)c;*x=mx;*y=my;*world=0;return 1;}
static void dune_view_pointer(SDLHost *h,const CPU *c){(void)h;(void)c;}
static int dune_view_map(const CPU *c){return dune_mouse_context(c)==1;}
#include "../genesis_recompiler/dune_mouse_sdl.h"
#endif
static void word(CPU *c,unsigned a,unsigned v){c->ram[a]=v>>8;c->ram[a+1]=v;}
int main(void){
 CPU c={0};DuneMouse m={.enabled=1};
 c.vdp.rendered_frames=1;c.vdp.frame_width=320;c.vdp.frame_height=224;
 word(&c,0xe002,0);word(&c,0xe004,0x6092);
 word(&c,0xbf1a,24);word(&c,0xbf1c,24);word(&c,0xbf1e,296);word(&c,0xbf20,200);
 assert(!dune_mouse_point(&m,&c,-1,50));
 dune_mouse_click(&m,&c,0,223,DUNE_CLICK_SELECT);
 c.pc=0x6176;dune_mouse_observe(&m,&c);
 assert(dune_word(&c,0xbf12)==24 && dune_word(&c,0xbf14)==200);
 c.pc=0x617a;dune_mouse_observe(&m,&c);assert(m.clicks==0);
 word(&c,0xbf12,100);word(&c,0xbf14,100);
 c.pc=0x6176;dune_mouse_observe(&m,&c);
 assert(dune_word(&c,0xbf12)==24 && dune_word(&c,0xbf14)==200);
 c.pc=0x617a;dune_mouse_observe(&m,&c);assert(m.clicks==1 && (c.d[0]&65535)==0x40);
 dune_mouse_click(&m,&c,100,100,DUNE_CLICK_ORDER);
 c.pc=0x6176;dune_mouse_observe(&m,&c);
 c.pc=0x617a;dune_mouse_observe(&m,&c);dune_mouse_observe(&m,&c);
 assert((c.d[0]&65535)==0x10);
 dune_mouse_click(&m,&c,100,100,DUNE_CLICK_SELECT);
 c.pad_buttons[0]=PAD_A;c.pc=0x6176;dune_mouse_observe(&m,&c);assert(m.count==0);
 c.pad_buttons[0]=0;word(&c,0xe004,0);assert(!dune_mouse_point(&m,&c,100,100));
 /* Production: hover runs native arrow navigation; click uses its A parser. */
 word(&c,0xe004,0x4504);c.ram[0xc618]=0x13;
 word(&c,0xc578,0xff);word(&c,0xc57a,0x4eb8);
 c.ram[0x4eba]=c.ram[0xbf85]=8;
 word(&c,0xc61c,0xff);word(&c,0xc61e,0xc63c);
 memset(c.ram+0xbf8e,0x80,18);c.ram[0xbf8e]=0xfe;c.ram[0xbf91]=0;
 dune_mouse_click(&m,&c,48,84,DUNE_CLICK_SELECT);
 c.pc=0x847e;dune_mouse_observe(&m,&c);assert(c.a[0]==PAD_DOWN*2);
 word(&c,0xbf8c,1);c.pc=0x28884;dune_mouse_observe(&m,&c);
 assert(c.d[0]=='A' && c.ram[0xbf2b]=='A' && !m.count);
 dune_mouse_click(&m,&c,310,210,DUNE_CLICK_CANCEL);
 c.pc=0x847e;dune_mouse_observe(&m,&c);assert(c.a[0]==PAD_UP*2);
 word(&c,0xbf8c,0);c.pc=0x28884;dune_mouse_observe(&m,&c);assert(!m.count);
 /* Every native building menu, including passive repair/sell screens. */
 for(unsigned type=2;type<=18;++type){
  c.ram[0x4eba]=c.ram[0xbf85]=type;
  assert(dune_mouse_context(&c)==(type==6 || type==14?0:type==11?3:2));
 }
 c.ram[0x4eba]=c.ram[0xbf85]=11;
 memset(c.ram+0xbfcc,0xff,18);c.ram[0xbfcc]=28;c.ram[0xbfcf]=0;
 word(&c,0xbfc8,0);word(&c,0xbfca,0);
 dune_mouse_click(&m,&c,48,84,DUNE_CLICK_SELECT);
 c.pc=0x9188;dune_mouse_observe(&m,&c);assert(c.a[0]==PAD_DOWN*2);
 word(&c,0xbfca,1);c.pc=0x28ac0;dune_mouse_observe(&m,&c);
 assert(!m.count && c.d[0]=='A');
 dune_mouse_click(&m,&c,300,210,DUNE_CLICK_ORDER);
 c.pc=0x9188;dune_mouse_observe(&m,&c);assert(c.a[0]==PAD_UP*2);
 word(&c,0xbfca,0);c.pc=0x28ac0;dune_mouse_observe(&m,&c);assert(!m.count);
 assert(!dune_mouse_cell(&c,48,156,&(int){0},&(int){0}));
 /* Sparse production grid: route around an empty cell in the top row. */
 c.ram[0x4eba]=c.ram[0xbf85]=3;
 memset(c.ram+0xbf8e,0x80,18);
 c.ram[0xbf8e]=0xfe;c.ram[0xbf91]=0;c.ram[0xbf92]=1;c.ram[0xbf93]=2;
 word(&c,0xbf8a,0);word(&c,0xbf8c,0);
 dune_mouse_point(&m,&c,112,84);c.pc=0x847e;dune_mouse_observe(&m,&c);
 assert(c.a[0]==PAD_DOWN*2);
 word(&c,0xbf8c,1);dune_mouse_observe(&m,&c);assert(c.a[0]==PAD_RIGHT*2);
 /* Placement presenter uses a different valid cursor range. */
 word(&c,0xe004,0x6092);word(&c,0xbf1a,0);word(&c,0xbf1c,0);
 word(&c,0xbf1e,256);word(&c,0xbf20,160);assert(dune_mouse_point(&m,&c,100,100));
 /* Native front-end polling: title hover/click, cooldown and house arrows. */
 word(&c,0xe004,0);c.a[7]=0xf000;
 word(&c,0xf000,1);word(&c,0xf002,0x78d2);c.pc=0x4d4e;
 word(&c,0xd70e,160);c.vdp.frames=100;dune_mouse_observe(&m,&c);
 dune_mouse_click(&m,&c,160,176,DUNE_CLICK_SELECT);
 dune_mouse_observe(&m,&c);assert(c.d[0]==2 && m.count==1);
 word(&c,0xd70e,176);dune_mouse_observe(&m,&c);
 assert(c.d[0]==0x40 && !m.count);
 dune_mouse_click(&m,&c,160,160,DUNE_CLICK_SELECT);assert(!m.count);
 c.vdp.frames=121;c.pc=0x4938;dune_mouse_observe(&m,&c);
 word(&c,0xbef8,32);word(&c,0xbf02,0);
 dune_mouse_click(&m,&c,240,100,DUNE_CLICK_SELECT);
 dune_mouse_observe(&m,&c);assert(c.d[0]==8 && m.count==1);
 word(&c,0xbef8,208);dune_mouse_observe(&m,&c);
 assert(c.d[0]==0x40 && !m.count);
 c.vdp.frames=142;c.pc=0x4724;dune_mouse_observe(&m,&c);
 dune_mouse_click(&m,&c,160,100,DUNE_CLICK_SELECT);dune_mouse_observe(&m,&c);
 assert(c.d[0]==0x40 && !m.count);
 c.vdp.frames=170;c.pc=0x4724;dune_mouse_observe(&m,&c);
 dune_mouse_click(&m,&c,160,100,DUNE_CLICK_SELECT);
 word(&c,0xe004,0x1234);dune_mouse_observe(&m,&c);assert(!m.count);
 /* Shared pad parser called by another screen must discard an old click. */
 c.vdp.frames=200;c.pc=0x4d4e;word(&c,0xf000,1);word(&c,0xf002,0xf56a);
 dune_mouse_observe(&m,&c);dune_mouse_click(&m,&c,160,100,DUNE_CLICK_SELECT);
 word(&c,0xf002,0xf5f2);dune_mouse_observe(&m,&c);assert(!m.count);
 /* Dialog overrides stale building state; both answers use native arrows. */
 c.vdp.frames=230;c.pc=0x25cb2;word(&c,0xe004,0x4504);
 dune_mouse_observe(&m,&c);assert(dune_mouse_active(&m,&c)==7);
 word(&c,0xa62c,0x128);dune_mouse_click(&m,&c,160,192,DUNE_CLICK_SELECT);
 dune_mouse_observe(&m,&c);assert(c.d[0]==2 && m.count==1);
 word(&c,0xa62c,0x140);dune_mouse_observe(&m,&c);assert(c.d[0]==0x40 && !m.count);
 c.vdp.frames=251;dune_mouse_observe(&m,&c);
 dune_mouse_click(&m,&c,160,168,DUNE_CLICK_SELECT);dune_mouse_observe(&m,&c);
 assert(c.d[0]==1 && m.count==1);
 word(&c,0xa62c,0x128);dune_mouse_observe(&m,&c);assert(!m.count);
 /* Zoom clicks retain absolute world coordinates while native camera pans. */
 word(&c,0xe004,0x6092);word(&c,0xbf1a,24);word(&c,0xbf1c,24);
 word(&c,0xbf1e,296);word(&c,0xbf20,200);word(&c,0xe3ec,500);word(&c,0xe3ee,500);
 word(&c,0xe3fc,0);word(&c,0xe3fe,1024);word(&c,0xe400,0);word(&c,0xe402,1024);
 dune_mouse_world_click(&m,&c,1000,700,DUNE_CLICK_SELECT);
 c.pc=0x78d0;dune_mouse_observe(&m,&c);assert((c.d[0]&65535)==7 && (c.d[1]&65535)==7);
 c.pc=0x6176;dune_mouse_observe(&m,&c);assert(!m.stage && m.count==1);
 word(&c,0xe3ec,840);word(&c,0xe3ee,588);dune_mouse_observe(&m,&c);
 assert(dune_word(&c,0xbf12)==144 && dune_word(&c,0xbf14)==96);
 c.pc=0x617a;dune_mouse_observe(&m,&c);assert(m.count==1);
 c.pc=0x6176;dune_mouse_observe(&m,&c);c.pc=0x617a;dune_mouse_observe(&m,&c);assert(!m.count);
 word(&c,0xe3ec,0);word(&c,0xe3ee,388);dune_mouse_world_click(&m,&c,0,500,DUNE_CLICK_SELECT);
 c.pc=0x78d0;dune_mouse_observe(&m,&c);assert(!m.count);
 /* Legal mission border cell, outside the controller's 24px cursor inset. */
 for(unsigned mode=0;mode<2;++mode){
 for(unsigned corner=0;corner<4;++corner){
  dune_mouse_reset(&m);word(&c,0xe004,mode?0x6d10:0x6092);
  word(&c,0xe3ce,100);word(&c,0xe3d0,600);word(&c,0xe3d2,100);word(&c,0xe3d4,600);
  word(&c,0xe3fc,100);word(&c,0xe3fe,600);word(&c,0xe400,100);word(&c,0xe402,600);
  int tx=corner&1?919:101,ty=corner&2?823:101;
  word(&c,0xe3ec,corner&1?600:100);word(&c,0xe3ee,corner&2?600:100);
  dune_mouse_world_click(&m,&c,tx,ty,DUNE_CLICK_SELECT);
  c.pc=0x78d0;dune_mouse_observe(&m,&c);assert(m.count==1);
  c.pc=0x6176;dune_mouse_observe(&m,&c);assert(dune_word(&c,0xbf12)==(unsigned)(corner&1?296:24) && dune_word(&c,0xbf14)==(unsigned)(corner&2?200:24));
  c.pc=0x617a;dune_mouse_observe(&m,&c);c.pc=0x6176;dune_mouse_observe(&m,&c);c.pc=0x617a;dune_mouse_observe(&m,&c);
  c.pc=mode?0x701c:0x6486;dune_mouse_observe(&m,&c);
  assert(!m.count && dune_word(&c,0xc240)==(unsigned)((ty/32)*64+tx/32));
 }
 }
#ifdef DUNE_MOUSE_SDL_TEST
 word(&c,0xe004,0x6092);
 SDLHost host={.dune_mouse={.enabled=1}};
 SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};
 e.button.button=SDL_BUTTON_RIGHT;e.button.x=72;e.button.y=130;
 dune_mouse_event(&host,&c,&e);
 assert(host.dune_mouse.count==1 && host.dune_mouse.queue[0].action==DUNE_CLICK_ORDER);
 e.type=SDL_WINDOWEVENT;e.window.event=SDL_WINDOWEVENT_FOCUS_LOST;
 dune_mouse_event(&host,&c,&e);assert(host.dune_mouse.count==0);
 host.paused=1;e.type=SDL_MOUSEBUTTONDOWN;
 dune_mouse_event(&host,&c,&e);assert(host.dune_mouse.count==0);
#endif
 return 0;
}
