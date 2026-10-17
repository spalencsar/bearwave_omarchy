# BearWave for Omarchy

BearWave is a local Qt 6 internet-radio player designed specifically for
Omarchy and Hyprland.

The application follows the current Omarchy theme from
`~/.local/state/omarchy/current/theme/colors.toml`, uses the system monospace
font, and renders its window as native frosted glass. Theme changes are picked
up while BearWave is running.

## Features

- Radio Browser station search and browsing
- favorites, recent stations, and manually added streams
- ICY metadata and cover artwork
- MPRIS media controls and desktop notifications
- system-tray background playback
- German, Dutch, and Russian translations
- Omarchy palette, typography, translucent surfaces, and compact layout

## Requirements

- Omarchy with an active Hyprland session
- CMake 3.16 or newer
- Ninja or Make
- Qt 6 Base, Declarative, Multimedia, Multimedia FFmpeg, and Tools

On an Omarchy/Arch installation the development dependencies are available as:

```bash
sudo pacman -S --needed cmake ninja qt6-base qt6-declarative \
  qt6-multimedia qt6-multimedia-ffmpeg qt6-tools
```

## Local build

Run from this directory:

```bash
cmake -S . -B build-omarchy -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-omarchy
ctest --test-dir build-omarchy --output-on-failure
./build-omarchy/src/bearwave
```

## Install as an Omarchy plugin

Add and enable the BearWave bar plugin:

```bash
omarchy plugin add https://hostbw.de/spalencsar/bearwave_omarchy.git --enable
```

Omarchy intentionally does not execute install hooks for plugins. On first
click, the BearWave bar icon therefore opens the visible local installer in an
Omarchy terminal. You can run the same installer directly:

```bash
~/.config/omarchy/plugins/de.nerdbear.bearwave/install-omarchy.sh
```

The installer uses `omarchy pkg add` for the required Arch packages, builds
BearWave from the checked-out source, and installs the application only for the
current user below `~/.local`. It does not use Flatpak or the AUR.

After installation, left-click the bar icon to open BearWave, middle-click to
play or pause, and right-click to stop playback. The application is launched
with its own tray icon disabled so the bar does not show BearWave twice.

To remove the local application and then its plugin:

```bash
~/.config/omarchy/plugins/de.nerdbear.bearwave/uninstall-omarchy.sh
omarchy plugin remove de.nerdbear.bearwave
```

## Omarchy integration

BearWave recognizes Omarchy through the desktop-session environment and the
local `/usr/share/omarchy` installation. It reads these local design sources:

- palette: `~/.local/state/omarchy/current/theme/colors.toml`
- font: the Fontconfig `monospace` alias selected by Omarchy
- compositor: Hyprland supplies the blur behind BearWave's translucent window

BearWave does not modify the Omarchy or Hyprland configuration.

## Local data

- settings and stations: `~/.config/bearwave/`
- API and artwork cache: `~/.cache/bearwave/`

Set `BEARWAVE_THEME=light` or `BEARWAVE_THEME=dark` only for temporary theme
debugging outside the normal Omarchy palette.

## Useful controls

- `Space`: play or pause
- `Ctrl+F`: focus station search
- closing the window hides BearWave in the system tray

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
