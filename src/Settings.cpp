#include "Settings.h"

namespace
{
	constexpr auto INI_PATH = "Data/SKSE/Plugins/SprintJumpStamina.ini";

	struct FloatOption
	{
		const char* key;
		float Settings::* member;
	};

	constexpr auto COST_OPTIONS = std::to_array<FloatOption>({
		{ "SprintJumpSeconds", &Settings::sprintJumpSeconds },
		{ "SprintJumpWeightScale", &Settings::sprintJumpWeightScale },
		{ "SprintJumpFlat", &Settings::sprintJumpFlat },
		{ "NormalJump", &Settings::normalJump },
	});

	bool EqualsNoCase(std::string_view a, std::string_view b)
	{
		return std::ranges::equal(a, b, [](char x, char y) { return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y)); });
	}

	bool IsKnownKey(std::string_view a_section, std::string_view a_key)
	{
		if (EqualsNoCase(a_section, "General"))
		{
			return EqualsNoCase(a_key, "EnableLogging");
		}
		return EqualsNoCase(a_section, "Cost") && std::ranges::any_of(COST_OPTIONS, [&](const FloatOption& a_opt) { return EqualsNoCase(a_key, a_opt.key); });
	}
}

bool Settings::ReadEnableLogging()
{
	CSimpleIniA ini;
	ini.SetUnicode();
	// A missing INI is logged so the warning about it can be read.
	return ini.LoadFile(INI_PATH) < 0 || ini.GetBoolValue("General", "EnableLogging", false);
}

void Settings::Load()
{
	CSimpleIniA ini;
	ini.SetUnicode();
	if (const auto rc = ini.LoadFile(INI_PATH); rc < 0)
	{
		logger::warn("Could not read {} (error {}); using defaults.", INI_PATH, static_cast<int>(rc));
		return;
	}

	enableLogging = ini.GetBoolValue("General", "EnableLogging", enableLogging);
	logger::info("  [General] EnableLogging = {}", enableLogging);

	for (const auto& opt : COST_OPTIONS)
	{
		const auto value = static_cast<float>(ini.GetDoubleValue("Cost", opt.key, this->*opt.member));
		if (value < 0.0F)
		{
			logger::warn("  [Cost] {} = {} is negative - using 0.", opt.key, value);
		}
		this->*opt.member = std::max(value, 0.0F);
		logger::info("  [Cost] {} = {}", opt.key, this->*opt.member);
	}

	// Typos in option names would otherwise be silently ignored.
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	for (const auto& section : sections)
	{
		CSimpleIniA::TNamesDepend keys;
		ini.GetAllKeys(section.pItem, keys);
		for (const auto& key : keys)
		{
			if (!IsKnownKey(section.pItem, key.pItem))
			{
				logger::warn("  Unknown option [{}] {} in INI - ignored (check spelling).", section.pItem, key.pItem);
			}
		}
	}
}

bool Settings::ChargesAnything() const noexcept
{
	return sprintJumpSeconds > 0.0F || sprintJumpFlat > 0.0F || normalJump > 0.0F;
}
