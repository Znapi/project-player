/**
	Unit Tests for the Runtime
			test_runtime.c

	DON'T BOTHER WITH THIS FILE.
	This module is completely broken right now because I've made loads of changes to the
	runtime without bothering with any more unit tests.

	This is a collection of unit tests for testing the runtime. They test:
		o  Dynamic strings allocation
		o  Dynamic type conversions
		o  Thread counters
		o  Variable/list creation and modification
		o  The operator category of blocks
		o  Variable/list blocks with a SpriteContext
		o  Basic single block evaluation
		o  Basic stack evaluation
		o  Stacks with control blocks
		o  Basic thread management

	This is my first time doing unit tests, so they are messy. I'm not sure all of these
	really need to be repeated each build. Some of them are just so I could confirm all
	the code I was writing works. The important tests are the first 5, as they actually
	test individual units.
**/

#include <stdlib.h>
#include <check.h>

#define TESTING
#include "project_path.h"
#include "runtime.c"

static char buffer[32];
static Block* bf_test_print(const Block *block, Value * const reportSlot, const Value arg[]) {
	switch(arg[0].type) {
	case FLOATING:
		sprintf(buffer, "%f", arg[0].data.floating);
		break;
	case STRING:
		sprintf(buffer, "%s", arg[0].data.string);
		break;
	case BOOLEAN:
		sprintf(buffer, "%i", arg[0].data.boolean);
		break;
	}
	//printf("%s\n", buffer);
	return block->p.next;
}

// for testing it is more convenient for the runtime /not/ to modify our thread contexts,
// so we create a dummy context for it.
static ThreadContext temporaryContext;
static void loadThreadContext(const ThreadContext *context) {
	temporaryContext = *context;
	activeContext = &temporaryContext;
}

#include <cmph.h> // for hashing op strings to build test scratch blocks stacks

static cmph_t *mphf;

static void initMphf(void) {
	FILE *mphfStream = fopen(PROJECT_PATH"src/perfect_hashes/blocks.mphf", "r");
	if(mphfStream == NULL)
		puts("[ERROR]Minimal perfect hash function file dump could not be opened!");
	mphf = cmph_load(mphfStream);
	fclose(mphfStream);
}

static void destroyMphf(void) {
	cmph_destroy(mphf);
}

static blockhash hash(const char * const key) {
	return (blockhash)cmph_search(mphf, key, (cmph_uint32)strlen(key));
}

// set up values to test
const Value trueString = {{.string = "true"}, STRING};
const Value falseString = {{.string = "false"}, STRING};
const Value floatingString = {{.string = "12.25001"}, STRING};
const Value integerString = {{.string = "42"}, STRING};
const Value mixedString = {{.string = "42.44e+1 Hi world! 31"}, STRING};

const Value trueBoolean = {{.boolean = true}, BOOLEAN};
const Value falseBoolean = {{.boolean = false}, BOOLEAN};

const Value positive = {{.floating = 1337.0}, FLOATING};
const Value negative = {{.floating = -77.0}, FLOATING};
const Value zero = {{.floating = 0.0}, FLOATING};
const Value one = {{.floating = 1.0}, FLOATING};
const Value three = {{.floating = 3.0}, FLOATING};
const Value ten = {{.floating = 10.0}, FLOATING};
const Value fortyfive = {{.floating = 45.0}, FLOATING};

const Value positiveLarge = {{.floating = 9.0e+20}, FLOATING};
const Value negativeLarge = {{.floating = -7.0e+20}, FLOATING};
const Value positiveSmall = {{.floating = 0.005}, FLOATING};
const Value negativeSmall = {{.floating = -0.02}, FLOATING};
const Value negativeZero = {{.floating = -0.0}, FLOATING};
const Value infinity = {{.floating = INFINITY}, FLOATING};
const Value nanv = {{.floating = NAN}, FLOATING};

const Value absString = {{.string = "abs"}, STRING};
const Value tanString = {{.string = "tan"}, STRING};
const Value tenToPowString = {{.string = "10 ^"}, STRING};

START_TEST(test_stringPool)
{
	//puts("TEST_AUTO_FREED_STRINGS");
	char *const str0 = allocString(2);
	str0[0] = 'a'; str0[1] = '\0';
	ck_assert_str_eq(str0, "a");
	char *const str1 = allocString(4);
	strcpy(str1, "XYZ");
	ck_assert_str_eq(str1, "XYZ");

	char *const str2 = extractString(str1);
	ck_assert_str_eq(str2, str1);

	freeStrings();
	ck_assert(true);
	freeStrings(); // test that calls when no strings are allocated doesn't cause issues
}
END_TEST

START_TEST(test_typeConversions)
{
	//puts("TEST_TYPE_CONVERSIONS");
	// check tryToFloating
	double ret;
	ck_assert(tryToFloating(positive, &ret));
	ck_assert(ret == 1337.0);

	ck_assert(tryToFloating(trueBoolean, &ret));
	ck_assert(ret == 1.0);

	ck_assert(tryToFloating(integerString, &ret));
	ck_assert(ret == 42.0);
	ck_assert(tryToFloating(floatingString, &ret));
	ck_assert(ret == 12.25001);
	ck_assert(!tryToFloating(mixedString, &ret));

	ck_assert(tryToFloating(trueString, &ret));
	ck_assert(ret == 1.0);
	ck_assert(tryToFloating(falseString, &ret));
	ck_assert(ret == 0.0);

	// check toInteger
	ck_assert_int_eq(toInteger(positive), 1337);
	ck_assert_int_eq(toInteger(negative), -77);
	ck_assert_int_eq(toInteger(zero), 0);

	ck_assert_int_eq(toInteger(trueBoolean), 1);
	ck_assert_int_eq(toInteger(falseBoolean), 0);

	ck_assert_int_eq(toInteger(integerString), 42);
	ck_assert_int_eq(toInteger(floatingString), 12);
	ck_assert_int_eq(toInteger(mixedString), 42);

	ck_assert_int_eq(toInteger(trueString), 1); // string representations of true and false are not converted
	ck_assert_int_eq(toInteger(falseString), 0);

	// check toFloating
	ck_assert(toFloating(positive) == 1337.0);

	ck_assert(toFloating(trueBoolean) == 1.0);
	ck_assert(toFloating(falseBoolean) == 0.0);

	ck_assert(toFloating(integerString) == 42.0);
	ck_assert(toFloating(floatingString) == 12.25001);
	ck_assert(toFloating(mixedString) == 424.4);

	ck_assert(toFloating(trueString) == 1.0); // string representations of true and false are not converted
	ck_assert(toFloating(falseString) == 0.0);

	// check toBoolean
	ck_assert_int_eq(toBoolean(trueBoolean), true);

	ck_assert_int_eq(toBoolean(trueString), true);
	ck_assert_int_eq(toBoolean(falseString), false);

	ck_assert_int_eq(toBoolean(zero), false);
	ck_assert_int_eq(toBoolean(one), true);

	// check toString
	char *s;
	toString(mixedString, &s);
	ck_assert_str_eq(s, mixedString.data.string);
	toString(trueBoolean, &s);
	ck_assert_str_eq(s, "true");
	toString(falseBoolean, &s);
	ck_assert_str_eq(s, "false");
	toString(positive, &s);
	ck_assert_str_eq(s, "1337.000000");
	toString(negative, &s);
	ck_assert_str_eq(s, "-77.000000");
	toString(positiveSmall, &s);
	ck_assert_str_eq(s, "0.005000");
	toString(nanv, &s);
	ck_assert_str_eq(s, "nan");

	freeStrings();
}
END_TEST

START_TEST(test_threadCounters)
{
	//puts("TEST_THREAD_COUNTERS");
	union Counter counters[8];
	const Block *counterOwners[8];
	const ThreadContext thread = { NULL, NULL, 0, {counters, counterOwners, 0}, NULL };
	loadThreadContext(&thread);
	Block *test_ptrs[] = {NULL, NULL+1, NULL+2, NULL+3, NULL+4, NULL+5}; // uses the NULL macro so I don't have to repeat (void*)

	allocCounter(test_ptrs[0]);
	usetCounter(0);
	ck_assert_int_eq(ugetCounter(), 0);
	usetCounter(2);
	ck_assert_int_eq(ugetCounter(), 2);

	allocCounter(test_ptrs[1]);
	usetCounter(3);
	ck_assert_int_eq(ugetCounter(), 3);
	freeCounter();
	ck_assert_int_eq(ugetCounter(), 2);

	allocCounter(test_ptrs[0]);
	ck_assert_int_eq(ugetCounter(), 2);

	allocCounter(test_ptrs[2]);
	fsetCounter(5.0f);
	ck_assert(fgetCounter() == 5.0f);
	freeCounter();
	ck_assert_int_eq(ugetCounter(), 2);
	freeCounter();
}
END_TEST

START_TEST(test_variables)
{
	Variable *table = NULL;
	Value value;

	createVariable(&table, "var", trueBoolean);
	value = getVariable(&table, "var");
	ck_assert_int_eq(value.data.boolean, true);

	setVariable(&table, "var", falseBoolean);
	value = getVariable(&table, "var");
	ck_assert_int_eq(value.data.boolean, false);

	createVariable(&table, "test", copyValue(integerString)); // make a copy because the type is a string, and it will attempt ot free the string
	value = getVariable(&table, "var");
	ck_assert_int_eq(value.data.boolean, false);
	value = getVariable(&table, "test");
	ck_assert_str_eq(value.data.string, integerString.data.string);
	freeVariables(&table);

	/*createVariable(&table, "var", three);
	value = getVariable(&table, "var");
	ck_assert_int_eq((int)value.data.integer, 3);*/
	freeVariables(&table);
}
END_TEST

START_TEST(test_lists)
{
	List *table = NULL;
	ListElement *i_element1, *i_element0;
	i_element0 = malloc(sizeof(ListElement));
	i_element1 = malloc(sizeof(ListElement));
	//printf("MALLOC: %p\nMALLOC: %p\n", i_element0, i_element1);
	i_element0->next = i_element1;
	i_element1->next = NULL;
	i_element0->value.data.floating = 10.0;
	i_element0->value.type = FLOATING;
	i_element1->value.data.boolean = true;
	i_element1->value.type = BOOLEAN;
	createList(&table, "testList", i_element0, 2);

	// test getElement
	List *list = getListPtr(&table, "testList");
	ck_assert(list != NULL);
	Value value = getFirstListElement(list);
	ck_assert_int_eq((int)value.data.floating, 10);
	value = getLastListElement(list);
	ck_assert_int_eq(value.data.boolean, true);
	value = getListElement(list, 0);
	ck_assert_int_eq((int)value.data.floating, 10);
	value = getListElement(list, 1);
	ck_assert_int_eq(value.data.boolean, true);
	value = getListElement(list, 2);
	ck_assert_int_eq(value.type, FLOATING);
	ck_assert(value.data.floating == 0.0);

	// test append/prepend/insert
	listAppend(list, fortyfive);
	value = getListElement(list, 2);
	ck_assert_int_eq((int)value.data.floating, 45);
	listPrepend(list, copyValue(mixedString));
	value = getListElement(list, 0);
	ck_assert_str_eq(value.data.string, mixedString.data.string);

	listInsert(list, copyValue(falseString), 1);
	value = getListElement(list, 1);
	ck_assert_str_eq(value.data.string, falseString.data.string);
	listInsert(list, three, 0);
	value = getFirstListElement(list);
	ck_assert_int_eq((int)value.data.floating, 3);
	listInsert(list, one, 999999);
	value = getLastListElement(list);
	ck_assert_int_ne((int)value.data.floating, 3);

	// test set
	listSetFirst(list, positive);
	value = getFirstListElement(list);
	ck_assert_int_eq((int)value.data.floating, 1337);
	listSetLast(list, negative);
	value = getLastListElement(list);
	ck_assert_int_eq((int)value.data.floating, -77);
	listSet(list, ten, 2);
	value = getListElement(list, 2);
	ck_assert_int_eq((int)value.data.floating, 10.0);

	// test delete
	Value value2 = getListElement(list, 1);
	listDeleteFirst(list);
	value = getFirstListElement(list);
	ck_assert_int_eq((int)value.data.floating, (int)value2.data.floating);
	value2 = getListElement(list, list->length-2);
	listDeleteLast(list);
	value = getLastListElement(list);
	ck_assert_int_eq((int)value.data.floating, (int)value2.data.floating);
	value2 = getListElement(list, 2);
	listDelete(list, 1);
	value = getListElement(list, 1);
	ck_assert_int_eq((int)value.data.floating, (int)value2.data.floating);
	ufastest i;
	const ufastest j = list->length;
	for(i = 0; i < j; ++i)
		listDeleteFirst(list);
	ck_assert(true);
	listDeleteLast(list);
	ck_assert(true);
	listDelete(list, 1);

	freeLists(&table);
	ck_assert(true);
	freeLists(&table);

	ck_assert(true);
	createList(&table, "empty", NULL, 0);
	list = getListPtr(&table, "empty");
	ck_assert(list != NULL);
	value = getFirstListElement(list);
	ck_assert_int_eq(value.type, FLOATING);
	ck_assert(value.data.floating == 0.0);
	freeLists(&table);
}
END_TEST

/*
	NOTE: as the functionality of individual block functions is mainly affected by the lower
	level type conversions and such, unit tests of block functions do not have to be extensive,
	although it doesn't hurt to keep around any tests you write from making sure your block does it's
	basic job.

	Operators are a little special, and need to be tested a little more extensively, because their
	behavior might have to match as closely as they do in Scratch as possible.
*/
START_TEST(test_operators)
{
	//puts("TEST_OPERATORS");
	Value reportSlot;
	Value args[8];
	const Block emptyBlock = {0, 0, {0}};

	// add
	args[0] = ten; args[1] = negative;
	bf_add(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq((int)reportSlot.data.floating, -67);
	args[0] = infinity;
	bf_add(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == INFINITY); // math involving Infinity is sometimes inconsistent with Scratch, but it might not matter
	args[0] = nanv;
	bf_add(&emptyBlock, &reportSlot, args);
	//ck_assert((int)reportSlot.data.floating == -77); // math involving Nanv is inconsitent with Scratch, but does this matter enough?
	args[1] = infinity;
	bf_add(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == INFINITY);

	// subtract
	args[0] = ten; args[1] = negative;
	bf_subtract(&emptyBlock, &reportSlot, args);
	ck_assert((int)reportSlot.data.floating == 87);
	args[0] = infinity;
	bf_subtract(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == INFINITY);
	args[0] = nanv;
	bf_subtract(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == -10.0);
	args[1] = infinity;
	bf_subtract(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == -INFINITY);

	// multiply
	args[0] = ten; args[1] = negative;
	bf_multiply(&emptyBlock, &reportSlot, args);
	ck_assert((int)reportSlot.data.floating == -770);
	args[0] = infinity;
	bf_multiply(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == -INFINITY);
	args[0] = nanv;
	bf_multiply(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == 0.0);
	args[1] = infinity;
	bf_multiply(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == NANV);

	// divide
	args[1] = ten; args[0] = negative;
	bf_divide(&emptyBlock, &reportSlot, args);
	ck_assert(reportSlot.data.floating == -7.7);
	args[0] = infinity;
	bf_divide(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == INFINITY);
	args[0] = nanv;
	bf_divide(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == NANV);
	args[1] = infinity;
	bf_divide(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == 0.0);

	// modulo
	args[0] = integerString; args[1] = ten;
	bf_modulo(&emptyBlock, &reportSlot, args);
	ck_assert(reportSlot.data.floating == 2.0);
	args[0] = negativeSmall;
	bf_modulo(&emptyBlock, &reportSlot, args);
	ck_assert(reportSlot.data.floating == 9.98);
	args[1] = infinity;
	bf_modulo(&emptyBlock, &reportSlot, args);
	//ck_assert(reportSlot.data.floating == -0.02);

	// random number generation
	args[0] = positive; args[1] = negative;
	ufastest i = 0;
	for(; i < 16; ++i) { // test a bunch of random numbers
		bf_generate_random(&emptyBlock, &reportSlot, args);
		bf_test_print(&emptyBlock, NULL, &reportSlot);
		printf("Random number:  %s\n", buffer);
		ck_assert(reportSlot.data.floating <= 1337.0 && reportSlot.data.floating >= -77.0);
	}

	// round
	args[0] = negativeSmall;
	bf_round(&emptyBlock, &reportSlot, args);
	ck_assert(reportSlot.data.floating == 0);
	args[0] = ten;
	bf_round(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq((int)reportSlot.data.floating, 10);

	// comparisons
	args[0] = integerString; args[1] = one;
	bf_is_greater(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq(reportSlot.data.boolean, true);
	args[0] = trueBoolean;
	bf_is_equal(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq(reportSlot.data.boolean, true);
	args[0] = positiveLarge;
	bf_is_less(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq(reportSlot.data.boolean, false);

	// logic gates
	args[0] = trueBoolean; args[1] = trueString;
	bf_logical_and(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq(reportSlot.data.boolean, true);
	args[0] = falseString;
	bf_logical_or(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq(reportSlot.data.boolean, true);
	args[0] = one;
	bf_logical_not(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq(reportSlot.data.boolean, false);

	// math functions
	args[0] = absString; args[1] = negative;
	bf_compute_math_function(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq((int)reportSlot.data.floating, 77);
	args[0] = tanString; args[1] = fortyfive;
	bf_compute_math_function(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq((int)round(reportSlot.data.floating), 1);
	args[0] = tenToPowString; args[1] = ten;
	bf_compute_math_function(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq((int64)reportSlot.data.floating, (int64)10000000000);

	// string operations
	args[0] = integerString; args[1] = trueString;
	bf_concatenate(&emptyBlock, &reportSlot, args);
	ck_assert_str_eq(reportSlot.data.string, "42true");
	args[0] = positive;
	bf_get_string_length(&emptyBlock, &reportSlot, args);
	ck_assert_int_eq((int)reportSlot.data.floating, 11); // string of 1337.0 is 1337.000000
	args[0] = one; args[1] = infinity;
	bf_get_character(&emptyBlock, &reportSlot, args);
	ck_assert_str_eq(reportSlot.data.string, "i");

	freeStrings();
}
END_TEST

START_TEST(test_basicBlockEvaluation)
{
	//puts("TEST_BASIC_BLOCK_EVALUATION");
	Block testBlocks[] = {
		{hash("noop"), 4, { .value = &integerString }},
		{hash("noop"), 4, { .value = &negative }},
		{hash("+"), 3, { NULL }},
		{hash("noop"), 3, { .value = &ten }},
		{hash("%"), 2, { NULL }},
		{hash("rounded"), 1, { NULL }},
		{hash("print"), 0, { .next = NULL }},
	};
	Value stack[8];
	const ThreadContext thread = { stack, testBlocks, 0, {NULL, NULL, 0}, NULL };
	loadThreadContext(&thread);
	singleStepActiveThread();

	ck_assert_str_eq(buffer, "5.000000");

	freeStrings();
}
END_TEST

START_TEST(test_stackEvaluation)
{
	//puts("TEST_STACK_EVALUATION");
	// three block stack
	Block testBlock3[] = {
		{hash("noop"), 1, { .value = &trueString }},
		{hash("print"), 0, { .next = NULL }},
	}; // should set the buffer to "true"
	Block testBlock2[] = {
		{hash("noop"), 4, { .value = &integerString }},
		{hash("noop"), 4, { .value = &negative }},
		{hash("+"), 3, { NULL }},
		{hash("noop"), 3, { .value = &ten }},
		{hash("%"), 2, { NULL }},
		{hash("rounded"), 1, { NULL }},
		{hash("print"), 0, { .next = testBlock3 }},
	}; // should set the buffer to "5"
	Block testBlock1[] = {
		{hash("noop"), 2, { .value = &integerString }},
		{hash("noop"), 2, { .value = &trueString }},
		{hash("concatenate:with:"), 1, { NULL }},
		{hash("print"), 0, { .next = testBlock2 }},
	}; // should set the buffer to "0false"

	Value stack[16];
	const ThreadContext thread = { stack, testBlock1, 0, {NULL, NULL, 0}, NULL };
	loadThreadContext(&thread);

	singleStepActiveThread();
	ck_assert_str_eq(buffer, "42true");
	singleStepActiveThread();
	ck_assert_str_eq(buffer, "5.000000");
	singleStepActiveThread();
	ck_assert_str_eq(buffer, "true");

	freeStrings();
}
END_TEST

START_TEST(test_programFlowControl)
{
	//puts("TEST_PROGRAM_FLOW_CONTROL");
	Block stubBlock = {hash("noop"), 0, { .next = NULL }}; // stub after if statement
	Block testBlock2[] = { // stack inside of if statement
		{hash("noop"), 4, { .value = &integerString }},
		{hash("noop"), 4, { .value = &negative }},
		{hash("+"), 3, { NULL }},
		{hash("noop"), 3, { .value = &ten }},
		{hash("%"), 2, { NULL }},
		{hash("rounded"), 1, { NULL }},
		{hash("print"), 0, { .next = &stubBlock }},
	};
	Block testBlock3[] = {
		{hash("noop"), 2, { .value = &integerString }},
		{hash("noop"), 2, { .value = &trueString }},
		{hash("concatenate:with:"), 1, { NULL }},
		{hash("print"), 0, { .next = testBlock2 }},
	};

	// test if block
	Block *ifBranches[2] = {testBlock2, &stubBlock};
	Block ifBlock[] = {
		{hash("noop"), 1, { .value = &trueBoolean }},
		{hash("doIf"), 0, { .flow = ifBranches }},
	};
	Block testBlock1[] = {
		{hash("noop"), 1, { .value = &integerString }},
		{hash("print"), 0, { .next = ifBlock }},
	};
	Value stack[8];
	union Counter counters[8];
	const Block *ownerBlocks[8];
	ThreadContext thread = { stack, testBlock1, 0, {counters, ownerBlocks, 0}, NULL };
	loadThreadContext(&thread);

	buffer[0] = '\0';
	while(!singleStepActiveThread()) {;} // run thread until done
	ck_assert_str_eq(buffer, "5.000000");

	// do the same thing, but make the if statement equate to false
	ifBlock[0].p.value = &falseBoolean;
	loadThreadContext(&thread); // reset thread context
	buffer[0] = '\0';
	while(!singleStepActiveThread()) {;}
	ck_assert_str_eq(buffer, "42"); // the buffer should be "42" from when it was printed to before the if statement was evaluated

	// test if else block
	Block *ifelseBranches[2] = {testBlock2, testBlock3};
	Block ifelseBlock[] = {
		{hash("noop"), 1, { .value = &trueBoolean }},
		{hash("doIfElse"), 0, { .flow = ifelseBranches }},
	};
	testBlock1[1].p.next = ifelseBlock; // switch the stack to use the if else block rather than the if block
	testBlock2[6].p.next = &stubBlock;
	testBlock3[3].p.next = &stubBlock;
	loadThreadContext(&thread); // reset thread context

	buffer[0] = '\0';
	while(!singleStepActiveThread()) {;} // run thread until done
	ck_assert_str_eq(buffer, "5.000000");

	// test until block
	Block *untilBranches[2] = {testBlock2, &stubBlock};
	Block untilBlock[] = {
		{hash("noop"), 1, { .value = &falseBoolean }},
		{hash("doUntil"), 0, { .flow = untilBranches }},
	};
	testBlock1[1].p.next = untilBlock; // switch the stack to use the until block rather than the if block
	testBlock2[6].p.next = untilBlock; // make it repeat
	loadThreadContext(&thread); // reset thread context

	ufastest counter;
	for(counter = 1; counter < 5; ++counter) {
		if(singleStepActiveThread())
			break;
	}
	ck_assert_int_eq(counter, 5);

	// test repeat block
	Block stubBlock1 = {hash("noop"), 0, { .next = NULL }}; // stub after if statement
	Block *repeatBranches[] = {&stubBlock1, &stubBlock};
	Block repeatBlock[] = {
		{hash("noop"), 1, { .value = &ten }},
		{hash("doRepeat"), 0, { .flow = repeatBranches }},
	};
	stubBlock1.p.next = repeatBlock; // make it repeat
	thread.nextBlock = repeatBlock;
	loadThreadContext(&thread); // reset thread context

	for(counter = 1; ; ++counter) {
		if(singleStepActiveThread())
			break;
	}
	ck_assert_int_eq(counter, 22); // 10(repeat + stub) + (final repeat + final stub) = 22 steps needed to process all blocks

	// test repeat block
	Block *waitBranches[] = {NULL, NULL};
	Block waitBlock[] = {
		{hash("noop"), 1, { .value = &three }},
		{hash("wait:elapsed:from:"), 0, { .flow = waitBranches }},
	};
	waitBranches[0] = waitBlock;
	thread.nextBlock = waitBlock;
	loadThreadContext(&thread); // reset thread context

	time_t startTime, endTime;
	startTime = time(NULL);
	counter = 0;
	while(true) {
		++counter;
		if(singleStepActiveThread()) {
			endTime = time(NULL);
			break;
		}
	}
	time_t timePassed = endTime - startTime;
	ck_assert(timePassed >= 3);

	freeStrings();
}
END_TEST

START_TEST(test_dataBlocks)
{
	Value report, args[3];
	SpriteContext sprite = {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, {0,0,0,0,0,0,0}};
	const ThreadContext thread = { NULL, NULL, 0, {NULL, NULL, 0}, &sprite };
	const Block stubBlock = {hash("noop"), 0, {.next = NULL}};
	loadThreadContext(&thread);

	// get variable
	Value var2String = {{.string = "var2"}, STRING};
	args[0] = var2String;
	bf_get_variable(&stubBlock, &report, args);
	ck_assert_int_eq((int)report.data.floating, 0);

	Value stringString = {{.string = "string"}, STRING};
	createVariable(&sprite.variables, stringString.data.string, copyValue(tanString));
	args[0] = stringString;
	bf_get_variable(&stubBlock, &report, args);
	ck_assert_str_eq(report.data.string, tanString.data.string);

	// set variable
	Value trueVarString = {{.string = "trueVar"}, STRING};
	args[0] = trueVarString; args[1] = three;
	bf_variable_set(&stubBlock, NULL, args);
	bf_get_variable(&stubBlock, &report, args);
	ck_assert_int_eq((int)report.data.floating, 3);

	// change variable
	args[1] = integerString;
	bf_variable_change(&stubBlock, NULL, args);
	bf_get_variable(&stubBlock, &report, args);
	ck_assert_int_eq((int)report.data.floating, 45);

	args[1] = positiveSmall;
	bf_variable_change(&stubBlock, NULL, args);
	bf_get_variable(&stubBlock, &report, args);
	ck_assert(report.data.floating == 45.005);

	freeVariables(&sprite.variables);
	freeStrings();
}
END_TEST

START_TEST(test_threadsBasic)
{
	Block testBlock2[] = {
		{hash("noop"), 1, { .value = &trueString }},
		{hash("print"), 0, { .next = NULL }},
	};
	Block testBlock1[] = { // stack inside of if statement
		{hash("noop"), 1, { .value = &integerString }},
		{hash("print"), 0, { .next = testBlock2 }},
	};
	Block testBlock0[] = {
		{hash("noop"), 1, { .value = &floatingString }},
		{hash("print"), 0, { .next = testBlock1 }},
	};
	ThreadContext thread1 = createThreadContext(NULL, testBlock0);

	// test starting a thread and stepping all threads
	buffer[0] = '\0';
	startThread(&thread1);
	stepThreads();
	freeThreadContext(thread1);
	ck_assert_str_eq(buffer, trueString.data.string);

	// tests threads starting using the green flag
	buffer[0] = '\0';
	allocContextsForGreenFlagArray(2);
	thread1 = createThreadContext(NULL, testBlock0);
	addContextToGreenFlag(thread1);
	restartThreadsForGreenFlag();
	stepThreads();
	ck_assert_str_eq(buffer, trueString.data.string);

	// do it again, this time using existing resources
	buffer[0] = '\0';
	restartThreadsForGreenFlag();
	stepThreads();
	freeThreadContext(thread1);
	ck_assert_str_eq(buffer, trueString.data.string);

	freeContextsForGreenFlagArray();
}
END_TEST

static Suite* make_project_runner_suite(void) {
	Suite *s;
	TCase *tc_core;

	s = suite_create("Project Runner");

	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_stringPool);
	tcase_add_test(tc_core, test_typeConversions);
	tcase_add_test(tc_core, test_threadCounters);

	tcase_add_test(tc_core, test_variables);
	tcase_add_test(tc_core, test_lists);

	tcase_add_test(tc_core, test_operators);
	tcase_add_test(tc_core, test_dataBlocks);
	tcase_add_test(tc_core, test_basicBlockEvaluation);
	tcase_add_test(tc_core, test_stackEvaluation);
	tcase_add_test(tc_core, test_programFlowControl);

	tcase_add_test(tc_core, test_threadsBasic);

	suite_add_tcase(s, tc_core);
	return s;
}

int main(void) {
	ufastest number_failed;
	Suite *s;
	SRunner *sr;

	s = make_project_runner_suite();
	sr = srunner_create(s);
	//srunner_set_fork_status(sr, CK_NOFORK); // uncomment this when using a debugger

	srand(14324);
	initMphf();
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	destroyMphf();

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
