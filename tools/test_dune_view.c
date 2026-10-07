/* Real SDL software renderer, native map/sprite state and zoomed mouse orders.
   Run with SDL_VIDEODRIVER=dummy: no user window or background game process. */
#define GENESIS_NO_MAIN
#define GENESIS_SDL2
#include "../build/dune.c"
static CPU c,before;
static SDLHost host;
static void frames(unsigned n){uint64_t end=c.vdp.frames+n;while(!c.fault && c.vdp.frames<end){uint64_t previous=c.vdp.frames;dune_mouse_observe(&host.dune_mouse,&c);machine_step(&c);if(host.renderer && c.vdp.frames!=previous)dune_view_draw(&host,&c);}}
static int check(int ok,const char *label){if(!ok || c.fault){fprintf(stderr,"FAIL %s pc=%06x cam=%u,%u queue=%u fault=%s\n",label,c.pc,dune_word(&c,0xe3ec),dune_word(&c,0xe3ee),host.dune_mouse.count,c.reason?c.reason:"none");return 0;}return 1;}
static void click_world(int wx,int wy,int button){
 int w,h;SDL_GetRendererOutputSize(host.renderer,&w,&h);DuneLayout p=dune_view_host_layout(&host,&c,w,h);
 SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};e.button.button=button;e.button.x=(int)((wx-p.left+0.5)*p.world_scale);e.button.y=(int)((wy-p.top+0.5)*p.world_scale);
 dune_mouse_event(&host,&c,&e);
}
static int show_world(int wx,int wy){
 for(unsigned n=0;n<400;++n){
  if(!dune_view_draw(&host,&c))return 0;
  DuneLayout p=dune_view_host_layout(&host,&c,1280,720);
  int x=(int)((wx-p.left)*p.world_scale),y=(int)((wy-p.top)*p.world_scale);
  if(x>=16 && x<1264 && y>=16 && y<704)return 1;
  SDL_Event e={.type=SDL_MOUSEMOTION};e.motion.x=x<16?4:x>=1264?1276:640;e.motion.y=y<16?4:y>=704?716:360;
  dune_mouse_event(&host,&c,&e);frames(1);
 }
 return check(0,"show border target by SDL edge scrolling");
}
static int unit_border(void){
 unsigned unit=0;
 for(unsigned u=0x1000;u<0x47cc;u+=140)
  if((dune_word(&c,u+4)&3)==3 && c.ram[u+2]==15 && c.ram[u+8]==0){unit=u;break;}
 if(!check(unit!=0,"native Harkonnen border test unit"))return 0;
 int x=dune_word(&c,unit+12)/8+4,y=dune_word(&c,unit+10)/8+4;
 if(!show_world(x,y))return 0;click_world(x,y,SDL_BUTTON_LEFT);frames(30);
 if(!check(dune_long(&c,0xc25c)==0xff0000+unit,"select native unit"))return 0;
 int top=dune_word(&c,0xe3ce);if(!show_world(x,top+1))return 0;
 click_world(x,top+1,SDL_BUTTON_RIGHT);
 for(unsigned frame=0;frame<30;++frame){frames(1);if(!dune_view_draw(&host,&c))return 0;}
 if(!check(!host.dune_mouse.count && dune_word(&c,0xc240)==(unsigned)((top/32)*64+x/32),"unit order retains exact top border cell"))return 0;
 unsigned n=0;while(!c.fault && dune_word(&c,unit+10)/8>=top+32 && n++<50)frames(60);
 printf("unit=%06x position=%u,%u mission top=%d frames=%u\n",0xff0000+unit,dune_word(&c,unit+12)/8,dune_word(&c,unit+10)/8,top,n*60);fflush(stdout);
 if(!check(dune_word(&c,unit+10)/8<top+32,"unit reaches northernmost mission row"))return 0;
 x=dune_word(&c,unit+12)/8+4;y=dune_word(&c,unit+10)/8+4;
 if(!show_world(x,y))return 0;click_world(x,y,SDL_BUTTON_MIDDLE);frames(20);click_world(x,y,SDL_BUTTON_LEFT);frames(30);
 if(!check(dune_long(&c,0xc25c)==0xff0000+unit,"select unit at mission border"))return 0;
 puts("PASS Harkonnen unit order and selection at northern mission border");return 1;
}
static int minimap_isolation(void){
  DuneLayout p=dune_view_host_layout(&host,&c,1280,720);
  DuneView reference={.cursor_hidden=1},overlap={.cursor_hidden=1};
  if(!dune_view_build(&reference,&c,&p))return 0;
  before=c;unsigned cursor=dune_word(&before,0xbf18);
  write_mem(&before,0xffbf12,2,270);write_mem(&before,0xffbf14,2,160);
  write_mem(&before,0xffbf18,2,0);overlap.shadow=malloc(sizeof c);
  uint8_t sprites[320*224];
  if(!overlap.shadow || !dune_view_sprites(&overlap,&before,dune_word(&before,0xe3ec),dune_word(&before,0xe3ee),sprites,1))return 0;
  vdp_render_pixels(&overlap.shadow->vdp,320,224,sprites,before.vdp.frame);
  write_mem(&before,0xffbf18,2,cursor);
  unsigned contamination=0;
  for(unsigned y=144;y<208;++y)for(unsigned x=240;x<304;++x)
   contamination+=!!memcmp(before.vdp.frame+(y*320+x)*3,reference.ui+(y*320+x)*4,3);
  if(!check(contamination>20,"native cursor overlaps minimap fixture"))return 0;
  if(!dune_view_build(&overlap,&before,&p))return 0;
  if(!check(!memcmp(reference.ui,overlap.ui,320*224*4),"minimap never copies world cursor from live framebuffer"))return 0;
  if(!check(!memcmp(reference.pixels,overlap.pixels,reference.width*reference.height*3),"HUD hover hides world cursor and skips live frame copy"))return 0;
  free(reference.shadow);free(reference.pixels);free(reference.ui);
  free(overlap.shadow);free(overlap.pixels);free(overlap.ui);
  puts("PASS native cursor/minimap overlap isolation");
 return 1;
}
int main(int argc,char **argv){
 c.rom=rom_data;c.rom_size=sizeof rom_data;c.sr=0x2700;c.audio_mode=AUDIO_MUTE;
 c.io_tx[0]=c.io_tx[1]=255;c.io_tx[2]=251;c.a[7]=read_mem(&c,0,4);c.ssp=c.a[7];c.pc=read_mem(&c,4,4)&0xffffff;
 if(argc>1 && !strcmp(argv[1],"--cached")){
  FILE *f=fopen("build/mouse-baseline.bin","rb");if(!f || fread(&c,sizeof c,1,f)!=1)return 2;fclose(f);
  c.rom=rom_data;c.rom_size=sizeof rom_data;c.reason=NULL;
 }
 if(argc>1 && (!strcmp(argv[1],"--hark") || !strcmp(argv[1],"--hark-border"))){
  host.dune_mouse.enabled=1;int selected=0,sent=0;
  while(!c.fault && c.vdp.frames<7000){
   dune_mouse_observe(&host.dune_mouse,&c);int front=dune_mouse_active(&host.dune_mouse,&c);
   SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};e.button.button=SDL_BUTTON_LEFT;e.button.x=160;e.button.y=100;
   if(front==5){e.button.y=160;if(!host.dune_mouse.count && c.vdp.frames>=host.dune_mouse.cooldown)dune_mouse_event(&host,&c,&e);}
   else if(front==6 && !sent){e.button.x=224;e.button.y=112;dune_mouse_event(&host,&c,&e);sent=1;}
   else if((front==4 || front==7) && !host.dune_mouse.count && c.vdp.frames>=host.dune_mouse.cooldown){if(front==7)e.button.y=168;dune_mouse_event(&host,&c,&e);}
   if(c.pc==0x1f394 && c.d[4]==0)selected=1;
   if(selected && dune_mouse_context(&c)==1)break;
   machine_step(&c);
  }
  host.dune_view.pointer_valid=0;dune_mouse_reset(&host.dune_mouse);frames(180);
 }else while(!c.fault && c.vdp.frames<4944){unsigned f=c.vdp.frames;c.pad_buttons[0]=f>=3350?0:f>600 && f%180<8?PAD_START:f>900 && f%180>=90 && f%180<98?PAD_C:0;machine_step(&c);}
 if(!check(dune_mouse_context(&c)==1,"boot"))return 1;
 SDL_setenv("SDL_VIDEODRIVER","dummy",0);if(SDL_Init(SDL_INIT_VIDEO))return 2;
 host.window=SDL_CreateWindow("Dune test",0,0,960,672,SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE);
 host.renderer=SDL_CreateRenderer(host.window,-1,SDL_RENDERER_SOFTWARE);if(!host.renderer)return 3;
 host.dune_mouse.enabled=1;host.dune_view.zoom=100;
 const int sizes[][2]={{960,672},{1280,720},{800,1000},{1920,480},{320,224}};
 for(unsigned i=0;i<5;++i)for(unsigned zoom=50;zoom<=100;zoom+=10){
  SDL_SetWindowSize(host.window,sizes[i][0],sizes[i][1]);SDL_PumpEvents();
  SDL_Event pending;while(SDL_PollEvent(&pending))dune_view_event(&host,&c,&pending);
  host.dune_view.zoom=zoom;before=c;
  if(!check(dune_view_draw(&host,&c),"adaptive draw"))return 4;
  if(!check(!memcmp(&before,&c,sizeof c),"presentation changes no live CPU/device state"))return 5;
  DuneLayout p=dune_view_layout(&c,sizes[i][0],sizes[i][1],zoom);
  if(!check(host.dune_view.width==(unsigned)p.world_width && host.dune_view.height==(unsigned)p.world_height,"automatic texture dimensions"))return 6;
  if(i==0 && zoom==100){
   uint8_t *image=malloc(960*672*3);if(!image || SDL_RenderReadPixels(host.renderer,NULL,SDL_PIXELFORMAT_RGB24,image,960*3))return 7;
   for(unsigned y=0;y<224;++y)for(unsigned x=0;x<320;++x)
    if(memcmp(image+(((y*3+1)*960+x*3+1)*3),c.vdp.frame+(y*320+x)*3,3)){free(image);return 7;}
   free(image);puts("PASS 100% original frame pixel-exact");
  }
  int mx=(int)((sizes[i][0]-320*p.ui_scale)+280*p.ui_scale),my=(int)(sizes[i][1]-224*p.ui_scale+160*p.ui_scale),x,y,world;
  if(!check(dune_view_mouse(&host,&c,mx,my,&x,&y,&world) && !world && abs(x-280)<=1 && abs(y-160)<=1,"HUD hit at all aspect ratios"))return 7;
  printf("PASS view %dx%d zoom=%u world=%ux%u\n",sizes[i][0],sizes[i][1],zoom,host.dune_view.width,host.dune_view.height);fflush(stdout);
 }
 SDL_SetWindowSize(host.window,1280,720);SDL_PumpEvents();
 SDL_Event pending;while(SDL_PollEvent(&pending))dune_view_event(&host,&c,&pending);host.dune_view.zoom=50;
 if(!dune_view_draw(&host,&c))return 8;
 FILE *out=fopen("build/dune-zoom.ppm","wb");fprintf(out,"P6\n1280 720\n255\n");uint8_t *rgb=malloc(1280*720*3);
 if(SDL_RenderReadPixels(host.renderer,NULL,SDL_PIXELFORMAT_RGB24,rgb,1280*3))return 9;fwrite(rgb,3,1280*720,out);fclose(out);free(rgb);
 if(!minimap_isolation())return 38;
 /* The mouse cursor is a world sprite, never a fixed HUD sprite. Exercise
    its full shape across both replay patch seams and several zoom factors. */
 for(unsigned z=50;z<=100;z+=25){
  host.dune_view.zoom=z;
  for(unsigned seam=0;seam<3;++seam){
   DuneLayout p=dune_view_host_layout(&host,&c,1280,720);
   int wx=p.left+256+seam*4,wy=p.top+192+seam*4;
   host.dune_view.pointer_valid=0;dune_mouse_world_point(&host.dune_mouse,&c,wx,wy);
   host.dune_view.frame=UINT64_MAX;if(!dune_view_draw(&host,&c))return 24;
   if(z==50 && seam==0){
    FILE *f=fopen("build/dune-mouse-view.ppm","wb");uint8_t *image=malloc(1280*720*3);
    if(!f || !image || SDL_RenderReadPixels(host.renderer,NULL,SDL_PIXELFORMAT_RGB24,image,1280*3))return 36;
    fprintf(f,"P6\n1280 720\n255\n");fwrite(image,3,1280*720,f);fclose(f);free(image);
   }
   before=c;before.ram[dune_word(&c,0xbf18)+7]|=0x80;
   DuneView no_cursor={.cursor_valid=1,.cursor_x=wx,.cursor_y=wy};if(!dune_view_build(&no_cursor,&before,&p))return 25;
   unsigned changed=0;int minx=2048,miny=2048,maxx=-1,maxy=-1;
   for(unsigned y=0;y<host.dune_view.height;++y)for(unsigned x=0;x<host.dune_view.width;++x){
    unsigned at=(y*host.dune_view.width+x)*3;
    if(memcmp(host.dune_view.pixels+at,no_cursor.pixels+at,3)){
     int dx=(int)x+p.left-wx,dy=(int)y+p.top-wy;
     if(dx<minx)minx=dx;if(dx>maxx)maxx=dx;if(dy<miny)miny=dy;if(dy>maxy)maxy=dy;++changed;
    }
   }
   free(no_cursor.shadow);free(no_cursor.pixels);free(no_cursor.ui);
   int centered=dune_long(&c,0xe002)!=0x6d10 || (minx<0 && maxx>0 && miny<0 && maxy>0);
   if(!check(changed>20 && minx>=-32 && maxx<=32 && miny>=-32 && maxy<=32 && centered,"complete world cursor at mouse and replay seams"))return 26;
   unsigned native_cursor=dune_word(&c,0xbf14)*320+dune_word(&c,0xbf12);
   if(!check(!host.dune_view.ui[native_cursor*4+3],"no ghost cursor in HUD"))return 27;
  }
 }
 host.dune_view.zoom=50;
 DuneLayout stable=dune_view_host_layout(&host,&c,1280,720);
 int original_x=dune_word(&c,0xe3ec),original_y=dune_word(&c,0xe3ee);
 dune_mouse_world_point(&host.dune_mouse,&c,original_x+48,original_y+48);frames(120);
 if(!check(dune_word(&c,0xe3ec)==original_x && dune_word(&c,0xe3ee)==original_y,"interior hover never scrolls camera"))return 28;
 puts("PASS cursor seams, HUD isolation and stable hover");
 /* Select a real friendly unit exposed by zoom, outside the original cursor bounds. */
 unsigned unit=0;int wx=0,wy=0;
 for(unsigned u=0x1000;u<0x47cc;u+=140){
  if((dune_word(&c,u+4)&3)!=3 || c.ram[u+8]!=dune_word(&c,0xc274) || c.ram[u+2]>=17)continue;
  int ux=dune_word(&c,u+12)/8+4,uy=dune_word(&c,u+10)/8+4,x,y;
  if(!dune_mouse_world_local(&c,ux,uy,&x,&y)){unit=0xff0000+u;wx=ux;wy=uy;break;}
 }
 if(!check(unit!=0,"offscreen friendly unit"))return 10;
 printf("unit=%06x world=%d,%d camera limits=%u..%u,%u..%u\n",unit,wx,wy,dune_word(&c,0xe3fc),dune_word(&c,0xe3fe),dune_word(&c,0xe400),dune_word(&c,0xe402));fflush(stdout);
 click_world(wx,wy,SDL_BUTTON_LEFT);if(!check(host.dune_mouse.count==1,"zoom click queued"))return 11;
 frames(180);if(!check(dune_long(&c,0xc25c)==unit && !host.dune_mouse.count,"native selection outside old viewport"))return 12;
 DuneLayout after_select=dune_view_host_layout(&host,&c,1280,720);
 if(!check(after_select.left==stable.left && after_select.top==stable.top,"offscreen selection preserves displayed camera"))return 29;
 int tx=wx+128,ty=wy;uint32_t previous=dune_long(&c,(unit&65535)+10);
 if(!dune_view_draw(&host,&c))return 13;click_world(tx,ty,SDL_BUTTON_RIGHT);frames(600);
 if(!check(dune_long(&c,(unit&65535)+10)!=previous && !host.dune_mouse.count,"native movement from zoomed right click"))return 14;
 /* Production is letterboxed normally, then placement returns to the zoomed world. */
 int base_x=dune_word(&c,0x4ec4)/8,base_y=dune_word(&c,0x4ec2)/8;
 if(!dune_view_draw(&host,&c))return 18;
 click_world(base_x+16,base_y+16,SDL_BUTTON_LEFT);frames(20);
 if(!dune_view_draw(&host,&c))return 18;
 click_world(base_x+16,base_y+16,SDL_BUTTON_LEFT);frames(80);
 if(!check(dune_mouse_context(&c)==2,"zoomed building opens native menu"))return 19;
 if(!dune_view_draw(&host,&c))return 20;
 DuneLayout menu=dune_view_layout(&c,1280,720,host.dune_view.zoom);
 SDL_Event buy={.type=SDL_MOUSEBUTTONDOWN};buy.button.button=SDL_BUTTON_LEFT;
 buy.button.x=(int)(menu.ui_x+48*menu.ui_scale);buy.button.y=(int)(menu.ui_y+84*menu.ui_scale);
 dune_mouse_event(&host,&c,&buy);frames(100);
 unsigned producer=dune_long(&c,0xc578)&65535;
 for(unsigned n=0;n<100 && !(dune_word(&c,producer+6)&0x2000) && !c.fault;++n)frames(60);
 if(!check(dune_word(&c,producer+6)&0x2000,"mouse purchase ready"))return 21;
 if(!dune_view_draw(&host,&c))return 22;
 if(!minimap_isolation())return 39;
 DuneLayout hud=dune_view_host_layout(&host,&c,1280,720);
 SDL_Event minimap_hover={.type=SDL_MOUSEMOTION};
 minimap_hover.motion.x=(int)(1280-320*hud.ui_scale+280*hud.ui_scale);
 minimap_hover.motion.y=(int)(720-224*hud.ui_scale+160*hud.ui_scale);
 dune_mouse_event(&host,&c,&minimap_hover);frames(12);
 if(!dune_view_draw(&host,&c) || !check(host.dune_view.cursor_hidden && !host.dune_mouse.point_world,"SDL minimap hover hides placement cursor"))return 44;
 DuneLayout after_menu=dune_view_host_layout(&host,&c,1280,720);
 if(!check(after_menu.left==stable.left && after_menu.top==stable.top,"building menu preserves displayed overview"))return 37;
 int place_x=base_x-32+8,place_y=base_y+8;
 DuneLayout placement=dune_view_host_layout(&host,&c,1280,720);
 SDL_Event hover={.type=SDL_MOUSEMOTION};hover.motion.x=(int)((place_x-placement.left+0.5)*placement.world_scale);
 hover.motion.y=(int)((place_y-placement.top+0.5)*placement.world_scale);dune_mouse_event(&host,&c,&hover);frames(12);
 unsigned tile=dune_word(&c,0xc240);uint32_t old_tile=read_mem(&c,0xff7d9c+tile*4,4);
 if(!check(tile==(unsigned)((place_y/32)*64+place_x/32),"placement preview matches hovered world cell"))return 30;
 click_world(place_x,place_y,SDL_BUTTON_LEFT);frames(120);
 if(!check(!(dune_word(&c,producer+6)&0x2000) && read_mem(&c,0xff7d9c+tile*4,4)!=old_tile,"zoomed native building placement"))return 23;
 puts("PASS scaled production menu and zoomed native placement");
 if(argc>1 && !strcmp(argv[1],"--hark-border")){if(!unit_border())return 45;sdl_host_close(&host);return 0;}
 SDL_Event pointer={.type=SDL_MOUSEMOTION};pointer.motion.x=600;pointer.motion.y=300;dune_mouse_event(&host,&c,&pointer);
 int ax,ay,aw,bx,by,bw;dune_view_mouse(&host,&c,600,300,&ax,&ay,&aw);
 SDL_Event e={.type=SDL_MOUSEWHEEL};e.wheel.y=1;dune_view_event(&host,&c,&e);
 if(!check(host.dune_view.zoom==60,"wheel zoom"))return 15;
 dune_view_mouse(&host,&c,600,300,&bx,&by,&bw);
 if(!check(aw && bw && abs(ax-bx)<=1 && abs(ay-by)<=1,"zoom keeps the same world point under the mouse"))return 31;
 e.wheel.direction=SDL_MOUSEWHEEL_FLIPPED;dune_view_event(&host,&c,&e);if(!check(host.dune_view.zoom==50,"flipped wheel"))return 16;
 e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_0;dune_view_event(&host,&c,&e);if(!check(host.dune_view.zoom==100,"reset zoom"))return 17;
 pointer.motion.x=4;pointer.motion.y=300;dune_mouse_event(&host,&c,&pointer);
 DuneLayout edge=dune_view_host_layout(&host,&c,1280,720);
 for(unsigned n=0;n<12;++n){frames(1);if(!dune_view_draw(&host,&c))return 32;}
 DuneLayout scrolled=dune_view_host_layout(&host,&c,1280,720);
 if(!check(scrolled.left<edge.left && scrolled.top==edge.top,"mouse scrolling at actual window edge"))return 33;
 pointer.motion.x=600;dune_mouse_event(&host,&c,&pointer);edge=dune_view_host_layout(&host,&c,1280,720);
 frames(60);if(!dune_view_draw(&host,&c))return 34;scrolled=dune_view_host_layout(&host,&c,1280,720);
 if(!check(scrolled.left==edge.left && scrolled.top==edge.top,"edge scrolling stops in window interior"))return 35;
 sdl_host_close(&host);puts("PASS adaptive world + HUD + native zoomed selection/order");return 0;
}
