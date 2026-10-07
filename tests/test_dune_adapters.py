"""ROM-free checks of Dune's input/audio adapters and frontend integration."""
import os
from pathlib import Path
import subprocess
import unittest

from genesis_recompiler.build import BuildError, sdl2_flags
from genesis_recompiler.decode import analyze
from genesis_recompiler.emit import emit
from support import CompiledTestCase, rom_with

ROOT = Path(__file__).resolve().parents[1]


class DuneAdapters(CompiledTestCase):
    def check_fixture(self, name, sdl=False):
        flags, libs = [], []
        if sdl:
            try:
                flags, libs = sdl2_flags()
            except BuildError as exc:
                self.skipTest(str(exc))
            flags = ['-DDUNE_MOUSE_SDL_TEST', *flags]
        binary = self.root / name
        result = subprocess.run(['cc', '-std=c11', '-O2', *flags,
                                 str(ROOT / 'tools' / (name + '.c')),
                                 '-o', str(binary), *libs], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_volume_bounds_and_monotonic_limiter(self):
        self.check_fixture('test_dune_audio')

    def test_native_input_intents(self):
        self.check_fixture('test_dune_mouse')

    def test_mouse_event_adapter(self):
        self.check_fixture('test_dune_mouse', sdl=True)

    def test_generated_dune_frontend_zoom_and_minimap(self):
        try:
            flags, libs = sdl2_flags()
        except BuildError as exc:
            self.skipTest(str(exc))
        rom = bytearray(rom_with('4e72 2700'))
        rom.extend(bytes(0x1000-len(rom)))
        rom[0xfda:0xfe2] = bytes.fromhex('4e71 4e75 4e71 4e75')
        program = analyze(bytes(rom), [0x200, 0xfda, 0xfde])
        harness = '''
#include <assert.h>
static void word(CPU *c,unsigned a,unsigned value) {
    c->ram[a]=value>>8;c->ram[a+1]=value;
}
int main(void) {
    CPU c={0}; c.rom=rom_data;c.rom_size=sizeof rom_data;
    c.vdp.frame_width=320;c.vdp.frame_height=224;c.vdp.rendered_frames=1;
    word(&c,0xe004,0x6092);word(&c,0xbf1a,24);word(&c,0xbf1c,24);
    word(&c,0xbf1e,296);word(&c,0xbf20,200);
    word(&c,0xe402,1728);word(&c,0xe3fe,1824);
    SDLHost h={0};assert(sdl_host_open(&h));
    SDL_SetWindowSize(h.window,960,672);
    assert(dune_view_map(&c));
    SDL_Event e={0};e.type=SDL_MOUSEWHEEL;e.wheel.y=-20;
    assert(dune_view_event(&h,&c,&e));assert(h.dune_view.zoom==50);
    e.wheel.y=20;assert(dune_view_event(&h,&c,&e));assert(h.dune_view.zoom==100);
    e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_0;h.dune_view.zoom=60;
    assert(dune_view_event(&h,&c,&e));assert(h.dune_view.zoom==100);
    /* Minimap click moves only the presentation camera and consumes the event. */
    e.type=SDL_MOUSEBUTTONDOWN;e.button.button=SDL_BUTTON_LEFT;
    e.button.x=272*3;e.button.y=176*3;
    word(&c,0xe3ec,160);word(&c,0xe3ee,224);c.pad_buttons[0]=PAD_C;
    assert(dune_view_event(&h,&c,&e));
    assert(h.dune_view.camera_x==880 && h.dune_view.camera_y==928);
    assert(dune_word(&c,0xe3ec)==160 && dune_word(&c,0xe3ee)==224);
    assert(c.pad_buttons[0]==PAD_C && !h.dune_mouse.count);
    assert(h.dune_mouse.camera_hold && h.dune_view.zoom==100);
    /* Busy-cycle estimate excludes native waiting, includes all other work. */
    e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_F3;e.key.repeat=0;
    assert(dune_view_event(&h,&c,&e) && h.dune_cpu_overlay);
    c.cycles=1000;c.dune_wait_cycles=800;c.vdp.frames=1;dune_cpu_sample(&h,&c);
    c.cycles=1100;c.dune_wait_cycles=880;c.vdp.frames=16;dune_cpu_sample(&h,&c);
    assert(h.dune_cpu_percent==20);
    c.cycles=1200;c.vdp.frames=31;dune_cpu_sample(&h,&c);assert(h.dune_cpu_percent==100);
    h.dune_speed_valid=0;c.master_cycles=0;
    dune_speed_sample(&h,&c,1000,1000);
    c.master_cycles=vdp_master_frequency(&c.vdp)/2;
    dune_speed_sample(&h,&c,1500,1000);assert(h.dune_speed_percent==100);
    c.master_cycles+=vdp_master_frequency(&c.vdp)/4;
    dune_speed_sample(&h,&c,2000,1000);assert(h.dune_speed_percent==50);
    h.paused=1;dune_speed_sample(&h,&c,2500,1000);assert(h.dune_speed_percent==0);h.paused=0;
    c.master_cycles=0;dune_speed_sample(&h,&c,2600,1000);assert(h.dune_speed_percent==0);
    CPU before=c;assert(dune_cpu_draw(&h,&c));assert(!memcmp(&before,&c,sizeof c));
    c.cycles=0;c.dune_wait_cycles=0;c.vdp.frames=0;dune_cpu_sample(&h,&c);
    c.cycles=100;c.dune_wait_cycles=100;c.vdp.frames=15;dune_cpu_sample(&h,&c);
    assert(h.dune_cpu_percent==0);
    assert(dune_view_event(&h,&c,&e) && !h.dune_cpu_overlay);
    /* A halted virtual CPU contributes waiting cycles, not busy work. */
    c.audio_mode=AUDIO_MUTE;c.z80_bus.reset_released=0;c.pc=0x200;c.sr=0x2700;
    machine_step(&c);assert(c.halted && !c.fault);
    uint64_t idle=c.dune_wait_cycles;machine_step(&c);
    assert(!c.fault && c.dune_wait_cycles==idle+4);
    c.halted=0;c.pc=0xfda;machine_step(&c);assert(!c.fault && c.dune_wait_cycles==idle+8);
    c.pc=0xfde;machine_step(&c);assert(!c.fault && c.dune_wait_cycles==idle+12);
    /* Options and password clicks feed original pad decoding, not setting fields. */
    DuneMouse menu={.enabled=1};c.pc=0x4d4e;c.a[7]=0xfff000;
    write_mem(&c,0xfff000,4,0x20f24);c.a[4]=1;c.pad_buttons[0]=0;
    word(&c,0xdbfc,0);word(&c,0xdbf8,0);c.vdp.rendered_frames=1;
    dune_mouse_observe(&menu,&c);assert(menu.front_kind==8);
    dune_mouse_click(&menu,&c,200,36,DUNE_CLICK_SELECT);
    dune_mouse_observe(&menu,&c);assert(c.d[0]==0x40 && !menu.count);
    menu.cooldown=0;dune_mouse_click(&menu,&c,200,132,DUNE_CLICK_SELECT);
    dune_mouse_observe(&menu,&c);assert(c.d[0]==PAD_DOWN && menu.count);
    word(&c,0xdbfc,5);dune_mouse_observe(&menu,&c);assert(c.d[0]==0x40 && !menu.count);
    write_mem(&c,0xfff000,4,0x21660);dune_mouse_observe(&menu,&c);
    assert(menu.front_kind==9);menu.cooldown=0;
    word(&c,0xdbf0,84);word(&c,0xdbf2,52);word(&c,0xdbfa,0);
    dune_mouse_click(&menu,&c,84,52,DUNE_CLICK_SELECT);
    dune_mouse_observe(&menu,&c);assert(c.d[0]==0x40 && !menu.count);
    menu.cooldown=0;dune_mouse_click(&menu,&c,84,52,DUNE_CLICK_CANCEL);
    dune_mouse_observe(&menu,&c);assert(c.d[0]==PAD_START && !menu.count);
    uint64_t master=c.master_cycles,cycles=c.cycles;
    c.dune_cpu_double=1;c.dune_cpu_clock_remainder=0;
    machine_advance(&c,1);assert(c.master_cycles==master+3);
    machine_advance(&c,1);assert(c.master_cycles==master+7 && c.cycles==cycles+2);
    e.type=SDL_KEYDOWN;e.key.repeat=0;e.key.keysym.sym=SDLK_F4;
    assert(dune_view_event(&h,&c,&e) && !c.dune_cpu_double);
    master=c.master_cycles;machine_advance(&c,2);assert(c.master_cycles==master+14);
    c.fault=0;word(&c,0xe002,0);
    /* The visible cursor reaches the edge before the OS pointer does. */
    h.dune_mouse.enabled=1;
    for(unsigned zoom=50;zoom<=100;zoom+=25)for(unsigned grid=0;grid<2;grid++){
        h.dune_view.zoom=zoom;
        DuneLayout p=dune_view_layout(&c,960,672,zoom);
        h.dune_view.camera_x=512+p.world_width/2-160;
        h.dune_view.camera_y=512+p.world_height/2-112;
        word(&c,0xe004,grid?0x6092:0x6d10);
        int near=(int)((grid?10:12)*p.world_scale);
        int inside=(int)((grid?65:15)*p.world_scale)+1;
        const int points[][4]={
            {near,336,-3,0},{960-near,336,3,0},
            {480,near,0,-3},{480,672-near,0,3},
            {inside,336,0,0},{960-inside,336,0,0},
            {480,inside,0,0},{480,672-inside,0,0}
        };
        e.type=SDL_MOUSEMOTION;
        for(unsigned i=0;i<8;i++){
            e.motion.x=points[i][0];e.motion.y=points[i][1];
            dune_mouse_event(&h,&c,&e);
            assert(h.dune_mouse.pan_x==points[i][2]);
            assert(h.dune_mouse.pan_y==points[i][3]);
        }
        e.type=SDL_WINDOWEVENT;e.window.event=SDL_WINDOWEVENT_LEAVE;
        dune_mouse_event(&h,&c,&e);
        assert(!h.dune_mouse.pan_x && !h.dune_mouse.pan_y);
    }
    sdl_host_close(&h);return 0;
}
'''
        path = self.root / 'dune.c'
        path.write_text('#define GENESIS_DUNE_MOUSE\n#define GENESIS_NO_MAIN\n' + emit(program) + harness)
        binary = self.root / 'dune'
        result = subprocess.run(['cc', '-std=c11', '-O1', '-DGENESIS_SDL2',
                                 *flags, str(path), '-o', str(binary), *libs],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software')
        result = subprocess.run([str(binary)], env=environment,
                                capture_output=True, text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)
