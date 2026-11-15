#!/usr/bin/env bash
# Copy plugin-facing files into a real directory under
# ~/.config/omarchy/plugins/. A symlink to this checkout is not used:
# inotifywait does not follow it, and the shell caches QML by URL so
# rescanPlugins would keep the old widget.

set -euo pipefail

readonly PLUGIN_ID="de.nerdbear.bearwave"
readonly PLUGINS_DIR="${XDG_CONFIG_HOME:-${HOME}/.config}/omarchy/plugins"
readonly REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
readonly TARGET="${PLUGINS_DIR}/${PLUGIN_ID}"
readonly BACKUP="${PLUGINS_DIR}/.${PLUGIN_ID}.clone.bak"

mkdir -p -- "${PLUGINS_DIR}"

if [[ -L ${TARGET} ]]; then
  echo "Replacing plugin symlink with a real directory"
  rm -f -- "${TARGET}"
fi

if [[ ! -d ${TARGET} && -d ${BACKUP} ]]; then
  echo "Restoring ${BACKUP}"
  mv -- "${BACKUP}" "${TARGET}"
fi

mkdir -p -- "${TARGET}/omarchy" "${TARGET}/assets/app"
rsync -a --delete "${REPO_ROOT}/omarchy/" "${TARGET}/omarchy/"
rsync -a "${REPO_ROOT}/assets/app/" "${TARGET}/assets/app/"
cp -a -- "${REPO_ROOT}/manifest.json" "${TARGET}/manifest.json"
cp -a -- "${REPO_ROOT}/install-omarchy.sh" "${TARGET}/install-omarchy.sh"
cp -a -- "${REPO_ROOT}/uninstall-omarchy.sh" "${TARGET}/uninstall-omarchy.sh"
if [[ -f ${REPO_ROOT}/preview.png ]]; then
  cp -a -- "${REPO_ROOT}/preview.png" "${TARGET}/preview.png"
fi

if command -v omarchy-plugin-validate >/dev/null 2>&1; then
  omarchy-plugin-validate "${TARGET}"
fi

echo "Synced plugin files into ${TARGET}"
echo "Restart the Omarchy shell so it drops the cached QML component."
