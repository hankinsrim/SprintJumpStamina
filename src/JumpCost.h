#pragma once

namespace JumpCost
{
	// Hooks the player's own animation graph sink (re-attached by the engine on every graph rebuild) and the player's per-frame Update.
	void Install();
}
