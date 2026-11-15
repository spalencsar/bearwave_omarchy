# Contributing to BearWave

BearWave is a local Qt 6 internet-radio app for Omarchy, built with C++/QML and QtMultimedia.

The project favors:

- stability over feature churn
- minimal dependencies (Qt only at runtime)
- readable, conservative code
- solid Omarchy integration (theme, MPRIS, tray)

## Before You Change Code

- Prefer focused local changes over broad refactors.
- Keep behavior predictable for desktop users.
- Avoid adding dependencies unless there is a clear payoff.

## Local Setup

Build from the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

Optional local install:

```bash
cmake --install build --prefix "$HOME/.local"
```

If you changed QML:

```bash
qmllint src/qml/Main.qml src/qml/components/*.qml src/qml/theme/BearTheme.qml
```

The UI is the Omarchy layout only: sidebar, wide station list, search toolbar,
and a full-width bottom player. Compact mode hides the sidebar below 760px.
There are no inner frames; Hyprland draws the outer window border.

### Layout modes to smoke-test after UI changes

Test the wide Omarchy station list with its bottom player and the compact
single-column layout below the sidebar threshold.

```bash
# Example short-window check
./build/src/bearwave
# then resize to ~1440×900
```

Run unit tests after backend changes:

```bash
ctest --test-dir build --output-on-failure
```

Tests must keep their XDG configuration and cache state isolated. Never run
fixtures against or leave fixture data in a contributor's real
`~/.config/bearwave` profile.

### Live Omarchy bar plugin

The shell loads `~/.config/omarchy/plugins/de.nerdbear.bearwave/`, not this
checkout. Sync the bar files into that real directory (not a symlink — the
shell's inotify watcher does not follow one, and it caches QML by URL):

```bash
./scripts/link-omarchy-plugin.sh
omarchy restart shell
```

After the first sync, saving files under the plugin directory hot-reloads.
Saving only in this checkout does not. Run the script again after widget
edits, then restart the shell if the bar still shows the previous widget.

## Change Guidelines

- Follow the existing style in each file.
- Keep changes minimal and scoped.
- Do not block the UI thread.
- Keep network operations asynchronous.
- Treat station data, stream metadata, artwork URLs, and manually entered
  stream URLs as untrusted input. Do not weaken URL validation, private-file
  permissions, or explicit plain-text rendering at QML display boundaries.
- Preserve MPRIS behavior and user state compatibility.
- When changing resources, register them in `src/qml.qrc`.
- When changing the About page, keep the BearWave wordmark, the full GPL text,
  and accurate third-party technology notes.

## Manual Checks

Please smoke-test the area you changed when possible:

- app launches successfully
- frosted-glass rendering and window behavior work under Hyprland
- station list loads
- search and filtering behave correctly
- play/pause/stop work
- favorites persist across restart
- resume restores last station and volume
- tray and MPRIS behavior still work
- About page opens from the sidebar footer, the wordmark is visible, and the GPL license text is scrollable

Record important local changes and verification commands in the changelog or
the handoff notes for this working directory.

## CI and deployment

Forgejo Actions builds and tests pull requests, manual runs, and pushes to
`main`. A successful push to `main` then mirrors the exact tested revision to
the public GitHub repository. Do not push the mirror directly or add local
`.agents` notes to Git; the workflow rejects tracked `AGENTS.md` and
`Memory.md` files.
