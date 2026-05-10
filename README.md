# Warrior Kings Universal Unofficial Patch (UUP)

Patch for both Warrior Kings, and Warrior Kings - Battles, games developed by Black Cactus.

[Download](https://github.com/AdrienTD/WK-UniversalUnofficialPatch/releases)

## Usage

1. Download from the [Releases page](https://github.com/AdrienTD/WK-UniversalUnofficialPatch/releases).
2. Extract the zip file.
3. Copy `d3d8.dll` file to the game's folder (next to the executable).
   - If you installed the game from CD, the game folder may be `C:\Program Files (x86)\Empire Interactive\Warrior Kings - Battles` or similar.
   - If you installed the game from Steam, it might be found in `C:\Program Files (x86)\Steam\steamapps\common\Warrior Kings (Battles)`.
   - If you installed the game from GOG, it might be found in `C:\Program Files (x86)\GOG Galaxy\Games\Warrior Kings (Battles)`.

4. Launch the game.

See [extra/readme.txt](extra/readme.txt) for more extensive documentation (it is long, this may change in the future).

## Building

You need Visual Studio 2017 or more recent versions.

1. Open the Visual Studio x86 command prompt.
2. `cd` to the repository.
3. Launch `buildr.bat`
4. That's it! 😉
   You can find the built d3d8.dll in the `out` folder.
