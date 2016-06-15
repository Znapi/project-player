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

#define BF(name) static Block* bf_##name(const Block *const block, Value *const reportSlot, const Value arg[])

static Block* bf_noop(const Block *const block, Value *const reportSlot, const Value arg[]) {puts("NOOP CALLED!"); return block->p.next;}

BF(add) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 + arg1;
	return NULL;
}

BF(subtract) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 - arg1;
	return NULL;
}

BF(multiply) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 * arg1;
	return NULL;
}

BF(divide) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = arg0 / arg1;
	return NULL;
}

BF(modulo) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);

	double r = fmod(arg0, arg1);
	if ((r<0 || arg1<0) && !(arg0<0 && arg1<0)) r+=arg1; // calculate modulo

	reportSlot->data.floating = r;
	reportSlot->type = FLOATING;
	return NULL;
}

BF(round) {
	double r = round(toFloating(arg+0));
	reportSlot->type = FLOATING;
	reportSlot->data.floating = r;
	return NULL;
}

BF(is_less) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);
	reportSlot->data.boolean = arg0 < arg1;

	reportSlot->type = BOOLEAN;
	return NULL;
}

BF(is_equal) {
	double arg0, arg1;
	if(tryToFloating(arg+0, &arg0)) {
		if(tryToFloating(arg+1, &arg1))
			reportSlot->data.boolean = (arg0 == arg1);
		else
			reportSlot->data.boolean = false;
	}
	else {
		if(tryToFloating(arg+1, &arg1))
			reportSlot->data.boolean = false;
		else if(strcmp(arg[0].data.string, arg[1].data.string) == 0)
			reportSlot->data.boolean = true;
		else
			reportSlot->data.boolean = false;
	}
	reportSlot->type = BOOLEAN;
	return NULL;
}

BF(is_greater) {
	double arg0 = toFloating(arg+0);
	double arg1 = toFloating(arg+1);
	reportSlot->data.boolean = arg0 > arg1;

	reportSlot->type = BOOLEAN;
	return NULL;
}

BF(logical_and) {
	reportSlot->type = BOOLEAN;
	reportSlot->data.boolean = toBoolean(arg+0) && toBoolean(arg+1);
	return NULL;
}

BF(logical_or) {
	reportSlot->type = BOOLEAN;
	reportSlot->data.boolean = toBoolean(arg+0) || toBoolean(arg+1);
	return NULL;
}

BF(logical_not) {
	reportSlot->type = BOOLEAN;
	reportSlot->data.boolean = !toBoolean(arg+0);
	return NULL;
}

// bad, but functional, random number generation
BF(generate_random) {
	double low = toFloating(arg+0), high = toFloating(arg+1);
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

BF(compute_math_function) {
	char *function;
	toString(arg+0, &function);

	switch(function[1]) { // switch with the second letter
	case 'b': // abs
		reportSlot->data.floating = fabs(toFloating(arg+1));
		reportSlot->type = FLOATING;
		break;
	case 'l': // floor
		reportSlot->data.floating = floor(toFloating(arg+1));
		reportSlot->type = FLOATING;
		break;
	case 'e': // ceiling
		reportSlot->data.floating = ceil(toFloating(arg+1));
		reportSlot->type = FLOATING;
		break;
	case 'q': // sqrt
		reportSlot->data.floating = sqrt(toFloating(arg+1));
		reportSlot->type = FLOATING;
		break;
	case 'i': // sin
		reportSlot->data.floating = sin(toFloating(arg+1) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 'o': // cos or log
		if(function[0] == 'c') {
			reportSlot->data.floating = sin(toFloating(arg+1) * (M_PI/180));
			reportSlot->type = FLOATING;
			break;
		}
		else if(function[0] == 'l') {
			reportSlot->data.floating = log10(toFloating(arg+1));
			reportSlot->type = FLOATING;
			break;
		}
		else
			break;
	case 'a': // tan
		reportSlot->data.floating = tan(toFloating(arg+1) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 's': // asin
		reportSlot->data.floating = asin(toFloating(arg+1) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 'c': // acos
		reportSlot->data.floating = acos(toFloating(arg+1) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 't': // atan
		reportSlot->data.floating = atan(toFloating(arg+1) * (M_PI/180));
		reportSlot->type = FLOATING;
		break;
	case 'n': // ln
		reportSlot->data.floating = log(toFloating(arg+1)); // this is log base e
		reportSlot->type = FLOATING;
		break;
	case ' ': // e ^
		reportSlot->data.floating = pow(M_E, toFloating(arg+1));
		reportSlot->type = FLOATING;
		break;
	case '0': // 10 ^
		reportSlot->data.floating = pow(10, toFloating(arg+1));
		reportSlot->type = FLOATING;
		break;
	}

	return NULL;
}

BF(concatenate) {
	char *arg0, *arg1, *r;
	toString(arg+0, &arg0);
	toString(arg+1, &arg1);
	r = strpool_alloc(strlen(arg0) + strlen(arg1) + 1);
	strcpy(r, arg0);
	strcat(r, arg1);

	reportSlot->type = STRING;
	reportSlot->data.string = r;
	return NULL;
}

BF(get_string_length) {
	char *s;
	toString(arg+0, &s);
	int64 l = strlen(s);

	reportSlot->type = FLOATING;
	reportSlot->data.floating = (double)l;
	return NULL;
}

BF(get_character) {
	int64 index = toInteger(arg+0) - 1; // subtract one because Scratch indices start at one, not zero
	char *s;
	toString(arg+1, &s);

	char *r = strpool_alloc(2);
	r[0] = s[index];
	r[1] = '\0';
	reportSlot->type = STRING;
	reportSlot->data.string = r;
	return NULL;
}

// Control
BF(do_if) {
	if(toBoolean(arg+0)) {
		// go inside C of if block
		pushStackFrame(block->p.subStacks[1]);
		return block->p.subStacks[0]; // advance thread to stub block inside of if block
	}
	else {
		// skip past C if block
		return block->p.subStacks[1]; // advance thread to stub block at end of if block
	}
}

BF(do_if_else) {
	pushStackFrame(block->p.subStacks[2]);
	if(toBoolean(arg+0))
		return block->p.subStacks[0];
	else
		return block->p.subStacks[1];
}

BF(do_until) {
	if(toBoolean(arg+0)) {
		// exit/skip loop
		return block->p.subStacks[1];
	}
	else {
		// enter loop
		pushStackFrame(activeContext->nextBlock); // return to this block after substack inside loop is finished
		return block->p.subStacks[0];
	}
}

BF(do_wait_until) {
	if(toBoolean(arg+0))
		return (void*)block;
	else
		return block->p.next;
}

BF(do_repeat) {
	if(allocCounter(block))
		usetCounter((uint16)toInteger(arg+0));

	if(ugetCounter() != 0) {
		usetCounter(ugetCounter()-1);
		pushStackFrame(activeContext->nextBlock);
		return block->p.subStacks[0];
	}
	else {
		freeCounter();
		return block->p.subStacks[1];
	}
}

BF(do_wait) {
	if(allocCounter(block)) {
		fsetCounter((float)toFloating(arg+0) * CLOCKS_PER_SEC);
	}
	else if (fgetCounter() > 0) {
		fsetCounter(fgetCounter()-dtime);
	}
	else {
		freeCounter();
		return block->p.next;
	}
	return (void*)block;
}

BF(do_forever) {
	pushStackFrame(block);
	return block->p.subStacks[0];
}

BF(stop_scripts) {
	return NULL; // stop the thread
}

BF(get_variable) {

	char *name;
	toString(arg+0, &name);
	*reportSlot = getVariable(&activeContext->spriteCtx->variables, name);
	return NULL;
}

BF(variable_set) {
	char *name;
	toString(arg+0, &name);
	setVariable(&activeContext->spriteCtx->variables, name, arg+1);
	return block->p.next;
}

BF(variable_change) {
	char *name;
	toString(arg+0, &name);

	Value value = getVariable(&activeContext->spriteCtx->variables, name);
	double incr = toFloating(arg+1);
	value.data.floating = toFloating(&value);
	value.data.floating += incr;
	value.type = FLOATING;
	setVariable(&activeContext->spriteCtx->variables, name, &value);

	return block->p.next;
}

// TODO: this might be inefficient
BF(list_getContents) {
	char *str;
	toString(arg+0, &str);
	List *list = getListPtr(&activeContext->spriteCtx->lists, str);
	char **elements = malloc(list->length*sizeof(char**));
	if(elements == NULL) {
		reportSlot->data.floating = 0.0;
		reportSlot->type = FLOATING;
		puts("[ERROR]Could not allocate list of strings in bf_list_getContents.");
		return NULL;
	}
	uint32 i = 0, nRequiredChars = 0;
	ListElement *listElement = list->first;

	while(listElement != NULL) { // todo: lists where each element is a single character don't get spaces
		nRequiredChars += toString(&listElement->value, elements+i) + 1;
		listElement = listElement->next;
		++i;
	}

	str = strpool_alloc(nRequiredChars);
	reportSlot->data.string = str;
	reportSlot->type = STRING;

	for(i = 0; i < list->length; ++i) {
		str = stpcpy(str, elements[i]);
		*(str++) = ' ';
	}
	*(--str) = '\0';

	free(elements);
	return NULL;
}

BF(list_append) {
	char *name;
	toString(arg+1, &name);
	listAppend(getListPtr(&activeContext->spriteCtx->lists, name), arg+0);
	return block->p.next;
}

BF(list_delete) {
	char *name;
	toString(arg+1, &name);
	List *list = getListPtr(&activeContext->spriteCtx->lists, name);
	if(arg[0].type == STRING) {
		switch(arg[1].data.string[0]) {
		case '1': listDeleteFirst(list); return block->p.next;
		case 'l': listDeleteLast(list); return block->p.next;
		case 'a': listDeleteAll(list); return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		listDelete(list, (uint32)i);
	return block->p.next;
}

BF(list_insert) {
	char *name;
	toString(arg+1, &name);
	List *list = getListPtr(&activeContext->spriteCtx->lists, name);
	if(arg[0].type == STRING) {
		switch(arg[1].data.string[0]) {
		case '1': listPrepend(list, arg+2); return block->p.next;
		case 'l': listAppend(list, arg+2); return block->p.next;
		case 'r':
			listInsert(list, arg+2, (uint32)round((double)rand()/RAND_MAX * list->length));
			return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		listInsert(list, arg+2, (uint32)i);
	return block->p.next;
}

BF(list_setElement) {
	char *name;
	toString(arg+1, &name);
	List *list = getListPtr(&activeContext->spriteCtx->lists, name);
	if(arg[0].type == STRING) {
		switch(arg[1].data.string[0]) {
		case '1': listSetFirst(list, arg+2); return block->p.next;
		case 'l': listSetLast(list, arg+2); return block->p.next;
		case 'r':
			listSet(list, arg+2, (uint32)round((double)rand()/RAND_MAX * list->length));
			return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		listSet(list, arg+2, (uint32)i);
	return block->p.next;
}

BF(list_getElement) {
	char *name;
	toString(arg+1, &name);
	List *list = getListPtr(&activeContext->spriteCtx->lists, name);
	if(arg[0].type == STRING) {
		switch(arg[1].data.string[0]) {
		case '1': *reportSlot = listGetFirst(list); return block->p.next;
		case 'l': *reportSlot = listGetLast(list); return block->p.next;
		case 'r':
			*reportSlot = listGet(list, (uint32)round((double)rand()/RAND_MAX * list->length));
			return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		*reportSlot = listGet(list, (uint32)i);
	return block->p.next;
}

BF(list_contains) {
	char *name;
	toString(arg+0, &name);
	List *list = getListPtr(&activeContext->spriteCtx->lists, name);
	Value value = extractSimplifiedValue(arg+1);
	switch(value.type) {
	case FLOATING:
		reportSlot->data.boolean = listContainsFloating(list, value.data.floating);
		break;
	case BOOLEAN:
		reportSlot->data.boolean = listContainsBoolean(list, value.data.boolean);
		break;
	case STRING:
		reportSlot->data.boolean = listContainsString(list, value.data.string);
		break;
	}
	reportSlot->type = BOOLEAN;
	return NULL;
}

BF(list_length) {
	char *name;
	toString(arg+0, &name);
	reportSlot->type = FLOATING;
	reportSlot->data.floating = (double)getListPtr(&activeContext->spriteCtx->lists, name)->length;
	return NULL;
}

BF(say) {
	char *msg;
	toString(arg+0, &msg);
	printf("bf_say: ");
	puts(msg);
	return block->p.next;
}

BF(test_print);
#ifndef TESTING
BF(test_print) {return block->p.next;}
#endif
