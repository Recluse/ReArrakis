# Source provenance

ReArrakis was extracted on 2026-10-07 from [kruzeman/GenesisRecomp](https://github.com/kruzeman/GenesisRecomp), branch `codex/dune`, commit [`93c39a46467f6c65ed77986a8ede2684407a2426`](https://github.com/kruzeman/GenesisRecomp/commit/93c39a46467f6c65ed77986a8ede2684407a2426).

Dune's shared RROP foundation is `291ea812663e69451251b6ab8e6ef50c22015d4a`, as recorded in the source development journal.

This project starts a clean repository history. The source repository and branches were not altered. Earlier engineering results are preserved in [dune-status.md](dune-status.md).

Cleanup removed Rings-specific runtime modules and conditional paths, CLI options, builders, tests, launchers, screenshots and unrelated documentation. Dune's profile, discovery logic, input and presentation modules, audio integration and game fixtures were preserved. Project metadata, documentation, CI and window branding now refer to ReArrakis.

The shared Python package keeps its internal `genesis_recompiler` name; its installed command is `rearrakis-recompile`. Normal game builds use `tools/build_dune.py`.

MIT notices and the vendored ymfm revision/BSD license are retained. No original game data or generated game translation is published.
