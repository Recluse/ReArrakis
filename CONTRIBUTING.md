# Contributing

Fork [ReArrakis](https://github.com/kruzeman/ReArrakis), create a branch in your fork and submit a pull request. Describe the problem, resulting behavior and relevant checks; keep changes focused.

Primary documentation is English with supplementary Russian versions. Update [status](docs/status.md) and the [development journal](docs/dune-status.md) when findings or compatibility change.

Run `make test` for engine changes and `make demo` for synthetic execution. Neither needs a game ROM. Dune checks require your own supported ROM; report house, mission and tested actions. Presentation changes must preserve live simulation and original gameplay rules.

Never commit or attach ROMs, extracted resources, generated game C, game executables or saves. Public regression tests should use synthetic data. Preserve provenance and third-party notices.

Bug reports should include OS, commit, build command, ROM hash and the exact fault line or reproducible input sequence. Screenshots can help explain visual issues.
