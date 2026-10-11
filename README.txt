S-NAV SIMULATOR 0.2.1 - by Inward 3D

TWO EXECUTABLES
Extract the ZIP, then choose:

S-NAV-Simulator.exe
Configurable desktop version: seed box, Generate/Random, Navigator selector,
current-room name and zone at the top, status, map import/export, display menu.

S-NAV-Screen.exe
Borderless, resizable LCD-only version: no menu, toolbar, room label or status.
The mouse pointer is hidden over the LCD. F11 enters/exits full screen.
Both executables are 64-bit DESKTOP WINDOWS builds. They are NOT Windows CE
executables. A Recon ARM/CE port still needs its exact model and OS/SDK.

SHARED SETTINGS
Both read S-NAV.ini next to the executables. The desktop version saves the
seed and model when you generate a map or select a different Navigator.
The screen-only version uses those same settings next time it starts.
Close/reopen it after changing settings; already-running instances do not sync.
You can also edit the INI with Notepad:
seed=any string
model=damaged / 300 / 310 / ultimate
monochrome=0 (game red rings) or 1 (dark rings)
fullscreen=0 (window) or 1 (start full screen)
Place the folder somewhere writable to allow settings to save.

CONTROLS (BOTH BUILDS)
Left/right arrows  Turn (2.6 radians/second, about 37% faster than v0.1)
Up/down arrows     Forward/backward (4.2 units/second, 40% faster)
Shift              Faster movement (7.2 units/second)
Space              Blink
Enter              Provoke an EXISTING 096 within 8 units; does not spawn one
Tab                Pause/resume
M                  Monochrome / red radar rings
P                  Power on/off
B                  Fresh battery
R                  Restart this seed
N                  New random seed, also saved to INI
F11                Full screen toggle
Esc                Leave full screen; close screen-only window if already windowed
D                  Device artwork toggle (configurable build only)
F1                 Controls help, when explicitly pressed
The configurable build's illustrated D-pad is holdable with the mouse.
Its OK button provokes nearby 096; minus pauses; power toggles; return restarts.
There is no sound in either build.

CHANGES IN 0.2
- Split the configurable and LCD-only executables.
- Added shared INI settings and a top current-room name/zone label.
- Replaced drawing directly over a black clear with a persistent offscreen
  bitmap and a single BitBlt. Status text updates only when changed.
- Added circular collision against empty-cell walls AND corners. Radius 2.25
  world units covers the arrow's 6-pixel/2-unit tip with a small raster margin.
- Increased movement and turn speed.
- Added a room catalog from the supplied Data/rooms.ini: shape, zone,
  commonness, reserved placement fraction/priority, and no-decals flag.
- Changed NPCs from nearby-at-start to inactive until room events/timers.
- Added named-room tagged map import/export (SNAV-MAP-2).

NPC PLACEMENT AND TIMING
173: specific room events, including room2closets, room2testroom2, probabilistic
lockrooms and a small subset of tunnel/008 events. Each room trigger runs once.
It does not start next to the player. Additional generic LCZ appearances are
rare room-entry relocations: 8% chance after a 90-180 second cooldown, excluding
start/checkpoint1. This is a deliberate low-frequency approximation of source
waypoint relocation, not its exact per-frame RNG. 173 freezes when observed;
its simulated active episode expires rather than repeatedly respawning nearby.

106: initial normal-difficulty timer is 12-17 minutes, directly from
Main.bb: Curr106\State = 70 * 60 * Rand(12,17).
The generic timer pauses in rooms with disabledecals=true, like the source.
After a timed 45-second episode, the next cooldown derives from source
Rand(22000,27000)/70, about 314-386 seconds. The exact source despawn depends
on visibility, distance and difficulty; 45 seconds is our simplified episode.
One selected LCZ endroom can give an earlier 12-second visual appearance;
this does not enable instant global pursuit. 106 CAN appear in LCZ in the game.
Not all of the game's many 106 room events are implemented in this build.

096: only spawns on entering an assigned eligible source room: HCZ server room,
probabilistic HCZ tunnel/pit/pipes events, or the EZ open lockroom (lockroom2).
No generated LCZ room has a 096 spawn trigger. An existing provoked 096 can
pursue across zones; it is not globally teleported into LCZ at startup.
It stays in its spawn room until you provoke it with Enter, waits eight
seconds, then follows the room graph. Actual face/guard/animation logic is omitted.

049: a room049 visit activates its HCZ chamber approximation. It does not
wander out of a random spawn toward LCZ. The real chamber has vertical levels
and elevator triggers; these are represented only by a 2D room-entry event.

895: assigned to an actual coffin template instead of an arbitrary dead end.
Map interference near it remains approximate.

All encounter resets return to room/cooldown logic, rather than picking a new
random nearby room. Source event chances and special-room names are used where
practical, but this remains a radar-location simulation, not a complete AI/event
port. Doors, vertical levels, exact world offsets and scripts are not simulated.

MAP FIDELITY
Seeds are repeatable SIMULATOR seeds, not byte-for-byte SCP-CB maps.
The seed string XOR/shift conversion comes from the source. The PRNG, extra
connectivity/dead ends and room-placement collision resolution differ.
Room names come from source shape/zone pools and approximate reserved positions;
they are accurate catalog names but not guaranteed to match a game's same seed.
You begin in the start dead end at grid 9,17.
Numeric seed text is treated as a string, not a Blitz3D numeric seed override.

EXACT GAME GRID AND ROOM NAMES
The included game_map_export.bb is a source snippet to add to the game.
Call ExportSNavGrid("facility.snavmap") after ConnectAdjacentRooms() at the
end of CreateMap() in MapSystem.bb, rebuild/run the game, then import the file
via the desktop build's Simulation menu. It exports the actual occupied grid
AND room template names. The snippet has not been compiled in Blitz3D here.
The simulator's movement and NPC event scheduling still remain approximations.
Legacy SNAV-MAP-1 files work, but get estimated room names.
The importer requires one connected 19x19 grid. Disconnected maps are rejected.
Restart regenerates the seed map; imported maps are not saved as the default
screen-only startup layout in this version.

MAP FILE FORMAT
SNAV-MAP-2
19 rows of 19 #/. characters (occupied/empty)
Then optional lines: x y room_template_name
Every supplied name must be recognized in the source room catalog.
The simulator's Export room grid writes this format.

NAVIGATOR DISPLAY
300: discovered rooms; offline database notice; battery.
310: full local map; battery; no SCP detector.
Ultimate: full local map; unlimited battery; SCP proximity rings.
Damaged: discovered map and red arrow/notice.
The map scrolls while heading changes; it remains north-up.
Rooms are 24 logical pixels wide / 8 world units = 3 pixels per world unit.
173 detection uses ceil(distance/8)*8 < 32 and radius=distance*3.
106/096/049 use raw distance < 32 and radius=distance*1.5.
Rings show distance, not SCP bearing or exact map position.
The arrow/notice deliberately blink 700 ms on per second; radar blinks 400 ms
on per 600 ms. Those are original source effects. The whole LCD should not
flash black; only the marker/rings should disappear during their blink phases.

BUILD / VERIFICATION
Run build-windows.bat with MinGW-w64 GCC to rebuild both executables.
No asset conversion is required: artwork/fonts/catalog are embedded.
Portable code: src/snav.c, src/render.c, src/snav.h, src/room_data.h.
Windows frontend: src/win32.c; SNAV_SCREEN_ONLY chooses the reduced frontend.
Run build-linux-tests.sh to check the portable simulation.
Read VERIFICATION.txt for exactly what was tested and remaining limitations.
HARDWARE-ROADMAP.md carries the original Recon/custom-device plan.

CHANGING THE ICON
Replaced src/icon.ico, then run build-windows.bat with MinGW-w64 GCC and
windres available. The resource is compiled into both executables.
No separate icon.ico is needed when running the finished executables.
