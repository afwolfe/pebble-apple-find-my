#include <pebble.h>
#include "libs/pebble-assist.h"
#include "commands.h"
#include "device.h"
#include "menu.h"


static Window *s_window;
static TextLayer *s_text_layer;
static MenuLayer *s_menu_layer;

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  Tuple *tuple = dict_find(iter, MESSAGE_KEY_Command);
  if (!tuple) {
    return;
  }

  switch (tuple->value->uint8) {
    case COMMAND_LIST: {
      INFO("LIST command received");
      Tuple *id_tuple = dict_find(iter, MESSAGE_KEY_DeviceId);
      Tuple *name_tuple = dict_find(iter, MESSAGE_KEY_DeviceName);
      Tuple *class_tuple = dict_find(iter, MESSAGE_KEY_DeviceClass);

      if (id_tuple && name_tuple && class_tuple) {
        LOG("Received device: %s", name_tuple->value->cstring);
        add_device(id_tuple->value->uint8, name_tuple->value->cstring, class_tuple->value->uint8);
        menu_layer_reload_data_and_mark_dirty(s_menu_layer);
      }
      break;
    }
    case COMMAND_FIND: {
      INFO("FIND command received");
      // TODO: Handle find response
      // menu_layer_reload_data(s_menu_layer);
      break;
    }
    default: {
      ERROR("Unknown command");
      break;
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  ERROR("Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  ERROR("Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  ERROR("Outbox send success!");
}

static void window_load(Window *window) {

  // Add menu to window
  s_menu_layer = menu_layer_create_fullscreen(window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
    .draw_header = PBL_IF_RECT_ELSE(menu_draw_header_callback, NULL),
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
  });

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_add_to_window(s_menu_layer, window);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}


static int outbound_size;
static int inbound_size;

/*
 * Accurate buffer size count
 */
static void in_out_size_calc() {

  Tuplet out_values[] = {
      TupletInteger(MESSAGE_KEY_Command, COMMAND_FIND),  // Only send one value at a time so this is right
      TupletInteger(MESSAGE_KEY_DeviceId, 123)
  };
  outbound_size = dict_calc_buffer_size_from_tuplets(out_values, ARRAY_LENGTH(out_values)) + 8;

  Tuplet in_values[] = {
    TupletInteger(MESSAGE_KEY_Command, COMMAND_FIND),
    TupletCString(MESSAGE_KEY_DeviceName, "abcdefghijklmnopqrstuvwxyz123456"),
    TupletInteger(MESSAGE_KEY_DeviceId, 123),
    TupletInteger(MESSAGE_KEY_DeviceClass, TABLET)
  };
  inbound_size = dict_calc_buffer_size_from_tuplets(in_values, ARRAY_LENGTH(in_values)) + 8;

  INFO("In buff %d, Out buff %d", inbound_size, outbound_size);
}

static void init(void) {
  app_message_register_inbox_received(inbox_received_callback);

  in_out_size_calc();
  app_message_open(inbound_size, outbound_size);

  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = false;
  window_stack_push(s_window, animated);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
