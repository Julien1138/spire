#include <pebble.h>

#define PHASE_MS  5000
#define TOTAL_MS  (5 * 60 * 1000)
#define TICK_MS   50

static Window   *s_window;
static Layer    *s_canvas;
static AppTimer *s_timer;
static int       s_elapsed = 0;
static bool      s_finished = false;

static void canvas_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Phase
  int cycle_ms = s_elapsed % (PHASE_MS * 2);
  bool inhaling = (cycle_ms < PHASE_MS);
  int frac = inhaling
    ? (cycle_ms * 1000) / PHASE_MS
    : ((PHASE_MS * 2 - cycle_ms) * 1000) / PHASE_MS;

  // Locale-based phase label (fr if locale starts with "fr", en otherwise)
  const char *locale = i18n_get_system_locale();
  bool french = locale && locale[0] == 'f' && locale[1] == 'r';
  const char *phase_label = inhaling
    ? (french ? "Inspire" : "Inhale")
    : (french ? "Expire"  : "Exhale");

  // Layout
  int countdown_h = 24;
  int phase_h     = 38;
  int circle_top  = countdown_h;
  int circle_bot  = bounds.size.h - phase_h;
  int circle_area_h = circle_bot - circle_top;

  int cx = bounds.size.w / 2;
  int cy = circle_top + circle_area_h / 2;

  int half  = (bounds.size.w < circle_area_h ? bounds.size.w : circle_area_h) / 2 - 4;
  int r_max = half * 9 / 10;
  int r_min = half * 6 / 10;
  int radius = s_finished ? r_min : r_min + ((r_max - r_min) * frac) / 1000;

  // Circle
#if defined(PBL_COLOR)
  GColor circle_color = inhaling ? GColorVividCerulean : GColorMediumAquamarine;
#else
  GColor circle_color = GColorWhite;
#endif
  graphics_context_set_fill_color(ctx, circle_color);
  graphics_fill_circle(ctx, GPoint(cx, cy), radius);

  graphics_context_set_text_color(ctx, GColorWhite);

  // Countdown
  int remaining_s = (TOTAL_MS - s_elapsed + 999) / 1000;
  if (remaining_s < 0) remaining_s = 0;
  char countdown[16];
  snprintf(countdown, sizeof(countdown), "%d:%02d", remaining_s / 60, remaining_s % 60);
  GFont font_count = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  graphics_draw_text(ctx, countdown, font_count,
    GRect(0, 3, bounds.size.w, countdown_h),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // Phase text
  GFont font_phase = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  graphics_draw_text(ctx, phase_label, font_phase,
    GRect(0, circle_bot + 2, bounds.size.w, phase_h),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void tick(void *context) {
  int prev_phase = s_elapsed / PHASE_MS;
  s_elapsed += TICK_MS;
  int curr_phase = s_elapsed / PHASE_MS;

  if (curr_phase != prev_phase) {
    vibes_short_pulse();
  }

  layer_mark_dirty(s_canvas);

  if (s_elapsed >= TOTAL_MS) {
    s_finished = true;
    return;
  }

  s_timer = app_timer_register(TICK_MS, tick, NULL);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, canvas_update);
  layer_add_child(root, s_canvas);

  vibes_short_pulse();
  s_timer = app_timer_register(TICK_MS, tick, NULL);
}

static void window_unload(Window *window) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  layer_destroy(s_canvas);
}

static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
