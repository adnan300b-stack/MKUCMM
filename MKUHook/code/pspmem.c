#include "pspsdk.h"
#include "pspkernel.h"
#include "pspmem.h"
#include "eLog.h"
#include <stdint.h>

static uint32_t pBaseAddress;

static uint32_t game_address(unsigned int offset)
{
	return pBaseAddress + (uint32_t)offset;
}

/* MIPS control-transfer instructions have a delay slot at the following word. */
static int is_control_transfer(uint32_t instruction)
{
	uint32_t opcode = instruction >> 26;
	uint32_t function = instruction & 0x3FU;
	uint32_t rs = (instruction >> 21) & 0x1FU;

	if (opcode == 0x00 && (function == 0x08 || function == 0x09)) /* JR/JALR */
		return 1;
	if (opcode == 0x02 || opcode == 0x03) /* J/JAL */
		return 1;
	if (opcode == 0x01 || (opcode >= 0x04 && opcode <= 0x07) ||
	    (opcode >= 0x14 && opcode <= 0x17)) /* REGIMM, branch and likely */
		return 1;
	if ((opcode == 0x10 || opcode == 0x11 || opcode == 0x12) && rs == 0x08)
		return 1; /* BC0/BC1/BC2 */

	return 0;
}

/* A patch at N replaces a delay-slot instruction if N-4 is a branch/jump. */
static int patch_site_is_delay_slot(unsigned int offset, uint32_t* previous_instruction)
{
	uint32_t instruction;

	if (offset < sizeof(uint32_t))
		return 0;

	instruction = (uint32_t)_lw(game_address(offset - sizeof(uint32_t)));
	if (previous_instruction != NULL)
		*previous_instruction = instruction;
	return is_control_transfer(instruction);
}

static int jump_region_is_valid(unsigned int offset, unsigned int target)
{
	uint32_t pc_plus_4 = game_address(offset) + 4U;
	return ((pc_plus_4 ^ target) & 0xF0000000U) == 0;
}

void PSPPatcher_Init(int base)
{
	pBaseAddress = (uint32_t)base;
	LOG_Messagef("PSPPatcher_Init: game base=%08X\n", (unsigned int)pBaseAddress);
}

int GetBaseAddr(void)
{
	return (int)pBaseAddress;
}

void PatchInt(unsigned int addr, unsigned int value)
{
	_sw(value, game_address(addr));
}

void PatchShort(unsigned int addr, unsigned short value)
{
	_sh(value, game_address(addr));
}

void PatchChar(unsigned int addr, unsigned char value)
{
	_sb(value, game_address(addr));
}

void MakeCall(unsigned int addr, unsigned int func)
{
	uint32_t previous_instruction = 0;
	uint32_t site = game_address(addr);

	if ((site & 3U) != 0 || (func & 3U) != 0) {
		LOG_Messagef("MakeCall rejected: unaligned site=%08X target=%08X\n", site, func);
		return;
	}
	if (patch_site_is_delay_slot(addr, &previous_instruction)) {
		LOG_Messagef("MakeCall rejected: delay-slot site=%08X previous=%08X\n", site, previous_instruction);
		return;
	}
	if (!jump_region_is_valid(addr, func)) {
		LOG_Messagef("MakeCall rejected: JAL region mismatch site=%08X target=%08X\n", site, func);
		return;
	}

	_sw(0x0C000000U | ((func >> 2) & 0x03FFFFFFU), site);
	LOG_Messagef("MakeCall installed: site=%08X target=%08X\n", site, func);
}

void MakeJump(unsigned int addr, unsigned int func)
{
	uint32_t previous_instruction = 0;
	uint32_t site = game_address(addr);

	if ((site & 3U) != 0 || (func & 3U) != 0) {
		LOG_Messagef("MakeJump rejected: unaligned site=%08X target=%08X\n", site, func);
		return;
	}
	if (patch_site_is_delay_slot(addr, &previous_instruction)) {
		LOG_Messagef("MakeJump rejected: delay-slot site=%08X previous=%08X\n", site, previous_instruction);
		return;
	}
	if (!jump_region_is_valid(addr, func)) {
		LOG_Messagef("MakeJump rejected: J region mismatch site=%08X target=%08X\n", site, func);
		return;
	}

	_sw(0x08000000U | ((func & 0x0FFFFFFCU) >> 2), site);
	LOG_Messagef("MakeJump installed: site=%08X target=%08X\n", site, func);
}

void MakeLUI(unsigned int addr, unsigned int value)
{
	_sw(0x3C000000U | (value >> 16), game_address(addr));
}

void MakeADDIU(unsigned int addr, unsigned int value)
{
	_sw(0x24000000U | value, game_address(addr));
}

void Nop(unsigned int addr)
{
	PatchInt(addr, 0);
}

int ReadInt(unsigned int addr)
{
	return _lw(game_address(addr));
}

short ReadShort(unsigned int addr)
{
	return _lh(game_address(addr));
}

char ReadChar(unsigned int addr)
{
	return _lb(game_address(addr));
}
