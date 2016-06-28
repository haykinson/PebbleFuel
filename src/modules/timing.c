#include "timing.h"

static int interval;

int get_interval() {
  return interval;
}

void set_interval(int new_value) {
  interval = new_value;
}
