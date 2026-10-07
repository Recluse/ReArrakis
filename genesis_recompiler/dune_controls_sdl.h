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
/* Diagram geometry is rasterized in drawable pixels, not enlarged pixel blocks. */
static void dune_controls_oval(SDL_Renderer *r,int x,int y,int rx,int ry,int scale) {
    rx*=scale;ry*=scale;
    for(int dy=-ry;dy<=ry;dy++) {
        int dx=rx;
        while(dx>0 && (double)dx*dx*ry*ry+(double)dy*dy*rx*rx>(double)rx*rx*ry*ry)--dx;
        SDL_RenderDrawLine(r,x-dx,y+dy,x+dx,y+dy);
    }
}
/* Front-view reference: https://www.gamerlifestore.com/products/sega-genesis-3-button-controller-original
   Hand-traced shell, action-button recess and Start; photo is not distributed. */
static void dune_controls_shape(SDL_Renderer *r,const SDL_Point *p,int n,int x,int y,int scale,SDL_Color color) {
    SDL_FPoint v[256];float top=1e9f,bottom=0;
    /* Smooth the traced outline between its measured landmarks. */
    for(int i=0;i<n;i++)for(int j=0;j<4;j++) {
        SDL_Point a=p[(i+n-1)%n],b=p[i],c=p[(i+1)%n],d=p[(i+2)%n];
        float t=j/4.f,t2=t*t,t3=t2*t;
        float px=.5f*(2*b.x+(-a.x+c.x)*t+(2*a.x-5*b.x+4*c.x-d.x)*t2+(-a.x+3*b.x-3*c.x+d.x)*t3);
        float py=.5f*(2*b.y+(-a.y+c.y)*t+(2*a.y-5*b.y+4*c.y-d.y)*t2+(-a.y+3*b.y-3*c.y+d.y)*t3);
        v[i*4+j]=(SDL_FPoint){x+px*.09f*scale,y+py*.09f*scale};
        if(v[i*4+j].y<top)top=v[i*4+j].y;if(v[i*4+j].y>bottom)bottom=v[i*4+j].y;
    }
    n*=4;
    SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a);
    /* Scanline pairs preserve the concave grip cutout. */
    for(int row=(int)top;row<bottom;row++) {
        float cross[256],scan=row+.5f;int count=0;
        for(int i=0,j=n-1;i<n;j=i++) {
            if((v[i].y>scan)==(v[j].y>scan))continue;
            float at=v[i].x+(scan-v[i].y)*(v[j].x-v[i].x)/(v[j].y-v[i].y);
            int k=count++;while(k && cross[k-1]>at){cross[k]=cross[k-1];--k;}cross[k]=at;
        }
        for(int i=0;i+1<count;i+=2)SDL_RenderDrawLine(r,(int)(cross[i]+.5f),row,(int)cross[i+1],row);
    }
}
static void dune_controls_pad(SDLHost *h,int x,int y,int scale) {
    SDL_Renderer *r=h->renderer;x+=8*scale;y+=54*scale;
    static const SDL_Point shell[]={{12,537},{21,441},{51,351},{96,270},{168,204},{258,150},{366,102},{483,60},{615,30},{750,12},{879,0},{999,-3},{1137,9},{1269,39},{1401,81},{1530,132},{1641,189},{1746,255},{1833,333},{1896,420},{1935,519},{1950,621},{1938,729},{1905,837},{1854,933},{1788,1014},{1716,1083},{1644,1128},{1575,1143},{1512,1131},{1461,1092},{1425,1038},{1398,972},{1356,912},{1302,870},{1230,840},{1146,819},{1062,807},{969,804},{879,813},{798,834},{720,867},{657,909},{603,966},{570,1035},{537,1095},{486,1140},{423,1158},{360,1152},{294,1119},{222,1062},{159,990},{105,903},{63,810},{33,711},{15,621}};
    static const SDL_Point bezel[]={{1179,600},{1194,534},{1233,471},{1296,420},{1380,375},{1470,333},{1563,300},{1659,276},{1734,279},{1797,309},{1845,363},{1863,432},{1851,510},{1809,570},{1746,618},{1665,657},{1575,699},{1482,744},{1398,780},{1323,789},{1260,768},{1209,723},{1185,663}};
    static const SDL_Point start[]={{1311,246},{1329,234},{1443,183},{1467,189},{1482,210},{1476,234},{1362,294},{1335,300},{1317,285}};

    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y+2*scale,scale,(SDL_Color){7,9,12,255});
    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y,scale,(SDL_Color){110,119,133,255});
    dune_controls_shape(r,shell,sizeof shell/sizeof *shell,x,y+1,scale,(SDL_Color){39,43,51,255});
    dune_controls_shape(r,bezel,sizeof bezel/sizeof *bezel,x,y,scale,(SDL_Color){9,11,15,255});
    int cx=x+42*scale,cy=y+47*scale;
    SDL_SetRenderDrawColor(r,80,87,100,255);dune_controls_oval(r,cx,cy,27,27,scale);
    SDL_SetRenderDrawColor(r,12,15,21,255);dune_controls_oval(r,cx,cy,26,26,scale);
    SDL_SetRenderDrawColor(r,65,72,84,255);dune_controls_oval(r,cx,cy,19,19,scale);
    SDL_SetRenderDrawColor(r,26,30,38,255);dune_controls_oval(r,cx,cy,18,18,scale);
    SDL_SetRenderDrawColor(r,77,84,96,255);
    SDL_Rect crossx={cx-16*scale,cy-5*scale,33*scale,11*scale};SDL_RenderFillRect(r,&crossx);
    SDL_Rect crossy={cx-5*scale,cy-16*scale,11*scale,33*scale};SDL_RenderFillRect(r,&crossy);
    SDL_SetRenderDrawColor(r,27,31,38,255);dune_controls_oval(r,cx,cy,3,3,scale);
    SDL_SetRenderDrawColor(r,223,226,232,255);
    for(int row=0;row<4*scale;row++) {
        int span=row/2;
        SDL_RenderDrawLine(r,cx-span,cy-24*scale+row,cx+span,cy-24*scale+row);
        SDL_RenderDrawLine(r,cx-span,cy+24*scale-row,cx+span,cy+24*scale-row);
        SDL_RenderDrawLine(r,cx-24*scale+row,cy-span,cx-24*scale+row,cy+span);
        SDL_RenderDrawLine(r,cx+24*scale-row,cy-span,cx+24*scale-row,cy+span);
    }
    int small=(scale*2+1)/3;if(small<1)small=1;
    dune_controls_text(r,x+91*scale-12*small,y+23*scale,small,"SEGA",4);
    dune_controls_text(r,x+91*scale-21*small,y+29*scale,small,"GENESIS",7);
    int active=h->controls.remap==4;
    dune_controls_shape(r,start,sizeof start/sizeof *start,x,y+scale,scale,(SDL_Color){6,8,12,255});
    dune_controls_shape(r,start,sizeof start/sizeof *start,x,y,scale,active?(SDL_Color){240,190,80,255}:(SDL_Color){194,200,209,255});
    SDL_SetRenderDrawColor(r,223,226,232,255);
    dune_controls_text(r,x+126*scale-14*small,y+13*scale,small,"START",5);
    const int bx[3]={119,138,155},by[3]={56,46,39};const char *label[3]={"A","B","C"};
    for(int i=0;i<3;i++) {
        int px=x+bx[i]*scale,py=y+by[i]*scale;active=h->controls.remap==i+1;
        SDL_SetRenderDrawColor(r,4,6,10,255);dune_controls_oval(r,px,py+2*scale,9,9,scale);
        SDL_SetRenderDrawColor(r,active?240:107,active?190:116,active?80:130,255);dune_controls_oval(r,px,py,9,9,scale);
        SDL_SetRenderDrawColor(r,active?240:42,active?190:48,active?80:60,255);dune_controls_oval(r,px,py,8,8,scale);
        SDL_SetRenderDrawColor(r,active?20:164,active?24:173,active?30:189,255);
        dune_controls_text(r,px-2*small,py-3*small,small,label[i],1);
        SDL_SetRenderDrawColor(r,225,229,237,255);
        dune_controls_text(r,px-2*small,py-15*scale,small,label[i],1);
    }
}
static void dune_controls_read(SDLHost *h) {
    FILE *f=fopen("gamepad.cfg","r");if(!f)return;
    unsigned version,enabled,layout,custom,b[4],stick=0;char extra,guid[33]={0};
    int n=fscanf(f,"ReArrakis controls %u %u %u %u %u %u %u %u",&version,&enabled,&layout,&custom,&b[0],&b[1],&b[2],&b[3]);
    int valid=n==8 && (version>=1 && version<=3);
    if(valid && version==3)valid=fscanf(f," %u",&stick)==1 && stick<=2;
    if(valid && version>=2) {
        valid=fscanf(f," %32s",guid)==1 && strlen(guid)==32;
        for(int i=0;valid && i<32;i++)valid=(guid[i]>='0' && guid[i]<='9') || (guid[i]>='a' && guid[i]<='f');
    }
    if(fscanf(f," %c",&extra)==1)valid=0;fclose(f);
    if(!valid || enabled>1 || layout>1 || custom>1)return;
    uint8_t binding[4];for(unsigned i=0;i<4;i++){if(b[i]>=SDL_CONTROLLER_BUTTON_MAX)return;binding[i]=(uint8_t)b[i];}
    if(!sdl_pad_bindings_valid(binding))return;
    h->input.cursor_stick=(int)stick;h->input.enabled=enabled;h->input.layout=layout;h->input.custom=custom;memcpy(h->input.binding,binding,4);memcpy(h->input.preferred,guid,33);
}
static void dune_controls_write(SDLHost *h) {
    FILE *f=fopen("gamepad.cfg.tmp","w");int ok=0;
    if(f){
        ok=fprintf(f,"ReArrakis controls 3\n%d %d %d %u %u %u %u\n%d\n%s\n",h->input.enabled,h->input.layout,h->input.custom,
            sdl_pad_binding(&h->input,0),sdl_pad_binding(&h->input,1),sdl_pad_binding(&h->input,2),sdl_pad_binding(&h->input,3),h->input.cursor_stick,h->input.preferred[0]?h->input.preferred:"00000000000000000000000000000000")>0;
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
    h->stick_cursor_active=0;h->stick_cursor_blocked=1;
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
        if(key==SDLK_UP)s->selected=(s->selected+6)%7;
        if(key==SDLK_DOWN)s->selected=(s->selected+1)%7;
        activate=key==SDLK_RETURN || key==SDLK_LEFT || key==SDLK_RIGHT;
    }
    if(e->type==SDL_MOUSEBUTTONDOWN && e->button.button==SDL_BUTTON_LEFT){
        int ww,wh,w,hg;SDL_GetWindowSize(h->window,&ww,&wh);SDL_GetRendererOutputSize(h->renderer,&w,&hg);
        if(ww<=0 || wh<=0)return 1;
        int scale=w/320;if(hg/224<scale)scale=hg/224;if(scale<1)scale=1;
        int x=e->button.x*w/ww-(w-320*scale)/2,y=e->button.y*hg/wh-(hg-224*scale)/2;
        if(x>=200*scale && x<308*scale && y>=54*scale && y<166*scale){s->selected=(y/scale-54)/16;activate=1;}
    }
    if(activate){
        if(s->selected==0){
            sdl_pad_cycle(&h->input,e->type==SDL_KEYDOWN && e->key.keysym.sym==SDLK_LEFT?-1:1);
#ifdef GENESIS_DUNE_MOUSE
            h->stick_cursor_active=0;h->stick_cursor_blocked=1;
#endif
        }
        if(s->selected==1){
            h->input.cursor_stick=(h->input.cursor_stick+(e->type==SDL_KEYDOWN && e->key.keysym.sym==SDLK_LEFT?2:1))%3;
#ifdef GENESIS_DUNE_MOUSE
            h->stick_cursor_active=0;h->stick_cursor_blocked=1;
#endif
        }
        if(s->selected==6){dune_controls_toggle(h,c);return 1;}
        if(s->selected==4){dune_controls_assign(h);return 1;}
        if(s->selected==2)h->input.enabled=!h->input.enabled;
        if(s->selected==3){h->input.layout=!h->input.layout;h->input.custom=0;}
        if(s->selected==5){h->input.cursor_stick=0;h->input.layout=0;h->input.custom=0;h->input.enabled=1;}
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
    int count=0,chosen=0;
    for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_IsGameController(i)){
        ++count;if(SDL_JoystickGetDeviceInstanceID(i)==h->input.instance)chosen=count;
    }
    char device[32];snprintf(device,sizeof device,"Pad: %d/%d",chosen,count);
    for(int i=0;i<7;i++){
        if(h->controls.remap){
            if(i<4)snprintf(text,sizeof text,"%s %s: %s",h->controls.remap==i+1?"-":" ",actions[i],i<h->controls.remap-1?sdl_pad_label(h->controls.pending[i]):"...");
            else snprintf(text,sizeof text,i==4?"Release to assign":"");
        }else{
            const char *rows[7]={device,h->input.cursor_stick==0?"Cursor: Left":h->input.cursor_stick==1?"Cursor: Right":"Cursor: Off",h->input.enabled?"Input: On":"Input: Off",h->input.custom?"Layout: Custom":h->input.layout?"Layout: A B X":"Layout: X A B","Assign buttons","Reset defaults","Back to game"};
            snprintf(text,sizeof text,"%s %s",h->controls.selected==i?"-":" ",rows[i]);
        }
        dune_controls_text(h->renderer,x+200*scale,y+(56+i*16)*scale,scale,text,18);
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
