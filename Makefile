CC		= cc
CFLAGS		= -Wall -pedantic -O2
LDFLAGS		= -lpthread -lm 
PREFIX		= /usr/local
MANDIR		= $(PREFIX)/share/man
OBJFILES	= main.o stb_body.o imageHandling.o
TARGET		= difDemo

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

rebuild: clean
rebuild: all

debug: CFLAGS = -Wall -pg -Wextra -Wpedantic -ggdb -Og 
debug: CFLAGS += -fsanitize=address -fsanitize=leak 
debug: CFLAGS += -fsanitize=undefined
debug: CFLAGS += -Wdouble-promotion -Wformat -Wformat-overflow
debug: CFLAGS += -Wnull-dereference -Winfinite-recursion
debug: CFLAGS += -Wstrict-overflow -Wno-unused-function -Wconversion 
debug: all

magick: CFLAGS += `MagickCore-config --cflags`
magick: CFLAGS += -DDIF_USE_IMAGEMAGICK
magick: LDFLAGS += `MagickCore-config --ldflags --libs`
magick: all

magick-debug: CFLAGS = -Wall -pg -Wextra -Wpedantic -ggdb -Og 
magick-debug: CFLAGS += -fsanitize=address -fsanitize=leak 
magick-debug: CFLAGS += -fsanitize=undefined
magick-debug: CFLAGS += -Wdouble-promotion -Wformat -Wformat-overflow
magick-debug: CFLAGS += -Wnull-dereference -Winfinite-recursion
magick-debug: CFLAGS += -Wstrict-overflow -Wno-unused-function -Wconversion 
magick-debug: magick

threadless: clean
threadless: LDFLAGS = -lm
threadless: CFLAGS += -DDIF_DISABLE_THREADING
threadless: all

clean:
	rm -f $(OBJFILES) $(TARGET)

help:
	@echo "Duplicate Image Detection Program Build Options:"
	@echo ""
	@echo "make              : Builds the program normally"
	@echo "make debug        : Builds with ASan and more warnings"
	@echo "make clean        : Removes object files and target"
	@echo "make rebuild      : Calls clean than builds"
	@echo "make threadless   : Builds without pthread multi-threading"
	@echo "make magick       : Builds with ImageMagick instead of stb"
	@echo "make magick-debug : As above but with ASAN and more warnings"
	@echo "make help         : Prints this message"
	@echo ""

.PHONY: debug clean rebuild threadless magick magick-debug help
