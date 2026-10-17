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

### Removed

- Container packaging manifests, build scripts, generated artifacts, and
  deployment documentation.
- Remote publishing workflows.
