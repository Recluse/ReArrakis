#!/usr/bin/env bash
set -eu
cd -- "$(dirname -- "$0")"
if [[ ! -x build/dune ]]; then
    printf '%s\n' 'Build Dune first: python3 tools/build_dune.py /path/to/Dune.gen' >&2
    exit 1
fi
exec ./build/dune --window --audio on "$@"
