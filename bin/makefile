EXE=out
CC=cc

#SRCNAMES=main.o json_parser.c runtime.c variables.c value.c strpool.c jsmn.c
MODNAMES=main json_parser runtime variables value strpool jsmn
#SRCS= $(addprefix ../src/, $(SRCNAMES))
#OBJS=$(addsuffix .o, $(basename $(SRCNAMES)))
OBJS=$(addsuffix .o, $(MODNAMES))
INCLUDE_PATHS=

WARNING_FLAGS=-Wall
#-Waddress -Warray-bounds -Wchar-subscripts -Wenum-compare -Wimplicit-int -Wimplicit-function-declaration -Wformat -Wmain -Wmissing-braces -Wnonnull -Wopenmp-clauses -Wparentheses -Wpointer-sign -Wreorder -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wconsumed -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wout-of-line-declaration -Wtype-limits -Wshift-sign-overflow
#-Wcomment
#-Warray-bounds=1 -Wbool-compare -Wmaybe-uninitialized -Wopenmp-simd -Wsign-compare -Wunused-function -Wclobbered -Wmissing-parameter-type -Wold-style-declaration -Woverride-init -Wshift-negative-value -Wunused-but-set-parameter -Wunused-parameter -Wc++11-compat -Wc++14-compat
GLOBAL_FLAGS=-O0 -g -fstandalone-debug
CFLAGS=$(WARNING_FLAGS) -c -DHASH_FUNCTION=HASH_OAT $(GLOBAL_FLAGS)

FRAMEWORKS=-framework OpenGL -framework SDL2 -framework Cocoa
LIBS=-lSOIL -lcmph

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(INCLUDE_PATHS) $(FRAMEWORKS) $(GLOBAL_FLAGS)

main.o: ../src/main.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

runtime.o: ../src/runtime.c ../src/runtime_lib.c
	$(CC) -c ../src/runtime.c $(CFLAGS) $(INCLUDE_PATHS)

json_parser: json_parser.o jsmn.o value.o variables.o strpool.o runtime.o
	$(CC) -o $@ $^ $(LIBS) $(INCLUDE_PATHS) $(GLOBAL_FLAGS)

json_parser.o: ../src/json_parser.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

variables.o: ../src/variables.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

value.o: ../src/value.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

strpool.o: ../src/strpool.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

jsmn.o: ../src/jsmn/jsmn.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

.PHONY: clean
clean:
	rm -f $(OBJS) test_runtime.o $(EXE)

.PHONY: all
all: clean $(EXE)

# UNIT TESTS
.PHONY: unit_tests
unit_tests: test_runtime.o variables.o value.o strpool.o
	$(CC) -o test_runtime $^ -lcheck $(LIBS) $(INCLUDE_PATHS) $(GLOBAL_FLAGS)

test_runtime.o: ../src/test_runtime.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE_PATHS)

.PHONY: clean_unit_tests
clean_unit_tests:
	rm -f test_runtime.o test_runtime

# BLOCK HASH TABLE BUILDING
HASH_GEN_FILES=../src/perfect_hashes/blocks.mphf ../src/perfect_hashes/opstable.c ../src/perfect_hashes/map.txt
#../src/perfect_hashes/hat_blocks.h
.PHONY: hashtable
hashtable: $(HASH_GEN_FILES)

$(HASH_GEN_FILES): phtg
	rm -f $(HASH_GEN_FILES)
	./$^

phtg: phtg.o
	$(CC) -o $@ $^ -lcmph

phtg.o: ../src/perfect_hashes/src/phtg.c ../src/perfect_hashes/src/specs.h
	$(CC) -c ../src/perfect_hashes/src/phtg.c $(CFLAGS)

.PHONY: clean_hashtable
clean_hashtable:
	rm -f phtg phtg.o
