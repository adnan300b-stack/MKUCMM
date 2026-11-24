#include "debug_hooks.h"
#include "eLog.h"
#include "mkuhook.h"
#include "mkdeception.h"
#include "stage.h"
#include <stdint.h>

#define DEBUG_HOOK_LOG_LIMIT 8

int debug_hook_stage_lock_status(int id)
{
	static unsigned int calls;
	unsigned int call_number = ++calls;
	int result;

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK stage_lock #%u enter id=%d mode=%d delegate=%08X\n",
			call_number, id, ReadInt(0x58EE98), (unsigned int)(uintptr_t)is_bgnd_locked);

	result = hook_stage_lock_status(id);

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK stage_lock #%u return result=%d\n", call_number, result);
	return result;
}

void debug_play_kon_music(void)
{
	static unsigned int calls;
	unsigned int call_number = ++calls;

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK play_kon_music #%u enter game-base=%08X randu=%08X set_music=%08X\n",
			call_number, (unsigned int)GetBaseAddr(),
			(unsigned int)(uintptr_t)randu, (unsigned int)(uintptr_t)set_music);

	play_kon_music();

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK play_kon_music #%u return\n", call_number);
}
