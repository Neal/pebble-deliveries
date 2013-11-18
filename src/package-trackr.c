#include <pebble.h>

static Window *window;

static TextLayer *item_name_layer;
static TextLayer *description_layer;
static TextLayer *time_layer;
static TextLayer *location_layer;
static TextLayer *delivery_estimate_layer;

enum {
	KEY_ITEM_NAME,
	KEY_DESCRIPTION,
	KEY_TIME,
	KEY_LOCATION,
	KEY_DELIVERY_ESTIMATE
};

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send AppMessage to Pebble");
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *item_name_tuple = dict_find(iter, KEY_ITEM_NAME);
	Tuple *description_tuple = dict_find(iter, KEY_DESCRIPTION);
	Tuple *time_tuple = dict_find(iter, KEY_TIME);
	Tuple *location_tuple = dict_find(iter, KEY_LOCATION);
	Tuple *delivery_estimate_tuple = dict_find(iter, KEY_DELIVERY_ESTIMATE);

	if (item_name_tuple) {
		text_layer_set_text(item_name_layer, item_name_tuple->value->cstring);
	}
	if (description_tuple) {
		text_layer_set_text(description_layer, description_tuple->value->cstring);
	}
	if (time_tuple) {
		text_layer_set_text(time_layer, time_tuple->value->cstring);
	}
	if (location_tuple) {
		text_layer_set_text(location_layer, location_tuple->value->cstring);
	}
	if (delivery_estimate_tuple) {
		text_layer_set_text(delivery_estimate_layer, delivery_estimate_tuple->value->cstring);
	}
}

void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message from Pebble dropped");
}

void update() {
	text_layer_set_text(item_name_layer, "Refreshing...");
	text_layer_set_text(description_layer, "");
	text_layer_set_text(time_layer, "");
	text_layer_set_text(location_layer, "");
	text_layer_set_text(delivery_estimate_layer, "");
	app_message_outbox_send();
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	update();
}

static void click_config_provider(void *context) {
	const uint16_t repeat_interval_ms = 1000;
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, repeat_interval_ms, select_single_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	item_name_layer = text_layer_create((GRect) { .origin = { 4, 0 }, .size = { bounds.size.w - 2, bounds.size.h } });
	text_layer_set_text_color(item_name_layer, GColorBlack);
	text_layer_set_background_color(item_name_layer, GColorClear);
	text_layer_set_font(item_name_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	description_layer = text_layer_create((GRect) { .origin = { 4, 28 }, .size = { bounds.size.w - 2, 36 } });
	text_layer_set_text_color(description_layer, GColorBlack);
	text_layer_set_background_color(description_layer, GColorClear);
	text_layer_set_font(description_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));

	time_layer = text_layer_create((GRect) { .origin = { 4, 64 }, .size = { bounds.size.w - 2, 20 } });
	text_layer_set_text_color(time_layer, GColorBlack);
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

	location_layer = text_layer_create((GRect) { .origin = { 4, 86 }, .size = { bounds.size.w - 2, 30 } });
	text_layer_set_text_color(location_layer, GColorBlack);
	text_layer_set_background_color(location_layer, GColorClear);
	text_layer_set_font(location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	delivery_estimate_layer = text_layer_create((GRect) { .origin = { 4, 116 }, .size = { bounds.size.w - 2, 30 } });
	text_layer_set_text_color(delivery_estimate_layer, GColorBlack);
	text_layer_set_background_color(delivery_estimate_layer, GColorClear);
	text_layer_set_font(delivery_estimate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	layer_add_child(window_layer, text_layer_get_layer(item_name_layer));
	layer_add_child(window_layer, text_layer_get_layer(description_layer));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(location_layer));
	layer_add_child(window_layer, text_layer_get_layer(delivery_estimate_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(item_name_layer);
	text_layer_destroy(description_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(location_layer);
	text_layer_destroy(delivery_estimate_layer);
}

static void app_message_init(void) {
	app_message_open(128 /* inbound_size */, 2 /* outbound_size */);
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
}

static void init(void) {
	app_message_init();

	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true /* animated */);

	update();
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
