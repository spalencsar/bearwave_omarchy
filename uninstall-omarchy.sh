#!/usr/bin/env bash

set -euo pipefail

readonly BEARWAVE_PREFIX="${HOME}/.local"

rm -f -- \
  "${BEARWAVE_PREFIX}/bin/bearwave" \
  "${BEARWAVE_PREFIX}/share/applications/de.nerdbear.bearwave.desktop" \
  "${BEARWAVE_PREFIX}/share/icons/hicolor/scalable/apps/de.nerdbear.bearwave.svg" \
  "${BEARWAVE_PREFIX}/share/icons/hicolor/symbolic/apps/de.nerdbear.bearwave-symbolic.svg"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${BEARWAVE_PREFIX}/share/applications" >/dev/null 2>&1 || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "${BEARWAVE_PREFIX}/share/icons/hicolor" >/dev/null 2>&1 || true
fi

echo "The locally installed BearWave application was removed."
echo "To remove the bar plugin too, run:"
echo "  omarchy plugin remove de.nerdbear.bearwave"
