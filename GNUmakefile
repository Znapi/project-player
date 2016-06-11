CC=cc

WARNING_FLAGS=-Wall
## fine tuned warnings
#-Waddress -Warray-bounds -Wchar-subscripts -Wenum-compare -Wimplicit-int -Wimplicit-function-declaration -Wformat -Wmain -Wmissing-braces -Wnonnull -Wopenmp-clauses -Wparentheses -Wpointer-sign -Wreorder -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wconsumed -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wout-of-line-declaration -Wtype-limits -Wshift-sign-overflow
## excluded warnings
#-Wcomment
## incompatable warnings
#-Warray-bounds=1 -Wbool-compare -Wmaybe-uninitialized -Wopenmp-simd -Wsign-compare -Wunused-function -Wclobbered -Wmissing-parameter-type -Wold-style-declaration -Woverride-init -Wshift-negative-value -Wunused-but-set-parameter -Wunused-parameter

INCLUDE_PATHS=
LIBS=-lSOIL -lcmph
GLOBAL_FLAGS=-O0 -g -fstandalone-debug
CFLAGS=-DHASH_FUNCTION=HASH_OAT $(WARNING_FLAGS) $(GLOBAL_FLAGS) $(INCLUDE_PATHS)

### building the executables

EXECUTABLES=phtg player json_parser

.PHONY: all
all: $(EXECUTABLES)

PLAYER_MODS=main runtime variables value strpool
player: $(addprefix obj/, $(addsuffix .o, $(PLAYER_MODS)))
	$(CC) -o $@ $^ $(LIBS) -framework OpenGL -framework SDL2 -framework Cocoa $(GLOBAL_FLAGS)

JSON_PARSER_MODS=json_parser runtime variables value strpool jsmn
json_parser: $(addprefix obj/, $(addsuffix .o, $(JSON_PARSER_MODS))) blockops.mphf
	$(CC) -o $@ $(filter-out %.mphf, $^) $(LIBS) $(GLOBAL_FLAGS)

PHTG_MODS=phtg
phtg: $(addprefix obj/, $(addsuffix .o, $(PHTG_MODS)))
	$(CC) -o $@ $^ -lcmph

TEST_RUNTIME_MODS=runtime variables value strpool
test_runtime.exe: $(addprefix obj/, $(addsuffix .o, $(TEST_RUNTIME_MODS)))
	$(CC) -o $@ $^ -lcheck $(GLOBAL_FLAGS)


DEPS=$(addprefix obj/, $(addsuffix .d, \
	$(sort $(PLAYER_MODS) $(JSON_PARSER_MODS) $(PHTG_MODS) $(TEST_RUNTIME_MODS))))

.PHONY: deps
deps: $(DEPS)
	@echo Made dependencies.

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

-include $(DEPS)

obj/%.o: */%.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/%.o: */*/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

### building the blockhash tables

BLOCKHASH_GENERATED_FILES=blockops.mphf blockhash/opstable.c blockhash/typestable.c blockhash/map.txt

.PHONY: blockhash
blockhash: clean_blockshash phtg
	./phtg

$(BLOCKHASH_GENERATED_FILES): phtg
	./phtg

clean_blockhash:
	rm -rf $(BLOCKHASH_GENERATED_FILES)

### cleaning

.PHONY: clean spotless

clean:
	rm -f obj/*

spotless: clean clean_blockhash
	rm -f $(EXECUTABLES)
