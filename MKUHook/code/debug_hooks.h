#pragma once

/* Diagnostic wrappers around hooks which call game function pointers. */
int debug_hook_character_lock_status(int id, int param);
int debug_hook_stage_lock_status(int id);
void debug_play_kon_music(void);
