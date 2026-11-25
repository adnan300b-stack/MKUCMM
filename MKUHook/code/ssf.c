#include "ssf.h"
#include "mkdeception.h"
#include "pspmem.h"
#include "eLog.h"

#include <pspsdk.h>
#include <pspkernel.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define PSP_USER_MIN 0x08000000U
#define PSP_USER_MAX 0x0C000000U
#define SSF_ENTRY_LIMIT 250
#define SSF_NAME_LIMIT 120

char ssfLogBuffer[256];

/* Range/alignment/overflow checks protect arithmetic and reject bogus user
 * pointers. PSP SDK does not expose a portable non-faulting probe for every
 * user mapping, so this cannot prove that a page within the range is mapped. */
static int ssf_user_range(uintptr_t address, size_t length, size_t alignment)
{
	if (address == 0 || length == 0 || alignment == 0 ||
	    (address & (alignment - 1U)) != 0 ||
	    address < PSP_USER_MIN || address >= PSP_USER_MAX ||
	    length > PSP_USER_MAX - address)
		return 0;
	return 1;
}

static int ssf_copy_name(char *dst, size_t dst_size, const char *src)
{
	uintptr_t address = (uintptr_t)src;
	size_t i;

	if (dst == NULL || dst_size < 2 || !ssf_user_range(address, 1, 1))
		return 0;
	for (i = 0; i + 1 < dst_size; ++i) {
		uintptr_t current = address + i;
		if (current < address || !ssf_user_range(current, 1, 1))
			return 0;
		dst[i] = src[i];
		if (dst[i] == '\0')
			return i > 0;
	}
	dst[dst_size - 1] = '\0';
	return 0; /* Refuse truncated names rather than parse an incomplete path. */
}

void hook_load_ssf(struct mem_ssf_header *ptr)
{
	uintptr_t header_address = (uintptr_t)ptr;
	char source_name[SSF_NAME_LIMIT];
	char new_name[SSF_NAME_LIMIT];
	int dump_uid = -1;
	int count;
	int i;

	LOG_Messagef("Entering hook_load_ssf header=%08X\n", (unsigned int)header_address);
	if (ptr == NULL || !ssf_user_range(header_address, sizeof(*ptr), 4)) {
		LOG_Messagef("ERROR: hook_load_ssf rejected invalid/truncated SSF header=%08X size=%u\n",
			(unsigned int)header_address, (unsigned int)sizeof(*ptr));
		return;
	}
	/* SSF names are pointers, not pointers-to-pointer values. Validate and copy
	 * into bounded local buffers before using them in formatting/path logic. */
	if (!ssf_copy_name(source_name, sizeof(source_name), ptr->fileName)) {
		LOG_Messagef("ERROR: hook_load_ssf rejected invalid SSF filename pointer=%08X\n",
			(unsigned int)(uintptr_t)ptr->fileName);
		return;
	}
	LOG_Messagef("INFO: Loading SSF %s Files:\n", source_name);

	/* Keep the logger's optional file-dump behavior bounded. Do not remove an
	 * extension unless the copied name is long enough to contain one. */
	if (strlen(source_name) >= 5) {
		memcpy(new_name, source_name, strlen(source_name) + 1);
		new_name[strlen(new_name) - 4] = '\0';
		if (snprintf(ssfLogBuffer, sizeof(ssfLogBuffer),
			"ms0:/PSP/PLUGINS/mkuhook/ssf/%s.txt", new_name) < (int)sizeof(ssfLogBuffer)) {
			SceIoStat info;
			int status = sceIoGetstat(ssfLogBuffer, &info);
			if (status < 0)
				dump_uid = sceIoOpen(ssfLogBuffer,
					PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
		}
	}

	for (i = 0; i < SSF_ENTRY_LIMIT; ++i) {
		uintptr_t entry_address = header_address + offsetof(struct mem_ssf_header, files) +
			(size_t)i * sizeof(struct mem_ssf_entry);
		struct mem_ssf_entry *entry;
		char entry_name[SSF_NAME_LIMIT];

		if (entry_address < header_address ||
		    !ssf_user_range(entry_address, sizeof(struct mem_ssf_entry), 4)) {
			LOG_Messagef("ERROR: SSF entry address overflow/out of range index=%d addr=%08X\n",
				i, (unsigned int)entry_address);
			break;
		}
		entry = &ptr->files[i];
		if (entry->fileName == NULL && entry->fileSize == 0 && entry->totalFileSize == 0)
			break;
		if (entry->fileSize < 0 || entry->totalFileSize < 0 ||
		    entry->fileSize > entry->totalFileSize) {
			LOG_Messagef("ERROR: malformed SSF entry index=%d fileSize=%d total=%d\n",
				i, entry->fileSize, entry->totalFileSize);
			break;
		}
		if (!ssf_copy_name(entry_name, sizeof(entry_name), entry->fileName)) {
			LOG_Messagef("ERROR: malformed SSF entry name index=%d pointer=%08X\n",
				i, (unsigned int)(uintptr_t)entry->fileName);
			break;
		}
		if (dump_uid >= 0) {
			int written = snprintf(ssfLogBuffer, sizeof(ssfLogBuffer), "%s\n", entry_name);
			if (written < 0 || written >= (int)sizeof(ssfLogBuffer)) {
				LOG_Messagef("ERROR: SSF entry name too long index=%d\n", i);
				break;
			}
			if (sceIoWrite(dump_uid, ssfLogBuffer, (SceSize)written) < 0)
				LOG_Messagef("ERROR: failed writing SSF diagnostic entry index=%d\n", i);
		}
	}
	if (dump_uid >= 0)
		sceIoClose(dump_uid);

	/* The callback guard checks the expected ULUS10102 function address and
	 * alignment before the indirect call. */
	load_ssf(ptr);
}
