# A physical S-NAV

The useful sequence is laptop simulator, Recon proof of concept, display/button
breadboard, measured enclosure, then a custom PCB. Freeze the enclosure and PCB
only after the display's actual viewing area, module outline, controller,
interface, voltage and mounting holes have been checked.

## 1. Laptop prototype

The included executable is a small native Windows application, using a portable
C simulation and software-drawn display. It can run without the game. A future
exact generator would replace the simulator PRNG/layout stage with verified
Blitz3D RNG and every map postprocessing stage, then compare final grids for
known seeds against game exports. The included grid exporter/importer provides
a practical bridge for testing actual maps before that port is complete.

## 2. Your Recon

Find its exact model and a photo/transcription of Settings > System > About
(OS/build, processor and memory). CE and Windows Mobile versions differ, and
an ARM desktop Windows build is not interchangeable with ARM Windows CE.

Keep snav.c and the display rules; replace win32.c with a small Unicode CE window
front end compiled with the matching legacy SDK. Remove desktop menus/file
pickers/full-screen window assumptions. Bind its actual D-pad/buttons and use
a bitmap presentation path supported by that CE version. This front end uses
ANSI calls and StretchDIBits and is explicitly not a ready-to-compile CE app.
The portable core's C99 integer types, snprintf, hypot, fmin/fmax, and mixed
declarations may also need compatibility wrappers for older CE compilers.

For a commonly encountered 240 x 320 Recon screen, render a scaled LCD area
with a simple seed/settings page below it. Keep the original green screen
texture as a mode; do not expect the whole S-NAV artwork to fit at full scale.
CPU/OS and resolution must be verified on your own unit.

Microsoft describes CE as a Win32 subset with a separate embedded toolchain:
https://learn.microsoft.com/en-us/archive/msdn-magazine/2001/january/windows-ce-embedded-visual-tools-3-0-provide-a-flexible-and-robust-development-environment
A manufacturer Recon datasheet (not proof of your particular revision):
https://www.barcodesinc.com/media/pdf/Trimble/recon.pdf

## 3. Decide which accuracy matters for the screen

A calculator-like display is possible, but use a **pixel-addressable graphic
LCD**. A segmented calculator LCD has fixed digit shapes and cannot display
arbitrary room outlines, rotating arrows and circles.

Positive-mode reflective or transflective STN/FSTN graphic LCDs give the
calculator-style appearance. Compare gray and yellow-green variants in person:
the supplied texture is a muted gray-green, not vivid neon green. Manufacturer
reference examples:
https://support.newhavendisplay.com/hc/en-us/articles/4415459819927-LCD-Backlights-STN-and-FSTN
https://newhavendisplay.com/lcd-displays/

A true monochrome panel can display dark rings and dark text but cannot show
the source's red warnings/rings. The prototype's M key demonstrates that
compromise. For the game's green background AND red contacts, a color TFT
rendered with the green texture is the straightforward choice. A green overlay
can alter red visibility; software tinting should be tested before physical film.

The asset's viewing window is nearly square and its reference render is roughly
287 x 264 logical pixels. Typical catalog graphic panels are different aspect
ratios. A 320 x 240 panel may be usable with layout changes or a masked window,
but is not automatically a 1:1 match. Measure the intended device/window in mm.
Only then choose a module. Truly exact glass dimensions may require a custom
LCD module; get quotations before committing to that approach.

## 4. Breadboard before a custom board

For the eventual independent device, a modern microcontroller with sufficient
RAM and a supported display interface is appropriate for this 2D simulation.
An ESP32-S3-class board with PSRAM is a candidate to evaluate after choosing the
screen; it is not a sourced/verified bill of materials yet. A small Linux SBC
would simplify a rich color display but adds OS, boot-time and power overhead.

Use the simulation core without the artwork. Drive physical D-pad and auxiliary
buttons into debounced inputs. Separate the simulation tick from LCD refresh:
an STN panel's optical response is slower than a laptop and may need 10-20 Hz
refresh, while simulation remains 60 Hz. Test readability and ghosting on the
actual module rather than assuming identical motion.

The current 32-bit LCD framebuffer is about 303 KB, plus skin assets/output.
For a mono target, replace this with a packed 1-bit framebuffer or scanline
renderer: a 320 x 240 framebuffer then takes 9,600 bytes. Do not transfer the
whole desktop artwork/frame allocation into a small MCU build.

Prototype the display, buttons, contrast/backlight control, regulator, battery
charger/protection and power switch with modules. Measure current consumption
before selecting a cell or designing charging/power routing. Then create a
custom PCB around the validated components, connectors and enclosure mounting.

## 5. Mechanical 1:1 reconstruction

Use your own Recon and the supplied navigator.x model as references, not an
assumption that their proportions are identical. Reconstruct the S-NAV shell,
bezel, speaker grilles, button shapes and labels in Blender/CAD. A 3D-printed
fit-test shell can verify the screen viewing window, thumb access, wall thickness,
battery space and assembly before final finishing. The source texture alone
does not establish a physical scale.

## What the device would know

This software navigates a fictional facility and fictional NPCs. Physical button
inputs can explore that map. An IMU alone will not map a real building or supply
reliable indoor position, and GPS does not reliably locate rooms indoors. Real
indoor navigation would be a separate mapping/tracking project. It is unnecessary
for a faithful self-contained S-NAV prop simulator.
