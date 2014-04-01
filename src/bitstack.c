/*

  Bitstack watch.

  A vertical binary clock, based on Just a Bit, by Joshua Kronengold <mneme@labcats.org>

  This app and source is released under the terms of the original source, and adds
  no further restrictions beyond those terms.

    <http://en.wikipedia.org/wiki/Binary_clock>

 */

#include "pebble.h"

Window *mywindow;

Layer *display_layer;


#define CIRCLE_RADIUS 11 
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

#define CELLS_PER_ROW 5
#define CELLS_PER_COLUMN 6

#define CIRCLE_PADDING 14 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define SIDE_PADDING (144 - (CELLS_PER_ROW * CELL_SIZE))/2
#define TOP_PADDING (168 - (CELLS_PER_COLUMN * CELL_SIZE))/2



GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
  // Cell location (0,0) is upper left, location (4, 6) is lower right.
  return GPoint(SIDE_PADDING + (CELL_SIZE/2) + (CELL_SIZE * x),
		TOP_PADDING + (CELL_SIZE/2) + (CELL_SIZE * y));
}

void draw_cell_row_for_digit_at(GContext* ctx, unsigned short digit, unsigned short max_rows_to_display, unsigned short cell_column, unsigned short starting_row) {
  // Converts the supplied decimal digit into Binary Coded Decimal form and
  // then draws a row of cells on screen--'1' binary values are filled, '0' binary values are not filled.
  // `max_columns_to_display` restricts how many binary digits are shown in the row.
  for (int cell_row_index = 0; cell_row_index < max_rows_to_display; cell_row_index++) {
    // we're drawing from the bottom, so idx is actually 6-idx
    draw_cell(ctx, get_center_point_from_cell_location(cell_column,
                        5-(starting_row+cell_row_index)), (digit >> cell_row_index) & 0x1);
  }
}

void draw_cell_row_for_digit(GContext* ctx, unsigned short digit, unsigned short max_rows_to_display, unsigned short cell_column) {
	draw_cell_row_for_digit_at(ctx,digit, max_rows_to_display, cell_column, 0);
}

// The cell row offsets for each digit
#define MONTHS_COLUMN 0
#define DAYS_COLUMN 1
#define HOURS_COLUMN 2
#define MINUTES_COLUMN 3
#define SECONDS_COLUMN 4
#define AMPM_COLUMN 2

// The maximum number of cell columns to display
// (Used so that if a binary digit can never be 1 then no un-filled
// placeholder is shown.)
#define MAX_ROWS 6
#define MONTHS_MAX_ROWS 4
#define DAYS_MAX_ROWS 5
#define HOURS_24_MAX_ROWS 5
#define HOURS_12_MAX_ROWS 4
#define AMPM_MAX_ROWS 1
#define AMPM_ROW 5
#define MINUTES_MAX_ROWS MAX_ROWS
#define SECONDS_MAX_ROWS MAX_ROWS

unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_layer_update_callback(Layer *me, GContext* ctx) {
  time_t rawtime;
  struct tm *t;
  time(&rawtime);
  t = localtime(&rawtime);

  unsigned short display_hour = get_display_hour(t->tm_hour);

  draw_cell_row_for_digit(ctx, t->tm_mon+1, MONTHS_MAX_ROWS, MONTHS_COLUMN);
  draw_cell_row_for_digit(ctx, t->tm_mday, DAYS_MAX_ROWS, DAYS_COLUMN);
  if(clock_is_24h_style()) {
  	draw_cell_row_for_digit(ctx, display_hour, HOURS_24_MAX_ROWS, HOURS_COLUMN);
  } else {
  	draw_cell_row_for_digit(ctx, display_hour, HOURS_12_MAX_ROWS, HOURS_COLUMN);
        draw_cell_row_for_digit_at(ctx, t->tm_hour > 11 ? 1 : 0, AMPM_MAX_ROWS, AMPM_COLUMN, AMPM_ROW);
  }
  draw_cell_row_for_digit(ctx, t->tm_min, MINUTES_MAX_ROWS, MINUTES_COLUMN);
  draw_cell_row_for_digit(ctx, t->tm_sec, SECONDS_MAX_ROWS, SECONDS_COLUMN);
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

  layer_mark_dirty(display_layer);
}


void handle_init(void) {

  mywindow = window_create();
  display_layer = layer_create(GRect(0, 0, 144, 168));

  window_stack_push(mywindow, true);

  window_set_background_color(mywindow, GColorBlack);
  
  Layer *root_layer = window_get_root_layer(mywindow);
  GRect frame = layer_get_frame(root_layer);

  // Init the layer for the display
  display_layer = layer_create(frame);
  layer_set_update_proc(display_layer, &display_layer_update_callback);
  layer_add_child(root_layer, display_layer);
  tick_timer_service_subscribe(SECOND_UNIT,&handle_second_tick);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  layer_destroy(display_layer);
  window_destroy(mywindow);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
  return(0);
}
