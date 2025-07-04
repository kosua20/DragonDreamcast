
EXEC_NAME = dragon
EXEC_ELF = build/$(EXEC_NAME).elf
EXEC_CDI = $(EXEC_NAME).cdi

FILES = helpers scene main romdisk

OBJECTS=$(patsubst %, build/%.o, $(FILES))

all: $(EXEC_ELF)

clean:
	-rm -rf build/
	-rm $(EXEC_CDI)

build/%.o: src/%.c
	$(shell mkdir -p build) 
	kos-cc $(CFLAGS) -std=c99 -Og -c $< -o $@

$(EXEC_ELF): $(OBJECTS)
	kos-cc -o $(EXEC_ELF) $(OBJECTS) -lpcx -lkosutils -lkmg -lz -lm

build/romdisk.img:
	$(KOS_GENROMFS) -f build/romdisk.img -d data -v

build/romdisk.o: build/romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o build/romdisk.img romdisk build/romdisk.o

dist: $(EXEC_ELF)
	$(KOS_MKDCDISC) -e $(EXEC_ELF) -o $(EXEC_CDI) --no-padding --name $(EXEC_NAME)

run: dist
	$(KOS_EMULATOR) $(EXEC_CDI)

test: $(EXEC_ELF)
	$(KOS_EMULATOR) $(EXEC_ELF)