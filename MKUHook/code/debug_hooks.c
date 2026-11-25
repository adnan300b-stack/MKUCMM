#include "debug_hooks.h"
#include "eLog.h"
#include "mkuhook.h"
#include "mkdeception.h"
#include "stage.h"
#include "injector.h"
#include "pspmem.h"
#include <stdint.h>

static int is_expected_game_pointer(uintptr_t actual, unsigned int offset)
{
	uintptr_t expected = MKDeception_GameAddress(offset);
	return expected != 0 && actual == expected && (actual & 3U) == 0;
}

int debug_hook_character_lock_status(int id, int param)
{
	uintptr_t actual = (uintptr_t)is_char_locked;
	uintptr_t expected = MKDeception_GameAddress(0x16167C);
	int valid = is_expected_game_pointer(actual, 0x16167C);

	LOG_Messagef("character_lock: base=%08X id=%d param=%d callback=%08X expected=%08X valid=%d\n",
		(unsigned int)injector.base_addr, id, param, (unsigned int)actual,
		(unsigned int)expected, valid);
	if (!valid) {
		LOG_Message("ERROR: character_lock callback/base mismatch; delegate not called\n");
		return 0;
	}
	return hook_character_lock_status(id, param);
}

int debug_hook_stage_lock_status(int id)
{
	uintptr_t actual = (uintptr_t)is_bgnd_locked;
	uintptr_t expected = MKDeception_GameAddress(0x24CD8);
	int valid = is_expected_game_pointer(actual, 0x24CD8);
	int mode = ReadInt(0x58EE98);

	LOG_Messagef("stage_lock: base=%08X id=%d mode=%d callback=%08X expected=%08X valid=%d\n",
		(unsigned int)injector.base_addr, id, mode, (unsigned int)actual,
		(unsigned int)expected, valid);
	if (!valid) {
		LOG_Message("ERROR: stage_lock callback/base mismatch; delegate not called\n");
		return 0;
	}
	return hook_stage_lock_status(id);
}

void debug_play_kon_music(void)
{
	uintptr_t randu_actual = (uintptr_t)randu;
	uintptr_t set_music_actual = (uintptr_t)set_music;
	uintptr_t randu_expected = MKDeception_GameAddress(0x1AA9F0);
	uintptr_t set_music_expected = MKDeception_GameAddress(0x9FD1C);
	int randu_valid = randu_actual == randu_expected && randu_expected != 0 && (randu_actual & 3U) == 0;
	int set_music_valid = set_music_actual == set_music_expected && set_music_expected != 0 && (set_music_actual & 3U) == 0;

	LOG_Messagef("play_kon_music: base=%08X randu=%08X expected=%08X valid=%d set_music=%08X expected=%08X valid=%d\n",
		(unsigned int)injector.base_addr, (unsigned int)randu_actual,
		(unsigned int)randu_expected, randu_valid, (unsigned int)set_music_actual,
		(unsigned int)set_music_expected, set_music_valid);
	if (!randu_valid || !set_music_valid) {
		LOG_Message("ERROR: play_kon_music callback/base mismatch; callbacks not called\n");
		return;
	}
	play_kon_music();
}
