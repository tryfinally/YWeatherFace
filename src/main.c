#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

//for android     "13E75799-5FB0-4979-A07D-73BE447B933F"
#define MY_UUID { 0x13, 0xE7, 0x57, 0x99, 0x5F, 0xB0, 0x49, 0x79, 0xA0, 0x7D, 0x73, 0xBE, 0x44, 0x7B, 0x93, 0x3F }
PBL_APP_INFO(MY_UUID,
             "Yahoo Weather", "TryFinally",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

enum {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
};

static uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN,
  RESOURCE_ID_IMAGE_CLOUD,
  RESOURCE_ID_IMAGE_RAIN,
  RESOURCE_ID_IMAGE_SNOW
};

static struct WeatherData {
	Window window;
	TextLayer temperature_layer;
	BitmapLayer icon_layer;
	uint32_t current_icon;
	HeapBitmap icon_bitmap;
		
	TextLayer text_date_layer;
	Layer line_layer;
	TextLayer text_time_layer;
	
	AppSync sync;
	uint8_t sync_buffer[32];
} app;


static void load_bitmap(uint32_t resource_id) {
  // If that resource is already the current icon, we don't need to reload it
  if (app.current_icon == resource_id) {
    return;
  }
  
  if (app.current_icon != 0) {
    heap_bitmap_deinit(&app.icon_bitmap);
  }
  // Keep track of what the current icon is
  app.current_icon = resource_id;
  // Load the new icon
  heap_bitmap_init(&app.icon_bitmap, resource_id);
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
}


void line_layer_update_callback(Layer *me, GContext* ctx) {

  graphics_context_set_stroke_color(ctx, GColorWhite);

  graphics_draw_line(ctx, GPoint(8, 97), GPoint(131, 97));
  graphics_draw_line(ctx, GPoint(8, 98), GPoint(131, 98));

}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {

  switch (key) {
  case WEATHER_ICON_KEY:
    load_bitmap(WEATHER_ICONS[new_tuple->value->uint8]);
    bitmap_layer_set_bitmap(&app.icon_layer, &app.icon_bitmap.bmp);
    break;
	  
  case WEATHER_TEMPERATURE_KEY:
    text_layer_set_text(&app.temperature_layer, new_tuple->value->cstring);
    break;
  default:
    return;
  }
}

void handle_init(AppContextRef ctx) {
  resource_init_current_app(&WEATHER_APP_RESOURCES);
  app.current_icon = 0;
  
  window_init(&app.window, "YWeather");
  window_stack_push(&app.window, true /* Animated */);
  window_set_background_color(&app.window, GColorBlack);

  text_layer_init(&app.text_date_layer, app.window.layer.frame);
  text_layer_set_text_color(&app.text_date_layer, GColorWhite);
  text_layer_set_background_color(&app.text_date_layer, GColorClear);
  layer_set_frame(&app.text_date_layer.layer, GRect(8, 68, 144-8, 168-68));
  text_layer_set_font(&app.text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&app.window.layer, &app.text_date_layer.layer);


  text_layer_init(&app.text_time_layer, app.window.layer.frame);
  text_layer_set_text_color(&app.text_time_layer, GColorWhite);
  text_layer_set_background_color(&app.text_time_layer, GColorClear);
  layer_set_frame(&app.text_time_layer.layer, GRect(7, 92, 144-7, 168-92));
  text_layer_set_font(&app.text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49)));
  layer_add_child(&app.window.layer, &app.text_time_layer.layer);


  layer_init(&app.line_layer, app.window.layer.frame);
  app.line_layer.update_proc = &line_layer_update_callback;
  layer_add_child(&app.window.layer, &app.line_layer);
  
	
  GRect icon_rect = (GRect) {(GPoint) {4, 4}, (GSize) { 50, 50 }};
  bitmap_layer_init(&app.icon_layer, icon_rect);
  layer_add_child(&app.window.layer, &app.icon_layer.layer);

  text_layer_init(&app.temperature_layer, GRect(0, 10, 144, 40));
  text_layer_set_text_color(&app.temperature_layer, GColorWhite);
  text_layer_set_background_color(&app.temperature_layer, GColorClear);
  text_layer_set_font(&app.temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(&app.temperature_layer, GTextAlignmentRight);
  layer_add_child(&app.window.layer, &app.temperature_layer.layer);

	
  PblTm pt;
  get_time(&pt);
  void _handle_minute_tick(PblTm *tick_time);
  void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t);
  _handle_minute_tick(&pt);
	
  Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "99\u00B0C"),
  };
  app_sync_init(&app.sync, app.sync_buffer, sizeof(app.sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL);
}



void _handle_minute_tick(PblTm *tick_time);
void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
	_handle_minute_tick(t->tick_time);
}
void _handle_minute_tick(PblTm *tick_time) {

  // Need to be static because they're used by the system later.
  static char temp_text[] = "24 X";
  static char time_text[] = "00 00";
  static char date_text[] = "Xxxxxxxxx: 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  string_format_time(date_text, sizeof(date_text), "%B:%d", tick_time);
  text_layer_set_text(&app.text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  string_format_time(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  text_layer_set_text(&app.text_time_layer, time_text);
	
  //text_layer_set_text(&app.temperature_layer, temp_text);

  //load_bitmap(WEATHER_ICONS[2]);
  //bitmap_layer_set_bitmap(&app.icon_layer, &app.icon_bitmap.bmp);
}

static void handle_deinit(AppContextRef c) {
  //app_sync_deinit(&app.sync);
  if (app.current_icon != 0) {
    heap_bitmap_deinit(&app.icon_bitmap);
  }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 64,
        .outbound = 16,
      }
    }
  };
  app_event_loop(params, &handlers);
}
