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

threadless: clean
threadless: LDFLAGS = -lm
threadless: CFLAGS += -DFOO_DISABLE_THREADING
threadless: all

clean:
	rm -f $(OBJFILES) $(TARGET)

help:
	@echo "Duplicate Image Detection Program Build Options:"
	@echo ""
	@echo "make            : Builds the program normally"
	@echo "make clean      : Removes object files and target"
	@echo "make rebuild    : Calls clean than builds"
	@echo "make threadless : Builds without pthread multi-threading"
	@echo "make help       : Prints this message"
	@echo ""

.PHONY: rebuild threadless clean help
