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
*/

#define BF(name) static const Block* bf_##name(const Block *const block, Value *const reportSlot, const Value arg[])

BF(noop) {
	puts("noop called"); // simple notification for debugging purposes
	if(reportSlot != NULL) {        // if the run time is expecting this block to report
		reportSlot->type = FLOATING; // something, better fill out the report slot to prevent
		reportSlot->data.floating = 0.0; // weird behavior or accessing a freed string
	}
	return block->p.next;
}

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
		usetTmpData((uint32)toInteger(arg+0));

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
	switch(str[0]) {
	case 'o': // other scripts in sprite
		stopThreadsForSprite();
		return block->p.next;
	case 'a': // all scripts
		stopAllThreads();
	}
	return NULL; // stop this script
}

BF(clone) {
	if(activeSprite->scope != STAGE) {
		SpriteContext *clone = malloc(sizeof(SpriteContext));
		memcpy(clone, activeSprite, sizeof(SpriteContext));
		clone->scope = CLONE;

		clone->threads = malloc(activeSprite->nThreads*sizeof(ThreadLink)); // don't check for 0 threads because it must have at least one to even be creating clones
		clone->nThreads = activeSprite->nThreads;
		for(uint16 i = 0; i < clone->nThreads; ++i) {
			ThreadLink *link = clone->threads+i;
			threadContext_init(&link->thread, activeSprite->threads[i].thread.topBlock);
			link->sprite = clone;
			link->prev = link->next = NULL;
		}

		clone->variables = copyVariables((const Variable *const *const)&activeSprite->variables); // not sure why the typecast is needed to suppress warinings
		clone->lists = copyLists((const List *const *const)&activeSprite->lists);

		threadList_copyArray(&clone->whenClonedThreads, &activeSprite->whenClonedThreads, clone->threads, activeSprite->threads);
		if(clone->nBroadcastThreadLists != 0) {
			clone->broadcastThreadLists = malloc(clone->nBroadcastThreadLists*sizeof(ThreadList*));
			for(uint16 i = 0; i < clone->nBroadcastThreadLists; ++i) {
				clone->broadcastThreadLists[i] = threadList_copy(activeSprite->broadcastThreadLists[i], clone->threads, activeSprite->threads);
				threadList_insert(activeSprite->broadcastThreadLists[i], clone->broadcastThreadLists[i]);
			}
		}

		startThreadsInList(&clone->whenClonedThreads);
	}
	return block->p.next;
}

BF(destroy_clone) {
	if(activeSprite->scope == CLONE) {
		stopThreadsForSprite();

		for(uint16 i = 0; i < activeSprite->nThreads; ++i)
			threadContext_done(&activeSprite->threads[i].thread);
		destroy = activeSprite->threads;

		freeVariables(&activeSprite->variables);
		freeLists(&activeSprite->lists);

		threadList_done(&activeSprite->whenClonedThreads);
		for(uint16 i = 0; i < activeSprite->nBroadcastThreadLists; ++i) {
			threadList_remove(NULL, activeSprite->broadcastThreadLists[i]);
			threadList_free(activeSprite->broadcastThreadLists[i]);
		}

		free(activeSprite);
	}
	return block->p.next;
}

/* Data */

BF(get_variable) {
	char *name;
	const size_t nameLen = toString(arg+0, &name);
	if(getVariable(&activeSprite->variables, name, reportSlot)) {
		if(getVariable(&stage->variables, name, reportSlot)) {
			variable_new(&activeSprite->variables, name, nameLen, NULL);
		}
	}
	return NULL;
}

BF(variable_set) {
	char *name;
	const size_t nameLen = toString(arg+0, &name);
	if(setVariable(&activeSprite->variables, name, arg+1)) {
		if(setVariable(&stage->variables, name, arg+1))
			variable_new(&activeSprite->variables, name, nameLen, arg+1);
	}
	return block->p.next;
}

BF(variable_change) {
	char *name;
	const size_t nameLen = toString(arg+0, &name);

	Value value;
	Variable **variables = &activeSprite->variables;
	if(getVariable(&activeSprite->variables, name, &value)) {
		if(getVariable(&stage->variables, name, &value))
			variable_new(&activeSprite->variables, name, nameLen, NULL);
		else
			variables = &stage->variables;
	}
	double incr = toFloating(arg+1);
	value.data.floating = toFloating(&value);
	value.data.floating += incr;
	value.type = FLOATING;
	setVariable(variables, name, &value);

	return block->p.next;
}

#define getOrCreateList(name, nameLen, list) {										\
		if(getListContents(&activeSprite->lists, name, &list)) {			\
			if(getListContents(&stage->lists, name, &list))							\
				list = list_new(&activeSprite->lists, name, nameLen);			\
		}																															\
	}

// TODO: this might be inefficient
BF(list_getContents) {
	char *str;
	size_t nRequiredChars = toString(arg+0, &str);

	UT_array *list;
	getOrCreateList(str, nRequiredChars, list);

	char **elements = malloc(utarray_len(list)*sizeof(char**));
	if(elements == NULL) {
		reportSlot->data.floating = 0.0;
		reportSlot->type = FLOATING;
		puts("[ERROR]Could not allocate list of strings in bf_list_getContents.");
		return NULL;
	}
	nRequiredChars = 0;
	for(uint32 i = 0; i < utarray_len(list); ++i)
		nRequiredChars += toString((Value*)utarray_eltptr(list, i), elements+i);

	if(nRequiredChars != utarray_len(list))
		nRequiredChars += utarray_len(list); // make room for spaces
	++nRequiredChars; // make room for terminator
	str = strpool_alloc(nRequiredChars);
	reportSlot->data.string = str;
	reportSlot->type = STRING;

	if(nRequiredChars == utarray_len(list) + 1) { // if each element is a single character
		for(uint32 i = 0; i < utarray_len(list); ++i)
			str = stpcpy(str, elements[i]);
	}
	else {
		for(uint32 i = 0; i < utarray_len(list); ++i) {
			str = stpcpy(str, elements[i]);
			*(str++) = ' ';
		}
		*(str-1) = '\0';
	}

	free(elements);
	return NULL;
}

BF(list_append) {
	char *name;
	const size_t nameLen = toString(arg+1, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
	listAppend(list, arg+0);
	return block->p.next;
}

BF(list_delete) {
	char *name;
	const size_t nameLen = toString(arg+1, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
	if(arg[0].type == STRING) {
		switch(arg[0].data.string[0]) {
		case '1': listDeleteFirst(list); return block->p.next;
		case 'l': listDeleteLast(list); return block->p.next;
		case 'a': listDeleteAll(list); return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		listDelete(list, (uint32)i-1);
	return block->p.next;
}

BF(list_insert) {
	char *name;
	const size_t nameLen = toString(arg+1, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
	if(arg[0].type == STRING) {
		switch(arg[0].data.string[0]) {
		case '1': listPrepend(list, arg+2); return block->p.next;
		case 'l': listAppend(list, arg+2); return block->p.next;
		case 'r':
			listInsert(list, arg+2, (uint32)round((double)rand()/RAND_MAX * utarray_len(list)));
			return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		listInsert(list, arg+2, (uint32)i-1);
	return block->p.next;
}

BF(list_setElement) {
	char *name;
	const size_t nameLen = toString(arg+1, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
	if(arg[0].type == STRING) {
		switch(arg[0].data.string[0]) {
		case '1': listSetFirst(list, arg+2); return block->p.next;
		case 'l': listSetLast(list, arg+2); return block->p.next;
		case 'r':
			listSet(list, arg+2, (uint32)round((double)rand()/RAND_MAX * utarray_len(list)));
			return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		listSet(list, arg+2, (uint32)i-1);
	return block->p.next;
}

BF(list_getElement) {
	char *name;
	const size_t nameLen = toString(arg+1, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
	if(arg[0].type == STRING) {
		switch(arg[0].data.string[0]) {
		case '1': *reportSlot = listGetFirst(list); return block->p.next;
		case 'l': *reportSlot = listGetLast(list); return block->p.next;
		case 'r':
			*reportSlot = listGet(list, (uint32)round((double)rand()/RAND_MAX * utarray_len(list)));
			return block->p.next;
		}
	}
	double i;
	if(tryToFloating(arg+0, &i))
		*reportSlot = listGet(list, (uint32)i-1);
	return block->p.next;
}

BF(list_contains) {
	char *name;
	const size_t nameLen = toString(arg+0, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
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
		free(value.data.string);
		break;
	}
	reportSlot->type = BOOLEAN;
	return NULL;
}

BF(list_length) {
	char *name;
	const size_t nameLen = toString(arg+0, &name);
	UT_array *list;
	getOrCreateList(name, nameLen, list);
	reportSlot->type = FLOATING;
	reportSlot->data.floating = (double)utarray_len(list);
	return NULL;
}

/* Custom Blocks (More Blocks, but no extensions) */

BF(call) {
	char *procName;
	const size_t procNameLen = toString(arg+0, &procName);
	const struct ProcedureLink *const procedure = getProcedure(procName, procNameLen);

	dynarray_push_back(&activeThread->nParametersStack, (void*)&procedure->nParameters);
	for(uint16 j = 1; j < procedure->nParameters+1; ++j)
		dynarray_push_back(&activeThread->parametersStack, (void*)(arg+j));
	activeThread->parameters = (Value*)dynarray_eltptr(&activeThread->parametersStack, dynarray_len(&activeThread->parametersStack) - procedure->nParameters);

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
	size_t msgLen = toString(arg+0, &msg);
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
		size_t msgLen = toString(arg+0, &msg);
		doYield = true;
		if(startBroadcastThreads(msg, msgLen, (struct BroadcastThreads**)&getTmpDataPointer()->d.p)) // set the counter to the number of broadcast threads
			return activeThread->topBlock;
		else
			return block;
	}
	else {
		struct BroadcastThreads *const broadcastLink = pgetTmpData();
		if(broadcastLink == NULL) // if the broadcast message was sent by another thread
			return block->p.next; // don't continue; start at the top
		else { // check each broadcast thread for whether or not it was stopped
			struct ThreadList *threadList;
			ThreadLink **threadLink;
			THREADLIST_ITER(broadcastLink->threadList, threadList) {
				threadLink = threadList->array+0;
				for(uint16 i = 0; i < threadList->nThreads; ++i) {
					if(!isThreadStopped((*threadLink)->thread)) {
						doYield = true;
						return block;
					}
					++threadLink;
				}
			}
			broadcastLink->nullifyOnRestart = NULL; // empty this field out so that when the message is broadcast again we don't overwrite memory
			return block->p.next; // if all broadcast threads are stopped, continue on to next block
		}
	}
}

/* Sensing */

BF(prompt) { // TODO: this is a temporary command line based implementation until graphics are implemented
	char *msg;
	toString(arg+0, &msg);
	printf("%s prompts: %s\n>> ", activeSprite->name, msg);
	fgets(askResponse.d, 1024, stdin);
	askResponse.i = strlen(askResponse.d);
	*(askResponse.d+askResponse.i-1) = '\0'; // remove newline
	return block->p.next;
}

BF(prompt_get) {
	reportSlot->type = STRING;
	reportSlot->data.string = strpool_alloc(askResponse.i);
	memcpy(reportSlot->data.string, askResponse.d, askResponse.i*sizeof(char));
	return NULL;
}

BF(timer_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = (double)(currentTime - lastTimerReset) / (double)CLOCKS_PER_SEC;
	return NULL;
}

BF(timer_reset) {
	lastTimerReset = currentTime;
	return block->p.next;
}

BF(distanceToSprite) { // TODO: distance to _mouse_
	char *spriteName;
	const size_t nameLen = toString(arg+0, &spriteName);
	const SpriteContext *const target = getSprite(spriteName, nameLen);
	if(target == NULL)
		reportSlot->data.floating = 0.0;
	else
		reportSlot->data.floating = hypot(activeSprite->xpos - target->xpos, activeSprite->ypos - target->ypos);
	reportSlot->type = FLOATING;
	return NULL;
}

// TODO: optimize
BF(attribute_get) {
	char *attribute, *spriteName;

	size_t len = toString(arg+1, &spriteName);
	SpriteContext *const sprite = getSprite(spriteName, len);

	len = toString(arg+0, &attribute);

#define RETURN_NONE() {reportSlot->type = FLOATING; reportSlot->data.floating = 0.0; return NULL;}
#define RETURN_FLOAT(att) {reportSlot->type = FLOATING; reportSlot->data.floating = sprite->att; return NULL;}
	if(sprite->scope != STAGE) {
		if(strncmp("x position", attribute, len) == 0) RETURN_FLOAT(xpos);
		if(strncmp("y position", attribute, len) == 0) RETURN_FLOAT(ypos);
		if(strncmp("direction", attribute, len) == 0) RETURN_FLOAT(direction);
		if(strncmp("costume #", attribute, len) == 0) RETURN_NONE(); // TODO
		if(strncmp("costume name", attribute, len) == 0) RETURN_NONE();
		if(strncmp("size", attribute, len) == 0) RETURN_FLOAT(size * 100);
	}
	else {
		if(strncmp("background #", attribute, len) == 0) RETURN_NONE();
		if(strncmp("backdrop #", attribute, len) == 0) RETURN_NONE();
		if(strncmp("backdrop name", attribute, len) == 0) RETURN_NONE();
	}
	if(strncmp("volume", attribute, len) == 0) {reportSlot->type = FLOATING; reportSlot->data.floating = volume; return NULL;}

	getVariable(&sprite->variables, attribute, reportSlot); // will fill out the report slot with 0.0 if it doesn't exist
	return NULL;
}

BF(username_get) { // just default to a null string
	reportSlot->type = STRING;
	reportSlot->data.string = strpool_alloc(1);
	reportSlot->data.string[0] = '\0';
	return NULL;
}

/* Sound */

BF(volume_change) {
	const double diff = toFloating(arg+0);
	volume += diff;
	if(volume > 100.0f) volume = 100.0f;
	else if(volume < 0.0f) volume = 0.0f;
	return block->p.next;
}

BF(volume_set) {
	const double newValue = toFloating(arg+0);
	if(newValue > 100.0f) volume = 100.0f;
	else if(newValue < 0.0f) volume = 0.0f;
	else volume = newValue;
	return block->p.next;
}

BF(volume_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = volume;
	return block->p.next;
}

BF(tempo_change) {
	const double diff = toFloating(arg+0);
	tempo += diff;
	if(tempo > 500.0f) tempo = 500.0f;
	else if(tempo < 20.0f) tempo = 20.0f;
	return block->p.next;
}

BF(tempo_set) {
	const double newValue = toFloating(arg+0);
	if(newValue > 500.0f) tempo = 500.0f;
	else if(newValue < 20.0f) tempo = 20.0f;
	else tempo = newValue;
	return block->p.next;
}

BF(tempo_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = tempo;
	return block->p.next;
}

/* Motion */

BF(move_forward) {
	const double h = toFloating(arg+0);
	if(!isnan(h)) {
		const double dir = M_PI/180*(activeSprite->direction-90.0);
		activeSprite->xpos += h*cos(dir);
		activeSprite->ypos += h*sin(dir);
		doRedraw = true;
	}
	return block->p.next;
}

BF(direction_change_cw) {
	const double diff = toFloating(arg+0);
	if(isfinite(diff))
		activeSprite->direction = fmod(activeSprite->direction+180.0 + diff, 360.0) - 180.0;
	doRedraw = true;
	return block->p.next;
}

BF(direction_change_ccw) {
	const double diff = toFloating(arg+0);
	if(isfinite(diff))
		activeSprite->direction = fmod(activeSprite->direction+180.0 - diff, 360.0) - 180.0;
	doRedraw = true;
	return block->p.next;
}

BF(direction_set) {
	const double d = toFloating(arg+0);
	if(isfinite(d))
		activeSprite->direction = fmod(d+180.0, 360.0) - 180.0;
	doRedraw = true;
	return block->p.next;
}

BF(direction_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = activeSprite->direction;
	return NULL;
}

BF(move_to_coordinates) {
	activeSprite->xpos = toFloating(arg+0);
	activeSprite->ypos = toFloating(arg+1);
	doRedraw = true;
	return block->p.next;
}

BF(move_to_coordinates_for_duration) {
	double xDest = toFloating(arg+0);
	double yDest = toFloating(arg+1);

	if(allocTmpData(block)) {
		fsetTmpData((float)toFloating(arg+2) * CLOCKS_PER_SEC);
	}
	else {
		const float timeLeft = fgetTmpData();
		if(timeLeft > 0) {
			fsetTmpData(fgetTmpData()-dtime);

			if(isnan(xDest)) xDest = 0.0;
			if(isnan(yDest)) yDest = 0.0;
			// calculate the speed to go at and multiply by time
			activeSprite->xpos = ((activeSprite->xpos - xDest) /  timeLeft) * dtime;
			doRedraw = true;
		}
		else { // time's up
			freeTmpData();
			activeSprite->xpos = xDest;
			activeSprite->ypos = yDest;
			doRedraw = true;
			return block->p.next;
		}
	}
	doYield = true;
	return block;
}

BF(x_change) {
	activeSprite->xpos += toFloating(arg+0);
	doRedraw = true;
	return block->p.next;
}

BF(x_set) {
	activeSprite->xpos = toFloating(arg+0);
	doRedraw = true;
	return block->p.next;
}

BF(y_change) {
	activeSprite->ypos += toFloating(arg+0);
	doRedraw = true;
	return block->p.next;
}

BF(y_set) {
	activeSprite->ypos = toFloating(arg+0);
	doRedraw = true;
	return block->p.next;
}

BF(x_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = activeSprite->xpos;
	return NULL;
}

BF(y_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = activeSprite->ypos;
	return NULL;
}

/* Looks */

BF(say) {
	char *msg;
	toString(arg+0, &msg);
	printf("%s: %s\n", activeSprite->name, msg);
	doRedraw = true;
	return block->p.next;
}

BF(say_and_do_wait) {
	if(allocTmpData(block)) {
		fsetTmpData((float)toFloating(arg+0) * CLOCKS_PER_SEC);
		char *msg;
		toString(arg+0, &msg);
		printf("%s: %s\n", activeSprite->name, msg);
	}
	else if (fgetTmpData() > 0) {
		fsetTmpData(fgetTmpData()-dtime);
	}
	else {
		freeTmpData();
		return block->p.next;
	}
	doRedraw = true;
	doYield = true;
	return block;
}

BF(think) {
	char *msg;
	toString(arg+0, &msg);
	printf("%s thinks: %s\n", activeSprite->name, msg);
	return block->p.next;
}

BF(think_and_do_wait) {
	if(allocTmpData(block)) {
		fsetTmpData((float)toFloating(arg+0) * CLOCKS_PER_SEC);
		char *msg;
		toString(arg+0, &msg);
		printf("%s thinks: %s\n", activeSprite->name, msg);
	}
	else if (fgetTmpData() > 0) {
		fsetTmpData(fgetTmpData()-dtime);
	}
	else {
		freeTmpData();
		return block->p.next;
	}
	doRedraw = true;
	doYield = true;
	return block;
}

BF(gfx_change) { // TODO: check if Scratch bounds some of these
	char *fxName;
	toString(arg+0, &fxName);
	const double diff = toFloating(arg+1);
	switch(fxName[0]) {
	case 'c': // color
		activeSprite->effects.color += diff;
		break;
	case 'b': // brightness
		activeSprite->effects.brightness += diff;
		break;
	case 'g': // ghost
		activeSprite->effects.ghost += diff;
		break;
	case 'p': // pixelate
		activeSprite->effects.pixelate += diff;
		break;
	case 'm': // mosaic
		activeSprite->effects.mosaic += diff;
		break;
	case 'f': // fisheye
		activeSprite->effects.fisheye += diff;
		break;
	case 'w': // whirl
		activeSprite->effects.whirl += diff;
		break;
	}
	doRedraw = true;
	return block->p.next;
}

BF(gfx_set) {
	char *fxName;
	toString(arg+0, &fxName);
	const double newValue = toFloating(arg+1);
	switch(fxName[0]) {
	case 'c': // color
		activeSprite->effects.color = newValue;
		break;
	case 'b': // brightness
		activeSprite->effects.brightness = newValue;
		break;
	case 'g': // ghost
		activeSprite->effects.ghost = newValue;
		break;
	case 'p': // pixelate
		activeSprite->effects.pixelate = newValue;
		break;
	case 'm': // mosaic
		activeSprite->effects.mosaic = newValue;
		break;
	case 'f': // fisheye
		activeSprite->effects.fisheye = newValue;
		break;
	case 'w': // whirl
		activeSprite->effects.whirl = newValue;
		break;
	}
	doRedraw = true;
	return block->p.next;
}

BF(gfx_reset) {
	activeSprite->effects.color = activeSprite->effects.brightness = activeSprite->effects.ghost
		= activeSprite->effects.pixelate = activeSprite->effects.mosaic
		= activeSprite->effects.fisheye = activeSprite->effects.whirl
		= 0.0f;
	doRedraw = true;
	return block->p.next;
}

BF(size_change) {
	activeSprite->size += toFloating(arg+0) / 100.0;
	doRedraw = true;
	return block->p.next;
}

BF(size_set) {
	activeSprite->size = toFloating(arg+0) / 100.0;
	doRedraw = true;
	return block->p.next;
}

BF(size_get) {
	reportSlot->type = FLOATING;
	reportSlot->data.floating = activeSprite->size * 100.0;
	return NULL;
}

BF(test_print);
#ifndef TESTING
BF(test_print) {return block->p.next;}
#endif
