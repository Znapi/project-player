#pragma once

extern bool windowIsShowing;

extern bool initPeripherals(void);
extern void destroyPeripherals(void);
extern void peripheralsInputTick(void);
extern void peripheralsOutputTick(void);
