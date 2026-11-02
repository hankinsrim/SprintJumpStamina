#include "JumpCost.h"
#include "Settings.h"

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::InitInfo info;
	info.log = Settings::ReadEnableLogging();
	SKSE::Init(a_skse, info);
	if (!info.log)
	{
		spdlog::set_level(spdlog::level::off);
	}

	const auto plugin = SKSE::GetPluginVersion();
	logger::info("{} v{}.{}.{} loading on Skyrim {}", SKSE::GetPluginName(), plugin.major(), plugin.minor(), plugin.patch(), REL::Module::get().version().string("."sv));

	logger::info("Settings:");
	auto& settings = Settings::Get();
	settings.Load();
	if (!settings.ChargesAnything())
	{
		logger::info("Every cost is 0 - no hook installed, jumping stays free.");
		return true;
	}

	JumpCost::Install();
	return true;
}
