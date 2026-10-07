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
        program = analyze(rom_with('4e72 2700'), [0x200])
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
