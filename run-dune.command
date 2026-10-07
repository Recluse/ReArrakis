#!/bin/bash
set -eu
cd -- "$(dirname -- "$0")"
exec ./run-dune.sh "$@"
