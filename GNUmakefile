CC=cc

WARNING_FLAGS=-Wall -Wno-visibility

GLOBAL_FLAGS=-O0 -g -fstandalone-debug
CFLAGS=-DHASH_FUNCTION=HASH_OAT $(WARNING_FLAGS) $(GLOBAL_FLAGS)

LIBS=-lSOIL -lcmph -liconv -lz
ifeq ($(OS),Windows_NT)
LIBS += -lopengl32 -lSDL2
else
ifeq ($(shell uname -s),Darwin)
LIBS += -framework OpenGL -framework SDL2 -framework Cocoa
else
LIBS += `sdl2-config --cflags --libs`
endif
endif

### building the executables

EXECUTABLES=phtg player graphics_demo

PLAYER_MODS=main project_loader zip_loader runtime variables value strpool jsmn
player: $(addprefix obj/, $(addsuffix .o, $(PLAYER_MODS))) blockops.mphf
	$(CC) -o $@ $(filter-out %.mphf, $^) $(LIBS) $(GLOBAL_FLAGS)

GRAPHICS_MODS=graphics runtime variables value strpool
graphics_demo: $(addprefix obj/, $(addsuffix .o, $(GRAPHICS_MODS)))
	$(CC) -o $@ $^ $(LIBS) $(GLOBAL_FLAGS)

PHTG_MODS=phtg
phtg: $(addprefix obj/, $(addsuffix .o, $(PHTG_MODS)))
	$(CC) -o $@ $^ -lcmph

# Don't even try to build this target. The build is guarenteed to fail.
TEST_RUNTIME_MODS=runtime variables value strpool
test_runtime: $(addprefix obj/, $(addsuffix .o, $(TEST_RUNTIME_MODS)))
	$(CC) -o $@ $^ -lcheck $(GLOBAL_FLAGS)

.PHONY: all
all: $(EXECUTABLES)

obj/%.d: src/%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
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
