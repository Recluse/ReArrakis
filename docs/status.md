# Project status

ReArrakis targets the verified USA revision of **Dune — The Battle for Arrakis**. It is an experimental native Linux/macOS build, not a finished port or a general-purpose Genesis recompiler.

## Scope recorded in the source project

- SDL2 startup, title/intro, house selection and the first mission.
- A 7,200-frame cold-boot run with synthesized audio and no translation fault.
- The first Atreides mission through victory, including resource collection.
- Mouse selection, orders, construction, production menus and supported dialogs.
- Adaptive map rendering, 50–100% zoom, separate HUD and edge scrolling.
- YM2612/PSG synthesis with the translated Z80 driver and voice-path variants.

The preserved [Russian journal](dune-status.md) records source-project checks and fixture arguments. These observations do not verify the whole campaign.

## Standalone migration verification — 2026-10-07

The cleaned ReArrakis tree builds the supported USA ROM with 40,348 translated
68000 instructions and 3,095 guarded Z80 variants. All 286 retained engine
checks and four ROM-free Dune adapter checks passed. The synthetic demo and an
installed wheel build/run also passed; the wheel includes the required runtime
headers and ymfm license.

Native game fixtures passed for Z80 RET/NOP patches, projectile dispatch and
creation, encoded references, mouse-only front menus and house choices, native
building menus, and adaptive rendering/input. The view fixture covers 30
size/zoom combinations, cursor/HUD separation, zoomed orders and placement.
A 7,200-frame smoke run ended at PC $00118C after 83,059,078 instructions with
no fault. A separate 20-million-instruction synthesized-audio run completed
with 229,726 YM writes and 1,347,522 stereo frames, without a fault.

The new minimap click path has a synthetic SDL event regression check. Actual
minimap gameplay and hardware-dependent fullscreen/audio remain manual checks.
No full campaign completion is claimed. Source ROM data, generated C, sound
templates, executables and test captures remain local in ignored build/.

## macOS verification — 2026-10-07

On Apple Silicon (macOS 27.0, Python 3.14.8, Apple Clang and SDL2 2.32.74),
the unmodified engine passed all 290 retained tests, with the Linux-specific
`/dev/full` check skipped. The synthetic demo builds and runs as a native
Mach-O arm64 executable. Finder-launcher argument forwarding and working
directory handling were checked with a synthetic executable.

The verified USA ROM builds with the existing builder as a native arm64
executable. Local real-ROM checks passed for mouse-only intro/menu navigation,
all three house selections and first-mission construction menus, Z80 RET/NOP
patches, projectile creation and encoded references. The SDL software-renderer
fixture passed all 30 size/zoom combinations, pixel-exact original rendering,
cursor/HUD isolation, zoomed unit orders and building placement.

The existing construction and combat fixtures also passed: windtrap purchase
and placement, Harkonnen combat through enemy destruction, and an Atreides
refinery/harvester run through the original mission-completion transition
at frame 27,360. No credits or victory flags were edited.

A 20-million-instruction headless run with ymfm produced 1,347,522 stereo
frames and 229,726 YM writes without a translation fault or dropped samples.
The budget exit (status 2) is expected. These checks use software rendering
and synthesized audio; accelerated rendering, fullscreen and physical audio
output still require a manual desktop check.

## Passwords and 68000 occupancy — 2026-10-07

The password action table at `$2197C` now seeds all 11 native branches,
fixing the missing translation at `$21A0C` when entering PLAYTESTER.
The USA build contains 40,382 M68K and 3,095 Z80 instructions.
A bounded native fixture entered all 29 passwords through the original
controller-driven password keyboard and verified their handler effects.
It also checked both toggle codes a second time and rejected three invalid
inputs. The fixture invokes the password screen from an initialized first
mission snapshot; this does not verify playing every unlocked mission.

F3 toggles an approximate virtual 68000 occupancy overlay, sampled every
15 frames. It excludes cycles in the original VBlank wait loop at
`$0FDA`/`$0FDE` and halted CPU cycles. Interrupt work counts as occupied;
other unidentified busy waits also count as occupied. This is not host CPU
usage and does not change the console clock or game speed.
The 17 adapter/timing tests passed and the optimized Linux build completed.
The reported X11 Compose warning is separate and has not been fixed here.

## Limitations

- Other ROM revisions are rejected.
- Full campaign completion and every player command combination remain unverified.
- Minimap navigation was added in the last source commit and needs manual gameplay verification.
- Bounded rendering and extreme window proportions can limit effective zoom.
- Fullscreen, graphics drivers and audio hardware need testing on users' systems.
- Volume adjustment does not independently rebalance music, effects and voices.
- The standalone builder has no verified Windows packaging yet.
- RROP's host settings, external fonts and full-state save slots are not Dune features.

Report [issues](https://github.com/kruzeman/ReArrakis/issues) with OS, source revision, ROM hash, house/mission and the exact action or diagnostic line. Do not upload game data.
