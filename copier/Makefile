CC      = m68k-atari-tos-gnu-gcc
LD      = m68k-atari-tos-gnu-ld
CFLAGS  = -std=c99 -Wall -Wextra -O2

OBJS	= autocopy.o
TARGET 	= AUTOCOPY.TOS

.PHONY:
	all clean

all: $(TARGET)

$(PRG): $(OBJS)
	$(LD) $(LIBS) -o $@ $^

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(TARGET) $(OBJS)

