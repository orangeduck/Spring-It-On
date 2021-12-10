PLATFORM ?= PLATFORM_DESKTOP

RAYLIB_DIR = C:/raylib
INCLUDE_DIR = -I ./ -I $(RAYLIB_DIR)/raylib/src -I $(RAYLIB_DIR)/raygui/src
LIBRARY_DIR = -L $(RAYLIB_DIR)/raylib/src
DEFINES =  -D _DEFAULT_SOURCE -D RAYLIB_BUILD_MODE=RELEASE -D $(PLATFORM)

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    CC = g++
    EXT = .exe
    CFLAGS ?= $(DEFINES) -O3 $(RAYLIB_DIR)/raylib/src/raylib.rc.data $(INCLUDE_DIR) $(LIBRARY_DIR) 
    LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
endif

.PHONY: all

SOURCE = \
    damper.c \
    springdamper.c \
    smoothing.c \
    controller.c \
    inertialization.c \
    interpolation.c \
    resonance.c \
    timedspring.c \
    velocityspring.c \
    doublespring.c
	
EXECUTABLE = $(SOURCE:.c=$(EXT))

all: $(EXECUTABLE)

%$(EXT): %.c
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) 

clean:
	rm $(EXECUTABLE)