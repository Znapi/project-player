/**
	Runtime Library
			runtime_lib.c

	A library of functions, one for each Scratch block.
	This is a port of all of the block primitives in scratch-flash/primitives/, with some
	control block prims in scratch-flash/interpreter/Interpreter.as.

	This file is meant to just be included in another C file rather than linked.
**/

/* 
	TODO
		make string manupulation utf-8 compatible
		finish stop_scripts when threading is added, and add cloning sometime
		showing/hiding monitors when graphics are implemented
*/

static Block* bf_noop(const Block *const block, Value *const reportSlot, const Value arg[]) {return block->p.next;}

static Block* bf_add(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 + arg1;
	return NULL;
}

static Block* bf_subtract(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 - arg1;
	return NULL;
}

static Block* bf_multiply(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 * arg1;
	return NULL;
}

static Block* bf_divide(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 / arg1;
	return NULL;
}

static Block* bf_modulo(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);

	double r = fmod(arg0, arg1);
	if ((r<0 || arg1<0) && !(arg0<0 && arg1<0)) r+=arg1; // calculate modulo

	reportSlot->data.floating = r;
	reportSlot->type = FLOATING;
	return NULL;
}

static Block* bf_round(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double r = round(toFloating(arg[0]));
	reportSlot->type = FLOATING;
	reportSlot->data.floating = r;
	return NULL;
}

static Block* bf_is_less(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);
	reportSlot->data.boolean = arg0 < arg1;

	reportSlot->type = BOOLEAN;
	return NULL;
}

static Block* bf_is_equal(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0, arg1;
	if(tryToFloating(arg[0], &arg0)) {
		if(tryToFloating(arg[1], &arg1))
			reportSlot->data.boolean = (arg0 == arg1);
		else
			reportSlot->data.boolean = false;
	}
	else {
		if(tryToFloating(arg[1], &arg1))
			reportSlot->data.boolean = false;
		else if(strcmp(arg[0].data.string, arg[1].data.string) == 0)
			reportSlot->data.boolean = true;
		else
			reportSlot->data.boolean = false;
	}
	reportSlot->type = BOOLEAN;
	return NULL;
}

static Block* bf_is_greater(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double arg0 = toFloating(arg[0]);
	double arg1 = toFloating(arg[1]);
	reportSlot->data.boolean = arg0 > arg1;

	reportSlot->type = BOOLEAN;
	return NULL;
}

static Block* bf_logical_and(const Block *const block, Value *const reportSlot, const Value arg[]) {
	reportSlot->type = BOOLEAN;
	reportSlot->data.boolean = toBoolean(arg[0]) && toBoolean(arg[1]);
	return NULL;
}

static Block* bf_logical_or(const Block *const block, Value *const reportSlot, const Value arg[]) {
	reportSlot->type = BOOLEAN;
	reportSlot->data.boolean = toBoolean(arg[0]) || toBoolean(arg[1]);
	return NULL;
}

static Block* bf_logical_not(const Block *const block, Value *const reportSlot, const Value arg[]) {
	reportSlot->type = BOOLEAN;
	reportSlot->data.boolean = !toBoolean(arg[0]);
	return NULL;
}

// bad, but functional, random number generation
static Block* bf_generate_random(const Block *const block, Value *const reportSlot, const Value arg[]) {
	double low = toFloating(arg[0]), high = toFloating(arg[1]);
	if(low > high) { // if the bounds are out of order
		double tmp = low;
		low = high;
		high = tmp;
	}

	double random = (double)rand() / RAND_MAX;
	random *= high - low;
	random += low;
	if(round(low) == low && round(high) == high) // if the bounds are whole numbers
		random = round(random); // then make the random number a whole number

	reportSlot->type = FLOATING;
	reportSlot->data.floating = random;
	return NULL;
}

static Block* bf_compute_math_function(const Block *const block, Value *const reportSlot, const Value arg[]) {
	char *function;
	toString(arg[0], &function);

	switch(function[1]) { // switch with the second letter
	case 'b': // abs
		reportSlot->data.floating = fabs(toFloating(arg[1]));
		reportSlot->type = FLOATING;
		break;
	case 'l': // floor
		reportSlot->data.floating = floor(toFloating(arg[1]));
		reportSlot->type = FLOATING;
		break;
	case 'e': // ceiling
		reportSlot->data.floating = ceil(toFloating(arg[1]));
		reportSlot->type = FLOATING;
		break;
	case 'q': // sqrt
		reportSlot->data.floating = sqrt(toFloating(arg[1])); // there is no sqrt() with an integer parameter
		reportSlot->type = FLOATING;
		break;
	case 'i': // sin
		reportSlot->data.floating = sin(toFloating(arg[1]) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 'o': // cos or log
		if(function[0] == 'c') {
			reportSlot->data.floating = sin(toFloating(arg[1]) * (M_PI/180));
			reportSlot->type = FLOATING;
			break;
		}
		else if(function[0] == 'l') {
			reportSlot->data.floating = log10(toFloating(arg[1]));
			reportSlot->type = FLOATING;
			break;
		}
		else
			break;
	case 'a': // tan
		reportSlot->data.floating = tan(toFloating(arg[1]) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 's': // asin
		reportSlot->data.floating = asin(toFloating(arg[1]) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 'c': // acos
		reportSlot->data.floating = acos(toFloating(arg[1]) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 't': // atan
		reportSlot->data.floating = atan(toFloating(arg[1]) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 'n': // ln
		reportSlot->data.floating = log(toFloating(arg[1])); // this is log base e
		reportSlot->type = FLOATING;
		break;
	case ' ': // e ^
		reportSlot->data.floating = pow(M_E, toFloating(arg[1]));
		reportSlot->type = FLOATING;
		break;
	case '0': // 10 ^
		reportSlot->data.floating = pow(10, toFloating(arg[1]));
		reportSlot->type = FLOATING;
		break;
	}

	//free(function);
	return NULL;
}

static Block* bf_concatenate(const Block *const block, Value *const reportSlot, const Value arg[]) {
	char *arg0, *arg1, *r;
	toString(arg[0], &arg0);
	toString(arg[1], &arg1);
	r = allocString(strlen(arg0) + strlen(arg1) + 1);
	strcpy(r, arg0);
	strcat(r, arg1);

	reportSlot->type = STRING;
	reportSlot->data.string = r;
	//free(arg0);
	//free(arg1);
	return NULL;
}

static Block* bf_get_string_length(const Block *const block, Value *const reportSlot, const Value arg[]) {
	char *s;
	toString(arg[0], &s);
	int64 l = strlen(s);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = (double)l;
	//free(s);
	return NULL;
}

static Block* bf_get_character(const Block *const block, Value *const reportSlot, const Value arg[]) {
	int64 index = toInteger(arg[0]) - 1; // subtract one because Scratch indices start at one, not zero
	char *s;
	toString(arg[1], &s);

	char *r = allocString(2);
	r[0] = s[index];
	r[1] = '\0';
	reportSlot->type = STRING;
	reportSlot->data.string = r;
	return NULL;
}

// Control
static Block* bf_do_if(const Block *const block, Value *const reportSlot, const Value arg[]) {
	if(toBoolean(arg[0])) {
		//puts("TRUE");
		// go inside C of if block
		return block->p.subStacks[0]; // advance thread to stub block inside of if block
	}
	else {
		//puts("FALSE");
		// skip past C if block
		return block->p.subStacks[1]; // advance thread to stub block at end of if block
	}
}

static Block* bf_do_if_else(const Block *const block, Value *const reportSlot, const Value arg[]) {
	if(toBoolean(arg[0]))
		return block->p.subStacks[0];
	else
		return block->p.subStacks[1];
}

static Block* bf_do_until(const Block *const block, Value *const reportSlot, const Value arg[]) {
	if(toBoolean(arg[0]))
		return block->p.subStacks[1];
	else
		return block->p.subStacks[0];
}

static Block* bf_do_wait_until(const Block *const block, Value *const reportSlot, const Value arg[]) {
	if(toBoolean(arg[0]))
		return block->p.subStacks[1];
	else
		return block->p.subStacks[0];
}

static Block* bf_do_repeat(const Block *const block, Value *const reportSlot, const Value arg[]) {
	if(allocCounter(block))
		usetCounter((uint16)toInteger(arg[0]));

	if(ugetCounter() > 0) {
		usetCounter(ugetCounter()-1);
		return block->p.subStacks[0];
	}
	else {
		freeCounter();
		return block->p.subStacks[1];
	}
}

static Block* bf_do_wait(const Block *const block, Value *const reportSlot, const Value arg[]) {
	if(allocCounter(block))
		fsetCounter((float)toFloating(arg[0]) * CLOCKS_PER_SEC);

	if(fgetCounter() > 0) {
		fsetCounter(fgetCounter()-dtime);
		return block->p.subStacks[0];
	}
	else {
		freeCounter();
		return block->p.subStacks[1];
	}
}

static Block* bf_stop_scripts(const Block *const block, Value *const reportSlot, const Value arg[]) {
	return NULL; // stop the thread
}

static Block* bf_get_variable(const Block *const block, Value *const reportSlot, const Value arg[]) {
	char *name;
	toString(arg[0], &name);
	*reportSlot = getVariable(&activeContext->spriteCtx->variables, name);
	return block->p.next;
}

static Block* bf_variable_set(const Block *const block, Value *const reportSlot, const Value arg[]) {
	char *name;
	toString(arg[0], &name);
	setVariable(&activeContext->spriteCtx->variables, name, copySimplifiedValue(arg[1]));
	return block->p.next;
}

static Block* bf_variable_change(const Block *const block, Value *const reportSlot, const Value arg[]) {
	char *name;
	toString(arg[0], &name);

	Value value = getVariable(&activeContext->spriteCtx->variables, name);
	double incr = toFloating(arg[1]);
	value.data.floating = toFloating(value);
	value.data.floating += incr;
	value.type = FLOATING;
	setVariable(&activeContext->spriteCtx->variables, name, value);

	return block->p.next;
}

static Block* bf_get_list_contents(const Block *const block, Value *const reportSlot, const Value arg[]) {
	return block->p.next;
}

static Block* bf_say(const Block*block, Value *const reportSlot, const Value arg[]) {
	char *msg;
	toString(arg[0], &msg);
	printf("bf_say: ");
	puts(msg);
	return block->p.next;
}

static Block* bf_test_print(const Block *block, Value *const reportSlot, const Value arg[]);
#ifndef TESTING
static Block* bf_test_print(const Block *const block, Value *const reportSlot, const Value arg[]) {return block->p.next;}
#endif
