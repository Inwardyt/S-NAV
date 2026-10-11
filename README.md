# S-NAV Simulator

A standalone S-NAV simulator based on **SCP – Containment Breach**, by **Inward 3D / Nick Ward**

I'm building this as the first step toward a working physical S-NAV replica. It recreates the scrolling map, player marker and SCP distance rings without needing the game running.

## Windows builds

- **S-NAV-Simulator.exe** — seed and Navigator settings, current room information, and map import/export.
- **S-NAV-Screen.exe** — just the LCD, with no menus or control panels.

Both use `S-NAV.ini` in the same folder. Choose a seed and Navigator in the simulator, then launch the screen-only version with those settings.

## Controls

| Key | Action |
| --- | --- |
| Left / Right | Turn |
| Up / Down | Move forward / backward |
| Shift | Move faster |
| Space | Blink |
| Enter | Provoke nearby SCP-096 |
| Tab | Pause |
| F11 | Toggle fullscreen |
| R / N | Restart / random seed |
| M / P / B | Monochrome / power / fresh battery |
| D | Device view, simulator only |
| F1 | Full controls |

## Build

Install MinGW-w64 with `gcc` and `windres` on your PATH, then run:

```bat
build-windows.bat
```

This builds both executables and creates the default settings file. To replace the app icon, change `src/icon.ico` and rebuild.

Portable simulation checks on Linux:

```sh
./build-linux-tests.sh
```

## Current limitations

Seed maps, room placement and SCP behavior are approximations. They don't exactly match the game's same seed. `game_map_export.bb` provides a game-side export snippet for importing actual room grids and names.

The current executables target **64-bit desktop Windows**. Windows CE/Recon support is planned separately.

See [CREDITS.md](CREDITS.md) for the original game assets and font.
