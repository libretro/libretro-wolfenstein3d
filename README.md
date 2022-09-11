## SDL4Wolf libretro WIP port by Wolf3s
SDL4Wolf is an open-source port/core of id Software's classic first-person shooter
Wolfenstein 3D. based on The [Wolf4SDL](https://github.com/Doom-modding-and-etc/Wolf4SDL) which the purpose is convert/remove all the SDL Functions to the vanilla Wolfenstein 3d
It is meant to try to keep the original feel while taking advantage of some improvements mentioned in the list below.

Main features:
--------------
 - Cross-platform:
      Supported operating systems are at least:
       - Windows 98, Windows ME, Windows 2000, Windows XP, Windows Vista
         (32 and 64 bit), Windows 7 (32 and 64 bit), Windows 10 (32 and 64 bit)
       - Linux
       - BSD variants
       - Mac OS X (x86) Only little endian platforms like x86, ARM and SH-4 are currently supported.
	   
 - AdLib sounds and music:
      This port includes the OPL2 emulator from MAME, so you can not only
      hear the AdLib sounds but also music without any AdLib-compatible
      soundcard in near to perfect quality!

 - Multichannel digitized sounds:
      Digitized sounds play on 8 channels! So in a fire fight you will
      always hear, when a guard opens the door behind you ;)

 - Higher screen resolutions:
      Aside from the original 320x200 resolution, SDL4Wolf currently
      supports any resolutions being multiples of 320x200 or 320x240,
      the default being 640x400.
      Unlike some other ports, SDL4Wolf does NOT apply any bilinear
      or similar filtering, so the graphics are NOT blurred but
      pixelated just as we love it.

 - Fully playable with only a game controller:
      SDL4Wolf can be played completely without a keyboard. At least two
      buttons are required (shoot/YES and open door/NO), but five or more
      are recommended (run, strafe, ESC).
	  
Additional features:
--------------------
 - Two additional view sizes:
      SDL4Wolf supports one view size using the full width of the screen
      and showing the status bar, like in Mac-enstein, and one view size
      filling the whole screen (press TAB to see the status bar).

 - (Nearly) unlimited sound and song lengths:
      Mod developers are not restricted to 64kB for digitized sounds and
      IMF songs anymore, so longer songs and digitized sounds with better
      quality are possible.

 - Resuming ingame music:
      When you come back to the game from the menu or load a save game, the
      music will be resumed where it was suspended rather than started from
      the beginning.

 - Freely movable pushwalls:
      Moving pushwalls can be viewed from all sides, allowing mod developers
      to place them with fewer restrictions. The player can also follow the
      pushwall directly instead of having to wait until the pushwall has left
      a whole tile.

 - Optional integrated features for mod developers:
      SDL4Wolf already contains the shading, directional 3D sprites,
      floor and ceiling textures, high resolution textures/sprites,
      parallax sky, cloud sky and outside atmosphere features, which
      can be easily activated in version.h.
	  
 - Toggle Screen Feature:
	  SDL4Wolf can toggle into fullscreen if you press ALT+ENTER

The following versions of Wolfenstein 3D data files are currently supported
by the source code (choose the version by commenting/uncommenting lines in
version.h as described in that file):

 - Wolfenstein 3D v1.1 full Apogee
 - Wolfenstein 3D v1.4 full Apogee
 - Wolfenstein 3D v1.4 full GT/ID/Activision
 - Wolfenstein 3D v1.4 full Imagineer (Japanese)
 - Wolfenstein 3D v1.0 shareware Apogee
 - Wolfenstein 3D v1.1 shareware Apogee
 - Wolfenstein 3D v1.2 shareware Apogee
 - Wolfenstein 3D v1.4 shareware
 - Spear of Destiny full
 - Spear of Destiny demo
 - Spear of Destiny - Mission 2: Return to Danger (not tested)
 - Spear of Destiny - Mission 3: Ultimate Challenge (not tested)

How to play:
------------
To play Wolfenstein 3D with SDL4Wolf, you just have to copy the original data
files (e.g. ``.WL6``) into the same directory as the SDL4Wolf executable.
Please make sure, that you use the correct version of the executable with the
according data files version as the differences are hardcoded into the binary!

For now:
SDL1.2:
On Windows SDL.dll and SDL_mixer.dll must also be copied into this directory.
SDL 1.2.15 Runtimes can be acquired at: 
https://github.com/wolfysdl/SDL-Archive/raw/main/SDL-1.2.15-win32.zip and download "SDL-1.2.15-win32.zip"

SDL_mixer 1.2.12 Runtimes can be acquired at:
https://github.com/wolfysdl/SDL_mixer-archive/raw/main/SDL_mixer-1.2.12-win32.zip and download "SDL_mixer-1.2.12-win32.zip"

or

SDL2:
On Windows SDL2.dll and SDL2_mixer.dll must also be copied into this directory.

SDL2 2.0.24 Runtimes can be acquired at:
https://github.com/wolfysdl/SDL-Archive/raw/main/SDL2-2.24.0-win32-x86.zip and download "SDL_mixer-2.0.24-win32.zip"

SDL_mixer 2.0.4 Runtimes can be acquired at:
https://github.com/wolfysdl/SDL_mixer-archive/raw/main/SDL2_mixer-2.6.2-win32-x86.zip and download "SDL_mixer-2.0.4-win32.zip"

## deprecated links:

[SDL1.2](https://www.libsdl.org/download-1.2.php) 
[SDL_mixer](https://www.libsdl.org/projects/SDL_mixer/release-1.2.html)

[SDL2](https://www.libsdl.org/download-2.0.php) 
[SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)

! ATTENTION ! Do NOT download the x64 version of SDL or SDL_mixer. x86 binairies are required.

If you play in windowed mode (--windowed parameter), press SCROLLLOCK or F12
to grab the mouse. Press it again to release the mouse.
Paramenter usage:
------
SDL4Wolf supports the following command line options:
 --help                 This help page
 --tedlevel <level>     Starts the game in the given level
 --baby                 Sets the difficulty to baby for tedlevel
 --easy                 Sets the difficulty to easy for tedlevel
 --normal               Sets the difficulty to normal for tedlevel
 --hard                 Sets the difficulty to hard for tedlevel
 --nowait               Skips intro screens
 --windowed[-mouse]     Starts the game in a window [and grabs mouse]
 --res <width> <height> Sets the screen resolution
                        (must be multiple of 320x200 or 320x240)
 --resf <w> <h>         Sets any screen resolution >= 320x200
                        (which may result in graphic errors)
 --bits <b>             Sets the screen color depth
                        (Use this when you have palette/fading problem
                        or perhaps to optimize speed on old systems.
                        Allowed: 8, 16, 24, 32, default: "best" depth)
 --nodblbuf             Don't use SDL's double buffering
 --extravbls <vbls>     Sets a delay after each frame, which may help to
                        reduce flickering (SDL does not support vsync...)
                        (unit is currently 8 ms, default: 0)
 --joystick <index>     Use the index-th joystick if available
 --joystickhat <index>  Enables movement with the given coolie hat
 --samplerate <rate>    Sets the sound sample rate (given in Hz)
 --audiobuffer <size>   Sets the size of the audio buffer (-> sound latency)
                        (given in bytes)
 --ignorenumchunks      Ignores the number of chunks in VGAHEAD.*
                        (may be useful for some broken mods)
 --configdir <dir>      Directory where config file and save games are stored
                        (Windows default: current directory,
                        others: $HOME/.wolf4sdl)

For Spear of Destiny the following additional options are available:
 --mission <mission>    Mission number to play (1-3)
 --goodtimes            Disable copy protection quiz


Compiling from source code:
---------------------------

The current version of the core source code is available on [GitHub](https://github.com/libretro/libretro-wolfenstein3d)

The following ways of compiling the source code are supported:
 - Makefile.libretro
 
To compile the source code you need(For now) the development libraries of
[SDL](https://github.com/wolfysdl/SDL-Archive) or [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.24.0) and
[SDL_mixer](https://github.com/wolfysdl/SDL_mixer-archive) or [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.6.2)

Warning: pick the devel version

Deprecated links:
 - [SDL](https://www.libsdl.org/download-1.2.php) and
 - [SDL_mixer](https://www.libsdl.org/projects/SDL_mixer/release-1.2.html)
 or
 - [SDL2](https://www.libsdl.org/) and
 - [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)

and you have to adjust the include and library paths in the projects accordingly.

IMPORTANT: Do not forget to take care of version.h!
   By default it compiles for "Wolfenstein 3D v1.4 full GT/ID/Activision"!

TODO´s:
------
 - Add support for any graphic resolution >= 320x200(40%)
 - Add latest fmopl sound
 - Fix some versions of Wolfenstein 3d.
 - Convert all the SDL Functions to not depends the SDL1.2/SDL2 Library
 - Remove all the features included on Wolf4SDL

Known bugs:
-----------
None for now :D

Troubleshooting:
----------------
 - If your frame rate is low, consider using the original screen resolution
   (--res 320 200) or lowering the sound quality (--samplerate 22050)

Credits:
--------
 - Thanks to Moritz "Ripper" Kroll For developing Wolf4SDL
 - Special thanks to id Software! Without the source code we would still have
   to pelt Wolfenstein 3D with hex editors and disassemblers ;D
 - Special thanks to the DOSBox team for providing a GPL'ed OPL2/3 emulator!
 - Special thanks to the MAME developer team for providing the source code
   of the OPL2 emulator!
 - Speacial thanks to Aryan for upgrade this source port
 - Many thanks to "Der Tron" for hosting the svn repository, making Wolf4SDL
   FreeBSD compatible, testing, bugfixing and cleaning up the code!
 - Thanks to Chris Chokan for his improvements on Wolf4GW (base of Wolf4SDL)
 - Thanks to Pickle for the GP2X support and help on 320x240 support
 - Thanks to fackue for the Dreamcast support
 - Thanks to Chris Ballinger for the Mac OS X support
 - Thanks to Xilinx, Inc. for providing a list of maximum-length LFSR counters
   used for higher resolutions of fizzle fade
 - Thanks to Terraformer9x and Nexion for the advice and help for the Mixer sound problem
 - Thanks to Fabian greffrath for the patches on the code 
 - Thanks to keeganatorr for the Nintendo Switch support
 - Thanks to Fabian Sangalard for the awesome crt feature used on Chocolate Wolfenstein 3d. 
 - Thanks to Andy_Nonymous and others  for the Modifications on the r262 version 
 - Thanks to Myself for mantaining Wolf4SDL 
 - Special Thanks to zZeck for support SDL2!!!
   
Licenses:
---------
 - The original source code of Wolfenstein 3D:
     At your choice:
     - license-id.txt or
     - license-gpl.txt
 - The OPL2 emulator:
     - license-mame.txt (fmopl.c)
