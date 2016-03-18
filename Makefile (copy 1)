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

#CFLAGS += -Wall
#CFLAGS += -W
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings
CFLAGS += -Wcast-align

CCFLAGS += $(CFLAGS)
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wsequence-point -Iinclude -ISDL_mixer -ISDL1.3/include

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

SRCS += SDL1.3/src/SDL.cpp
SRCS += SDL1.3/src/SDL_assert.cpp
SRCS += SDL1.3/src/SDL_error.cpp
SRCS += SDL1.3/src/SDL_log.cpp

SRCS += SDL1.3/src/stdlib/SDL_malloc.cpp
SRCS += SDL1.3/src/stdlib/SDL_getenv.cpp
SRCS += SDL1.3/src/stdlib/SDL_stdlib.cpp
SRCS += SDL1.3/src/stdlib/SDL_string.cpp

SRCS += SDL1.3/src/events/SDL_events.cpp

SRCS += SDL1.3/src/joystick/SDL_joystick.cpp

SRCS += SDL1.3/src/atomic/SDL_atomic.cpp
SRCS += SDL1.3/src/atomic/SDL_spinlock.cpp

SRCS += SDL1.3/src/video/SDL_blit.cpp
SRCS += SDL1.3/src/video/SDL_blit_0.cpp
SRCS += SDL1.3/src/video/SDL_blit_1.cpp
SRCS += SDL1.3/src/video/SDL_blit_A.cpp
SRCS += SDL1.3/src/video/SDL_blit_N.cpp
SRCS += SDL1.3/src/video/SDL_surface.cpp
SRCS += SDL1.3/src/video/SDL_fillrect.cpp
SRCS += SDL1.3/src/video/SDL_rect.cpp
SRCS += SDL1.3/src/video/SDL_RLEaccel.cpp
SRCS += SDL1.3/src/video/SDL_pixels.cpp
SRCS += SDL1.3/src/video/SDL_stretch.cpp
SRCS += SDL1.3/src/video/SDL_blit_auto.cpp
SRCS += SDL1.3/src/video/SDL_blit_copy.cpp
SRCS += SDL1.3/src/video/SDL_blit_slow.cpp

SRCS += SDL1.3/src/audio/SDL_audio.cpp
SRCS += SDL1.3/src/audio/SDL_audiocvt.cpp
SRCS += SDL1.3/src/audio/SDL_mixer.cpp
SRCS += SDL1.3/src/audio/SDL_wave.cpp

SRCS += SDL_mixer/mixer.cpp
SRCS += SDL_mixer/music.cpp

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
