#include "stage.h"
#include "eLog.h"
#include "mkdeception.h"
#include "pspmem.h"
#include "eSettingsMgr.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "mips.h"
#include "injector.h"
#include "mkstructs.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

struct mk_file_entry katakombs_entry_table[] = {
	{"katakombs.ssf", 0, 4},
	{"katakombs.mko", 0, 3},
	{"katakombs.sec", 0, 1},
	{"katakombs_fx.mko", 0, 3},
};

struct mk_toc_entry katakombs_file_table[] = {
	{&katakombs_entry_table[0], 0, 0},
	{&katakombs_entry_table[1], 2048, 2272},
	{&katakombs_entry_table[2], 6144, 1274880},
	{&katakombs_entry_table[3], 1282048, 1600},
	{0, 0, 0}
};

struct stage_info_entry pStageTable[] = {
	{0x8B07450, "beetlelair.mko", 18, 1},
	{0x8B078E8, "courtyard.mko", 28, 9},
	{0x8B07AB0, "darkprison.mko", 36, 1},
	{0x8B07C78, "deadpool.mko", 32, 9},
	{0x8B07CFC, "dka.mko", 38, 1},
	{0x8B07D80, "dragonmountain.mko", 26, 1},
	{0x8B08638, "fallingcliff.mko", 27, 1},
	{0x8B086BC, "fortress.mko", 34, 1},
	{0x8B08740, "hellsfoundry.mko", 25, 1},
	{0x8B087D0, "kangtomb.mko", 19, 1},
	{0x8B08AA8, "kuatanpalace.mko", 35, 1},
	{0x8B08B80, "livingforest.mko", 39, 9},
	{0x8B08BEC, "lostwarriors.mko", 22, 1},
	{0x8B08C70, "lower_mines.mko", 33, 1},
	{0x8B08CF4, "netherbelly.mko", 23, 1},
	{0x8B09264, "nexus_arena.mko", 40, 1},
	{0x8B09A38, "portal.mko", 29, 9},
	{0x8B09B4C, "reddragon.mko", 21, 1},
	{0x8B09C24, "skytemple.mko", 24, 1},
	{0x8B09CF0, "slaughterhouse.mko", 20, 1},
	{0x8B09DA4, "thepit.mko", 30, 9},
	{0x8B09E28, "yinyang.mko", 31, 1},
	{0x8B08B20, "ladder.mko", 0, 32},
	{0x8B09AA4, "pz_ladder.mko", 0, 32},
	{0x8B0894C, "krypt.mko", 0, 16},
	{0x8B07E88, "earth_1.mko", 0, 4},
	{0x8B07E88, "earth_1.mko", 0, 4},
	{0x8B08140, "earth_2.mko", 0, 4},
	{0x8B08E98, "netherrealm_1.mko", 0, 4},
	{0x8B076A8, "chaosrealm_1.mko", 0, 4},
	{0x8B09828, "outworld_1.mko", 0, 4},
	{0x8B09534, "orderrealm_1.mko", 0, 4},
	{0x8B0844C, "edenia_1.mko", 0, 4},
	{0x8B09114, "nexus_1.mko", 0, 4},
	{0x8B09354, "nexus_nis.mko", 0, 0},
};

struct stage_select_entry pStageSelectNormal[] = {
	{0, "BGS_BEETLE", 6}, {19, "BGS_SLAUGHTER", 7},
	{18, "BGS_SKYTEMPLE", 7}, {21, "BGS_YINYANG", 1},
	{17, "BGS_REDDRAGON", 6}, {14, "BGS_NETHERBELLY", 2},
	{8, "BGS_FOUNDRY", 3}, {6, "BGS_CLIFF", 1},
	{12, "BGS_LOSTWARRIORS", 3}, {2, "BGS_PRISON", 3},
	{13, "BGS_MINES", 3}, {5, "BGS_DRAGON", 2},
	{9, "BGS_KANGTOMB", 4}, {1, "BGS_COURTYARD", 0},
	{16, "BGS_PORTAL", 0}, {20, "BGS_THEPIT", 1},
	{3, "BGS_DEADPOOL", 1}, {7, "BGS_FORTRESS", 1},
	{10, "BGS_KUATAN", 1}, {11, "BGS_FOREST", 0},
	{4, "BGS_DKA", 1}, {15, "BGS_NEXUS", 1},
	{BGS_EARTH_1, "NULL", 0}, {BGS_NETHERREALM, "NULL", 0},
	{BGS_CHAOSREALM, "NULL", 0}, {BGS_OUTWORLD, "NULL", 0},
	{BGS_ORDERREALM, "NULL", 0}, {BGS_EDENIA, "NULL", 0},
};

static int index_is_valid(int index, size_t count)
{
	return index >= 0 && (size_t)index < count;
}

static void stage_patch_short(unsigned int offset, unsigned short value)
{
	unsigned int target = (unsigned int)GetBaseAddr() + offset;
	LOG_Messagef("stage patch16 begin offset=%08X target=%08X value=%04X\n", offset, target, (unsigned int)value);
	PatchShort(offset, value);
	LOG_Messagef("stage patch16 done target=%08X\n", target);
}

static void stage_patch_int(unsigned int offset, unsigned int value)
{
	unsigned int target = (unsigned int)GetBaseAddr() + offset;
	LOG_Messagef("stage patch32 begin offset=%08X target=%08X value=%08X\n", offset, target, value);
	PatchInt(offset, value);
	LOG_Messagef("stage patch32 done target=%08X\n", target);
}

static int stage_patch_preconditions_are_valid(void)
{
	const uint32_t base = (uint32_t)GetBaseAddr();
	const uint32_t largest_patch_offset = 0x161C54 + 4;
	const uintptr_t jump_site = (uintptr_t)base + 0x160504;
	const uintptr_t jump_target = (uintptr_t)get_arena_name;

	LOG_Messagef("stage precheck base=%08X jump-site=%08X hook=%08X\n", base,
		(unsigned int)jump_site, (unsigned int)jump_target);

	if (base == 0 || (base & 3U) != 0)
		return 0;
	if (base > UINT32_MAX - largest_patch_offset)
		return 0;
	if ((jump_target & 3U) != 0)
		return 0;
	if (((jump_site + 4) & (uintptr_t)0xF0000000U) !=
	    (jump_target & (uintptr_t)0xF0000000U))
		return 0;
	return 1;
}

void dump_stage_table(unsigned int addr)
{
	int stage_addr = addr + 0x2E6210;
	LOG_Message("STAGE DATA\n");
	char msgBuffer[1256];
	for (int i = 0; i < 35; i++)
	{
		struct stage_info_entry stage = *(struct stage_info_entry*)(stage_addr + (sizeof(struct stage_info_entry) * i));
		sprintf(msgBuffer, "{0x%X\t, \"%s\"\t, %d\t,%d,},\n", stage.file_table, stage.scriptName, stage.stringID, stage.unk2);
		LOG_Message(msgBuffer);
	}
}

void dump_select_stable(unsigned int addr)
{
	int stage_addr = addr + 0x3121D0;
	char msgBuffer[1256];
	for (int i = 0; i < 22; i++)
	{
		struct stage_select_entry stage = *(struct stage_select_entry*)(stage_addr + (sizeof(struct stage_select_entry) * i));
		sprintf(msgBuffer, "{%d\t, \"%s\"\t, %d\t},\n", stage.id, stage.previewImage, stage.unk);
		LOG_Message(msgBuffer);
	}
}

void play_kon_music(void)
{
	int cur_bgnd = ReadInt(0x58E920);
	if (cur_bgnd >= BGS_EARTH_1 && cur_bgnd <= BGS_EDENIA)
	{
		int music = randu(24);
		set_music(music + 6991);
	}
}

char* get_arena_name(void)
{
	int pselect_mode = ReadInt(0x4173A8);
	int cur_bgnd = ReadInt(0x4173AC);
	int background = TOTAL_BACKGROUNDS;
	size_t stage_count = ARRAY_COUNT(pStageTable);

	if (pselect_mode > 0)
	{
		if (!index_is_valid(cur_bgnd, ARRAY_COUNT(pStageSelectNormal)))
		{
			LOG_Messagef("get_arena_name rejected cur_bgnd=%d mode=%d\n", cur_bgnd, pselect_mode);
			return NULL;
		}
		if (pselect_mode < 2)
			background = ReadInt(0x3122D8 + (12 * cur_bgnd));
		else if (pselect_mode < 3)
			background = ReadInt(0x312314 + (12 * cur_bgnd));
		else
			return NULL;
	}
	else if (pselect_mode == 0)
	{
		if (!index_is_valid(cur_bgnd, ARRAY_COUNT(pStageSelectNormal)))
		{
			LOG_Messagef("get_arena_name rejected cur_bgnd=%d mode=0\n", cur_bgnd);
			return NULL;
		}
		background = pStageSelectNormal[cur_bgnd].id;
	}
	else
		return NULL;

	if (background == TOTAL_BACKGROUNDS)
		return NULL;
	if (background == BGS_EARTH_1) return "EARTHREALM";
	if (background == BGS_EARTH_1_2) return "EARTH 2";
	if (background == BGS_EARTH_2) return "EARTH 3";
	if (background == BGS_NETHERREALM) return "NETHERREALM";
	if (background == BGS_CHAOSREALM) return "CHAOSRREALM";
	if (background == BGS_ORDERREALM) return "ORDERREALM";
	if (background == BGS_OUTWORLD) return "OUTWORLD";
	if (background == BGS_EDENIA) return "EDENIA";
	if (background == BGS_NEXUS) background = 33;
	else if (background == BGS_NEXUS_NIS) background = 34;

	if (!index_is_valid(background, stage_count))
	{
		LOG_Messagef("get_arena_name rejected background=%d stage-count=%u\n", background, (unsigned int)stage_count);
		return NULL;
	}
	return get_string_by_id(pStageTable[background].stringID | 0x10000);
}

void patch_stage_data(void)
{
	uint32_t val;
	uint32_t output;
	int selectSize;

	LOG_Message("stage patch sequence: begin\n");
	if (!stage_patch_preconditions_are_valid())
	{
		LOG_Message("stage patch sequence: rejected; no stage patches written\n");
		return;
	}

	LOG_Messagef("stage patch sequence base=%08X jump-site=%08X hook=%08X table=%08X select-table=%08X\n",
		(unsigned int)GetBaseAddr(),
		(unsigned int)((uintptr_t)GetBaseAddr() + 0x160504),
		(unsigned int)(uintptr_t)get_arena_name,
		(unsigned int)(uintptr_t)&pStageTable[0],
		(unsigned int)(uintptr_t)&pStageSelectNormal[0]);

	for (int i = 0; i < (int)ARRAY_COUNT(katakombs_entry_table); i++)
		katakombs_entry_table[i].belong = &katakombs_file_table[0];

	LOG_Message("stage jump: before MakeJump offset=00160504\n");
	MakeJump(0x160504, (unsigned int)(uintptr_t)get_arena_name);
	LOG_Message("stage jump: MakeJump returned\n");

	selectSize = (int)ARRAY_COUNT(pStageSelectNormal);
	stage_patch_short(0x160AC4, (unsigned short)selectSize);
	stage_patch_short(0x160D50, (unsigned short)selectSize);
	stage_patch_short(0x160F38, (unsigned short)selectSize);
	stage_patch_short(0x160D54, (unsigned short)selectSize);
	stage_patch_short(0x160C3C, (unsigned short)selectSize);
	stage_patch_short(0x24D2C, (unsigned short)(TOTAL_BACKGROUNDS + 1));
	stage_patch_short(0x7DA4C, (unsigned short)TOTAL_BACKGROUNDS);

	val = (uint32_t)(uintptr_t)&pStageTable[0];
	output = lui(a1, HIWORD(val)); stage_patch_int(0x1774C, output);
	output = ori(a1, a1, LOWORD(val)); stage_patch_int(0x17754, output);
	output = lui(a3, HIWORD(val)); stage_patch_int(0x24CF8, output);
	output = ori(a3, a3, LOWORD(val)); stage_patch_int(0x24D00, output);
	output = lui(a1, HIWORD(val)); stage_patch_int(0x25230, output);
	output = ori(a1, a1, LOWORD(val)); stage_patch_int(0x25238, output);
	output = lui(a1, HIWORD(val)); stage_patch_int(0x25284, output);
	output = ori(a1, a1, LOWORD(val)); stage_patch_int(0x2528C, output);
	output = lui(a0, HIWORD(val)); stage_patch_int(0xBCE98, output);
	output = ori(a0, a0, LOWORD(val)); stage_patch_int(0xBCEA0, output);
	output = lui(a1, HIWORD(val)); stage_patch_int(0x1605B8, output);
	output = ori(a1, a1, LOWORD(val)); stage_patch_int(0x1605B8 + 4, output);

	val = (uint32_t)(uintptr_t)&pStageSelectNormal[0];
	output = lui(s1, HIWORD(val)); stage_patch_int(0x160AC0, output);
	output = ori(s1, s1, LOWORD(val)); stage_patch_int(0x160ACC, output);
	output = lui(s7, HIWORD(val)); stage_patch_int(0x160D44, output);
	output = ori(s7, s7, LOWORD(val)); stage_patch_int(0x160D58, output);
	output = lui(a1, HIWORD(val)); stage_patch_int(0x160F34, output);
	output = ori(a1, a1, LOWORD(val)); stage_patch_int(0x160F40, output);
	output = lui(a3, HIWORD(val)); stage_patch_int(0x161C48, output);
	output = ori(a3, a3, LOWORD(val)); stage_patch_int(0x161C54, output);

	LOG_Message("stage patch sequence: complete\n");
}
