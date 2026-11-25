#include "debug_hooks.h"
#include "eLog.h"
#include "mkuhook.h"
#include "mkdeception.h"
#include "stage.h"
#include "injector.h"
#include "pspmem.h"
#include <stdint.h>

/* PSP user code is normally in this range. This rejects NULL, misaligned,
 * and obviously corrupt values, but cannot prove a mapped address is code. */
static int is_plausible_code_pointer(uintptr_t pointer)
{
	return pointer >= 0x08000000U && pointer < 0x0C000000U &&
		(pointer & 3U) == 0;
}

static unsigned int log_code_pointer(uintptr_t pointer)
{
	return is_plausible_code_pointer(pointer) ? (unsigned int)pointer : 0U;
}

int debug_hook_character_lock_status(int id, int param)
{
	uintptr_t delegate = (uintptr_t)is_char_locked;

	LOG_Messagef("Entering character_lock id=%d param=%d delegate=%08X valid=%d\n",
		id, param, log_code_pointer(delegate), is_plausible_code_pointer(delegate));
	if (!is_plausible_code_pointer(delegate)) {
		LOG_Messagef("ERROR: character_lock rejected function pointer=%08X\n", (unsigned int)delegate);
		return 0;
	}
	return hook_character_lock_status(id, param);
}

int debug_hook_stage_lock_status(int id)
{
	uintptr_t delegate = (uintptr_t)is_bgnd_locked;
	uint32_t base = (uint32_t)injector.base_addr;
	int mode = -1;

	/* Read the game-relative state through the base-aware patcher only. */
	if (base != 0 && base <= UINT32_MAX - 0x58EE98U)
		mode = ReadInt(0x58EE98);

	LOG_Messagef("Entering stage_lock id=%d mode=%d delegate=%08X valid=%d\n",
		id, mode, log_code_pointer(delegate), is_plausible_code_pointer(delegate));
	if (!is_plausible_code_pointer(delegate)) {
		LOG_Messagef("ERROR: stage_lock rejected function pointer=%08X\n", (unsigned int)delegate);
		return 0;
	}
	return hook_stage_lock_status(id);
}

void debug_play_kon_music(void)
{
	uintptr_t randu_pointer = (uintptr_t)randu;
	uintptr_t set_music_pointer = (uintptr_t)set_music;

	LOG_Messagef("Entering play_kon_music randu=%08X valid=%d set_music=%08X valid=%d\n",
		log_code_pointer(randu_pointer), is_plausible_code_pointer(randu_pointer),
		log_code_pointer(set_music_pointer), is_plausible_code_pointer(set_music_pointer));
	if (!is_plausible_code_pointer(randu_pointer) ||
	    !is_plausible_code_pointer(set_music_pointer)) {
		LOG_Messagef("ERROR: play_kon_music skipped invalid callbacks randu=%08X set_music=%08X\n",
			(unsigned int)randu_pointer, (unsigned int)set_music_pointer);
		return;
	}
	play_kon_music();
}
