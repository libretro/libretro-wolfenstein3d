CONFIG ?= config.default
-include $(CONFIG)


BINARY    ?= Chocolate-Wolfenstein-3D
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)
UNAME := $(shell uname -s)

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 555 -s
INSTALL_MAN     ?= $(INSTALL) -m 444
INSTALL_DATA    ?= $(INSTALL) -m 444


SDL_CONFIG  ?= sdl-config
CFLAGS_SDL  ?= $(shell $(SDL_CONFIG) --cflags)
LDFLAGS_SDL ?= $(shell $(SDL_CONFIG) --libs)


CFLAGS += $(CFLAGS_SDL)

#CFLAGS += -Wall
#CFLAGS += -W
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings
CFLAGS += -Wcast-align
CFLAGS += -O0

CCFLAGS += $(CFLAGS)
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wsequence-point -Iinclude -ISDL_mixer -Isdl/include

CXXFLAGS += $(CFLAGS)

LDFLAGS += $(LDFLAGS_SDL) -lm

SRCS :=
SRCS += fmopl.cpp
SRCS += id_ca.cpp
SRCS += id_in.cpp
SRCS += id_pm.cpp
SRCS += id_sd.cpp
SRCS += id_us_1.cpp
SRCS += id_vh.cpp
SRCS += id_vl.cpp
SRCS += id_libretro.cpp
SRCS += signon.cpp
SRCS += wl_act1.cpp
SRCS += wl_act2.cpp
SRCS += wl_agent.cpp
SRCS += wl_debug.cpp
SRCS += wl_draw.cpp
SRCS += wl_game.cpp
SRCS += wl_inter.cpp
SRCS += wl_main.cpp
SRCS += wl_menu.cpp
SRCS += wl_play.cpp
SRCS += wl_state.cpp
SRCS += wl_text.cpp
SRCS += surface.cpp
SRCS += SDL_mixer/mixer.cpp
SRCS += SDL_mixer/music.cpp

SRCS += sdl/SDL_error.cpp \
		  sdl/file/SDL_rwops.cpp \
		  sdl/video/SDL_blit.cpp \
		  sdl/video/SDL_blit_0.cpp \
		  sdl/video/SDL_blit_1.cpp \
		  sdl/video/SDL_blit_A.cpp \
		  sdl/video/SDL_blit_N.cpp \
		  sdl/video/SDL_bmp.cpp \
		  sdl/video/SDL_pixels.cpp \
		  sdl/video/SDL_surface.cpp

OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o))

.SUFFIXES:
.SUFFIXES: .c .cpp .o

Q ?= @

all: $(BINARY)

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CCFLAGS) -c $< -o $@

.cpp.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(OBJS) $(BINARY)

install: $(BINARY)
	@echo '===> INSTALL'
	$(Q)$(INSTALL) -d $(PREFIX)/bin
	$(Q)$(INSTALL_PROGRAM) $(BINARY) $(PREFIX)/bin
