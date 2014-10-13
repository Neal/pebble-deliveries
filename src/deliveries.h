#pragma once

#include <pebble.h>

typedef struct {
	uint8_t index;
	char title[32];
	char subtitle[32];
} Delivery;

typedef struct {
	uint8_t index;
	char status[144];
} DeliveryProgress;

void deliveries_init(void);
void deliveries_deinit(void);
void deliveries_in_received_handler(DictionaryIterator *iter);
void deliveries_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
void deliveries_reload_data_and_mark_dirty();

void request_deliveries();
void request_progress();

uint8_t delivery_count();
Delivery *delivery_get(uint8_t index);
void delivery_set_current(uint8_t index);
char *delivery_get_error();

uint8_t progress_count();
DeliveryProgress *progress_get(uint8_t index);
char *progress_get_error();
char *estimate_get();
