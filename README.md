# BoostResonanceDurationMod

This Code is only a example of the most used method and logic in UE4SS CPP, may not working correctly in production env.

BoostResonanceDurationMod is a UE4SS mod for Vampire: The Masquerade � Bloodlines 2 that automatically restart  passive boost from ventrue clan.  

UE4SS loader (download and installation instructions):  
https://www.nexusmods.com/vtmbloodlines2/mods/123

Important: Build prerequisite (required)
- You must download the VTMB2 devkit / UE4SS modkit from NexusMods: https://www.nexusmods.com/vtmbloodlines2/mods/123
- After downloading the devkit, copy the `BoostResonanceDurationMod` folder into:
  - `VTMB2_ModKit\UE4SS_CPP_MIN\Mods_Source`
- Then run the build script once:
  - `VTMB2_ModKit\build_mods.bat`
  The build script will compile the code and place the resulting DLL into the proper output folder for the modkit.

Key features
- Automatic detection of `Passive Ability` and recast.

Requirements
- Vampire: The Masquerade � Bloodlines 2.
- UE4SS Mod Loader installed.
- Devkit / ModKit from NexusMods (see above).
- If building manually: Visual Studio 2022, CMake, Windows 10/11.

Installation (end user)
1. Install UE4SS following the NexusMods instructions.
2. Place the compiled DLL into your UE4SS Mods folder, for example:
   - `UE4SS/Mods/BoostResonanceDurationMod/main.dll`
3. Start the game via the UE4SS loader; the mod should load automatically.

Build from source (optional / alternate)
- Recommended: use the devkit build flow described above (copy into `VTMB2_ModKit\UE4SS_CPP_MIN\Mods_Source` and run `VTMB2_ModKit\build_mods.bat`).

Configuration & debugging
- Enable `DEBUG_MODE` in `BoostResonanceDurationMod/src/VentruePassive.hpp` and rebuild to get detailed diagnostic logs. Logs are emitted via `DynamicOutput::Output` and appear in the UE4SS loader console.

Known limitations
- If the game or UE4SS changes, function detection may fail and require updates.
- The initialization test intentionally modifies player health (50% then restore).
- The mod relies on calling `ProcessEvent` on game UObjects; future engine/game changes can break behavior.

Troubleshooting
- Mod not loaded: verify DLL placement (`UE4SS/Mods/BoostResonanceDurationMod/`) and that UE4SS is used to launch the game.
- Check if you have file enabled.txt in the mod folder to ensure UE4SS is loading the mod.
- No effect in-game: enable `DEBUG_MODE`, rebuild, and inspect UE4SS/loader logs for `[BoostResonanceDurationMod]` messages.

Contributing
- PRs welcome. Focus: detection robustness, stability, and edge cases (GC of UObjects, zero health states).
- Keep style consistent with existing sources in `src/`.

License
- MIT by default. Add a `LICENSE` file to change.

Credits
- Original code author: `Alrikh`
- Repository: https://github.com/gassoh-max/BoostResonanceDurationMod

Changelog
- v1.0 � Initial implementation: function detection, test init, gradual healing logic.
