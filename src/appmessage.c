#include <pebble.h>
#include "appmessage.h"
#include "common.h"
#include "windows/pkglist.h"
#include "windows/pkgstatus.h"

static void in_received_handler(DictionaryIterator *iter, void *context);

void appmessage_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_open(256 /* inbound_size */, 32 /* outbound_size */);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	if (dict_find(iter, KEY_STATUS)) {
		pkgstatus_in_received_handler(iter);
	} else {
		pkglist_in_received_handler(iter);
	}
}
