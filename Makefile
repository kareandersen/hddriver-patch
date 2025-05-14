CC      ?= gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2

SRC     = main.c endian_utils.c
OBJ     = $(SRC:.c=.o)

TARGET     = dmapatch
INPUT      = ./test/HDDRIVER.ORG
OUTPUT     = ./test/HDDRIVER.SYS
BUILD_DIR  = build

HD_IMG     ?= hd.img
CLEAN_IMG  ?= ../hd-clean.img
HATARI     ?= hatari-hrdb
TOS_ROM    ?= $(HOME)/ATARI/TOS/tos162uk.img
TIME       ?= $(shell date +%H%M%S)
DEBUG_SCRIPT = breakpoints.txt
BREAK_ADDR = 0xA8B2


BREAK_ADDR = 0xDA42

all: $(OUTPUT)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OUTPUT): $(TARGET) $(INPUT)
	mkdir -p output
	./$(TARGET) $(INPUT) $(OUTPUT)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: $(OUTPUT)
	echo "" > $(DEBUG_SCRIPT)
	echo 'a $(BREAK_ADDR)' >> $(DEBUG_SCRIPT)
	$(HATARI) \
		--fast-boot TRUE --fast-forward TRUE --timer-d TRUE \
		-w -m --machine ste --tos $(TOS_ROM) \
		--harddrive "$(shell pwd)/output/" \
		--parse $(DEBUG_SCRIPT) \
		--confirm-quit false

deploy: $(OUTPUT)
	echo "" > $(DEBUG_SCRIPT)
	echo 'a $(BREAK_ADDR)' >> $(DEBUG_SCRIPT)
	cp -f $(CLEAN_HD_IMG) $(HD_IMG)
	mkdir -p output/auto
	cp copier/COPY.PRG output/
	cp copier/DUMMY.PRG output/
	$(HATARI) \
		--fast-boot TRUE --fast-forward TRUE --timer-d TRUE \
		-w -m --machine ste --tos $(TOS_ROM) \
		--harddrive "$(shell pwd)/output/" --gemdos-drive G \
		--acsi 0=$(HD_IMG) \
		--parse $(DEBUG_SCRIPT) \
		--confirm-quit false \
		--auto G:COPY.PRG

clean:
	rm -f $(OBJ) $(TARGET) $(DEBUG_SCRIPT) $(OUTPUT)

