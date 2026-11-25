#include "mkdeception.h"
#include "pspmem.h"
#include "eSettingsMgr.h"
#include "eLog.h"
#include "mkuhook.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

char msgBuff[256];

/* Resolve an address from the target game's text base, never from a plugin-local
 * or fixed absolute address. Return zero if the addition cannot be represented. */
uintptr_t MKDeception_GameAddress(unsigned int offset)
{
	uintptr_t base = (uintptr_t)GetBaseAddr();
	if (base == 0 || base > UINTPTR_MAX - (uintptr_t)offset)
		return 0;
	return base + (uintptr_t)offset;
}

/* Reject NULL, misaligned, out-of-user-space, or unexpected game callbacks. */
static int is_expected_game_function(uintptr_t pointer, unsigned int offset)
{
	uintptr_t expected = MKDeception_GameAddress(offset);

	return expected != 0 && pointer == expected &&
		(pointer & 3U) == 0 &&
		pointer >= 0x08000000U && pointer < 0x0C000000U;
}

int get_game_tick(void)
{
	return ReadInt(0x58EEC4);
}

void MKDeception_Init(unsigned int addr)
{
	/* Every callback below is derived from the module text address supplied by
	 * the loader, which PSPPatcher_Init records as the single source of base. */
	mk_sprintf = (void *)(MKDeception_GameAddress(0x248048));
	load_hero_model = (void *)(MKDeception_GameAddress(0x1C0380));
	create_plr = (void *)(MKDeception_GameAddress(0x153960));
	p_game_loop = (void *)(MKDeception_GameAddress(0x5FC1C));
	string_right_xy = (void *)(MKDeception_GameAddress(0x796A4));
	string_left_xy = (void *)(MKDeception_GameAddress(0x79624));
	load_font = (void *)(MKDeception_GameAddress(0x7941C));
	wait_for_screen_close = (void *)(MKDeception_GameAddress(0x1335EC));
	gamelogic_jump = (void *)(MKDeception_GameAddress(0x7F924));
	p_pselect = (void *)(MKDeception_GameAddress(0x162C6C));
	fire_screen_event = (void *)(MKDeception_GameAddress(0x1343B0));
	is_char_locked = (void *)(MKDeception_GameAddress(0x16167C));
	is_bgnd_locked = (void *)(MKDeception_GameAddress(0x24CD8));
	create_mkproc = (void *)(MKDeception_GameAddress(0x15FA4C));
	refresh_screen = (void *)(MKDeception_GameAddress(0x1338B4));
	load_ssf = (void *)(MKDeception_GameAddress(0x10FB08));
	load_named_2d_pfxobj_xy = (void *)(MKDeception_GameAddress(0x84B04));
	impale_player = (void *)(MKDeception_GameAddress(0x64378));
	translate_id_from_pz = (void *)(MKDeception_GameAddress(0x15F87C));
	func_6F388 = (void *)(MKDeception_GameAddress(0x6F388));
	load_script_data = (void *)(MKDeception_GameAddress(0x10E438));
	render = (void *)(MKDeception_GameAddress(0x62460));
	get_game_state = (void *)(MKDeception_GameAddress(0x1AA794));
	snd_req = (void *)(MKDeception_GameAddress(0x1A5A68));
	get_string_by_id = (void *)(MKDeception_GameAddress(0x10F4B4));
	RwRasterCreate = (void *)(MKDeception_GameAddress(0x294A98));
	set_music = (void *)(MKDeception_GameAddress(0x9FD1C));
	sub_7F050 = (void *)(MKDeception_GameAddress(0x7F050));
	randu = (void *)(MKDeception_GameAddress(0x1AA9F0));
	bgnd_unhide_sobj = (void *)(MKDeception_GameAddress(0x18630));
	pressed_button = (void *)(MKDeception_GameAddress(0x8739C));

	LOG_Messagef("Game callbacks initialized: text-base=%08X supplied-base=%08X\n",
		(unsigned int)GetBaseAddr(), addr);
	LOG_Messagef("callback check: is_char_locked actual=%08X expected=%08X valid=%d\n",
		(unsigned int)(uintptr_t)is_char_locked,
		(unsigned int)MKDeception_GameAddress(0x16167C),
		is_expected_game_function((uintptr_t)is_char_locked, 0x16167C));
	LOG_Messagef("callback check: is_bgnd_locked actual=%08X expected=%08X valid=%d\n",
		(unsigned int)(uintptr_t)is_bgnd_locked,
		(unsigned int)MKDeception_GameAddress(0x24CD8),
		is_expected_game_function((uintptr_t)is_bgnd_locked, 0x24CD8));
	LOG_Messagef("callback check: randu actual=%08X expected=%08X valid=%d\n",
		(unsigned int)(uintptr_t)randu,
		(unsigned int)MKDeception_GameAddress(0x1AA9F0),
		is_expected_game_function((uintptr_t)randu, 0x1AA9F0));
	LOG_Messagef("callback check: set_music actual=%08X expected=%08X valid=%d\n",
		(unsigned int)(uintptr_t)set_music,
		(unsigned int)MKDeception_GameAddress(0x9FD1C),
		is_expected_game_function((uintptr_t)set_music, 0x9FD1C));
}

int hook_load_hero_model(int ptr)
{
	uintptr_t logger = (uintptr_t)mk_sprintf;
	uintptr_t delegate = (uintptr_t)load_hero_model;
	int p_konquest_info;

	LOG_Messagef("Entering hook_load_hero_model ptr=%08X logger=%08X delegate=%08X\n",
		(unsigned int)ptr, (unsigned int)logger, (unsigned int)delegate);
	if (!is_expected_game_function(logger, 0x248048) ||
	    !is_expected_game_function(delegate, 0x1C0380)) {
		LOG_Messagef("ERROR: invalid hero-model callback(s), logger=%08X/%08X delegate=%08X/%08X\n",
			(unsigned int)logger, (unsigned int)MKDeception_GameAddress(0x248048),
			(unsigned int)delegate, (unsigned int)MKDeception_GameAddress(0x1C0380));
		return 0;
	}

	p_konquest_info = ReadInt(KONQUEST_INFO);
	if (p_konquest_info == 0) {
		LOG_Message("ERROR: KONQUEST_INFO pointer is NULL; skipping model field access\n");
		return 0;
	}

	if (SettingsMgr_Get().iForceShujinkoModel != -1 &&
	    SettingsMgr_Get().iForceShujinkoModel <= 4)
		*(char *)(uintptr_t)(p_konquest_info + 92) =
			(char)SettingsMgr_Get().iForceShujinkoModel;

	mk_sprintf(msgBuff, "INFO: Loading player konquest model: %d\n",
		*(char *)(uintptr_t)(p_konquest_info + 92));
	LOG_Message(msgBuff);
	return load_hero_model(ptr);
}

int hook_create_player(int plr, struct player_info *obj)
{
	uintptr_t logger = (uintptr_t)mk_sprintf;
	uintptr_t delegate = (uintptr_t)create_plr;

	LOG_Messagef("Entering hook_create_player player=%d obj=%08X logger=%08X delegate=%08X\n",
		plr, (unsigned int)(uintptr_t)obj, (unsigned int)logger, (unsigned int)delegate);
	if (obj == NULL || !is_expected_game_function(logger, 0x248048) ||
	    !is_expected_game_function(delegate, 0x153960)) {
		LOG_Messagef("ERROR: invalid player callback/input obj=%08X logger=%08X/%08X delegate=%08X/%08X\n",
			(unsigned int)(uintptr_t)obj, (unsigned int)logger,
			(unsigned int)MKDeception_GameAddress(0x248048), (unsigned int)delegate,
			(unsigned int)MKDeception_GameAddress(0x153960));
		return 0;
	}

	mk_sprintf(msgBuff, "INFO: Loading player %d character %d alt: %x\n",
		plr, obj->characterID, ReadInt(0x4173CC));
	LOG_Message(msgBuff);
	if (plr == PLAYER1 && toggle_palette) {
		int is_alt = ReadInt(0x4173CC) != 0;
		obj->flags |= is_alt | 2;
	}
	return create_plr(plr, obj);
}
