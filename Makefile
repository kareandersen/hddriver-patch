CC      = m68k-atari-tos-gnu-gcc
LD      = m68k-atari-tos-gnu-ld
CFLAGS  = -std=c99 -Wall -Wextra -O2

OBJS		= main.o endian_utils.o

TARGET     	= dmapatch
PRG			= DMAPATCH.PRG
INPUT      	= ./test/HDDRIVER.ORG
OUTPUT     	= ./test/HDDRIVER.SYS
BUILD_DIR  	= build

HD_IMG     	?= hd.img
CLEAN_IMG	?= hd-clean.img
HATARI     	?= hatari-hrdb
TOS_ROM    	?= $(HOME)/ATARI/TOS/tos206uk.img
TIME       	?= $(shell date +%H%M%S)
DEBUG_SCRIPT = breakpoints.txt
#BREAK_ADDR = 0xA8B2
BREAK_ADDR 	= 0xDA42

all: $(PRG)

$(PRG): $(OBJS)
	$(LD) $(LIBS) -o $@ $^

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OUTPUT): $(TARGET) $(INPUT)
	mkdir -p output
	./$(TARGET) $(INPUT) $(OUTPUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run:
	echo "" > $(DEBUG_SCRIPT)
	echo 'a $(BREAK_ADDR)' >> $(DEBUG_SCRIPT)
	$(HATARI) \
		-w -m --machine ste --tos $(TOS_ROM) \
		--fullscreen \
		--harddrive "$(shell pwd)/output/" --gemdos-drive G \
		--acsi 0=$(HD_IMG) \
		--parse $(DEBUG_SCRIPT) \
		--fast-boot TRUE --fast-forward TRUE --timer-d TRUE \
		--confirm-quit false

deploy:
	echo "" > $(DEBUG_SCRIPT)
	echo 'a $(BREAK_ADDR)' >> $(DEBUG_SCRIPT)
	cp -f $(CLEAN_IMG) $(HD_IMG)
	mkdir -p output/AUTO
	cp copier/AUTOCOPY.TOS output/AUTO/
	$(HATARI) \
		--fast-boot 1 --fast-forward 0 --timer-d 1 \
		-w -m --machine ste --tos $(TOS_ROM) \
		--fullscreen \
		--harddrive "$(shell pwd)/output/" --gemdos-drive G \
		--acsi 0=$(HD_IMG) \
		--parse $(DEBUG_SCRIPT) \
		--confirm-quit false \
		--auto "G:\AUTO\AUTOCOPY.TOS"


clean:
	rm -f $(OBJS) $(TARGET) $(DEBUG_SCRIPT) $(OUTPUT) $(PRG)

