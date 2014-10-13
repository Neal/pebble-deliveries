#include <pebble.h>
#include "win-progress.h"
#include "libs/pebble-assist.h"
#include "deliveries.h"

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;

void win_progress_init(void) {
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
}

void win_progress_push(void) {
	request_progress();
	window_stack_push(window, true);
}

void win_progress_deinit(void) {
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void win_progress_reload_data_and_mark_dirty(void) {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return estimate_get() ? 2 : 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	if (estimate_get() && section_index == 0) return 1;
	return progress_count() ? progress_count() : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (estimate_get() && cell_index->section == 0) {
		return graphics_text_layout_get_content_size(estimate_get(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 0, 136, 48), GTextOverflowModeFill, GTextAlignmentLeft).h + 10;
	}
	if (progress_get_error()) {
		return graphics_text_layout_get_content_size(progress_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft).h + 12;
	}
	if (progress_get(cell_index->row)->status)
		return graphics_text_layout_get_content_size(progress_get(cell_index->row)->status, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(2, 0, 140, 128), GTextOverflowModeFill, GTextAlignmentLeft).h + 10;
	return 0;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	if (estimate_get() && section_index == 0) {
		menu_cell_basic_header_draw(ctx, cell_layer, "Delivery Estimate");
	} else {
		menu_cell_basic_header_draw(ctx, cell_layer, "Shipment Progress");
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	graphics_context_set_text_color(ctx, GColorBlack);
	if (estimate_get() && cell_index->section == 0 && cell_index->row == 0) {
		graphics_draw_text(ctx, estimate_get(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 0, 136, 48), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else if (progress_get_error()) {
		graphics_draw_text(ctx, progress_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		graphics_draw_text(ctx, progress_get(cell_index->row)->status, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(2, 2, 140, 128), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	request_progress();
}
