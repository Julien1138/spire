#include "done_window.h"
#include <pebble.h>

static Window *s_window;

static void canvas_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  const char *locale = i18n_get_system_locale();
  bool french = locale && locale[0] == 'f' && locale[1] == 'r';

  graphics_context_set_text_color(ctx, GColorWhite);

  const char *title    = french ? "Terminé"       : "Done";
  const char *subtitle = french ? "5 min • Bravo" : "5 min • Well done";

  GFont font_title    = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  GFont font_subtitle = fonts_get_system_font(FONT_KEY_GOTHIC_18);

  // Centre title vertically with subtitle
  int title_h    = 48;
  int subtitle_h = 22;
  int block_h    = title_h + subtitle_h;
  int block_y    = (bounds.size.h - block_h) / 2;

  graphics_draw_text(ctx, title, font_title,
    GRect(0, block_y, bounds.size.w, title_h),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  graphics_draw_text(ctx, subtitle, font_subtitle,
    GRect(0, block_y + title_h + 2, bounds.size.w, subtitle_h),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  Layer *canvas = layer_create(layer_get_bounds(root));
  layer_set_update_proc(canvas, canvas_update);
  layer_add_child(root, canvas);
}

void done_window_push(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
  });
  window_stack_push(s_window, true);
}

void done_window_destroy(void) {
  if (s_window) {
    window_destroy(s_window);
    s_window = NULL;
  }
}
