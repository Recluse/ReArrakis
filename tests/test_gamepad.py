"""Button assignment through SDL events, physical polling and persisted settings."""
import os
import subprocess
import unittest

from genesis_recompiler.build import BuildError, sdl2_flags
from genesis_recompiler.decode import analyze
from genesis_recompiler.emit import emit
from support import CompiledTestCase, rom_with


class GamepadRemapTests(CompiledTestCase):
    @classmethod
    def setUpClass(cls):
        try:
            cls.flags,cls.libs=sdl2_flags()
        except BuildError as exc:
            raise unittest.SkipTest(str(exc))

    def check(self, body):
        source='#define GENESIS_NO_MAIN\n#define GENESIS_DUNE_MOUSE\n'+emit(analyze(rom_with('4e72 2700'),[0x200]))
        source+=r'''
#include <assert.h>
static void key(SDLHost *h,CPU *c,SDL_Keycode key) {
 SDL_Event e={0};e.type=SDL_KEYDOWN;e.key.keysym.sym=key;assert(SDL_PushEvent(&e)==1);
 assert(sdl_host_service(h,c));
}
static void button(SDLHost *h,CPU *c,SDL_Joystick *j,unsigned b,int down) {
 assert(!SDL_JoystickSetVirtualButton(j,(int)b,(Uint8)down));SDL_PumpEvents();
 assert(sdl_host_service(h,c));
}
static unsigned pixel(SDLHost *h,int x,int y) {
 Uint32 value=0;SDL_Rect rect={x,y,1,1};
 assert(!SDL_RenderReadPixels(h->renderer,&rect,SDL_PIXELFORMAT_ARGB8888,&value,sizeof value));
 return value&0xffffff;
}
static void begin(SDLHost *h,CPU *c) {
 if(!h->controls.menu)dune_controls_toggle(h,c);h->controls.selected=2;
 key(h,c,SDLK_RETURN);assert(h->controls.remap==1 && !c->pad_buttons[0]);
}
int main(void) {
 CPU *c=calloc(1,sizeof *c);assert(c);c->rom=rom_data;c->rom_size=sizeof rom_data;
 SDLHost h={0};h.no_throttle=1;assert(sdl_host_open(&h));
 int device=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,SDL_CONTROLLER_AXIS_MAX,SDL_CONTROLLER_BUTTON_MAX,0);
 assert(device>=0);SDL_Joystick *j=SDL_JoystickOpen(device);assert(j);
 sdl_pad_connect(&h.input);assert(h.input.controller && h.input.instance==SDL_JoystickInstanceID(j));
 assert(sdl_host_service(&h,c));
''' + body + r'''
 SDL_JoystickClose(j);sdl_host_close(&h);free(c);return 0;
}
'''
        path=self.root/'remap.c';path.write_text(source);binary=self.root/'remap'
        result=subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-missing-field-initializers',
                               '-DGENESIS_SDL2',*self.flags,str(path),'-o',str(binary),*self.libs],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        result=subprocess.run([str(binary)],cwd=self.root,env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software'),
                              capture_output=True,text=True,timeout=15)
        self.assertEqual(result.returncode,0,result.stderr)

    def test_complete_wizard_persists_maps_buttons_and_blocks_confirmation_leak(self):
        self.check(r'''
SDL_SetWindowSize(h.window,320,224);
begin(&h,c);
/* The traced shell keeps its grip cutout, and A is highlighted during assignment. */
dune_controls_draw(&h);
assert(pixel(&h,95,100)==0x424750);
assert(pixel(&h,85,135)==0x14181e);
assert(pixel(&h,114,104)==0xf0be50);
unsigned mapping[4]={SDL_CONTROLLER_BUTTON_Y,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_X};
for(unsigned i=0;i<4;++i) {
 button(&h,c,j,mapping[i],1);assert(!c->pad_buttons[0]);
 if(i<3)assert(h.controls.remap==(int)i+2 && !h.input.custom);
 else assert(!h.controls.remap && h.input.custom);
 button(&h,c,j,mapping[i],0);
}
for(unsigned i=0;i<4;++i)assert(sdl_pad_binding(&h.input,i)==mapping[i]);
dune_controls_toggle(&h,c);
for(unsigned i=0;i<4;++i) {
 button(&h,c,j,mapping[i],1);assert(c->pad_buttons[0]==(PAD_A<<i));
 button(&h,c,j,mapping[i],0);assert(!c->pad_buttons[0]);
}
h.input.custom=0;dune_controls_read(&h);assert(h.input.custom);
for(unsigned i=0;i<4;++i)assert(sdl_pad_binding(&h.input,i)==mapping[i]);
/* Final wizard press stays held: closing settings must not press Start. */
begin(&h,c);
for(unsigned i=0;i<4;++i) {
 button(&h,c,j,mapping[i],1);if(i<3)button(&h,c,j,mapping[i],0);
}
dune_controls_toggle(&h,c);assert(sdl_host_service(&h,c));assert(!c->pad_buttons[0]);
button(&h,c,j,mapping[3],0);button(&h,c,j,mapping[3],1);assert(c->pad_buttons[0]==PAD_START);
button(&h,c,j,mapping[3],0);

/* Directions, deadzone, focus loss and hotplug use the same SDL event path. */
button(&h,c,j,SDL_CONTROLLER_BUTTON_DPAD_LEFT,1);assert(c->pad_buttons[0]==PAD_LEFT);
button(&h,c,j,SDL_CONTROLLER_BUTTON_DPAD_LEFT,0);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,16000));
assert(sdl_host_service(&h,c));assert(c->pad_buttons[0]==PAD_RIGHT);
SDL_Event e={0};e.type=SDL_WINDOWEVENT;e.window.event=SDL_WINDOWEVENT_FOCUS_LOST;
assert(SDL_PushEvent(&e)==1);assert(sdl_host_service(&h,c));assert(!c->pad_buttons[0]);
e.window.event=SDL_WINDOWEVENT_FOCUS_GAINED;assert(SDL_PushEvent(&e)==1);
assert(sdl_host_service(&h,c));assert(!c->pad_buttons[0]);
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,0));assert(sdl_host_service(&h,c));
assert(!SDL_JoystickSetVirtualAxis(j,SDL_CONTROLLER_AXIS_LEFTX,4000));
assert(sdl_host_service(&h,c));assert(!c->pad_buttons[0]);
begin(&h,c);button(&h,c,j,SDL_CONTROLLER_BUTTON_Y,1);button(&h,c,j,SDL_CONTROLLER_BUTTON_Y,0);
button(&h,c,j,SDL_CONTROLLER_BUTTON_Y,1);assert(h.controls.remap==2);
button(&h,c,j,SDL_CONTROLLER_BUTTON_Y,0);key(&h,c,SDLK_ESCAPE);assert(!h.controls.remap);
for(unsigned i=0;i<4;++i)assert(sdl_pad_binding(&h.input,i)==mapping[i]);
/* Invalid configs never replace a valid mapping. */
FILE *f=fopen("gamepad.cfg","w");assert(f);fputs("ReArrakis controls 1\n1 0 1 3 3 0 2\n",f);assert(!fclose(f));
dune_controls_read(&h);
for(unsigned i=0;i<4;++i)assert(sdl_pad_binding(&h.input,i)==mapping[i]);
dune_controls_toggle(&h,c);
button(&h,c,j,SDL_CONTROLLER_BUTTON_Y,1);assert(c->pad_buttons[0]==PAD_A);
assert(!SDL_JoystickDetachVirtual(device));assert(sdl_host_service(&h,c));
assert(!h.input.controller && !c->pad_buttons[0]);
''')

