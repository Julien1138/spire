#include "breathing.h"

#define TICK_MS 50

static BreathingState        s_state;
static AppTimer             *s_timer;
static BreathingTickCallback s_callback;
static void                 *s_context;

static void tap(void) {
  static const uint32_t dur[] = {80};
  vibes_enqueue_custom_pattern((VibePattern){.durations = dur, .num_segments = 1});
}

static void timer_cb(void *context) {
  int prev_phase = s_state.elapsed_ms / PHASE_MS;
  s_state.elapsed_ms += TICK_MS;
  int curr_phase = s_state.elapsed_ms / PHASE_MS;

  if (curr_phase != prev_phase) {
    tap();
  }

  int cycle_ms = s_state.elapsed_ms % (PHASE_MS * 2);
  s_state.inhaling = (cycle_ms < PHASE_MS);
  s_state.frac = s_state.inhaling
    ? (cycle_ms * 1000) / PHASE_MS
    : ((PHASE_MS * 2 - cycle_ms) * 1000) / PHASE_MS;

  if (s_state.elapsed_ms >= TOTAL_MS) {
    s_state.finished = true;
    s_state.frac = 0;
  }

  if (s_callback) s_callback(s_context);

  if (!s_state.finished) {
    s_timer = app_timer_register(TICK_MS, timer_cb, NULL);
  }
}

void breathing_start(BreathingTickCallback on_tick, void *context) {
  s_state    = (BreathingState){ .inhaling = true };
  s_callback = on_tick;
  s_context  = context;
  tap();
  s_timer = app_timer_register(TICK_MS, timer_cb, NULL);
}

void breathing_stop(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
}

const BreathingState *breathing_get_state(void) {
  return &s_state;
}
