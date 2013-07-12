/*

  Bitter watch.

  A binary clock, based on Just a Bit, by Joshua Kronengold <mneme@labcats.org>

  This app and source is released under the terms of the original source, and adds
  no further restrictions beyond those terms.

    <http://en.wikipedia.org/wiki/Binary_clock>

 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x90, 0xF6, 0xB6, 0x4D, 0x38, 0x15, 0x4D, 0xD3, 0xAD, 0x80, 0x19, 0xFE, 0x8C, 0x70, 0x72, 0x12 }
PBL_APP_INFO(MY_UUID, "Bitter", "Labcats", 0x4, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

Layer display_layer;


#define CIRCLE_RADIUS 8
#define CIRCLE_LINE_THICKNESS 2

void draw_cell(GContext* ctx, GPoint center, bool filled) {
  // Each "cell" represents a binary digit or 0 or 1.

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_circle(ctx, center, CIRCLE_RADIUS);

  if (!filled) {
    // This is our ghetto way of drawing circles with a line thickness
    // of more than a single pixel.
    graphics_context_set_fill_color(ctx, GColorBlack);

    graphics_fill_circle(ctx, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
  }

}

#define CELLS_PER_ROW 6
#define CELLS_PER_COLUMN 5

#define CIRCLE_PADDING 10 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define SIDE_PADDING (144 - (CELLS_PER_ROW * CELL_SIZE))/2
#define TOP_PADDING (168 - (CELLS_PER_COLUMN * CELL_SIZE))/2



GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
  // Cell location (0,0) is upper left, location (4, 6) is lower right.
  return GPoint(SIDE_PADDING + (CELL_SIZE/2) + (CELL_SIZE * x),
		TOP_PADDING + (CELL_SIZE/2) + (CELL_SIZE * y));
}

void draw_cell_row_for_digit(GContext* ctx, unsigned short digit, unsigned short max_columns_to_display, unsigned short cell_row) {
  // Converts the supplied decimal digit into Binary Coded Decimal form and
  // then draws a row of cells on screen--'1' binary values are filled, '0' binary values are not filled.
  // `max_columns_to_display` restricts how many binary digits are shown in the row.
  for (int cell_column_index = 0; cell_column_index < max_columns_to_display; cell_column_index++) {
    draw_cell(ctx, get_center_point_from_cell_location(cell_column_index, cell_row), (digit >> cell_column_index) & 0x1);
  }
}


// The cell row offsets for each digit
#define MONTHS_ROW 0
#define DAYS_ROW 1
#define HOURS_ROW 2
#define MINUTES_ROW 3
#define SECONDS_ROW 4

// The maximum number of cell columns to display
// (Used so that if a binary digit can never be 1 then no un-filled
// placeholder is shown.)
#define MAX_COLS 6
#define MONTHS_MAX_COLS 4
#define DAYS_MAX_COLS 5
#define HOURS_MAX_COLS 5
#define MINUTES_MAX_COLS MAX_COLS
#define SECONDS_MAX_COLS MAX_COLS

unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_layer_update_callback(Layer *me, GContext* ctx) {

  PblTm t;

  get_time(&t);

  unsigned short display_hour = get_display_hour(t.tm_hour);

  draw_cell_row_for_digit(ctx, t.tm_mon+1, MONTHS_MAX_COLS, MONTHS_ROW);
  draw_cell_row_for_digit(ctx, t.tm_mday, DAYS_MAX_COLS, DAYS_ROW);
  draw_cell_row_for_digit(ctx, display_hour, HOURS_MAX_COLS, HOURS_ROW);
  draw_cell_row_for_digit(ctx, t.tm_min, MINUTES_MAX_COLS, MINUTES_ROW);
  draw_cell_row_for_digit(ctx, t.tm_sec, SECONDS_MAX_COLS, SECONDS_ROW);
/*
  draw_cell_row_for_digit(ctx, display_hour / 10, HOURS_FIRST_DIGIT_MAX_COLS, HOURS_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, display_hour % 10, DEFAULT_MAX_COLS, HOURS_SECOND_DIGIT_ROW);

  draw_cell_row_for_digit(ctx, t.tm_min / 10, MINUTES_FIRST_DIGIT_MAX_COLS, MINUTES_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t.tm_min % 10, DEFAULT_MAX_COLS, MINUTES_SECOND_DIGIT_ROW);

  draw_cell_row_for_digit(ctx, t.tm_sec / 10, SECONDS_FIRST_DIGIT_MAX_COLS, SECONDS_FIRST_DIGIT_ROW);
  draw_cell_row_for_digit(ctx, t.tm_sec % 10, DEFAULT_MAX_COLS, SECONDS_SECOND_DIGIT_ROW);
*/
}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {

  layer_mark_dirty(&display_layer);
}


void handle_init(AppContextRef ctx) {

  window_init(&window, "Not Just A Bit watch");
  window_stack_push(&window, true);

  window_set_background_color(&window, GColorBlack);

  // Init the layer for the display
  layer_init(&display_layer, window.layer.frame);
  display_layer.update_proc = &display_layer_update_callback;
  layer_add_child(&window.layer, &display_layer);


}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
