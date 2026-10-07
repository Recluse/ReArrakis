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
static void dune_controls_pad(SDLHost *h,int x,int y,int scale) {
    SDL_Renderer *r=h->renderer;
    SDL_SetRenderDrawColor(r,85,91,102,255);
    dune_controls_oval(r,x+160*scale,y+71*scale,112,28,scale);
    dune_controls_oval(r,x+83*scale,y+83*scale,33,24,scale);
    dune_controls_oval(r,x+237*scale,y+83*scale,33,24,scale);
    SDL_SetRenderDrawColor(r,37,41,49,255);
    dune_controls_oval(r,x+160*scale,y+70*scale,108,25,scale);
    dune_controls_oval(r,x+83*scale,y+82*scale,29,21,scale);
    dune_controls_oval(r,x+237*scale,y+82*scale,29,21,scale);
    SDL_SetRenderDrawColor(r,10,12,16,255);
    SDL_Rect horizontal={x+65*scale,y+73*scale,34*scale,12*scale};SDL_RenderFillRect(r,&horizontal);
    SDL_Rect vertical={x+76*scale,y+62*scale,12*scale,34*scale};SDL_RenderFillRect(r,&vertical);
    SDL_SetRenderDrawColor(r,147,154,168,255);
    dune_controls_text(r,x+130*scale,y+52*scale,scale,"SEGA",4);
    const int bx[4]={199,226,251,148},by[4]={88,77,66,75};
    const char *label[4]={"A","B","C","START"};
    for(int i=0;i<4;i++) {
        int active=h->controls.remap==i+1;
        SDL_SetRenderDrawColor(r,active?240:15,active?190:17,active?80:22,255);
        dune_controls_oval(r,x+bx[i]*scale,y+by[i]*scale,i==3?19:11,i==3?7:11,scale);
        SDL_SetRenderDrawColor(r,active?20:230,active?24:230,active?30:235,255);
        dune_controls_text(r,x+(bx[i]-(i==3?14:2))*scale,y+(by[i]-3)*scale,scale,label[i],5);
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
        if(x>=12*scale && x<308*scale && y>=112*scale && y<182*scale){s->selected=(y/scale-112)/14;activate=1;}
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
            if(i<4)snprintf(text,sizeof text,"%s %s: %s",h->controls.remap==i+1?"-":" ",actions[i],i<h->controls.remap-1?sdl_pad_label(h->controls.pending[i]):"Press button");
            else snprintf(text,sizeof text,"Release each button before the next");
        }else{
            const char *rows[5]={h->input.enabled?"Gamepad: On":"Gamepad: Off",h->input.custom?"Layout: Custom":h->input.layout?"Layout: A B X":"Layout: X A B","Assign A / B / C / Start","Restore defaults","Back to game"};
            snprintf(text,sizeof text,"%s %s",h->controls.selected==i?"-":" ",rows[i]);
        }
        dune_controls_text(h->renderer,x+12*scale,y+(115+i*14)*scale,scale,text,48);
    }
    snprintf(text,sizeof text,"A/B/C: %s/%s/%s  START: %s",sdl_pad_label(sdl_pad_binding(&h->input,0)),sdl_pad_label(sdl_pad_binding(&h->input,1)),sdl_pad_label(sdl_pad_binding(&h->input,2)),sdl_pad_label(sdl_pad_binding(&h->input,3)));
    dune_controls_text(h->renderer,x+12*scale,y+187*scale,scale,text,48);
    dune_controls_text(h->renderer,x+12*scale,y+198*scale,scale,h->controls.remap?"ESC / BACK: Cancel":"Arrows / Enter / Click - ESC: Back",48);
    dune_controls_text(h->renderer,x+12*scale,y+212*scale,scale,h->controls.message,48);
    SDL_RenderPresent(h->renderer);
}
#endif
