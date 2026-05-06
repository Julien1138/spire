#include "breathing_window.h"
#include "breathing.h"
#include <pebble.h>

static Window *s_window;
static Layer  *s_canvas;

static void canvas_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const BreathingState *state = breathing_get_state();

  // Background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Layout
  const int countdown_h  = 24;
  const int phase_h      = 38;
  const int circle_top   = countdown_h;
  const int circle_bot   = bounds.size.h - phase_h;
  const int circle_area_h = circle_bot - circle_top;
  const int cx = bounds.size.w / 2;
  const int cy = circle_top + circle_area_h / 2;

  int half  = (bounds.size.w < circle_area_h ? bounds.size.w : circle_area_h) / 2 - 4;
  int r_max = half * 9 / 10;
  int r_min = half * 6 / 10;
  int radius = state->finished ? r_min : r_min + ((r_max - r_min) * state->frac) / 1000;

  // Circle
#if defined(PBL_COLOR)
  GColor circle_color = state->inhaling ? GColorVividCerulean : GColorMediumAquamarine;
#else
  GColor circle_color = GColorWhite;
#endif
  graphics_context_set_fill_color(ctx, circle_color);
  graphics_fill_circle(ctx, GPoint(cx, cy), radius);

  graphics_context_set_text_color(ctx, GColorWhite);

  // Countdown
  int remaining_s = (TOTAL_MS - state->elapsed_ms + 999) / 1000;
  if (remaining_s < 0) remaining_s = 0;
  char countdown[16];
  snprintf(countdown, sizeof(countdown), "%d:%02d", remaining_s / 60, remaining_s % 60);
  graphics_draw_text(ctx, countdown,
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GRect(0, 3, bounds.size.w, countdown_h),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Phase label – locale-aware
  const char *locale = i18n_get_system_locale();
  bool french = locale && locale[0] == 'f' && locale[1] == 'r';
  const char *label = state->inhaling
    ? (french ? "Inspire" : "Inhale")
    : (french ? "Expire"  : "Exhale");
  graphics_draw_text(ctx, label,
    fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
    GRect(0, circle_bot + 2, bounds.size.w, phase_h),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void on_tick(void *context) {
  layer_mark_dirty(s_canvas);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_canvas = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_canvas, canvas_update);
  layer_add_child(root, s_canvas);
  breathing_start(on_tick, NULL);
}

static void window_unload(Window *window) {
  breathing_stop();
  layer_destroy(s_canvas);
  s_canvas = NULL;
}

void breathing_window_push(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void breathing_window_destroy(void) {
  window_destroy(s_window);
  s_window = NULL;
}
