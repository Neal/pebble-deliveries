#include <pebble.h>
#include "deliveries.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "appmessage.h"
#include "windows/win-deliveries.h"
#include "windows/win-progress.h"

static void timer_callback(void *data);
static AppTimer *timer = NULL;

static Delivery *deliveries = NULL;
static DeliveryProgress *progresses = NULL;

static uint8_t num_deliveries = 0;
static uint8_t num_progresses = 0;
static uint8_t current_delivery = 0;

static char *estimate = NULL;
static char *delivery_error = NULL;
static char *progress_error = NULL;

void deliveries_init(void) {
	appmessage_init();

	timer = app_timer_register(1000, timer_callback, NULL);

	win_deliveries_init();
	win_progress_init();
}

void deliveries_deinit(void) {
	free_safe(estimate);
	free_safe(progress_error);
	free_safe(delivery_error);
	free_safe(progresses);
	free_safe(deliveries);
	win_progress_deinit();
	win_deliveries_deinit();
}

void deliveries_in_received_handler(DictionaryIterator *iter) {
	if (!dict_find(iter, APP_KEY_TYPE)) return;
	switch (dict_find(iter, APP_KEY_TYPE)->value->uint8) {
		case KEY_TYPE_PACKAGE:
			free_safe(delivery_error);
			switch (dict_find(iter, APP_KEY_METHOD)->value->uint8) {
				case KEY_METHOD_ERROR: {
					delivery_error = malloc(dict_find(iter, APP_KEY_TITLE)->length);
					strncpy(delivery_error, dict_find(iter, APP_KEY_TITLE)->value->cstring, dict_find(iter, APP_KEY_TITLE)->length);
					win_deliveries_reload_data_and_mark_dirty();
					break;
				}
				case KEY_METHOD_SIZE:
					free_safe(deliveries);
					num_deliveries = dict_find(iter, APP_KEY_INDEX)->value->uint8;
					deliveries = malloc(sizeof(Delivery) * num_deliveries);
					if (deliveries == NULL) num_deliveries = 0;
					break;
				case KEY_METHOD_DATA: {
					if (!delivery_count()) break;
					uint8_t index = dict_find(iter, APP_KEY_INDEX)->value->uint8;
					Delivery *delivery = delivery_get(index);
					delivery->index = index;
					strncpy(delivery->title, dict_find(iter, APP_KEY_TITLE)->value->cstring, sizeof(delivery->title) - 1);
					strncpy(delivery->subtitle, dict_find(iter, APP_KEY_SUBTITLE)->value->cstring, sizeof(delivery->subtitle) - 1);
					win_deliveries_reload_data_and_mark_dirty();
					LOG("delivery: %d '%s' '%s'", delivery->index, delivery->title, delivery->subtitle);
					break;
				}
			}
			break;
		case KEY_TYPE_PROGRESS:
			free_safe(progress_error);
			switch (dict_find(iter, APP_KEY_METHOD)->value->uint8) {
				case KEY_METHOD_ERROR: {
					free_safe(progress_error);
					progress_error = malloc(dict_find(iter, APP_KEY_TITLE)->length);
					strncpy(progress_error, dict_find(iter, APP_KEY_TITLE)->value->cstring, dict_find(iter, APP_KEY_TITLE)->length);
					win_progress_reload_data_and_mark_dirty();
					break;
				}
				case KEY_METHOD_SIZE:
					free_safe(progresses);
					num_progresses = dict_find(iter, APP_KEY_INDEX)->value->uint8;
					progresses = malloc(sizeof(DeliveryProgress) * num_progresses);
					if (progresses == NULL) num_progresses = 0;
					break;
				case KEY_METHOD_DATA: {
					if (!progress_count()) break;
					uint8_t index = dict_find(iter, APP_KEY_INDEX)->value->uint8;
					DeliveryProgress *progress = progress_get(index);
					progress->index = index;
					strncpy(progress->status, dict_find(iter, APP_KEY_TITLE)->value->cstring, sizeof(progress->status) - 1);
					win_progress_reload_data_and_mark_dirty();
					LOG("progress: %d '%s'", progress->index, progress->status);
					break;
				}
				case KEY_METHOD_ESTIMATE:
					free_safe(estimate);
					estimate = malloc(dict_find(iter, APP_KEY_TITLE)->length);
					strncpy(estimate, dict_find(iter, APP_KEY_TITLE)->value->cstring, dict_find(iter, APP_KEY_TITLE)->length);
					win_progress_reload_data_and_mark_dirty();
					break;
			}
			break;
	}
}

void deliveries_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	free_safe(delivery_error);
	delivery_error = malloc(sizeof(char) * 65);
	strncpy(delivery_error, "Unable to connect to phone! Make sure the Pebble app is running.", 64);
}

void deliveries_reload_data_and_mark_dirty() {
	win_deliveries_reload_data_and_mark_dirty();
	win_progress_reload_data_and_mark_dirty();
}

void request_deliveries() {
	free_safe(delivery_error);
	num_deliveries = 0;
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTPACKAGES);
	dict_write_end(iter);
	app_message_outbox_send();
	win_deliveries_reload_data_and_mark_dirty();
}

void request_progress() {
	free_safe(estimate);
	free_safe(progress_error);
	num_progresses = 0;
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTPROGRESS);
	dict_write_uint8(iter, APP_KEY_INDEX, current_delivery);
	dict_write_end(iter);
	app_message_outbox_send();
	win_progress_reload_data_and_mark_dirty();
}

uint8_t delivery_count() {
	return num_deliveries;
}

Delivery *delivery_get(uint8_t index) {
	if (index < delivery_count())
		return &deliveries[index];
	return NULL;
}

void delivery_set_current(uint8_t index) {
	current_delivery = index;
}

char *delivery_get_error() {
	if (delivery_error == NULL && !delivery_count()) return "Loading...";
	return &delivery_error[0];
}

uint8_t progress_count() {
	return num_progresses;
}

DeliveryProgress *progress_get(uint8_t index) {
	if (index < progress_count())
		return &progresses[index];
	return NULL;
}

char *progress_get_error() {
	if (progress_error == NULL && !progress_count()) return "Loading...";
	return &progress_error[0];
}

char *estimate_get() {
	if (estimate == NULL) return NULL;
	return &estimate[0];
}

static void timer_callback(void *data) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_READY);
	dict_write_end(iter);
	app_message_outbox_send();
}
