CC=cc

WARNING_FLAGS=-Wall -Wno-visibility

GLOBAL_FLAGS=-O0 -g -fstandalone-debug
CFLAGS=-DHASH_FUNCTION=HASH_OAT -DGL_GLEXT_PROTOTYPES $(WARNING_FLAGS) $(GLOBAL_FLAGS)

LIBS=-lcmph -lsoil2 -liconv -lz
ifeq ($(OS),Windows_NT)
G_LIBS = -lopengl32 -lSDLmain
else
ifeq ($(shell uname -s),Darwin)
G_LIBS += -framework OpenGL -framework SDL2 -framework Cocoa
else
G_LIBS += `sdl2-config --cflags --libs`
endif
endif

### building the executables

EXECUTABLES=phtg player

PLAYER_MODS=main runtime peripherals project_loader zip_loader jsmn variables value thread strpool
player: $(addprefix obj/, $(addsuffix .o, $(PLAYER_MODS))) blockops.mphf
	$(CC) -o $@ $(filter %.o, $^) $(LIBS) $(G_LIBS) $(GLOBAL_FLAGS)

PHTG_MODS=phtg
phtg: $(addprefix obj/, $(addsuffix .o, $(PHTG_MODS)))
	$(CC) -o $@ $^ -lcmph

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
