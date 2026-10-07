# Project status

ReArrakis targets the verified USA revision of **Dune — The Battle for Arrakis**. It is an experimental native Linux build, not a finished port or a general-purpose Genesis recompiler.

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
