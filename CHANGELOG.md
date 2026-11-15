# Changelog

## 1.4.0 — 2026-11-15

Omarchy-only desktop radio: split backend, list-first UI, live bar plugin.

### Added

- Native Omarchy session detection and live palette from `colors.toml`,
  including refresh after a theme change.
- Omarchy monospace typography and Hyprland frosted-glass surfaces.
- `scripts/link-omarchy-plugin.sh` rsyncs bar files into a real plugin
  directory at `~/.config/omarchy/plugins/de.nerdbear.bearwave/` (not a
  symlink: inotify does not follow one, and Quickshell caches QML by URL).
- Station identity, persisted libraries, and the Radio Browser catalog are
  separate types; `RadioBackend` remains the QML playback facade.
- Plugin `i18n/` catalogs (`en`, `de`, `nl`, `ru`) for bar tooltips.
- `omarchy/BarModel.js` plus a unit test for the bar MPRIS helper.

### Changed

- BearWave is a local Omarchy-only application. CMake/Ninja is the supported
  build and install path.
- A minimal fallback palette remains when the Omarchy palette cannot be read.
- Forgejo Actions builds and tests every change; successful pushes to `main`
  mirror the tested revision to the public GitHub repository.
- The bar widget follows MPRIS D-Bus properties through Quickshell instead of
  polling `gdbus`. Hover shows `(Sender) Artist - Titel`. Clicks stay launch,
  play/pause, and stop.
- Window UI follows the system locale (Omarchy Setup > Language). `--language`
  remains as a debug override. German, Dutch, and Russian catalogs are filled.
- Inner layout frames are gone. Dialogs use opaque surfaces so text stays
  readable over the blurred window.
- Sidebar brand and About hero use `assets/app/bearwave_line.png`.
- Desktop toolbar is search only. The list header is one row: title, count,
  Name / Bitrate / Votes. Favorites, my stations, and recents keep their own
  order.
- Quick-filter chips and the Search Results sidebar row are gone. US, GB, and
  FR sit under Netherlands; World stays in the sidebar. Search stays in the
  field and then replaces the current list.
- README screenshots show the 1.4.0 German list UI, the BearWave wordmark,
  and the Omarchy bar with MPRIS now-playing.
- Add station is the sidebar footer plus (and the empty-state button on My
  stations).
- Station rows: click plays, current row uses an accent rail, bitrate on the
  right, heart and play always visible with exclusive hover.
- Closing the window hides it; the bar restores it.

### Security

- Block artwork requests to loopback, private, link-local, reserved, and
  otherwise non-public network addresses, including unsafe DNS answers and
  redirects.
- Protect persisted stream URLs by enforcing owner-only configuration
  directories and files and by using atomic private-file writes.
- Render externally supplied station names, stream metadata, playback status,
  history, tags, and codec details as plain text so markup cannot trigger
  network requests from the UI.

### Removed

- In-app language picker and persisted `AppSettings` UI language.
- Container packaging manifests and deployment documentation.
- Leftover Plasma plasmoid tree and the obsolete `org.kde.bearwave.desktop`
  ignore rule.
- Unused multi-desktop QML (detail stage, chip cards, rounded chrome,
  `QuickFilters`).
