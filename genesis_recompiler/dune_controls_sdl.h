/* Controller settings; bitmap text reused from RROP. */
#ifndef GENESIS_DUNE_CONTROLS_SDL_H
#define GENESIS_DUNE_CONTROLS_SDL_H
static const uint8_t dune_controls_letters[36][5]={
 {0x3e,0x51,0x49,0x45,0x3e},{0,0x42,0x7f,0x40,0},{0x42,0x61,0x51,0x49,0x46},
 {0x21,0x41,0x45,0x4b,0x31},{0x18,0x14,0x12,0x7f,0x10},{0x27,0x45,0x45,0x45,0x39},
 {0x3c,0x4a,0x49,0x49,0x30},{1,0x71,9,5,3},{0x36,0x49,0x49,0x49,0x36},
 {6,0x49,0x49,0x29,0x1e},
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},
 {0x7f,0x41,0x41,0x22,0x1c},{0x7f,0x49,0x49,0x49,0x41},{0x7f,9,9,9,1},
 {0x3e,0x41,0x49,0x49,0x7a},{0x7f,8,8,8,0x7f},{0,0x41,0x7f,0x41,0},
 {0x20,0x40,0x41,0x3f,1},{0x7f,8,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,2,0x0c,2,0x7f},{0x7f,4,8,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},
 {0x7f,9,9,9,6},{0x3e,0x41,0x51,0x21,0x5e},{0x7f,9,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{1,1,0x7f,1,1},{0x3f,0x40,0x40,0x40,0x3f},
 {0x1f,0x20,0x40,0x20,0x1f},{0x3f,0x40,0x38,0x40,0x3f},{0x63,0x14,8,0x14,0x63},
 {7,8,0x70,8,7},{0x61,0x51,0x49,0x45,0x43}
};
static void dune_controls_text(SDL_Renderer *r,int x,int y,int scale,const char *text,size_t max) {
    for(size_t n=0;text[n] && n<max;++n,x+=6*scale) {
        unsigned ch=(unsigned char)text[n];if(ch>='a' && ch<='z')ch-=32;
        const uint8_t *glyph=NULL;uint8_t punctuation[5]={0};
        if(ch>='0' && ch<='9')glyph=dune_controls_letters[ch-'0'];
        else if(ch>='A' && ch<='Z')glyph=dune_controls_letters[ch-'A'+10];
        else {
            if(ch=='-')memset(punctuation,8,5);
            if(ch==':')punctuation[2]=0x24;
            if(ch=='.')punctuation[2]=0x40;
            if(ch=='/') {punctuation[0]=0x20;punctuation[1]=0x10;punctuation[2]=8;punctuation[3]=4;punctuation[4]=2;}
            glyph=punctuation;
        }
        for(int col=0;col<5;++col)for(int row=0;row<7;++row)if(glyph[col]&(1<<row)) {
            SDL_Rect dot={x+col*scale,y+row*scale,scale,scale};SDL_RenderFillRect(r,&dot);
        }
    }
}
/* Small pixel-art controller, drawn with SDL primitives at the menu scale. */
static void dune_controls_oval(SDL_Renderer *r,int x,int y,int rx,int ry,int scale) {
    for(int dy=-ry;dy<=ry;dy++) {
        int dx=rx;
        while(dx>0 && dx*dx*ry*ry+dy*dy*rx*rx>rx*rx*ry*ry)--dx;
        SDL_Rect row={x-dx*scale,y+dy*scale,(2*dx+1)*scale,scale};SDL_RenderFillRect(r,&row);
    }
}
/* Outline traced from Evan-Amos's public-domain photograph:
   https://commons.wikimedia.org/wiki/File:Sega-Genesis-3But-Cont.jpg
   Coordinates follow its 1920-pixel preview; preserve the photo's proportions. */
static void dune_controls_shape(SDL_Renderer *r,const SDL_Point *p,int n,int x,int y,int scale,SDL_Color color) {
    SDL_FPoint v[64];float top=1e9f,bottom=0;
    for(int i=0;i<n;i++) {
        v[i]=(SDL_FPoint){x+p[i].x*.09f*scale,y+p[i].y*.09f*scale};
        if(v[i].y<top)top=v[i].y;if(v[i].y>bottom)bottom=v[i].y;
    }
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a);
    /* Scanline pairs preserve the concave grip cutout. */
    for(int row=(int)top;row<bottom;row++) {
        float cross[64],scan=row+.5f;int count=0;
        for(int i=0,j=n-1;i<n;j=i++) {
            if((v[i].y>scan)==(v[j].y>scan))continue;
            float at=v[i].x+(scan-v[i].y)*(v[j].x-v[i].x)/(v[j].y-v[i].y);
            int k=count++;while(k && cross[k-1]>at){cross[k]=cross[k-1];--k;}cross[k]=at;
        }
        for(int i=0;i+1<count;i+=2)SDL_RenderDrawLine(r,(int)(cross[i]+.5f),row,(int)cross[i+1],row);
    }
}
static void dune_controls_pad(SDLHost *h,int x,int y,int scale) {
    SDL_Renderer *r=h->renderer;x+=8*scale;y+=39*scale;
    static const SDL_Point shell[]={
        {57,632},{73,542},{112,447},{170,368},{247,307},{339,260},
        {452,224},{579,199},{711,184},{849,181},{990,187},{1122,205},
        {1252,236},{1374,278},{1493,329},{1600,389},{1695,460},{1770,539},
        {1827,627},{1854,713},{1858,800},{1834,884},{1790,964},{1723,1037},
        {1640,1102},{1545,1159},{1443,1208},{1343,1240},{1269,1239},
        {1218,1205},{1181,1148},{1158,1085},{1143,1018},{1127,962},
        {1098,913},{1054,873},{998,837},{925,805},{842,783},{751,770},
        {653,767},{559,781},{479,807},{418,845},{368,899},{328,945},
        {279,977},{222,990},{170,980},{128,947},{94,892},{70,822},{57,731}};
    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y+3*scale,scale,(SDL_Color){10,12,16,255});
    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y,scale,(SDL_Color){66,71,80,255});
    enum {POINTS=sizeof shell/sizeof *shell};
    SDL_Point outline[POINTS+1];
    for(unsigned i=0;i<sizeof shell/sizeof *shell;i++)outline[i]=(SDL_Point){x+(int)(shell[i].x*.09f*scale),y+(int)(shell[i].y*.09f*scale)};
    outline[POINTS]=outline[0];SDL_SetRenderDrawColor(r,122,130,141,255);SDL_RenderDrawLines(r,outline,POINTS+1);
    static const SDL_Point bezel[]={
        {1032,667},{1052,610},{1112,555},{1200,507},{1305,477},{1416,461},
        {1520,461},{1621,483},{1701,527},{1749,590},{1756,652},{1721,697},
        {1655,720},{1572,722},{1490,725},{1411,735},{1325,753},{1240,782},
        {1170,801},{1103,791},{1055,746}};
    dune_controls_shape(r,bezel,sizeof bezel/sizeof *bezel,x,y,scale,(SDL_Color){19,22,28,255});
    SDL_SetRenderDrawColor(r,19,22,28,255);dune_controls_oval(r,x+43*scale,y+42*scale,25,21,scale);
    SDL_SetRenderDrawColor(r,95,102,113,255);dune_controls_oval(r,x+43*scale,y+42*scale,19,17,scale);
    SDL_SetRenderDrawColor(r,28,32,40,255);dune_controls_oval(r,x+43*scale,y+42*scale,18,16,scale);
    SDL_SetRenderDrawColor(r,65,73,87,255);
    SDL_Rect horizontal={x+28*scale,y+37*scale,30*scale,10*scale};SDL_RenderFillRect(r,&horizontal);
    SDL_Rect vertical={x+38*scale,y+27*scale,10*scale,30*scale};SDL_RenderFillRect(r,&vertical);
    SDL_SetRenderDrawColor(r,54,60,70,255);dune_controls_oval(r,x+43*scale,y+42*scale,4,4,scale);
    SDL_SetRenderDrawColor(r,198,204,215,255);
    dune_controls_text(r,x+80*scale,y+27*scale,scale,"SEGA",4);
    dune_controls_text(r,x+71*scale,y+36*scale,scale,"GENESIS",7);
    const int bx[4]={106,125,144,124},by[4]={60,55,53,35};
    const char *label[4]={"A","B","C","START"};
    for(int i=0;i<4;i++) {
        int active=h->controls.remap==i+1,px=x+bx[i]*scale,py=y+by[i]*scale;
        SDL_SetRenderDrawColor(r,5,7,10,255);dune_controls_oval(r,px,py+2*scale,i==3?10:9,i==3?4:8,scale);
        SDL_SetRenderDrawColor(r,active?240:i==3?188:98,active?190:i==3?192:106,active?80:i==3?198:119,255);
        dune_controls_oval(r,px,py,i==3?10:9,i==3?4:8,scale);
        if(i<3) {
            SDL_SetRenderDrawColor(r,active?240:38,active?190:43,active?80:52,255);dune_controls_oval(r,px,py,7,6,scale);
        }
        SDL_SetRenderDrawColor(r,active?20:231,active?24:234,active?30:241,255);
        dune_controls_text(r,px-(i==3?14:2)*scale,py-(i==3?13:3)*scale,scale,label[i],5);
    }
}
static void dune_controls_read(SDLHost *h) {
    FILE *f=fopen("gamepad.cfg","r");if(!f)return;
    unsigned enabled,layout,custom,b[4];char extra;
    int n=fscanf(f,"ReArrakis controls 1 %u %u %u %u %u %u %u %c",&enabled,&layout,&custom,&b[0],&b[1],&b[2],&b[3],&extra);fclose(f);
    if(n!=7 || enabled>1 || layout>1 || custom>1)return;
    uint8_t binding[4];for(unsigned i=0;i<4;i++){if(b[i]>=SDL_CONTROLLER_BUTTON_MAX)return;binding[i]=(uint8_t)b[i];}
    if(!sdl_pad_bindings_valid(binding))return;
    h->input.enabled=enabled;h->input.layout=layout;h->input.custom=custom;memcpy(h->input.binding,binding,4);
}
static void dune_controls_write(SDLHost *h) {
    FILE *f=fopen("gamepad.cfg.tmp","w");int ok=0;
    if(f){
        ok=fprintf(f,"ReArrakis controls 1\n%d %d %d %u %u %u %u\n",h->input.enabled,h->input.layout,h->input.custom,
            sdl_pad_binding(&h->input,0),sdl_pad_binding(&h->input,1),sdl_pad_binding(&h->input,2),sdl_pad_binding(&h->input,3))>0;
        if(fclose(f))ok=0;
        if(ok)ok=!rename("gamepad.cfg.tmp","gamepad.cfg");
    }
    snprintf(h->controls.message,sizeof h->controls.message,ok?"Settings saved":"Could not save gamepad.cfg");
}
static void dune_controls_toggle(SDLHost *h,CPU *c) {
    if(h->controls.menu){h->controls.menu=0;h->controls.remap=0;h->paused=h->controls.was_paused;}
    else{h->controls.menu=1;h->controls.was_paused=h->paused;h->paused=1;h->controls.message[0]=0;}
    sdl_pad_clear(&h->input);c->pad_buttons[0]=0;h->fast_forward=0;
#ifdef GENESIS_DUNE_MOUSE
    dune_mouse_reset(&h->dune_mouse);h->dune_view.pointer_valid=0;
#endif
    sdl_host_rebase(h,c);h->last_frame=UINT64_MAX;
}
static void dune_controls_assign(SDLHost *h) {
    if(!h->input.controller || !SDL_GameControllerGetAttached(h->input.controller)){
        snprintf(h->controls.message,sizeof h->controls.message,"Connect a supported gamepad first");return;
    }
    h->controls.remap=1;h->controls.held=0;h->controls.message[0]=0;
    for(unsigned i=0;i<SDL_CONTROLLER_BUTTON_MAX && i<32;i++)
        if(SDL_GameControllerGetButton(h->input.controller,(SDL_GameControllerButton)i))h->controls.held|=UINT32_C(1)<<i;
    sdl_pad_clear(&h->input);
}
/* Raw button capture precedes menu navigation, so B can be assigned too. */
static int dune_controls_capture(SDLHost *h,const SDL_Event *e) {
    DuneControls *s=&h->controls;if(!s->menu || !s->remap)return 0;
    if(((e->type==SDL_CONTROLLERDEVICEREMOVED || e->type==SDL_CONTROLLERDEVICEREMAPPED) && e->cdevice.which==h->input.instance) ||
       (e->type==SDL_WINDOWEVENT && e->window.event==SDL_WINDOWEVENT_FOCUS_LOST)){
        s->remap=0;snprintf(s->message,sizeof s->message,"Assignment cancelled");return 0;
    }
    if(e->type==SDL_KEYDOWN && !e->key.repeat && (e->key.keysym.sym==SDLK_ESCAPE || e->key.keysym.sym==SDLK_F10)){
        s->remap=0;s->message[0]=0;return e->key.keysym.sym==SDLK_ESCAPE;
    }
    if(e->type==SDL_CONTROLLERBUTTONDOWN || e->type==SDL_CONTROLLERBUTTONUP){
        if(e->cbutton.which!=h->input.instance || !h->input.focused)return 1;
        unsigned b=e->cbutton.button;if(b>=SDL_CONTROLLER_BUTTON_MAX || b>=32)return 1;
        uint32_t bit=UINT32_C(1)<<b;
        if(e->type==SDL_CONTROLLERBUTTONUP){s->held&=~bit;return 1;}
        if(b==SDL_CONTROLLER_BUTTON_BACK){s->remap=0;sdl_pad_clear(&h->input);return 1;}
        if(s->held){s->held|=bit;return 1;}s->held|=bit;
        if(!sdl_pad_bindable(b)){snprintf(s->message,sizeof s->message,"Button reserved for movement or menu");return 1;}
        unsigned step=s->remap-1;
        for(unsigned i=0;i<step;i++)if(s->pending[i]==b){snprintf(s->message,sizeof s->message,"Already assigned - choose another");return 1;}
        s->pending[step]=(uint8_t)b;s->message[0]=0;
        if(++s->remap==5){memcpy(h->input.binding,s->pending,4);h->input.custom=1;s->remap=0;sdl_pad_clear(&h->input);dune_controls_write(h);}
        return 1;
    }
    return e->type==SDL_CONTROLLERAXISMOTION || e->type==SDL_KEYDOWN || e->type==SDL_KEYUP || e->type==SDL_MOUSEBUTTONDOWN;
}
static int dune_controls_event(SDLHost *h,CPU *c,const SDL_Event *e) {
    DuneControls *s=&h->controls;
    if(e->type==SDL_KEYDOWN && !e->key.repeat && e->key.keysym.sym==SDLK_F10){dune_controls_toggle(h,c);return 1;}
    if(!s->menu)return 0;
    if(e->type==SDL_QUIT || e->type==SDL_WINDOWEVENT)return 0;
    int activate=0;
    if(e->type==SDL_KEYDOWN && !e->key.repeat){
        SDL_Keycode key=e->key.keysym.sym;
        if(key==SDLK_ESCAPE){dune_controls_toggle(h,c);return 1;}
        if(key==SDLK_UP)s->selected=(s->selected+4)%5;
        if(key==SDLK_DOWN)s->selected=(s->selected+1)%5;
        activate=key==SDLK_RETURN || key==SDLK_LEFT || key==SDLK_RIGHT;
    }
    if(e->type==SDL_MOUSEBUTTONDOWN && e->button.button==SDL_BUTTON_LEFT){
        int ww,wh,w,hg;SDL_GetWindowSize(h->window,&ww,&wh);SDL_GetRendererOutputSize(h->renderer,&w,&hg);
        if(ww<=0 || wh<=0)return 1;
        int scale=w/320;if(hg/224<scale)scale=hg/224;if(scale<1)scale=1;
        int x=e->button.x*w/ww-(w-320*scale)/2,y=e->button.y*hg/wh-(hg-224*scale)/2;
        if(x>=200*scale && x<308*scale && y>=54*scale && y<154*scale){s->selected=(y/scale-54)/20;activate=1;}
    }
    if(activate){
        if(s->selected==4){dune_controls_toggle(h,c);return 1;}
        if(s->selected==2){dune_controls_assign(h);return 1;}
        if(s->selected==0)h->input.enabled=!h->input.enabled;
        if(s->selected==1){h->input.layout=!h->input.layout;h->input.custom=0;}
        if(s->selected==3){h->input.layout=0;h->input.custom=0;h->input.enabled=1;}
        sdl_pad_clear(&h->input);dune_controls_write(h);
    }
    return 1;
}
static void dune_controls_draw(SDLHost *h) {
    int w,hg;SDL_GetRendererOutputSize(h->renderer,&w,&hg);
    int scale=w/320;if(hg/224<scale)scale=hg/224;if(scale<1)scale=1;
    int x=(w-320*scale)/2,y=(hg-224*scale)/2;
    SDL_RenderSetLogicalSize(h->renderer,0,0);SDL_RenderSetScale(h->renderer,1,1);SDL_RenderSetViewport(h->renderer,NULL);
    SDL_SetRenderDrawColor(h->renderer,20,24,30,255);SDL_RenderClear(h->renderer);
    SDL_SetRenderDrawColor(h->renderer,240,214,140,255);
    dune_controls_text(h->renderer,x+12*scale,y+12*scale,scale,"GAMEPAD SETTINGS - F10",48);
    const char *name=h->input.controller?SDL_GameControllerName(h->input.controller):"No supported controller connected";
    dune_controls_text(h->renderer,x+12*scale,y+32*scale,scale,name?name:"Controller",48);
    dune_controls_pad(h,x,y,scale);
    SDL_SetRenderDrawColor(h->renderer,240,214,140,255);
    char text[96];const char *actions[4]={"A","B","C","START"};
    for(int i=0;i<5;i++){
        if(h->controls.remap){
            if(i<4)snprintf(text,sizeof text,"%s %s: %s",h->controls.remap==i+1?"-":" ",actions[i],i<h->controls.remap-1?sdl_pad_label(h->controls.pending[i]):"...");
            else snprintf(text,sizeof text,"Release to assign");
        }else{
            const char *rows[5]={h->input.enabled?"Input: On":"Input: Off",h->input.custom?"Layout: Custom":h->input.layout?"Layout: A B X":"Layout: X A B","Assign buttons","Reset defaults","Back to game"};
            snprintf(text,sizeof text,"%s %s",h->controls.selected==i?"-":" ",rows[i]);
        }
        dune_controls_text(h->renderer,x+200*scale,y+(58+i*20)*scale,scale,text,18);
    }
    snprintf(text,sizeof text,h->controls.remap?"Press a button for %s":"SEGA GENESIS - 3 BUTTON CONTROL PAD",h->controls.remap?actions[h->controls.remap-1]:"");
    dune_controls_text(h->renderer,x+12*scale,y+167*scale,scale,text,48);
    snprintf(text,sizeof text,"A/B/C: %s/%s/%s  START: %s",sdl_pad_label(sdl_pad_binding(&h->input,0)),sdl_pad_label(sdl_pad_binding(&h->input,1)),sdl_pad_label(sdl_pad_binding(&h->input,2)),sdl_pad_label(sdl_pad_binding(&h->input,3)));
    dune_controls_text(h->renderer,x+12*scale,y+187*scale,scale,text,48);
    dune_controls_text(h->renderer,x+12*scale,y+198*scale,scale,h->controls.remap?"ESC / BACK: Cancel":"Arrows / Enter / Click - ESC: Back",48);
    dune_controls_text(h->renderer,x+12*scale,y+212*scale,scale,h->controls.message,48);
    SDL_RenderPresent(h->renderer);
}
#endif
