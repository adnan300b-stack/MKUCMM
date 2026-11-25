#pragma once
#include "mkstructs.h"
#include "ssf.h"
#include <stdint.h>
#include "eLog.h"


enum PLAYER_NUM
{
	INVALID_PLAYER_NUM = -1,
	PLAYER1,
	PLAYER2,
};

#define PLAYER1_OBJ   0x8D929AC
#define PLAYER2_OBJ   0x8D92A18
#define PLAYER1_ADDR  0x58E9AC
#define PLAYER2_ADDR  0x58EA18
#define KONQUEST_INFO 0x2FAE40

/* Game function pointers are defined once in game_functions.c. */
extern void (*mk_sprintf)(char* ptr, char* fmt, ...);

/* file */
extern void (*load_ssf)(struct mem_ssf_header* ssf);
extern void (*load_font)(int id);
extern int (*load_script_data)(int id, char* name);
extern void (*func_6F388)(int unk, int id);

/* proc */
extern void (*p_pselect)(void);
extern void (*p_game_loop)(void);
extern int (*create_mkproc)(int id, int unk, int func, int unk2, int unk3);
extern void (*gamelogic_jump)(int mode, int function);

/* screen */
extern void (*fire_screen_event)(int id, int param);
extern void (*refresh_screen)(void);
extern void (*wait_for_screen_close)(void);
extern int (*load_named_2d_pfxobj_xy)(int slot, int id, char* name, int orientation, int x, int y, int unk);

/* misc */
extern int (*string_right_xy)(int id, int font, char* text, int y, int x, int unk);
extern int (*string_left_xy)(int id, int font, char* text, int y, int x, int unk);
extern int (*load_hero_model)(int ptr);
extern int (*create_plr)(int plr, struct player_info* obj);
extern int (*is_char_locked)(int id, int param);
extern int (*is_bgnd_locked)(int id);
extern int (*translate_id_from_pz)(int id);
extern void (*snd_req)(int id);
extern int (*set_music)(int id);
extern void (*sub_7F050)(void);
extern int (*randu)(int range);
extern int (*bgnd_unhide_sobj)(int id);
extern char* (*get_string_by_id)(int id);

/* core */
extern void (*render)(void);
extern int (*get_game_state)(void);
int get_game_tick(void);
extern int (*pressed_button)(int plr, int id);
extern void (*impale_player)(void);

/* rw */
extern int (*RwRasterCreate)(int x, int y, int bits, int mode);

/* Resolve and validate function addresses against the ULUS10102 text base. */
uintptr_t MKDeception_GameAddress(unsigned int offset);
int MKDeception_IsExpectedGameFunction(uintptr_t pointer, unsigned int offset);

/* Guard callback calls used by character/resource hooks. Exact expected offsets
 * prevent a merely aligned data pointer from being treated as executable. */
static inline void MKCall_LoadSsf(struct mem_ssf_header *header)
{
	if (!MKDeception_IsExpectedGameFunction((uintptr_t)(load_ssf), 0x10FB08)) {
		LOG_Messagef("ERROR: load_ssf callback invalid actual=%08X expected=%08X\n",
			(unsigned int)(uintptr_t)(load_ssf),
			(unsigned int)MKDeception_GameAddress(0x10FB08));
		return;
	}
	(load_ssf)(header);
}

static inline int MKCall_LoadScriptData(int id, char *name)
{
	if (!MKDeception_IsExpectedGameFunction((uintptr_t)(load_script_data), 0x10E438)) {
		LOG_Messagef("ERROR: load_script_data callback invalid actual=%08X expected=%08X\n",
			(unsigned int)(uintptr_t)(load_script_data),
			(unsigned int)MKDeception_GameAddress(0x10E438));
		return -1;
	}
	return (load_script_data)(id, name);
}

static inline int MKCall_TranslateIdFromPz(int id)
{
	if (!MKDeception_IsExpectedGameFunction((uintptr_t)(translate_id_from_pz), 0x15F87C)) {
		LOG_Messagef("ERROR: translate_id_from_pz callback invalid actual=%08X expected=%08X\n",
			(unsigned int)(uintptr_t)(translate_id_from_pz),
			(unsigned int)MKDeception_GameAddress(0x15F87C));
		return -1;
	}
	return (translate_id_from_pz)(id);
}

static inline int MKCall_StringLeft(int id, int font, char *text, int y, int x, int unk)
{
	if (!MKDeception_IsExpectedGameFunction((uintptr_t)(string_left_xy), 0x79624)) {
		LOG_Messagef("ERROR: string_left_xy callback invalid actual=%08X expected=%08X\n",
			(unsigned int)(uintptr_t)(string_left_xy),
			(unsigned int)MKDeception_GameAddress(0x79624));
		return 0;
	}
	return (string_left_xy)(id, font, text, y, x, unk);
}

static inline int MKCall_StringRight(int id, int font, char *text, int y, int x, int unk)
{
	if (!MKDeception_IsExpectedGameFunction((uintptr_t)(string_right_xy), 0x796A4)) {
		LOG_Messagef("ERROR: string_right_xy callback invalid actual=%08X expected=%08X\n",
			(unsigned int)(uintptr_t)(string_right_xy),
			(unsigned int)MKDeception_GameAddress(0x796A4));
		return 0;
	}
	return (string_right_xy)(id, font, text, y, x, unk);
}

static inline void MKCall_Func6F388(int unk, int id)
{
	if (!MKDeception_IsExpectedGameFunction((uintptr_t)(func_6F388), 0x6F388)) {
		LOG_Messagef("ERROR: func_6F388 callback invalid actual=%08X expected=%08X\n",
			(unsigned int)(uintptr_t)(func_6F388),
			(unsigned int)MKDeception_GameAddress(0x6F388));
		return;
	}
	(func_6F388)(unk, id);
}

/* Function-like macros preserve pointer declarations/address inspection while
 * routing normal calls through the guards above. */
#define load_ssf(...) MKCall_LoadSsf(__VA_ARGS__)
#define load_script_data(...) MKCall_LoadScriptData(__VA_ARGS__)
#define translate_id_from_pz(...) MKCall_TranslateIdFromPz(__VA_ARGS__)
#define string_left_xy(...) MKCall_StringLeft(__VA_ARGS__)
#define string_right_xy(...) MKCall_StringRight(__VA_ARGS__)
#define func_6F388(...) MKCall_Func6F388(__VA_ARGS__)

enum pad_button {
	PAD_L2,
	PAD_R2,
	PAD_L1,
	PAD_R1,
	PAD_TRIANGLE,
	PAD_CIRCLE,
	PAD_CROSS,
	PAD_SQUARE,
	PAD_SELECT,
	PAD_L3,
	PAD_R3,
	PAD_START,
	PAD_UP,
	PAD_RIGHT,
	PAD_DOWN,
	PAD_LEFT,
	PAD_ANY,
	PAD_L2R2,
};

enum game_state {
	STATE_0,
	STATE_1,
	STATE_2,
	STATE_ATTRACT,
	STATE_SELECT,
	STATE_5,
	STATE_6,
	STATE_GAME,
	STATE_8,
	STATE_MENU,
};

void MKDeception_Init(unsigned int addr);

/* hooks */
int hook_load_hero_model(int ptr);
int hook_create_player(int plr, struct player_info* obj);
