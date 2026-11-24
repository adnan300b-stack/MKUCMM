#include "mkdeception.h"

/* Storage for the game function pointers declared in mkdeception.h. */
void (*mk_sprintf)(char* ptr, char* fmt, ...);

void (*load_ssf)(struct mem_ssf_header* ssf);
void (*load_font)(int id);
int (*load_script_data)(int id, char* name);
void (*func_6F388)(int unk, int id);

void (*p_pselect)(void);
void (*p_game_loop)(void);
int (*create_mkproc)(int id, int unk, int func, int unk2, int unk3);
void (*gamelogic_jump)(int mode, int function);

void (*fire_screen_event)(int id, int param);
void (*refresh_screen)(void);
void (*wait_for_screen_close)(void);
int (*load_named_2d_pfxobj_xy)(int slot, int id, char* name, int orientation, int x, int y, int unk);

int (*string_right_xy)(int id, int font, char* text, int y, int x, int unk);
int (*string_left_xy)(int id, int font, char* text, int y, int x, int unk);
int (*load_hero_model)(int ptr);
int (*create_plr)(int plr, struct player_info* obj);
int (*is_char_locked)(int id, int param);
int (*is_bgnd_locked)(int id);
int (*translate_id_from_pz)(int id);
void (*snd_req)(int id);
int (*set_music)(int id);
void (*sub_7F050)(void);
int (*randu)(int range);
int (*bgnd_unhide_sobj)(int id);
char* (*get_string_by_id)(int id);

void (*render)(void);
int (*get_game_state)(void);
int (*pressed_button)(int plr, int id);
void (*impale_player)(void);

int (*RwRasterCreate)(int x, int y, int bits, int mode);
