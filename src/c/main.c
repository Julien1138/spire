#include <pebble.h>
#include "breathing_window.h"
#include "done_window.h"

static void init(void) {
  breathing_window_push();
}

static void deinit(void) {
  done_window_destroy();
  breathing_window_destroy();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
