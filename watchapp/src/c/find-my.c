#include <pebble.h>
#include "libs/pebble-assist.h"
#include "find-my.h"
#include "appmessage.h"
#include "commands.h"
#include "dialog-message.h"
#include "device.h"

enum sections {
  DEVICE_SECTION,
  NUM_MENU_SECTIONS
};

static Window *s_window;
static MenuLayer *s_menu_layer;

void find_my_inbox_received_handler(DictionaryIterator *iter) {
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
      Tuple *status_tuple = dict_find(iter, MESSAGE_KEY_Status);
      if (status_tuple) {
        LOG("Status: %d", status_tuple->value->uint16);
        dialog_message_window_push(status_tuple->value->uint16);
      }

      // menu_layer_reload_data(s_menu_layer);
      break;
    }
    default: {
      ERROR("Unknown command");
      break;
    }
  }
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case DEVICE_SECTION: {
      return get_device_count();
    }
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    // Determine which section we're working with
    switch (section_index) {
      case DEVICE_SECTION:
        // Draw title text in the section header
        menu_cell_basic_header_draw(ctx, cell_layer, "Devices");
        break;
      default:
        break;
    }
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  const int index = cell_index->row;
  DeviceSummary* device = get_device_at_index(index);

  if (device == NULL) {
    return;
  }
  // TODO: Implement battery/status subtitle
  // TODO: Implement icons
  menu_cell_basic_draw(ctx, cell_layer, device->deviceName, NULL, NULL);    

}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item will receive the git action
    const int index = cell_index->row;
    send_find_request(index);

}

#ifdef PBL_ROUND
static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if (menu_layer_is_index_selected(menu_layer, cell_index)) {
    switch (cell_index->row) {
    case 0:
      return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
      break;
    default:
      return MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT;
    }
  }
  else {
    return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
  }
}
#endif

static void prv_window_load(Window *window) {

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

static void prv_window_unload(Window *window) {
  menu_layer_destroy_safe(s_menu_layer);
}

static void init(void) {
  appmessage_init();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload,
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
