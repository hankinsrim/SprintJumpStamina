# Sprint Jump Stamina

An SKSE plugin for Skyrim Special Edition / Anniversary Edition that charges stamina for
sprint-jumps, meant for use with [Better Jumping SE](https://www.nexusmods.com/skyrimspecialedition/mods/18967).

Better Jumping lets you jump while sprinting, and with its momentum option you keep sprint
speed in the air. The game pauses sprint drain while you are airborne, so chaining sprint-jumps
covers ground for far less stamina than sprinting. This plugin charges each sprint-jump a set
number of seconds of your own sprint drain, so heavier armor makes sprint-jumps cost more,
like sprinting does. Normal jumps stay free unless you set a cost for them.

Supported runtimes: 1.5.x, 1.6.x and 1.7.x in one DLL via CommonLibSSE-NG and Address Library,
built and checked against 1.7.104.

This code was written with LLM assistance.

Requirements: [SKSE64](https://skse.silverlock.org/) and
[Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444).

## Notable Files

* Config: `SKSE\Plugins\SprintJumpStamina.ini`
* Log: `Documents\My Games\Skyrim Special Edition\SKSE\SprintJumpStamina.log`

## INI options

* **`[General]`**
  * `EnableLogging` (`false`): write `SprintJumpStamina.log` with the loaded settings and the
    cost of every jump. `false` writes no log file.
* **`[Cost]`**
  * `SprintJumpSeconds` (`1.0`): seconds of sprint drain charged per sprint-jump. The drain
    is the game's own, `fSprintStaminaDrainMult x (fSprintStaminaWeightBase +
    fSprintStaminaWeightMult x worn weight)`, which is 7 stamina per second unarmored plus
    2% per point of worn weight, and follows mods that change those game settings.
  * `SprintJumpWeightScale` (`0.6`): scales the worn-weight part of that drain for
    sprint-jumps. `1.0` makes armor weigh on sprint-jumps exactly as on sprinting, lower
    values make it matter less.
  * `SprintJumpFlat` (`0`): flat stamina added to every sprint-jump.
  * `NormalJump` (`0`): stamina charged for a jump while not sprinting.

A jump never takes more stamina than you have left, and nothing is charged on a horse or in
god mode (`tgm`). Werewolf and Vampire Lord jumps are free too, because those forms don't send
the `JumpUp` animation event the plugin listens for. Unknown option names are reported as
warnings in the log.

## License

Sprint Jump Stamina, Copyright (C) 2026 hankinsrim

This program is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
the GNU General Public License for more details (`LICENSE`, or
<https://www.gnu.org/licenses/gpl-3.0.html>).

Source: <https://github.com/hankinsrim/SprintJumpStamina>

It statically links [CommonLibSSE-NG](https://github.com/alandtse/CommonLibVR/tree/ng), which
is GPL-3.0-or-later with exceptions (see `extern/CommonLibSSE-NG/EXCEPTIONS.md`).
