CC=cc

DEBUG=yes
PHTG_DEBUG=no

CFLAGS=-DHASH_FUNCTION=HASH_OAT -DGL_GLEXT_PROTOTYPES -Wall -Wno-visibility
LFLAGS=-lcmph -liconv -lz
ifeq ($(OS),Windows_NT)
LFLAGS += -lopengl32 -lSDLmain
else
ifeq ($(shell uname -s),Darwin)
LFLAGS += -framework OpenGL -framework SDL2 -framework Cocoa
else
LFLAGS += `sdl2-config --cflags --libs`
endif
endif

PHTG_CFLAGS=-DHASH_FUNCTION=HASH_OAT -Wall -Wno-visibility
PHTG_LFLAGS=-lcmph

DEBUG_GLOBAL_FLAGS=-g -fstandalone-debug
DEBUG_CFLAGS=-O0

ifeq ($(DEBUG),yes)
CFLAGS += $(DEBUG_CFLAGS) $(DEBUG_GLOBAL_FLAGS)
else
CFLAGS += -Ofast
endif

ifeq ($(PHGT_DEBUG),yes)
PHTG_CFLAGS += $(DEBUG_CFLAGS) $(DEBUG_GLOBAL_FLAGS)
else
PHTG_CFLAGS += -Ofast
endif

### building the executables

EXECUTABLES=phtg player

SOIL2_MODS=SOIL2 image_helper image_DXT etc1_utils
PLAYER_MODS=main runtime peripherals graphics project_loader zip_loader jsmn variables value thread strpool $(SOIL2_MODS)
player: $(addprefix obj/, $(addsuffix .o, $(PLAYER_MODS))) blockops.mphf
	$(CC) -o $@ $(filter %.o, $^) $(LFLAGS)

PHTG_MODS=phtg
phtg: $(addprefix obj/, $(addsuffix .o, $(PHTG_MODS)))
	$(CC) -o $@ $^ -lcmph $(PHTG_LFLAGS)

.PHONY: all
all: $(EXECUTABLES)

obj/%.d: src/%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.d: src/blockhash/%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(PHTG_CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.d: src/*/%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/runtime.d obj/project_loader.d: src/runtime.c src/blockhash/opstable.c src/blockhash/typestable.c

DEPS=$(addprefix obj/, $(addsuffix .d, \
	$(sort $(PLAYER_MODS) $(GRAPHICS_MODS) $(PHTG_MODS) $(TEST_RUNTIME_MODS))))
-include $(DEPS)

obj/%.o: src/%.c obj/%.d
	$(CC) $(CFLAGS) -c -o $@ $<

obj/%.o: src/blockhash/%.c obj/%.d
	$(CC) $(PHTG_CFLAGS) -c -o $@ $<

obj/%.o: src/*/%.c obj/%.d
	$(CC) $(CFLAGS) -c -o $@ $<

### building the blockhash tables

BLOCKHASH_GENERATED_FILES=blockops.mphf $(addprefix src/blockhash/, opstable.c typestable.c)

.PHONY: blockhash clean_blockhash
blockhash: $(BLOCKHASH_GENERATED_FILES)

src/%hash/opstable.c src/%hash/typestable.c %ops.mphf: phtg
	./phtg

clean_blockhash:
	rm -f $(BLOCKHASH_GENERATED_FILES) map.txt

### cleaning

.PHONY: clean spotless

clean:
	rm -f obj/*.o

spotless: clean clean_blockhash
	rm -f $(EXECUTABLES) obj/*.d
