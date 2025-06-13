CC		= cc
CFLAGS		= -Wall -pedantic -O2
LDFLAGS		= -lpthread -lm
PREFIX		= /usr/local
MANDIR		= $(PREFIX)/share/man
OBJFILES	= main.o stb_body.o
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

threadless: clean
threadless: LDFLAGS = -lm
threadless: CFLAGS += -DDIF_DISABLE_THREADING
threadless: all

clean:
	rm -f $(OBJFILES) $(TARGET)

help:
	@echo "Duplicate Image Detection Program Build Options:"
	@echo ""
	@echo "make            : Builds the program normally"
	@echo "make debug      : Builds with ASan and more warnings"
	@echo "make clean      : Removes object files and target"
	@echo "make rebuild    : Calls clean than builds"
	@echo "make threadless : Builds without pthread multi-threading"
	@echo "make help       : Prints this message"
	@echo ""

.PHONY: rebuild threadless clean help
