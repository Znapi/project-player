/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * specs.h                                                       *
 *                                                               *
 * This header file contains a table of block specifications     *
 * and is used for generating perfect hash tables in phgt.c.     *
 * This is the basically equivalent of Specs.as in the flash     *
 * implementation.                                               *
 * See phgt.c for details.                                       *
 *                                                               *
 * This could also be expanded and used in an implementation of  *
 * the Scratch project editor in C.                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

enum BlockType {
	s,     // stack block
	r,     // reporter
	b,     // boolean
	h,     // hat
	f,     // cap (final)
	c, cf, // C block, capped C block
	e,     // E block, no capped version
};

struct BlockSpec {
	const char *const opString;
	const char *const name;
	const enum BlockType type;
	//const ubyte category;
	//const char * const label;
	//const ScratchValue *defaultArgs;
};

const struct BlockSpec specs[] = {
	// op     name  type
	// --     ----  ----
	{"noop", "bf_noop", s},

	// Motion
	{"forward:", "bf_noop", s},
	{"turnRight:", "bf_noop", s},
	{"turnLeft:", "bf_noop", s},

	{"heading:", "bf_noop", s},
	{"pointTowards:", "bf_noop", s},
	{"gotoX:y:", "bf_noop", s},
	{"gotoSpriteOrMouse:", "bf_noop", s},
	{"glideSecs:toX:y:elapsed:from:", "bf_noop", s},

	{"changeXposBy:", "bf_noop", s},
	{"xpos:", "bf_noop", s},
	{"changeYposBy:", "bf_noop", s},
	{"ypos:", "bf_noop", s},

	{"bounceOffEdge", "bf_noop", s},

	{"setRotationStyle", "bf_noop", s},

	{"xpos", "bf_noop", r},
	{"ypos", "bf_noop", r},
	{"heading", "bf_noop", r},

	// Looks
	{"say:duration:elapsed:from:", "bf_noop", s},
	{"say:", "bf_say", s},
	{"think:duration:elapsed:from:", "bf_noop", s},
	{"think:", "bf_noop", s},

	{"show", "bf_noop", s},
	{"hide", "bf_noop", s},

	{"lookLike:", "bf_noop", s},
	{"nextCostume", "bf_noop", s},
	{"startScene", "bf_noop", s},

	{"changeGraphicEffect:by:", "bf_noop", s},
	{"setGraphicEffect:to:", "bf_noop", s},
	{"filterReset", "bf_noop", s},

	{"changeSizeBy:", "bf_noop", s},
	{"setSizeTo:", "bf_noop", s},

	{"comeToFront", "bf_noop", s},
	{"goBackByLayers:", "bf_noop", s},

	{"costumeIndex", "bf_noop", r},
	{"sceneName", "bf_noop", r},
	{"scale", "bf_noop", r},

	{"startSceneAndWait", "bf_noop", s},
	{"nextScene", "bf_noop", s},

	{"backgroundIndex", "bf_noop", r},

	// Sound
	{"playSound:", "bf_noop", s},
	{"doPlaySoundAndWait", "bf_noop", s},
	{"stopAllSounds", "bf_noop", s},

	{"playDrum", "bf_noop", s},
	{"rest:elapsed:from:", "bf_noop", s},
	{"noteOn:duration:elapsed:from:", "bf_noop", s},

	{"instrument:", "bf_noop", s},

	{"changeVolumeBy:", "bf_noop", s},
	{"setVolumeTo:", "bf_noop", s},
	{"volume", "bf_noop", r},

	{"changeTempoBy:", "bf_noop", s},
	{"setTempTo:", "bf_noop", s},
	{"tempo", "bf_noop", r},

	// Pen
	{"clearPenTrails", "bf_noop", s},
	{"stampCostume", "bf_noop", s},

	{"putPenDown", "bf_noop", s},
	{"putPenUp", "bf_noop", s},

	{"penColor:", "bf_noop", s},
	{"changePenHueBy:", "bf_noop", s},
	{"setPenHueTo:", "bf_noop", s},

	{"changePenShadeBy:", "bf_noop", s},
	{"setPenShadeTo:", "bf_noop", s},

	{"changePenSizeBy:", "bf_noop", s},
	{"penSize:", "bf_noop", r},

	// Events / Triggers
	{"whenGreenFlag", "bf_noop", h},
	{"whenKeyPressed", "bf_noop", h},
	{"whenClicked", "bf_noop", h},
	{"whenSceneStarts", "bf_noop", h},

	{"whenSensorGreaterThan", "bf_noop", h},

	{"whenIReceive", "bf_noop", h},
	{"broadcast:", "bf_noop", s},
	{"doBroadcastAndWait", "bf_noop", s},

	// Control
	{"wait:elapsed:from:", "bf_do_wait", s},

	{"doRepeat", "bf_do_repeat", c},
	{"doForever", "bf_do_forever", cf},
	{"doIf", "bf_do_if", c},
	{"doIfElse", "bf_do_if_else", e},
	{"doWaitUntil", "bf_do_wait_until", s},
	{"doUntil", "bf_do_until", c},

	{"stopScripts", "bf_stop_scripts", f}, // the type of this block depends on it's arguments

	{"whenCloned", "bf_noop", h},
	{"createCloneOf", "bf_noop", s},
	{"deleteClone", "bf_noop", f},

	// Sensing
	{"touching:", "bf_noop", b},
	{"touchingColor:", "bf_noop", b},
	{"color:sees:", "bf_noop", b},
	{"distanceTo:", "bf_noop", r},

	{"doAsk", "bf_noop", s},
	{"answer", "bf_noop", r},

	{"keyPressed:", "bf_noop", b},
	{"mousePressed", "bf_noop", b},
	{"mouseX", "bf_noop", r},
	{"mouseY", "bf_noop", r},

	{"soundLevel", "bf_noop", r},

	{"senseVideoMotion", "bf_noop", r},
	{"setVideoState", "bf_noop", s},
	{"setVideoTransparency", "bf_noop", s},

	{"timer", "bf_noop", r},
	{"timerReset", "bf_noop", s},

	{"getAttribute:of:", "bf_noop", r},

	{"timeAndDate", "bf_noop", r},
	{"timestamp", "bf_noop", r},
	{"getUserName", "bf_noop", r},

	// Operators
	{"+", "bf_add", r},
	{"-", "bf_subtract", r},
	{"*", "bf_multiply", r},
	{"/", "bf_divide", r},

	{"randomFrom:to:", "bf_generate_random", r},

	{"<", "bf_is_less", b},
	{"=", "bf_is_equal", b},
	{">", "bf_is_greater", b},

	{"&", "bf_logical_and", b},
	{"|", "bf_logical_or", b},
	{"not", "bf_logical_not", b},

	{"concatenate:with:", "bf_concatenate", r},
	{"letter:of:", "bf_get_character", r},
	{"stringLength:", "bf_get_string_length", r},

	{"%", "bf_modulo", r},
	{"rounded", "bf_round", r},

	{"computeFunction:of:", "bf_compute_math_function", r},

	// Variables
	{"readVariable", "bf_get_variable", s},

	{"setVar:to:", "bf_variable_set", s},
	{"changeVar:by:", "bf_variable_change", s},
	{"showVariable:", "bf_noop", s},
	{"hideVariable:", "bf_noop", s},

	// Lists
	{"contentsOfList:", "bf_noop", r},

	{"append:toList:", "bf_list_append", s},
	{"deleteLine:ofList:", "bf_list_delete", s},
	{"setLine:ofList:to:", "bf_noop", s},

	{"getLine:ofList:", "bf_noop", r},
	{"lineCountOfList:", "bf_list_length", r},
	{"list:contains:", "bf_list_contains", b},

	{"showList:", "bf_noop", s},
	{"hideList:", "bf_noop", s},

	// Custom blocks
	{"procDef", "bf_noop", h},
	{"call", "bf_noop", s},
	{"getParam", "bf_noop", r},

	// Scratch 2.0 alpha and beta blocks obselete in complete release
	{"hideAll", "bf_noop", s},
	{"getUserID", "bf_noop", r},

	// Scratch 1.4 blocks obselete in 2.0
	{"drum:duration:elapsed:from:", "bf_noop", s},
	{"midiInstrument:", "bf_noop", s},
	{"isLoud", "bf_noop", b},

	{"abs", "bf_noop", r},
	{"sqrt", "bf_noop", r},
	{"doReturn", "bf_noop", f},
	{"stopAll", "bf_noop", f},
	{"showBackground:", "bf_noop", s},
	{"nextBackground", "bf_noop", s},
	{"doForeverIf", "bf_noop", cf},

	{"sayNothing", "bf_noop", s},
	//{turnAwayFromEdge, s}, // never implemented in any full release

	// Testing blocks and experimental control blocks
	{"COUNT", "bf_noop", s},
	{"CLR_COUNT", "bf_noop", s},
	{"INCR_COUNT", "bf_noop", s},
	{"print", "bf_test_print", s},
	{"doForLoop", "bf_noop", c},
	{"doWhile", "bf_noop", c},
	{"warpSpeed", "bf_noop", c},

	// choosing not to implement stage motion/scrolling because no projects should be using them
	//{"scrollRight", "bf_noop", s},
	//{"scrollUp", "bf_noop", s},
	//{"scrollAlign", "bf_noop", s},
	//{"xScroll", "bf_noop", s},
	//{"yScroll", "bf_noop", s},

	// obsolete and undefined blocks
	//{"obsolete", "bf_noop", s},
	//{"undefined", "bf_noop", s},
	// obsolete and undefined blocks don't appear in a project.json (I don't think)
};
