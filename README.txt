S-NAV SIMULATOR 0.1 - by Inward 3D

START HERE
Extract this entire ZIP, then double-click S-NAV.exe on a 64-bit Windows PC.
No installation, Python, game installation, internet, or sound is needed.
The executable contains the supplied device artwork and bitmap font.
Default: Ultimate, LCD-only view, source-style green screen/red rings.
Click the screen after editing the seed to return keyboard control to it.

CONTROLS
Left/right arrows  Turn smoothly
Up/down arrows     Walk forward/backward in the direction you face
Shift              Faster movement
Space              Blink; lets an observed 173 move
Enter              Provoke simulated 096; eight-second delay before pursuit
Tab                Pause/resume
D                  Device artwork / LCD only
M                  Monochrome / game-style red radar rings
P                  Power on/off
B                  Fresh battery
R                  Restart the current seed
N                  Generate a new random seed
F11                Full screen
Esc                Leave full screen
F1                 Help
The device artwork's D-pad is clickable/holdable; OK provokes 096,
minus pauses, power toggles power, and the return button restarts.
These button assignments belong to this simulator, not the original game.

SEEDS AND MAPS - IMPORTANT
Any typed string generates a repeatable simulator map, including numeric
strings. Numeric text is treated as a string in this build. The seed-to-number
XOR/shift conversion comes from GenerateSeedNumber in your source.
The base hallway algorithm is adapted from CreateMapLayout (18 x 18
configured map extent, 19 x 19 occupied-grid storage).
It uses a different PRNG, adds connectivity safeguards and approximate
extra dead ends, and does not port ForceRooms, room templates, overlap
correction, door permissions, events, or the complete Blitz3D random stream.
Therefore the SAME seed does NOT currently give the SAME SCP-CB map.
The simulator is a room-footprint exploration, not collision against detailed
3D room meshes. Adjacent occupied rooms are traversable, without door logic.

For an exact room footprint from an actual game session:
1. Add game_map_export.bb's function to the game's source.
2. Add ExportSNavGrid("facility.snavmap") after ConnectAdjacentRooms() at
   the end of CreateMap() in MapSystem.bb, and rebuild/run that game.
3. In this simulator, choose Simulation > Import room grid and select the file.
This uses the game's final occupied grid; movement and NPCs remain simulated.
The exporter is supplied as a snippet and has not been executed in Blitz3D.
Maps must contain one connected component. A disconnected export is rejected,
as this prototype has no teleport/elevator navigation between components.
The importer requires a header SNAV-MAP-1 followed by 19 rows of 19 #/.
characters. # means occupied, . means empty. Export is available in the menu.
Imported grids are not retained by Restart; restart regenerates the seed map.

NAVIGATOR MODELS
S-NAV (damaged): discovered rooms, red player arrow, offline database notice.
S-NAV 300: discovered rooms, offline database notice, battery.
S-NAV 310: full nearby room map, battery, no SCP radar.
Ultimate: full nearby room map and SCP proximity rings, unlimited battery.
All variants show a local scrolling map, not the entire facility at once.
Battery models drain at 0.35 percent/second, matching FPSfactor * 0.005
with the source game's 70 FPS timing basis. B replenishes it.

NPC SIMULATION
173 freezes within the simulated player's viewing cone and unblocked room
line-of-sight. It approaches when unobserved or blinking. Automatic blinks
occur every 4.5 seconds. This models visibility without a 3D camera.
106 slowly approaches across walls.
096 stays stationary until you press Enter; then waits eight seconds and
pursues through connected rooms. Face visibility is NOT detected in this app.
049 wanders, then pursues when close.
A designated dead end represents 895 and causes local map interference.
Contacts/encounters reset NPC placement, so you can keep exploring without
a death screen. These are intentionally simplified behaviors, not full AI ports.
NPC movement and radar distances respond to actual simulated positions.

SOURCE DISPLAY RULES IMPLEMENTED
Main.bb 6961-7174 contains the renderer.
24 pixels per room, 8 world units per room => 3 pixels per world unit.
Room borders are drawn only against empty neighbouring grid cells.
Map stays north-up; player heading changes, map scrolls with movement.
Player arrow: 6 px nose / 5 px rear vertices, +/- 140 degrees.
Player arrow/database blink: visible 700 ms in each 1-second cycle.
Ultimate SCP labels/rings: visible 400 ms in each 600-ms cycle.
173: ceil(distance / 8) * 8; show only rounded distance < 32;
     ring radius = rounded distance * 3 pixels.
106, 096, 049: show raw distance < 32; radius = distance * 1.5 pixels.
All rings are centred on the player, 7 pixels above the arrow centre.
They do NOT indicate the bearing or exact screen position of an SCP.
895: proximity interference is approximated; it is not the original random
coffin rendering routine. Off-world location error screens are not implemented.
049's HideFromNVG flag is not simulated. Vertical distance is not simulated.
The game source uses EntityDistance, so height can affect a game's contacts.

SOURCE AND BUILD
src/snav.c      Portable simulation and map data, no Windows dependencies
src/render.c    Portable software raster renderer
src/win32.c     Desktop Windows controls, window and bitmap presentation
src/assets.h    Embedded supplied image / rasterized DS-Digital glyphs
src/snav.h      Shared API
Build-windows.bat builds with MinGW-w64 GCC; all artwork is already embedded.
An asset-generation script is included for development, but it expects your
original uploads/reference tree. It is not needed to rebuild the executable.
Run build-linux-tests.sh for the portable-core checks. See VERIFICATION.txt.

RECON AND HARDWARE
Read HARDWARE-ROADMAP.md. This executable is x86-64 DESKTOP WINDOWS.
It will not run on ARM Windows CE/Windows Mobile.
A Recon build needs the exact device model, CPU and OS version plus an
appropriate CE SDK/compiler. No tested CE binary is included.
