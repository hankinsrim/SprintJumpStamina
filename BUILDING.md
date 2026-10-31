# Building Sprint Jump Stamina

Requirements: Visual Studio 2022 or newer with *Desktop development with C++* (includes CMake,
Ninja and vcpkg), and Git.

## Building the plugin

```
git clone --recursive https://github.com/hankinsrim/SprintJumpStamina.git
cd SprintJumpStamina
powershell -ExecutionPolicy Bypass -File .\build.ps1
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Zip
```

* `build.ps1` enters the VS developer environment, uses `%VCPKG_ROOT%` if set (otherwise VS's
  bundled vcpkg), configures with the `release` preset and builds. The first build compiles
  the vcpkg dependencies and CommonLibSSE-NG, which takes a while.
* `-Zip` also writes `build\SprintJumpStamina-<major.minor>.zip`, a minimal FOMOD whose
  `fomod\info.xml` carries the name and version for mod managers that read it.
* `-Preset debug` builds the debug configuration.
* `-Lint` compiles the plugin with MSVC Code Analysis (`/analyze`) and then runs VS's
  `clang-tidy.exe` over `src\` with the checks in `.clang-tidy`. The next build without `-Lint`
  switches the analysis off again.
* Manual equivalent from a *Developer PowerShell for VS*: `cmake --preset release` then
  `cmake --build --preset release`.

The mod-manager-ready output is staged in `dist\`:

```
dist\
    SKSE\
        Plugins\
            SprintJumpStamina.dll
            SprintJumpStamina.ini
            SprintJumpStamina.pdb
```

Sources are formatted with `.clang-format` (VS ships `clang-format.exe` under
`VC\Tools\Llvm\x64\bin`).

## How it works

* Hook: `ProcessEvent` (slot 1) of `PlayerCharacter`'s `BSTEventSink<BSAnimationGraphEvent>`
  vtable (the one inherited from `TESObjectREFR`, via Address Library). The engine attaches
  the player to every animation graph it builds for them (loading a save, race changes,
  werewolf and vampire lord forms), so the hook never needs re-registering.
* On a `JumpUp` event from the player's graph, the sprint state (`ActorState::IsSprinting`) is
  read before the original handler runs and the jump is counted in an atomic. Graph events
  arrive on worker threads, so the stamina damage is applied in a second hook,
  `PlayerCharacter::Update` (primary vtable slot 0xAD), on the main thread. The log warns once
  if that update ever runs on another thread.
* Sprint-jump cost: `SprintJumpSeconds x fSprintStaminaDrainMult x (fSprintStaminaWeightBase +
  fSprintStaminaWeightMult x Actor::GetEquippedWeight() x SprintJumpWeightScale) +
  SprintJumpFlat`, clamped to the stamina left. Game settings are read at jump time.
