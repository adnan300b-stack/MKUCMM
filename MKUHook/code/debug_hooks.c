#include "debug_hooks.h"
#include "eLog.h"
#include "mkuhook.h"
#include "mkdeception.h"
#include "stage.h"
#include "injector.h"
#include "pspmem.h"
#include <stdint.h>

#define DEBUG_HOOK_LOG_LIMIT 8

static int is_plausible_code_pointer(uintptr_t pointer)
{
	/* Log-only check: validate alignment/range without dereferencing pointer. */
	return pointer != 0 && pointer <= UINT32_MAX && (pointer & 3U) == 0;
}

static unsigned int log_code_pointer(uintptr_t pointer)
{
	return is_plausible_code_pointer(pointer) ? (unsigned int)pointer : 0U;
}

int debug_hook_stage_lock_status(int id)
{
	static unsigned int calls;
	unsigned int call_number = ++calls;
	int result;
	uintptr_t delegate = (uintptr_t)is_bgnd_locked;
	uint32_t base = (uint32_t)injector.base_addr;
	int mode = -1;

	/* injector.base_addr is established at game initialization. Read only the
	 * known game-relative mode word, and do not probe any function pointer. */
	if (base != 0 && base <= UINT32_MAX - 0x58EE98U)
		mode = (int)ReadInt(0x58EE98);

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK stage_lock #%u enter id=%d mode=%d delegate=%08X valid=%d\n",
			call_number, id, mode, log_code_pointer(delegate),
			is_plausible_code_pointer(delegate));

	result = hook_stage_lock_status(id);

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK stage_lock #%u return result=%d\n", call_number, result);
	return result;
}

void debug_play_kon_music(void)
{
	static unsigned int calls;
	unsigned int call_number = ++calls;
	uintptr_t randu_pointer = (uintptr_t)randu;
	uintptr_t set_music_pointer = (uintptr_t)set_music;

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK play_kon_music #%u enter game-base=%08X randu=%08X valid=%d set_music=%08X valid=%d\n",
			call_number, (unsigned int)injector.base_addr,
			log_code_pointer(randu_pointer), is_plausible_code_pointer(randu_pointer),
			log_code_pointer(set_music_pointer), is_plausible_code_pointer(set_music_pointer));

	play_kon_music();

	if (call_number <= DEBUG_HOOK_LOG_LIMIT)
		LOG_Messagef("HOOK play_kon_music #%u return\n", call_number);
}
