#!/usr/bin/env python3
"""Build the verified user-provided Dune ROM; never download game data."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('rom', type=Path)
parser.add_argument('--emit-only', action='store_true')
args = parser.parse_args()
profile = json.loads((ROOT / 'profiles/dune-us.json').read_text())
try:
    rom = args.rom.read_bytes()
except OSError as error:
    parser.error(str(error))
if len(rom) != profile['rom_bytes'] or hashlib.sha256(rom).hexdigest() != profile['sha256']:
    parser.error('Expected the supported Dune USA ROM (see profiles/dune-us.json)')
build = ROOT / 'build'
build.mkdir(exist_ok=True)
z80 = build / 'dune-z80.bin'
z80.write_bytes(rom[0x2952:0x41dc])
boot = build / 'dune-z80-boot.bin'
boot.write_bytes(rom[0x2c4:0x2ea])
patched = build / 'dune-z80-patched.bin'
image = bytearray(rom[0x2952:0x41dc])
# Verified driver writes; these templates add guarded static variants only.
# The game still performs every upload/patch in live Z80 RAM itself.
image[0x2b3:0x2b8] = bytes.fromhex('18261826c9')
patched.write_bytes(image)
music = build / 'dune-z80-music.bin'
image[0x2b7:0x2b9] = bytes.fromhex('d908')
music.write_bytes(image)
playing = build / 'dune-z80-playing.bin'
image[0x2c4:0x2c6] = bytes.fromhex('0000')
image[0x302] = 0
playing.write_bytes(image)
voice = build / 'dune-z80-voice.bin'
# The live driver at $1405/$1409 -> $140B writes NOP or RET to $02CF,
# enabling the PCM/voice path. Add the NOP path with ordinary opcode guards.
assert image[0x2cf] == 0xc9
image[0x2cf] = 0
voice.write_bytes(image)
command = [sys.executable, '-m', 'genesis_recompiler', str(args.rom.resolve()),
           '-o', 'build/dune.c' if args.emit_only else 'build/dune',
           '--report', 'build/dune.json', '--z80-image', str(boot),
           '--z80-image', str(z80), '--z80-image', str(patched), '--z80-image', str(music), '--z80-image', str(playing), '--z80-image', str(voice),
           '--z80-image-entry', '2:0x2b3', '--z80-image-entry', '3:0x2b3', '--z80-image-entry', '4:0x2b3', '--z80-image-entry', '5:0x2b3', '--z80-image-entry', '5:0x2cf',
           '--z80-image-entry', '1:0x38', '--z80-image-entry', '2:0x38', '--z80-image-entry', '3:0x38', '--z80-image-entry', '4:0x38', '--z80-image-entry', '5:0x38',
           '--z80-overlay', '0:e9', '--z80-mutable-displacement', '0x1616',
           '--z80-mutable-displacement', '0x15f9']

# Dune's compiler emits signed switch bounds (BMI/BGT), after EXT.L
# or SUBQ.L/SUBI.L normalization (including projectile creation).
# The generic discovery does not yet resolve them. Confirm the full range check before seeding roots.
sys.path.insert(0, str(ROOT))
from genesis_recompiler.decode import analyze, Decoder, EA
entries = set(profile['entries'])
# Script loaders at $16C12/$16C30 install these ROM callback tables.
# The original dispatcher at $171D0-$171E4 calls table[opcode]; translate
# every declared handler, including commands absent from the boot path.
for table in profile.get('callback_tables', []):
    offset, count = table['offset'], table['count']
    assert 0 <= offset <= len(rom) - count * 4
    for pos in range(offset, offset + count * 4, 4):
        target = int.from_bytes(rom[pos:pos + 4], 'big')
        assert target % 2 == 0 and 0 <= target < len(rom)
        Decoder(rom, target).decode()
        entries.add(target)
# Verified signed-offset dispatch tables: intro scripts at $41936 and the
# password actions at $2197C. Intro opcode zero is a terminator, while all
# eleven password switch slots are valid (including intentional no-ops).
for table in profile.get('relative_callback_tables', []):
    base, first, count = table['offset'], table['first'], table['count']
    assert 0 <= first < count and 0 <= base <= len(rom) - count * 2
    for index in range(first, count):
        pos = base + index * 2
        target = base + int.from_bytes(rom[pos:pos + 2], 'big', signed=True)
        assert target % 2 == 0 and 0 <= target < len(rom)
        Decoder(rom, target).decode()
        entries.add(target)
for _ in range(16):
    program = analyze(rom, [0x200, *entries])
    preceding = {inst.end: inst for inst in program.instructions.values()}
    added = set()
    # $4796 installs the original VBlank screen callback in $FFE002.
    # Constant arguments are executable entry points, not ordinary ROM data.
    for call in program.instructions.values():
        if call.op != 'JSR' or call.target != 0x4796:
            continue
        argument = preceding.get(call.pc)
        if (argument is not None and argument.op == 'MOVE' and argument.size == 4
                and argument.src.mode == 7 and argument.src.reg == 4
                and argument.dst == EA(2, 7)
                and 0 < argument.src.value < len(rom)
                and argument.src.value % 2 == 0):
            Decoder(rom, argument.src.value).decode()
            added.add(argument.src.value)
    for jump in program.instructions.values():
        if jump.op != 'JMP' or jump.src.mode != 7 or jump.src.reg != 3:
            continue
        # Encoded references mask their two tag bits, then ROL.W #3 maps
        # them to byte offsets 0,2,4,6 in an inline instruction table.
        rotate = preceding.get(jump.pc)
        mask = preceding.get(rotate.pc) if rotate is not None else None
        if (rotate is not None and mask is not None
                and rotate.op == 'ROL' and rotate.size == 2
                and rotate.src == EA(7, 4, value=3)
                and rotate.dst.mode == 0
                and mask.op == 'ANDI' and mask.size == 2
                and mask.src == EA(7, 4, value=0xc000)
                and mask.dst == rotate.dst
                and jump.src.index == ((rotate.dst.reg << 12) | 2)):
            for index in range(4):
                target = jump.src.value + index * 2
                Decoder(rom, target).decode()
                added.add(target)
            continue
        sequence = []
        before = jump.pc
        for n in range(6):
            inst = preceding.get(before)
            if inst is None:
                break
            sequence.append(inst)
            before = inst.pc
        if len(sequence) != 6:
            continue
        load, scale, upper, compare, lower, normalize = sequence
        if not (load.op == 'MOVE' and load.size == 2 and load.src.mode == 7
                and load.src.reg == 3 and load.dst == EA(0, 0)
                and load.src.value == jump.src.value
                and load.src.index == 0x0806 and jump.src.index == 0x0002
                and scale.op == 'ADD' and scale.size == 4
                and scale.src == EA(0, 0) and scale.dst == EA(0, 0)
                and upper.op == 'BCC' and upper.condition == 14
                and compare.op == 'CMPI' and compare.size == 4
                and compare.dst == EA(0, 0) and compare.src.mode == 7
                and compare.src.reg == 4 and 0 <= compare.src.value < 256
                and lower.op == 'BCC' and lower.condition == 11
                and lower.target == upper.target
                and normalize.op in ('EXT', 'SUBQ', 'SUBI') and normalize.size == 4
                and normalize.dst == EA(0, 0)):
            continue
        base = load.src.value
        for index in range(compare.src.value + 1):
            offset = base + index * 2
            target = base + int.from_bytes(rom[offset:offset+2], 'big', signed=True)
            Decoder(rom, target).decode()
            added.add(target)
    new = added - entries
    if not new:
        break
    entries.update(new)

for entry in sorted(entries):
    command += ['--entry', hex(entry)]
if not args.emit_only:
    command += ['--build', '--frontend', 'sdl2', '--sound', 'ymfm', '--emit-c', 'build/dune.c']
raise SystemExit(subprocess.call(command, cwd=ROOT))
