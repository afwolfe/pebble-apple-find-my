#include <pebble.h>

enum sections {
  DEVICE_SECTION = 0,
  NUM_MENU_SECTIONS
};

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case DEVICE_SECTION: {
      return device_count;
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