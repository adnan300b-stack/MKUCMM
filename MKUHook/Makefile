TARGET = mkuhook
OBJS = main.o exports.o minIni.o code/pspmem.o code/mkdeception.o code/game_functions.o code/eLog.o code/mkuhook.o code/eSettingsMgr.o code/mkuplrinfo.o code/stage.o code/mips.o code/injector.o code/debug_hooks.o

# Debug build: retain source-level symbols and disable optimization.
CFLAGS = -g -O0 -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS += -g

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

LIBS = -lpspsystemctrl_user

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
