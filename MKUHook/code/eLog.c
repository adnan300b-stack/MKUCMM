#include "eLog.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <pspsdk.h>
#include <pspkernel.h>

void LOG_Initialise(void)
{
	int fd = sceIoOpen("ms0:/PSP/PLUGINS/mkuhook/log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd >= 0)
		sceIoClose(fd);

	LOG_Message("INFO: MKUHook debug logging initialized\n");
}

void LOG_Message(const char* message)
{
	int fd;
	size_t length;

	if (message == NULL)
		return;

	fd = sceIoOpen("ms0:/PSP/PLUGINS/mkuhook/log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
	if (fd < 0)
		return;

	length = strlen(message);
	if (length != 0)
		sceIoWrite(fd, message, length);
	sceIoClose(fd);
}

void LOG_Messagef(const char* format, ...)
{
	char buffer[256];
	va_list args;

	if (format == NULL)
		return;

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	LOG_Message(buffer);
}
