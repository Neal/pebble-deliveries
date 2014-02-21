#include <pebble.h>
#include "pkglist.h"
#include "../libs/pebble-assist/pebble-assist.h"
#include "../common.h"
#include "pkgstatus.h"

#define MAX_PACKAGES 20

static Package packages[MAX_PACKAGES];

static int num_packages;
static bool no_packages;
static bool out_sent;
static bool out_failed;

static void refresh_list();
static void request_data();
static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void pkglist_init(void) {
	window = window_create();

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	window_stack_push(window, true);

	refresh_list();
}

void pkglist_destroy(void) {
	pkgstatus_destroy();
	if (menu_layer) menu_layer_destroy(menu_layer);
	if (window) window_destroy(window);
}

void pkglist_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);
	Tuple *subtitle_tuple = dict_find(iter, KEY_SUBTITLE);

	if (index_tuple && title_tuple && subtitle_tuple) {
		if (index_tuple->value->int16 == 0) {
			num_packages = 0;
			no_packages = false;
		}
		Package package;
		package.index = index_tuple->value->int16;
		strncpy(package.title, title_tuple->value->cstring, sizeof(package.title) - 1);
		strncpy(package.subtitle, subtitle_tuple->value->cstring, sizeof(package.subtitle) - 1);
		packages[package.index] = package;
		num_packages++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
	else if (index_tuple) {
		no_packages = true;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
}

void pkglist_in_dropped_handler(AppMessageResult reason) {

}

void pkglist_out_sent_handler(DictionaryIterator *sent) {
	out_sent = true;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

void pkglist_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	out_failed = true;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

bool pkglist_is_on_top() {
	return window == window_stack_get_top_window();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void refresh_list() {
	memset(packages, 0x0, sizeof(packages));
	num_packages = 0;
	no_packages = false;
	out_sent = false;
	out_failed = false;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	request_data();
}

static void request_data() {
	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return (num_packages) ? num_packages : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, "Packages");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (out_failed) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "Unable to connect to phone!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 0 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else if (no_packages) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "No packages added.", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 8 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else if (num_packages == 0) {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, "Loading packages...", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 2, 8 }, .size = { PEBBLE_WIDTH - 4, 128 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		menu_cell_basic_draw(ctx, cell_layer, packages[cell_index->row].title, packages[cell_index->row].subtitle, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_packages > 0) {
		pkgstatus_init(packages[cell_index->row]);
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
