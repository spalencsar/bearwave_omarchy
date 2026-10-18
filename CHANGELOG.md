# Changelog

## Unreleased — Omarchy edition

### Added

- Native Omarchy session detection.
- Live palette loading from the active Omarchy `colors.toml`.
- Automatic palette refresh after an Omarchy theme change.
- Omarchy monospace typography and native Hyprland frosted-glass surfaces.
- A compact Omarchy layout with a framed sidebar, wide station list, plain
  metadata rows, and a full-width bottom player.

### Changed

- BearWave is now a local Omarchy-only application.
- Local CMake/Ninja builds are the supported build and installation path.
- A minimal fallback palette remains for diagnostics when the local Omarchy
  palette cannot be read.
- Forgejo Actions now builds and tests every change; successful pushes to
  `main` mirror the tested revision to the public GitHub repository.

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

- Container packaging manifests, build scripts, generated artifacts, and
  deployment documentation.
