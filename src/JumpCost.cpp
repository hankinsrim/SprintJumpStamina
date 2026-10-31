#include "JumpCost.h"
#include "Settings.h"

namespace JumpCost
{
	namespace
	{
		using Sink   = RE::BSTEventSink<RE::BSAnimationGraphEvent>;
		using Source = RE::BSTEventSource<RE::BSAnimationGraphEvent>;

		constexpr std::size_t PROCESS_EVENT_SLOT = 1;     // BSTEventSink::ProcessEvent
		constexpr std::size_t UPDATE_SLOT        = 0xAD;  // Actor::Update

		// Vanilla values, used only if a game setting can't be read.
		constexpr float DEFAULT_SPRINT_DRAIN       = 7.0F;
		constexpr float DEFAULT_SPRINT_WEIGHT_BASE = 1.0F;
		constexpr float DEFAULT_SPRINT_WEIGHT_MULT = 0.02F;

		float GameSetting(const char* a_name, float a_default)
		{
			auto* const collection = RE::GameSettingCollection::GetSingleton();
			auto* const setting    = collection ? collection->GetSetting(a_name) : nullptr;
			return setting && setting->GetType() == RE::Setting::Type::kFloat ? setting->GetFloat() : a_default;
		}

		// Vanilla sprint drain is fSprintStaminaDrainMult x (fSprintStaminaWeightBase + fSprintStaminaWeightMult x worn weight); a_weightScale scales the weight term.
		float SprintDrainPerSecond(float a_wornWeight, float a_weightScale)
		{
			const float drain        = GameSetting("fSprintStaminaDrainMult", DEFAULT_SPRINT_DRAIN);
			const float weightBase   = GameSetting("fSprintStaminaWeightBase", DEFAULT_SPRINT_WEIGHT_BASE);
			const float weightFactor = GameSetting("fSprintStaminaWeightMult", DEFAULT_SPRINT_WEIGHT_MULT) * a_wornWeight * a_weightScale;
			return drain * (weightBase + weightFactor);
		}

		// Jumps seen by the animation graph, charged on the player's next update.
		std::atomic<std::uint32_t> pendingSprintJumps{ 0 };
		std::atomic<std::uint32_t> pendingJumps{ 0 };

		std::thread::id mainThread;  // SKSE loads plugins on the game's main thread

		void Charge(bool a_sprinting)
		{
			auto* const player  = RE::PlayerCharacter::GetSingleton();
			auto* const avOwner = player ? player->AsActorValueOwner() : nullptr;
			if (!avOwner)
			{
				return;
			}

			const auto& settings   = Settings::Get();
			float       wornWeight = 0.0F;
			float       drain      = 0.0F;
			float       cost       = settings.normalJump;
			if (a_sprinting)
			{
				wornWeight = std::max(player->GetEquippedWeight(), 0.0F);
				drain      = SprintDrainPerSecond(wornWeight, 1.0F);
				cost       = (settings.sprintJumpSeconds * SprintDrainPerSecond(wornWeight, settings.sprintJumpWeightScale)) + settings.sprintJumpFlat;
			}

			const float before = avOwner->GetActorValue(RE::ActorValue::kStamina);
			cost               = std::clamp(cost, 0.0F, std::max(before, 0.0F));
			if (cost > 0.0F)
			{
				avOwner->DamageActorValue(RE::ActorValue::kStamina, cost);
			}

			if (a_sprinting)
			{
				logger::info("Sprint-jump: {:.1f} stamina ({:.1f}/s sprint drain, worn weight {:.1f}), stamina {:.1f} -> {:.1f}", cost, drain, wornWeight, before, avOwner->GetActorValue(RE::ActorValue::kStamina));
			}
			else
			{
				logger::info("Jump (not sprinting): {:.1f} stamina, stamina {:.1f} -> {:.1f}", cost, before, avOwner->GetActorValue(RE::ActorValue::kStamina));
			}
		}

		void OnJumpUp(const RE::TESObjectREFR* a_holder)
		{
			auto* const player = RE::PlayerCharacter::GetSingleton();
			if (!player || a_holder != player || player->IsOnMount() || RE::PlayerCharacter::IsGodMode())
			{
				return;
			}

			// Read here, before the graph event is processed further, so the sprint state matches this jump.
			auto* const state     = player->AsActorState();
			const bool  sprinting = state && state->IsSprinting();

			(sprinting ? pendingSprintJumps : pendingJumps).fetch_add(1, std::memory_order_relaxed);
		}

		struct UpdateHook
		{
			static void Thunk(RE::PlayerCharacter* a_this, float a_delta)
			{
				func(a_this, a_delta);

				const auto sprintJumps = pendingSprintJumps.exchange(0, std::memory_order_relaxed);
				const auto jumps       = pendingJumps.exchange(0, std::memory_order_relaxed);
				if (sprintJumps == 0 && jumps == 0)
				{
					return;
				}
				if (static bool warned = false; !warned && std::this_thread::get_id() != mainThread)
				{
					warned = true;
					logger::warn("PlayerCharacter::Update ran off the main thread - stamina is being charged from another thread");
				}
				for (std::uint32_t i = 0; i < sprintJumps; ++i)
				{
					Charge(true);
				}
				for (std::uint32_t i = 0; i < jumps; ++i)
				{
					Charge(false);
				}
			}

			static inline REL::Relocation<decltype(Thunk)> func;
		};

		struct ProcessEventHook
		{
			static RE::BSEventNotifyControl Thunk(Sink* a_this, const RE::BSAnimationGraphEvent* a_event, Source* a_source)
			{
				if (a_event && a_event->tag == "JumpUp"sv)
				{
					OnJumpUp(a_event->holder);
				}
				return func(a_this, a_event, a_source);
			}

			static inline REL::Relocation<decltype(Thunk)> func;
		};
	}

	void Install()
	{
		mainThread = std::this_thread::get_id();

		REL::Relocation<std::uintptr_t> sinkVtbl{ RE::VTABLE_PlayerCharacter[2] };  // TESObjectREFR's BSTEventSink<BSAnimationGraphEvent>
		ProcessEventHook::func = sinkVtbl.write_vfunc(PROCESS_EVENT_SLOT, ProcessEventHook::Thunk);

		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };
		UpdateHook::func = vtbl.write_vfunc(UPDATE_SLOT, UpdateHook::Thunk);
		logger::info("Hooked PlayerCharacter animation graph events and Update");
	}
}
