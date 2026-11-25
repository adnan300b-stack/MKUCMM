#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <systemctrl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "code/config.h"
#include "code/pspmem.h"
#include "code/mkdeception.h"
#include "code/mkuhook.h"
#include "code/ssf.h"
#include "code/eLog.h"
#include "code/mkuplrinfo.h"
#include "code/eSettingsMgr.h"
#include "code/stage.h"
#include "code/debug_hooks.h"
#include "code/injector.h"

#include "code/mips.h"

PSP_MODULE_INFO(MODULE_NAME, 0, 1, 0);
static STMOD_HANDLER previous;
SceUID thid;
u32 mod_text_addr;

void _exit(int status)
{
	sceKernelExitDeleteThread(status);
	for (;;) { }
}

static void log_make_call(const char* label, unsigned int offset, uintptr_t target)
{
	LOG_Messagef("Init: before MakeCall %-28s site=%08X target=%08X\n",
		label, (unsigned int)((uintptr_t)mod_text_addr + offset), (unsigned int)target);
	MakeCall(offset, (unsigned int)target);
	LOG_Messagef("Init: MakeCall returned %-28s site=%08X\n",
		label, (unsigned int)((uintptr_t)mod_text_addr + offset));
}

int GenericTrueReturn() { return 1; }
int GenericFalseReturn() { return 0; }
void GenericDummy() { }

int Init(u32 addr)
{
	LOG_Initialise();
	LOG_Messagef("Init: begin game-base=%08X\n", addr);
	PSPPatcher_Init(addr);
	/* The injector table is also used by diagnostic hooks. These hooks use the
	 * game's absolute base directly and do not rely on a guessed image size. */
	injector.base_addr = (uintptr_t)addr;
	MKDeception_Init(addr);
	SettingsMgr_Init();
	dump_select_stable(addr);
	calculate_custom_toc();

	if (SettingsMgr_Get().bUsePuzzleModifierInNormalGame)
		Nop(0x153DD4);
	if (SettingsMgr_Get().bHideFighterFaceInLifebar)
		PatchChar(0x2CECE0, 0x00);

	log_make_call("hook_load_hero_model/A0DB4", 0xA0DB4, (uintptr_t)hook_load_hero_model);
	log_make_call("hook_load_hero_model/1C0710", 0x1C0710, (uintptr_t)hook_load_hero_model);
	log_make_call("character_lock/6F7F4", 0x6F7F4, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/BD370", 0xBD370, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/BEADC", 0xBEADC, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/CE368", 0xCE368, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/F0528", 0xF0528, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/F0748", 0xF0748, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/14EF30", 0x14EF30, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/153A04", 0x153A04, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/15EE10", 0x15EE10, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/15EE78", 0x15EE78, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/15F1D0", 0x15F1D0, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/15F838", 0x15F838, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/15FDBC", 0x15FDBC, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/16072C", 0x16072C, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/1627F0", 0x1627F0, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("character_lock/163938", 0x163938, (uintptr_t)debug_hook_character_lock_status);
	log_make_call("get_char_name/13FF04", 0x13FF04, (uintptr_t)get_char_name);
	log_make_call("get_char_name/13FF14", 0x13FF14, (uintptr_t)get_char_name);
	log_make_call("movelist_name/13FC38", 0x13FC38, (uintptr_t)movelist_get_char_name);

	PatchShort(0x153994, TOTAL_CHARACTERS);
	PatchShort(0x1539C8, TOTAL_CHARACTERS);
	PatchShort(0x150698, TOTAL_CHARACTERS);
	PatchShort(0x1503A8, TOTAL_CHARACTERS);

	log_make_call("hook_unk_func/6F2A8", 0x6F2A8, (uintptr_t)hook_unk_func);
	log_make_call("hook_unk_func/6F2CC", 0x6F2CC, (uintptr_t)hook_unk_func);
	log_make_call("hook_unk_func/6F6A8", 0x6F6A8, (uintptr_t)hook_unk_func);
	log_make_call("unk_load_ssf/6F584", 0x6F584, (uintptr_t)unk_load_ssf);
	log_make_call("unk_load_ssf/6F3E8", 0x6F3E8, (uintptr_t)unk_load_ssf);
	log_make_call("unk_load_script/6F3F4", 0x6F3F4, (uintptr_t)unk_load_script_data);
	log_make_call("load_ssf_alt/153B00", 0x153B00, (uintptr_t)custom_load_ssf_alt);
	log_make_call("load_ssf/153B1C", 0x153B1C, (uintptr_t)custom_load_ssf);
	log_make_call("load_ssf_async/1506A8", 0x1506A8, (uintptr_t)custom_load_ssf_async);
	log_make_call("load_ssf_async/150730", 0x150730, (uintptr_t)custom_load_ssf_async);
	log_make_call("create_player/70990", 0x70990, (uintptr_t)custom_create_player);
	log_make_call("create_player/BCE4C", 0xBCE4C, (uintptr_t)custom_create_player);
	log_make_call("create_player/14EFAC", 0x14EFAC, (uintptr_t)custom_create_player);
	log_make_call("create_player/14EFC0", 0x14EFC0, (uintptr_t)custom_create_player);
	log_make_call("load_script/150424", 0x150424, (uintptr_t)custom_load_script_data);
	log_make_call("load_script_async/1506B4", 0x1506B4, (uintptr_t)custom_load_script_data_async);
	log_make_call("player1_name/16824C", 0x16824C, (uintptr_t)player1_name);
	log_make_call("player2_name/16836C", 0x16836C, (uintptr_t)player2_name);
	log_make_call("set_win_name/82B04", 0x82B04, (uintptr_t)set_win_name);
	log_make_call("hook_render/C0CFC", 0xC0CFC, (uintptr_t)hook_render);
	log_make_call("hook_render/C0D2C", 0xC0D2C, (uintptr_t)hook_render);

	LOG_Message("Init: before patch_stage_data\n");
	patch_stage_data();
	LOG_Message("Init: patch_stage_data returned; installing remaining hooks\n");
	log_make_call("stage_lock/24BEC", 0x24BEC, (uintptr_t)debug_hook_stage_lock_status);
	log_make_call("stage_lock/BD318", 0xBD318, (uintptr_t)debug_hook_stage_lock_status);
	log_make_call("stage_lock/160E4C", 0x160E4C, (uintptr_t)debug_hook_stage_lock_status);
	log_make_call("play_kon_music/7F7FC", 0x7F7FC, (uintptr_t)debug_play_kon_music);
	LOG_Message("Init: all patches installed; returning to module loader\n");
	return 0;
}

int OnModuleStart(SceModule* mod)
{
	LOG_Messagef("OnModuleStart: module=%s text=%08X\n", mod->modname, mod->text_addr);
	if (strcmp(mod->modname, TARGET_GAME) == 0) {
		mod_text_addr = mod->text_addr;
		if (Init(mod_text_addr) != 0)
			return -1;
		LOG_Message("OnModuleStart: Init returned; flushing caches\n");
		sceKernelDcacheWritebackAll();
		sceKernelIcacheInvalidateAll();
		LOG_Message("OnModuleStart: cache maintenance returned\n");
	}
	if (previous == NULL)
		return 0;
	LOG_Message("OnModuleStart: calling previous module-start handler\n");
	return previous(mod);
}

int module_start(SceSize argc, void* argp)
{
	(void)argc;
	(void)argp;
	LOG_Message("module_start: entered\n");
	if (sceIoDevctl("kemulator:", 0x00000003, NULL, 0, NULL, 0) == 0) {
		SceUID modules[10];
		int count = 0;
		if (sceKernelGetModuleIdList(modules, sizeof(modules), &count) >= 0) {
			SceKernelModuleInfo info;
			for (int i = 0; i < count; ++i) {
				info.size = sizeof(info);
				if (sceKernelQueryModuleInfo(modules[i], &info) < 0)
					continue;
				if (strcmp(info.name, TARGET_GAME) == 0) {
					mod_text_addr = info.text_addr;
					LOG_Messagef("module_start: target=%s text=%08X; calling Init\n", info.name, mod_text_addr);
					if (Init(mod_text_addr) != 0)
						return 0;
					LOG_Message("module_start: Init returned; flushing caches\n");
					sceKernelDcacheWritebackAll();
					sceKernelIcacheInvalidateAll();
					LOG_Message("module_start: cache maintenance returned\n");
					return 0;
				}
			}
		}
	} else {
		LOG_Message("module_start: PSP path; registering start-module handler\n");
		previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	}
	LOG_Message("module_start: returning\n");
	return 0;
}
