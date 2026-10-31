#pragma once

// Values from Data\SKSE\Plugins\SprintJumpStamina.ini.
struct Settings
{
	// [General]
	bool enableLogging{ false };

	// [Cost]
	float sprintJumpSeconds{ 1.0F };  // seconds of the player's current sprint drain charged per sprint-jump
	float sprintJumpWeightScale{ 0.6F };  // scales the worn-weight part of the sprint drain for sprint-jumps
	float sprintJumpFlat{ 0.0F };
	float normalJump{ 0.0F };

	[[nodiscard]] static Settings& Get() noexcept
	{
		static Settings singleton;
		return singleton;
	}

	// Read before SKSE::Init, which creates the log file.
	[[nodiscard]] static bool ReadEnableLogging();

	void Load();

	[[nodiscard]] bool ChargesAnything() const noexcept;
};
