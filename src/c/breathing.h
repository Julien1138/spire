#pragma once
#include <pebble.h>

#define PHASE_MS 5000
#define TOTAL_MS (5 * 60 * 1000)

typedef struct {
  int  elapsed_ms;
  int  frac;      // 0..1000, progress within current phase
  bool inhaling;
  bool finished;
} BreathingState;

typedef void (*BreathingTickCallback)(void *context);

void                  breathing_start(BreathingTickCallback on_tick, void *context);
void                  breathing_stop(void);
const BreathingState *breathing_get_state(void);
