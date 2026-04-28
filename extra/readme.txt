WK Universal Unofficial Patch
Version 0.91
By AdrienTD



This is an unofficial patch for Warrior Kings 1.4 (Build 366) and
Warrior Kings - Battles v1.23 (Build 152) that fixes certain bugs and adds
new features.

This patch contains:

 - Higher time precision, fixing the famous unit teleportation bug.

 - (WKO*) Bug where sight range events are sometimes not sent is now fixed.
    -> Fixes watchtowers/forts sometimes not firing/alerting automatically.
    -> Fixes bug where the barbarians won't come to you in the second celestial
       level (the Rescue the Train mission).
    -> Fixes lots of other bugs related to sight range.

 - Modding abilities, including:

    - You can use the "data" directory (placed in the same directory as the
      EXE) to add AND replace game files. No need to create new BCP files.

    - (WKO*) You can put multiple "mod"/"patch" BCP files in the same directory
      as the EXE, which then get loaded by the patch. Mods made for WK Battles
      CAN also work with the first WK (but this is not always the case!).
      This is a backport of the feature available in WK Battles 1.1/1.23.

    - (WKO*) Selecting a custom campaign (by placing a CAM file inside the
      "Campaigns" directory in data(.bcp)/saved) won't crash anymore.

    - NEW! You can add maps in the multiplayer/skirmish map list.

    - (WKB²) Map editor enabled, with some bug fixes.

Optional features:
 - Enable the TRACE and/or TRACE_VALUE actions.
 - (WKO*) Full memory with 0 after allocation.

* only for the first Warrior Kings
² only for Warrior Kings - Battles

It is a small d3d8.DLL file that, once loaded by the game, will patch the
game's code in the RAM memory before the game gets launched. It redirects
Direct3D calls to either "apd3d8.dll" if found in the executable directory,
or the system's real d3d8.dll.



+--------------------------+
|    Multiplayer notice    |
+--------------------------+

  The patch can be used to play multiplayer games.

  Persons using this patch may be able to play with others who don't have it.
However this is not recommended because this could cause some problems.

  If you aren't able to join multiplayer games, removing temporarily the patch
is a potential fix.

  If you want to play multiplayer games with mods and/or custom maps, then
everyone must have the mod files installed. Also, this patch must also
be installed by everyone if the mod(s) require it.



+--------------------+
|    Installation    |
+--------------------+

Definition:
  The "game's directory" is where there is the executable (exe) of the game.

To install the patch:

 1. If there is already a file named "d3d8.dll" in the game's directory
    (for example you are using the ENB series/converters, that you might have
    needed to solve graphical problems), rename it to "apd3d8.dll".

 2. Copy the "d3d8.dll" from the patch archive into the game's directory.

To verify if the installation went well, run the game. At the main menu, look
at the version numbers in the right-bottom corner of the screen. If the first
letter is an 'a', then that means that the patch is working. In Battles you
might also notice the Map Editor button under the Exit button.

To uninstall the patch:

 1. Remove (or rename) "d3d8.dll" from the game's directory.

 2. If there is "apd3d8.dll", rename it back to "d3d8.dll".



+----------------+
|    Settings    |
+----------------+

You can also specify which features/fixes that the patch provides should be
enabled or disabled.

To do this, create a new file named "wkuup_settings.txt" in the same directory
as the game's executable.

For every line you can specify the name of the setting you want to set and then
a number: 0 to disable or 1 to enable.

For example, to enable tracing:
enable_trace_action 1

When a setting is not specified in the file, it will take a default value.

Here is a list of available settings and their default values:

higher_time_precision           1
custom_campaign_crash_fix       1
use_data_directory              1
use_multi_bcp                   1
sight_range_events_bugfix       1
setting_custom_multiplayer_maps 1
no_tutorial_in_skirmish         1
map_editor_button               1
map_editor_hacks                1
enable_trace_action             0
enable_trace_value_action       0
trace_filter                    0
zero_allocated_memory           0



+---------------+
|    Modding    |
+---------------+

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



  The first WK allows you to add a new campaign by adding a .CAM file in the
"Campaigns" directory that you can find in "data\", "data.bcp", "saved", ...

  But selecting a new campaign will crash the game because the game tries to
get the localised name of the new campaign that doesn't have one. The patch
fixes this issue.

  A .CAM file is a simple text file. Take a look at the ones already included
with the game to know how to write them.



  The patch even allows you to put new maps to select in the skirmish or
multiplayer screen.

To do this, your level savegame must follow these requirements:

 - The filename must end with .lvl
 - The file must be inside Save_Games\
 - (For the first WK only,) GAME_SET must be set to:
     "Warrior Kings Game Set\multi-player extensions.cpp"

The game determinates the number of players required by looking at
NUM_HUMAN_PLAYERS at the beginning of the savegame.



  The patch adds back the TRACE and TRACE_VALUE actions, but this feature is
disabled by default. You can enable them in the settings file (see Settings
section of this document). The tracing will be printed as in-game messages.

  If you enable the "trace_filter" setting, then only trace messages that
begin with the @ character will be printed, thus disabling the already included
trace actions that you might not want.



+------------------+
|    Map Editor    |
+------------------+

  The patch also enables the map editor, that you can access from the main
menu via the "Map Editor" button.

  But first, be sure that the "data" directory (in the same directory as the
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



----------------------------------------------------------------

If you have problems or questions, you can contact me.
Email: /
Steam: /

Enjoy! :D
AdrienTD