CC=cc

WARNING_FLAGS=-Wall
## fine tuned warnings
#-Waddress -Warray-bounds -Wchar-subscripts -Wenum-compare -Wimplicit-int -Wimplicit-function-declaration -Wformat -Wmain -Wmissing-braces -Wnonnull -Wopenmp-clauses -Wparentheses -Wpointer-sign -Wreorder -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wconsumed -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wout-of-line-declaration -Wtype-limits -Wshift-sign-overflow
## excluded warnings
#-Wcomment
# incompatable warnings
#-Warray-bounds=1 -Wbool-compare -Wmaybe-uninitialized -Wopenmp-simd -Wsign-compare -Wunused-function -Wclobbered -Wmissing-parameter-type -Wold-style-declaration -Woverride-init -Wshift-negative-value -Wunused-but-set-parameter -Wunused-parameter

INCLUDE_PATHS=
LIBS=-lSOIL -lcmph

GLOBAL_FLAGS=$(LIBS) -O0 -g -fstandalone-debug
CFLAGS=$(WARNING_FLAGS) -DHASH_FUNCTION=HASH_OAT $(GLOBAL_FLAGS) $(INCLUDE_PATHS)

### building the executables

.PHONY: all
all: phtg.exe player.exe json_parser.exe

PLAYER_OBJS=main.o runtime.o variables.o value.o strpool.o
player.exe: $(addprefix obj/, $(PLAYER_OBJS))
	$(CC) -o $@ $^ -framework OpenGL -framework SDL2 -framework Cocoa $(GLOBAL_FLAGS)

JSON_PARSER_OBJS=json_parser.o runtime.o variables.o value.o strpool.o jsmn.o
json_parser.exe: $(addprefix obj/, $(JSON_PARSER_OBJS))
	$(CC) -o $@ $^ $(GLOBAL_FLAGS)

phtg.exe: $(addprefix obj/, phtg.o)
	$(CC) -o $@ $^ -lcmph

TEST_RUNTIME_OBJS=runtime.o variables.o value.o strpool.o
test_runtime.exe: $(addprefix obj/, $(TEST_RUNTIME_OBJS))
	$(CC) -o $@ $^ -lcheck $(GLOBAL_FLAGS)

obj/%.o: */%.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/%.o: */*/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/main.o: $(addprefix src/, main.c)
obj/json_parser.o: $(addprefix src/, json_parser.c)
obj/runtime.o: $(addprefix src/, runtime.c runtime_lib.c)
obj/variables.o: $(addprefix src/, variables.c variables.h)
obj/value.o: $(addprefix src/, value.c value.h)
obj/strpool.o: $(addprefix src/, strpool.c strpool.h)
obj/jsmn.o: $(addprefix src/, jsmn/jsmn.c jsmn/jsmn.h)

obj/phtg.o: $(addprefix src/blockhash/, phtg.c specs.h)

obj/test_runtime.o: $(addprefix src/, test_runtime.c)

### building the blockhash tables

BLOCKHASH_GENERATED_FILES=blockops.mphf blockhash/opstable.c blockhash/typestable.c blockhash/map.txt

.PHONY: blockhash
blockhash: $(BLOCKHASH_GENERATED_FILES)

$(BLOCKHASH_GENERATED_FILES): phtg.exe
	rm -f $(BLOCKHASH_GENERATED_FILES)
	./$^

### cleaning

.PHONY: clean spotless

clean:
	rm -f obj/*

spotless: clean
	rm -f *.exe
