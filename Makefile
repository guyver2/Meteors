TARGET = detect
OBJS = main.o color.o bmp.o chotto.o

INCDIR = 
CFLAGS = -O3 -G4 -Wall -I/usr/local/pspdev/psp/include/SDL
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION=271

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin

LIBDIR =
LDFLAGS =
STDLIBS = -lSDL_mixer -lSDLmain -lSDL_image -lSDL -lpng -ljpeg -lm -lz \
	-lvorbisfile -lvorbis  -logg \
	-lpspsdk -lpspctrl  -lpsprtc -lpsppower -lpspgu -lpspaudiolib -lpspaudio -lpsphprm\
	-lpspusb -lpsputility -lpspusbcam -lpspjpeg

LIBS=$(shell $(PSPBIN)/sdl-config --libs) $(STDLIBS) $(YOURLIBS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Evite moi ca.
PSP_EBOOT_ICON = ICON0.png


PSPSDK=$(shell psp-config --pspsdk-path)
DEFAULT_CFLAGS = $(shell $(SDL_CONFIG) --cflags)
include $(PSPSDK)/lib/build.mak
