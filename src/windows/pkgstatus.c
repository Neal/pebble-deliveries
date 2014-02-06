#include <pebble.h>
#include "pkgstatus.h"
#include "../libs/pebble-assist/pebble-assist.h"
#include "../common.h"

#define MAX_STATUSES 20

static Package statuses[MAX_STATUSES];
static Package package;

static int num_statuses;

static void refresh_list();
static void request_data();
static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void pkgstatus_init(Package pkg) {
	package = pkg;

	window = window_create();

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	window_stack_push(window, true);

	refresh_list();
}

void pkgstatus_destroy(void) {
	if (menu_layer) menu_layer_destroy(menu_layer);
	if (window) window_destroy(window);
}

void pkgstatus_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);

	if (index_tuple && title_tuple) {
		Package package;
		package.index = index_tuple->value->int16;
		strncpy(package.title, title_tuple->value->cstring, sizeof(package.title) - 1);
		statuses[package.index] = package;
		num_statuses++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "received package [%d] %s", package.index, package.title);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void refresh_list() {
	memset(statuses, 0x0, sizeof(statuses));
	num_statuses = 0;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	request_data();
}

static void request_data() {
	Tuplet status_tuple = TupletInteger(KEY_STATUS, 1);
	Tuplet index_tuple = TupletInteger(KEY_INDEX, package.index);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;
	dict_write_tuplet(iter, &status_tuple);
	dict_write_tuplet(iter, &index_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return (num_statuses) ? num_statuses : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return 0;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_statuses != 0) {
		return graphics_text_layout_get_content_size(statuses[cell_index->row].title, fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft).h + 8;
	}
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_statuses == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
	} else {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, statuses[cell_index->row].title, fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
