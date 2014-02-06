#include <pebble.h>
#include "appmessage.h"
#include "windows/pkglist.h"

static void init(void) {
	appmessage_init();
	pkglist_init();
}

static void deinit(void) {
	pkglist_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
