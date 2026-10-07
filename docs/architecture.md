# Architecture

ReArrakis has three layers:

1. **Static translation:** the Python package `genesis_recompiler` discovers 68000 code and emits C handlers. Z80 images and guarded variants are analyzed at build time. The internal package name is retained from the shared engine.
2. **Console runtime:** C headers implement CPU state, memory devices, VDP, interrupts, timing, controllers and sound. SDL2 handles presentation/input; ymfm synthesizes YM2612 audio.
3. **Dune integration:** `profiles/dune-us.json` identifies the ROM and callback tables; `tools/build_dune.py` adds verified discovery roots and sound variants. `dune_mouse*.h`, `dune_view*.h` and `dune_audio.h` implement input intent, expanded presentation and volume.

The default builder embeds ROM data in the executable. CPU opcodes are not decoded during execution: missing translations produce a diagnostic fault. Known mutable Z80 instructions use statically compiled variants with byte guards.

Mouse intent is applied at native input continuations; original game routines decide commands, selection and production. Expanded rendering uses map descriptors and native sprite traversal on a shadow CPU copy. The rendering copy does not advance live game time.

The standalone tree retains reusable engine functionality and its synthetic tests. Rings-specific menus, fonts, save slots, rendering modules, feature flags and builders were removed. Compatibility with arbitrary Genesis games is not promised.

See [provenance](provenance.md), [status](status.md) and the [Russian engineering journal](dune-status.md).
