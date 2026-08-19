WK Universal Unofficial Patch
Version 0.95
By AdrienTD
https://github.com/AdrienTD/WK-UniversalUnofficialPatch


This is an unofficial patch for Warrior Kings 1.4 (Build 366) and
Warrior Kings - Battles v1.23 (Build 152) that fixes certain bugs and adds
new features.

It is a small "d3d8.dll" file that patches the game's code in memory right when
the game launches.



+--------------------+
|    Installation    |
+--------------------+

   /!\ Please do NOT copy the patch DLL to system32.

Definition:
  The "game's directory" is where the folder/path where the game is installed,
  where the game executable is (Warrior_Kings.exe).
  Examples where the game could be found:
    C:\Program Files (x86)\Microids\Warrior Kings
    C:\Program Files (x86)\Empire Interactive\Warrior Kings Battles
    C:\Program Files (x86)\Steam\steamapps\common\Warrior Kings (Battles)
    C:\Program Files (x86)\GOG Galaxy\Games\Warrior Kings (Battles)

To install the patch:

 1. If there is already a file named "d3d8.dll" in the game's directory
    (for example you are using the ENB series or D3D8to9 converters, that you
    might have needed to solve graphical problems), rename it to "apd3d8.dll".

 2. Copy the "d3d8.dll" from the patch archive into the game's directory.
 
 If you use Linux/Wine:
 3. Run `winecfg`, go to the Libraries tab, add a DLL override to d3d8,
    and set it to "Native then builtin", if this hasn't been done before.
    For more info, see:
    https://gitlab.winehq.org/wine/wine/-/wikis/Wine-User's-Guide#configuring-wine

To verify if the installation went well, run the game. At the main menu, look
at the version numbers in the right-bottom corner of the screen. If you see
the letters "UUP" colored in green, then the patch is working.


+---------------------------+
|    Uninstall the patch    |
+---------------------------+

To uninstall the patch:

 1. Remove (or rename) "d3d8.dll" from the game's directory.

 2. If there is "apd3d8.dll", rename it back to "d3d8.dll".



+---------------+
|    Content    |
+---------------+

This patch contains:

 - Music bug fixes, including:

    - (WKB²) Fix game freezing for seconds every time the music changes.
      This happened when a MP2 codec other than the one provided by Microsoft
      was being used.

    - Improve game framerate when music is on.
      In the first WK, the mission introduction cinematic with the narrator now
      becomes significantly smoother, going from 16 FPS to 60 FPS!

 - (WKO*) Bug where sight range events were not working properly is now fixed.
    -> Fixes watchtowers/forts sometimes not firing/alerting automatically.
    -> Fixes bug where the barbarians won't come to you in the Rescue Tomas
       mission.
    -> Fixes lots of other bugs related to sight range.

 - Higher time precision, fixing the famous unit teleportation bug.
   [NOTE: this may still not be fully fixed, especially on fast systems]

 - (WKB²) Widescreen resolutions are now available in the graphics
   options screen.

 - Modding abilities, including:

    - You can use the "data" directory (placed in the same directory as the
      EXE) to add AND replace game files. No need to create new BCP files.

    - (WKO*) You can put multiple "mod"/"patch" BCP files in the same directory
      as the EXE, which then get loaded by the patch. Mods made for WK Battles
      CAN also work with the first WK (but this is not always the case!).
      This is a backport of the feature available in WK Battles 1.1/1.23.

    - (WKO*) Selecting a custom campaign (by placing a CAM file inside the
      "Campaigns" directory in data(.bcp)/saved) won't crash anymore.

    - You can add maps in the multiplayer/skirmish map list.

    - (WKB²) The unused and unfinished in-game Map editor can be enabled,
      with some bug fixes.

    - Enable the TRACE and/or TRACE_VALUE actions.

* only for the first Warrior Kings
² only for Warrior Kings - Battles



+---------------------+
|    Release notes    |
+---------------------+

## v0.95

* WKB Fix crash when displaying text on 4K/ultrawide resolutions
  The game was crashing when displaying the AI General bio with a 4K resolution,
  due to too many characters in a single line of text.

* WK1+WKB Fix game window becoming "unresponsive"
  Fixes an issue where Windows sometimes marks the game's window as unresponsive,
  which only happened when the music was on. (Always the music causing problems...)

* WKB Fix incorrect initial resolution in windowed mode
  When launching the game in windowed mode, the window size did not match the resolution,
  resulting in blurry text and graphics. Now fixed.

* WKB v1.1 data check
  On game launch, the patch will check that v1.1 game data is present
  (the "patch_1_1.bcp" file). If not, it will be reported as a warning message.

* WKB Apply fog+sky colors in BCM maps
  With custom maps, the fog and sky colors set in the map were only applied
  when the terrain was saved in the "SNR" format, but not the newer "BCM"
  format.
  Now with the patch, the colors will be applied on BCM maps as well.

* WKB Allow multiple instances
  When lauching the game, it checks if a previous process already exists, so
  it does not launch a second instance.
  The patch allows to bypass this, by adding this line to "wkuup_settings.txt":
  
  allow_multiple_instances 1

* Improvements to the Message boxes from the Patch.
  There is now a "Help" button which leads to the UUP Wiki page.


## v0.94

* Fix patch compatibility with Wine.
  Previously, when using the patch on Wine, the game would crash at launch.

* WKB Fix music freeze issue that was still present on Wine (as there is no
  Microsoft MPEG codec).
  In theory this should also fix the issue on Windows when using a MPEG codec
  other than Microsoft's one.

* The WKB Map editor button is now disabled by default.
  It is highly recommended to use the alternative map editor "wkbre" instead.
  See https://github.com/AdrienTD/wkbre
  Or its fork https://github.com/kbhau/wkbre
  If you still need it, enable it back in "wkuup_settings.txt" with:

  map_editor_button 1

* The version of the patch is now displayed in the main menu, below the
  game's version, in green.

* WKB Fix a bug that renders text of message boxes two times. This can help
  reduce lags in the Campaign province descriptions a little bit.

* WKB UI Performance improvements (experimental)
  The patch contains some rewritten parts of UI rendering code to improve the
  framerate.
  This is especially noticeable in places where there is too much text, like
  the AI general description, Campaign province description, and the Load Game
  menu.
  These improvements are disabled by default (due to bugs with some drivers),
  and must be enabled manually through "wkuup_settings.txt" by adding this line:

  ui_performance_improvements 1


## v0.92

Adds two fixes related to music:

* The first is forcing default MS MPEG codecs in Windows as these codecs are
  known to cause no problems. But if you experience problems and error messages,
  you can desactivate this fix by creating a file named "wkuup_settings.txt"
  in the game's directory and writing the following line:

  dshow_force_ms_mpeg_codecs 0

* The second is removing a useless wait when the game checks if the music has
  ended. The result is that the game gets higher FPS but also higher CPU usage.
  If you don't want a high CPU usage, then you can desactivate the fix by
  creating a file named "wkuup_settings.txt" in the game's directory and
  writing the following line:

  dshow_waitforcompletion_immediate 0



+--------------------------+
|    Multiplayer notice    |
+--------------------------+

  The patch can be used to play multiplayer games.

  Players using this patch may be able to play with others who don't have it.
However this is not recommended because this could cause some problems.

  If you aren't able to join multiplayer games, removing temporarily the patch
is a potential fix.

  If you want to play multiplayer games with mods and/or custom maps, then
everyone must have the mod files installed. Also, this patch must also
be installed by everyone if the mod(s) require it.



+----------------+
|    Settings    |
+----------------+

You can also specify which features/fixes that the patch provides should be
enabled or disabled.

To do this, create a new file named "wkuup_settings.txt" in the same directory
as the game's executable.

For each line, you type the name of the setting you want to set, followed
by a number: 0 to disable or 1 to enable.

For example, to enable tracing:
enable_trace_action 1

When a setting is not specified in the file, it will take a default value.

Here is a list of available settings and their default values:

allow_multiple_instances             0
apply_bcm_sky_texture_and_fog_color  1
custom_campaign_crash_fix            1
custom_multiplayer_maps              1
dshow_force_ms_mpeg_codecs           1
dshow_no_bitrate_limit               1
dshow_no_default_syncsrc             0
dshow_unresponsive_window_fix        1
dshow_waitforcompletion_immediate    1
enable_trace_action                  0
enable_trace_value_action            0
higher_time_precision                1
map_editor_button                    0
map_editor_hacks                     1
no_tutorial_in_skirmish              1
show_all_screen_resolutions          1
sight_range_events_bugfix            1
trace_filter                         0
ui_messagebox_double_text_render_fix 1
ui_performance_improvements          0
use_data_directory                   1
use_multi_bcp                        1
zero_allocated_memory                0



+---------------+
|    Modding    |
+---------------+

[---- Data directory ----]

  The patch enables the "data" directory, which allows you to add and replace
files without having to create BCP files.
To do this, create the "data" directory in the same directory as the game
executable.

  Now putting game files can be done by putting them in the data directory,like
if you put them in data.bcp. data.bcp and data directory are actually "merged".
If there is a file with the same name in both data.bcp and the data directory,
then the file with the most recent modification date and time will be opened.
This is, however, not the case with the "saved" directory, because data.bcp
gets the priority over it.



[---- Campaigns ----]

  The first WK allows you to add a new campaign by adding a .CAM file in the
"Campaigns" directory that you can find in "data\", "data.bcp", "saved", ...

  But selecting a new campaign will crash the game because the game tries to
get the localised name of the new campaign that doesn't have one. The patch
fixes this issue.

  A .CAM file is a simple text file. Take a look at the ones already included
with the game to know how to write them.



[---- Levels ----]

  The patch even allows you to put new maps to select in the skirmish or
multiplayer screen.

To do this, your level savegame must follow these requirements:

 - The filename must end with .lvl
 - The file must be inside Save_Games\
 - (For the first WK only,) GAME_SET must be set to:
     "Warrior Kings Game Set\multi-player extensions.cpp"

The game determines the number of players required by looking at
NUM_HUMAN_PLAYERS at the beginning of the savegame.



[---- Trace actions ----]

  The patch adds back the TRACE and TRACE_VALUE actions, but this feature is
disabled by default. You can enable them in the settings file (see Settings
section of this document). The tracing will be printed as in-game messages.

  If you enable the "trace_filter" setting, then only trace messages that
begin with the @ character will be printed, thus disabling the trace actions
that the developers left in the original gameset, that you might not want.



+----------------------------+
|    WKBattles Map Editor    |
+----------------------------+

  The patch also enables an unfinished map editor that was left in WKB,
that you can access from the main menu via the "Map Editor" button, but
this button is hidden by default.

  Note that this editor is incomplete: it can only edit terrains, it is not
possible to add units and objects. There are also a lot of bugs. The developers
were not able to finish it for public use.

  If you want to make your own maps, please check alternative tools.
"wkbre" is a map editor that allows you to edit terrain and place objects at
the same time, and is much more usable. See https://github.com/AdrienTD/wkbre
Or check this fork with additional features and improvements:
https://github.com/kbhau/wkbre

  To make the "Map Editor" button appear in the main menu, you have to enable it
in the patch's settings file (see Settings section of this document).

  Then, be sure that the "data" directory (in the same directory as the
game EXE) is created before using the map editor. It is used to load and save
map files.

  If you run the map editor for the first time, you might notice certain
error messages of type:

                  +-------------------------------------+---+
                  | EDITOR DECAL GROUP ERROR            | X |
                  +-------------------------------------+---+
                  | (X)   failed to open file "editor\..."  |
                  |                             <   OK   >  |
                  +-----------------------------------------+

  This is because the map editor looks for certain textures that it needs
to draw markings, areas you are currently selecting, ...

  Unfortunately these textures are not included with WK - Battles.

  The map editor can work without them, but then the editor would be
difficult to use.

  But fortunately, these textures are included with the first WK, so all you
have to do is to extract the "editor" directory from the data.bcp file of the
first WK, then copy the directory in the "data" directory (or "saved"
directory) of the last WK (Battles).



  It is recommended to run the editor in windowed mode, because in the editor
you will have to open certain windows that might not be visible if you're
in fullscreen. This can be done using the registry editor.

   /!\ Be careful when using the registry editor. Doing random things in it
       can harm your computer! Do this only if you know what you are doing.

To run the game in a window, go to the registry key:
   (HKEY_LOCAL_MACHINE)\SOFTWARE\(Wow6432Node)\Black Cactus\       |
   Warrior Kings Battles\Renderer                               <--/
then you add a new value of DWORD type, name it "fullscreen" and set its value
to 0. You can enable fullscreen back by setting it to 1.



+-------------------------+
|    Sample custom map    |
+-------------------------+

  A custom map (called "Dioxygen") is included with this patch to show the
capacities of the patch in modding. Note that it requires this patch to work!

  To install this map, copy the "Dioxygen Map For WK?.bcp" file into the same
directory as the game executable. Use the variant for the version of the game
you are using ("WKO" for the first WK, "WKB" for WK - Battles).

  Now you should be able to find the map in the multiplayer/skirmish map list.

