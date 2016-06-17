#include <pebble.h>

static Window *s_window;
static GBitmap* s_bitmap;
static BitmapLayer *s_bitmap_layer;
static Layer* drawing_layer;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void draw_drawing(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  GPoint left_center = GPoint(bounds.size.w/4, bounds.size.h/2);;
  GPoint right_center = GPoint(bounds.size.w*3/4, bounds.size.h/2);;
#ifdef PBL_RECT
  const int16_t outer_radius = 27;
#else
  const int16_t outer_radius = 32;
#endif
  const int16_t inner_radius = outer_radius - 6;
  const int16_t second_hand_length = bounds.size.w / 2 - 7;
  const int16_t second_hand_head_length = bounds.size.w / 2 / 3;
  const int16_t second_tail_length = bounds.size.w / 2 / 3;
#ifdef PBL_RECT
  const int16_t minute_hand_length = 14;
#else
  const int16_t minute_hand_length = 18;
#endif

  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  int32_t hour_angle = TRIG_MAX_ANGLE * (t->tm_hour % 12) / 12;
  GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  GPoint second_head = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_head_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_head_length / TRIG_MAX_RATIO) + center.y,
  };
  GPoint second_tail = {
    .x = (int16_t)(-sin_lookup(second_angle) * (int32_t)second_tail_length/ TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(cos_lookup(second_angle) * (int32_t)second_tail_length / TRIG_MAX_RATIO) + center.y,
  };
  GPoint hour_hand = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)minute_hand_length/ TRIG_MAX_RATIO),
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO),
  };
  GPoint minute_hand = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minute_hand_length/ TRIG_MAX_RATIO),
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO),
  };

  // Draw left rectangle
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_fill_rect(ctx, GRect(left_center.x - outer_radius, left_center.y - outer_radius,outer_radius*2,outer_radius*2), 6, GCornersAll);
  graphics_draw_round_rect(ctx, GRect(left_center.x - outer_radius, left_center.y - outer_radius,outer_radius*2,outer_radius*2), 6);
  graphics_draw_circle(ctx, left_center, inner_radius);

  // Draw right circle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, right_center, outer_radius);
  graphics_draw_circle(ctx, right_center, outer_radius);
  graphics_draw_circle(ctx, right_center, inner_radius);

  // Draw hour and minute hands
  graphics_draw_line(ctx, GPoint(left_center.x - hour_hand.x, left_center.y - hour_hand.y), GPoint(left_center.x + hour_hand.x, left_center.y + hour_hand.y));
  graphics_draw_line(ctx, GPoint(right_center.x - minute_hand.x, right_center.y - minute_hand.y), GPoint(right_center.x + minute_hand.x, right_center.y + minute_hand.y));

  // Draw the second hand
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, center, second_hand);
  graphics_draw_line(ctx, center, second_tail);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_line(ctx, center, second_head);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_fill_circle(ctx, center, 6);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_circle(ctx, center, 3);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bitmap_layer = bitmap_layer_create(bounds);
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BG);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  drawing_layer = layer_create(bounds);
  layer_set_update_proc(drawing_layer, draw_drawing);
  layer_add_child(window_layer, drawing_layer);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_destroy(s_bitmap);
  layer_destroy(drawing_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
