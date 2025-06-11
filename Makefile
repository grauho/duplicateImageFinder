CC		= cc
CFLAGS		= -Wall -pedantic -O2
LDFLAGS		= -lm
PREFIX		= /usr/local
MANDIR		= $(PREFIX)/share/man
OBJFILES	= main.o stb_body.o
TARGET		= a.out 

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

rebuild: clean
rebuild: all

clean:
	rm -f $(OBJFILES) $(TARGET)

.PHONY: rebuild clean
