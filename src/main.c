#include <pebble.h>
#include "deliveries.h"

int main(void) {
	deliveries_init();
	app_event_loop();
	deliveries_deinit();
}
