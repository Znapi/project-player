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
		***make string manupulation utf-8 compatible***
		add cloning sometime
		showing/hiding monitors when graphics are implemented
*/

#define BF(name) static const Block* bf_##name(const Block *const block, Value *const reportSlot, const Value arg[])

BF(noop) { puts("NOOP CALLED!"); return block->p.next; }

/* Operators */

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

/* Control */

BF(do_if) {
	if(toBoolean(arg+0)) {
		// go inside C of if block
		enterSubstack(block->p.substacks[1]);
		return block->p.substacks[0]; // advance thread to stub block inside of if block
	}
	else {
		// skip past C if block
		return block->p.substacks[1]; // advance thread to stub block at end of if block
	}
}

BF(do_if_else) {
	enterSubstack(block->p.substacks[2]);
	if(toBoolean(arg+0))
		return block->p.substacks[0];
	else
		return block->p.substacks[1];
}

BF(do_until) {
	if(toBoolean(arg+0)) {
		// exit/skip loop
		return block->p.substacks[1];
	}
	else {
		doYield = true;
		// enter loop
		enterSubstack(activeThread->frame.nextBlock); // return to this block after substack inside loop is finished
		return block->p.substacks[0];
	}
}

BF(do_wait_until) {
	if(toBoolean(arg+0))
		return block;
	else {
		doYield = true;
		return block->p.next;
	}
}

BF(do_repeat) {
	if(allocTmpData(block))
		usetTmpData((uint16)toInteger(arg+0));

	if(ugetTmpData() != 0) {
		doYield = true;
		usetTmpData(ugetTmpData()-1);
		enterSubstack(activeThread->frame.nextBlock);
		return block->p.substacks[0];
	}
	else {
		freeTmpData();
		return block->p.substacks[1];
	}
}

BF(do_wait) {
	if(allocTmpData(block)) {
		fsetTmpData((float)toFloating(arg+0) * CLOCKS_PER_SEC);
	}
	else if (fgetTmpData() > 0) {
		fsetTmpData(fgetTmpData()-dtime);
	}
	else {
		freeTmpData();
		return block->p.next;
	}
	doYield = true;
	return block;
}

BF(do_forever) {
	doYield = true;
	enterSubstack(block);
	return block->p.substacks[0];
}

BF(stop_scripts) {
	char *str;
	toString(arg+0, &str);
	ThreadLink *current, *next;
	switch(str[0]) {
	case 'o': // other scripts in sprite
		current = &runningThreads;
		while(current != NULL) {
			next = current->next;
			if(&current->thread != activeThread)
				current->next = NULL;
			current = next;
		}
		return block->p.next;
	case 'a': // all scripts
		stopAllThreads();
	}
	return NULL; // stop this script
}

/* Data */

BF(get_variable) {
	char *name;
	toString(arg+0, &name);
	*reportSlot = getVariable(&activeSprite->variables, name);
	return NULL;
}

// TODO: add scoping
BF(variable_set) {
	char *name;
	toString(arg+0, &name);
	setVariable(&activeSprite->variables, name, arg+1);
	return block->p.next;
}

BF(variable_change) {
	char *name;
	toString(arg+0, &name);

	Value value = getVariable(&activeSprite->variables, name);
	double incr = toFloating(arg+1);
	value.data.floating = toFloating(&value);
	value.data.floating += incr;
	value.type = FLOATING;
	setVariable(&activeSprite->variables, name, &value);

	return block->p.next;
}

// TODO: this might be inefficient
BF(list_getContents) {
	char *str;
	toString(arg+0, &str);
	List *list = getListPtr(&activeSprite->lists, str);
	char **elements = malloc(list->length*sizeof(char**));
	if(elements == NULL) {
		reportSlot->data.floating = 0.0;
		reportSlot->type = FLOATING;
		puts("[ERROR]Could not allocate list of strings in bf_list_getContents.");
		return NULL;
	}
	uint32 i = 0, nRequiredChars = 0;
	ListElement *listElement = list->first;

	while(listElement != NULL) {
		nRequiredChars += toString(&listElement->value, elements+i);
		listElement = listElement->next;
		++i;
	}

	if(nRequiredChars == list->length)
		++nRequiredChars; // make room for terminator
	else
		nRequiredChars += list->length;
	str = strpool_alloc(nRequiredChars);
	reportSlot->data.string = str;
	reportSlot->type = STRING;

	if(nRequiredChars == list->length + 1) { // if each element is a single character
		for(i = 0; i < list->length; ++i)
			str = stpcpy(str, elements[i]);
	}
	else {
		for(i = 0; i < list->length; ++i) {
			str = stpcpy(str, elements[i]);
			*(str++) = ' ';
		}
	}

	free(elements);
	return NULL;
}

BF(list_append) {
	char *name;
	toString(arg+1, &name);
	listAppend(getListPtr(&activeSprite->lists, name), arg+0);
	return block->p.next;
}

BF(list_delete) {
	char *name;
	toString(arg+1, &name);
	List *list = getListPtr(&activeSprite->lists, name);
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
	List *list = getListPtr(&activeSprite->lists, name);
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
	List *list = getListPtr(&activeSprite->lists, name);
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
	List *list = getListPtr(&activeSprite->lists, name);
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
	List *list = getListPtr(&activeSprite->lists, name);
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
	reportSlot->data.floating = (double)getListPtr(&activeSprite->lists, name)->length;
	return NULL;
}

/* Custom Blocks (More Blocks, but no extensions) */

BF(call) {
	char *procName;
	const uint32 procNameLen = toString(arg+0, &procName);
	const struct ProcedureLink *const procedure = getProcedure(procName, procNameLen);

	dynarray_push_back(activeThread->nParametersStack, (void*)&procedure->nParameters);
	for(uint16 j = 1; j < procedure->nParameters+1; ++j)
		dynarray_push_back(activeThread->parametersStack, (void*)(arg+j));
	activeThread->parameters = (Value*)dynarray_eltptr(activeThread->parametersStack, dynarray_len(activeThread->parametersStack) - procedure->nParameters);

	enterProcedure(block->p.next);
	return procedure->script;
}

BF(getParam) {
	*reportSlot = activeThread->parameters[arg[0].data.integer];
	return NULL;
}

/* Events */

BF(broadcast) {
	char *msg;
	uint16 msgLen = toString(arg+0, &msg);
	if(startBroadcastThreads(msg, msgLen, NULL)) {
		doYield = true;
		return activeThread->topBlock;
	}
	else
		return block->p.next;
}

// This one's implementation is kinda hacky. Basically it allocates a pointer to the
// BroadcastThreads structure for the given message, and the BroadcastsThreads also gets a
// pointer to this new pointer. This function uses the pointer to access the array of
// threads for the given message, so that it can check if all of those threads are
// stopped. The BroadcastThreads uses it's pointer to nullify the pointer this function
// allocated, acting as a notification that the message was re-sent, which is undetectable
// from this function alone.
// This function also uses it's pointer to the BroadcastThreads to nullify the
// BroadcastThreads's pointer to this functions allocated pointer when this function
// detects that it is done waiting and frees it's allocated pointer. This prevents memory
// being written when it shouldn't be. Did I mention that this has to do with pointers?
BF(broadcast_and_wait) {
	if(allocTmpData(block))	{
		char *msg;
		uint16 msgLen = toString(arg+0, &msg);
		doYield = true;
		if(startBroadcastThreads(msg, msgLen, (struct BroadcastThreads**)&getTmpDataPointer()->p)) // set the counter to the number of broadcast threads
			return activeThread->topBlock;
		else
			return block;
	}
	else {
		struct BroadcastThreads *const broadcastLink = pgetTmpData();
		if(broadcastLink == NULL) // if the broadcast message was sent by another thread
			return block->p.next; // don't continue; start at the top
		else { // check each broadcast thread for whether or not it was stopped
		  ThreadLink **threadPtr = (ThreadLink**)dynarray_front(broadcastLink->threads);
			do {
				if(!isThreadStopped((*threadPtr)->thread)) {
					doYield = true;
					return block;
				}
				threadPtr = (ThreadLink**)dynarray_next(broadcastLink->threads, threadPtr);
			} while(threadPtr != NULL);
			broadcastLink->nullifyOnRestart = NULL; // empty this field out so that when the message is broadcast again we don't overwrite memory
			return block->p.next; // if all broadcast threads are stopped, continue on to next block
		}
	}
}

/* Looks */

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
