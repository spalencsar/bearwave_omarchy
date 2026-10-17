#!/usr/bin/env bash

set -euo pipefail

readonly BEARWAVE_SOURCE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly BEARWAVE_PREFIX="${HOME}/.local"
readonly BEARWAVE_CACHE_ROOT="${XDG_CACHE_HOME:-${HOME}/.cache}/bearwave"
readonly BEARWAVE_BUILD_DIR="${BEARWAVE_CACHE_ROOT}/omarchy-build"

skip_dependencies=0
if [[ ${1:-} == "--skip-dependencies" ]]; then
  skip_dependencies=1
elif (( $# > 0 )); then
  echo "Usage: $0 [--skip-dependencies]" >&2
  exit 2
fi

if ! command -v omarchy >/dev/null 2>&1; then
  echo "BearWave requires Omarchy." >&2
  exit 1
fi

if (( ! skip_dependencies )); then
  echo "Installing the BearWave build and runtime dependencies through Omarchy..."
  omarchy pkg add base-devel cmake ninja qt6-base qt6-declarative \
    qt6-multimedia qt6-multimedia-ffmpeg qt6-tools playerctl
fi

mkdir -p -- "${BEARWAVE_BUILD_DIR}"

cmake -S "${BEARWAVE_SOURCE_DIR}" -B "${BEARWAVE_BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBEARWAVE_BUILD_TESTS=OFF \
  -DCMAKE_INSTALL_PREFIX="${BEARWAVE_PREFIX}"

cmake --build "${BEARWAVE_BUILD_DIR}" --parallel
cmake --install "${BEARWAVE_BUILD_DIR}"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${BEARWAVE_PREFIX}/share/applications" >/dev/null 2>&1 || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "${BEARWAVE_PREFIX}/share/icons/hicolor" >/dev/null 2>&1 || true
fi

echo
echo "BearWave is installed for the current user."
echo "Launch it from the Omarchy bar or application menu."
