/**
 * Example implementation of the dialog message UI pattern.
 */
#include <pebble.h>
#include "libs/pebble-assist.h"
#include "dialog-message.h"

static Window *s_window;
static TextLayer *s_label_layer;
static Layer *s_background_layer, *s_icon_layer;
static Animation *s_appear_anim = NULL;

// static GBitmap *s_icon_bitmap;
static GDrawCommandImage *s_command_image;

static int s_status;
static char s_message[DIALOG_MESSAGE_MAX_LENGTH];

static void prv_anim_stopped_handler(Animation *animation, bool finished, void *context) {
  s_appear_anim = NULL;
}

static void prv_background_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite));
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, 0);
}

static void prv_icon_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  // GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);
  // graphics_context_set_compositing_mode(ctx, GCompOpSet);
  // Set the origin offset from the context for drawing the image
  GPoint origin = GPoint(0, 0);

  // Draw the GDrawCommandImage to the GContext
  gdraw_command_image_draw(ctx, s_command_image, origin);
  

  // graphics_draw_bitmap_in_rect(ctx, s_icon_bitmap, (GRect){.origin = bounds.origin, .size = bitmap_bounds.size});
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const GEdgeInsets background_insets = {.top = bounds.size.h  /* Start hidden */};
  s_background_layer = layer_create(grect_inset(bounds, background_insets));
  layer_set_update_proc(s_background_layer, prv_background_update_proc);
  layer_add_child(window_layer, s_background_layer);

  // Create the canvas Layer
  GSize image_bounds = gdraw_command_image_get_bounds_size(s_command_image);
  // s_icon_layer = layer_create(GRect(30, 30, bounds.size.w, bounds.size.h));
  s_icon_layer = layer_create(PBL_IF_ROUND_ELSE(
      GRect((bounds.size.w - image_bounds.w) / 2, bounds.size.h + DIALOG_MESSAGE_WINDOW_MARGIN, image_bounds.w, image_bounds.h),
      GRect(DIALOG_MESSAGE_WINDOW_MARGIN, bounds.size.h + DIALOG_MESSAGE_WINDOW_MARGIN, image_bounds.w, image_bounds.h)
  ));
  layer_set_update_proc(s_icon_layer, prv_icon_update_proc);
  layer_add_child(window_layer, s_icon_layer);


  s_label_layer = text_layer_create(bounds);

  text_layer_set_text_color(s_label_layer, GColorBlack);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  text_layer_set_text(s_label_layer, s_message);
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void prv_window_unload(Window *window) {
  layer_destroy_safe(s_background_layer);
  layer_destroy_safe(s_icon_layer);

  text_layer_destroy(s_label_layer);

  window_destroy(window);
  s_window = NULL;
}


static void prv_window_appear(Window *window) {
  if(s_appear_anim) {
     // In progress, cancel
    animation_unschedule(s_appear_anim);
  }

  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  // GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);
  GSize image_bounds = gdraw_command_image_get_bounds_size(s_command_image);

  Layer *label_layer = text_layer_get_layer(s_label_layer);

  GRect start = layer_get_frame(s_background_layer);
  GRect finish = bounds;
  Animation *background_anim = (Animation*)property_animation_create_layer_frame(s_background_layer, &start, &finish);

  start = layer_get_frame(s_icon_layer);
  const GEdgeInsets icon_insets = {
    .top = DIALOG_MESSAGE_WINDOW_MARGIN,
    .left = PBL_IF_ROUND_ELSE((bounds.size.w - image_bounds.w) / 2, DIALOG_MESSAGE_WINDOW_MARGIN)};
  finish = grect_inset(bounds, icon_insets);
  Animation *icon_anim = (Animation*)property_animation_create_layer_frame(s_icon_layer, &start, &finish);

  start = layer_get_frame(label_layer);
  const GEdgeInsets finish_insets = {
    .top = DIALOG_MESSAGE_WINDOW_MARGIN + 25 + 5 /* small adjustment */,
    .right = DIALOG_MESSAGE_WINDOW_MARGIN, .left = DIALOG_MESSAGE_WINDOW_MARGIN};
  finish = grect_inset(bounds, finish_insets);
  Animation *label_anim = (Animation*)property_animation_create_layer_frame(label_layer, &start, &finish);

  s_appear_anim = animation_spawn_create(background_anim, icon_anim, label_anim, NULL);
  animation_set_handlers(s_appear_anim, (AnimationHandlers) {
    .stopped = prv_anim_stopped_handler
  }, NULL);
  animation_set_delay(s_appear_anim, 700);
  animation_schedule(s_appear_anim);
}

static void prv_set_icon_and_message() {
  switch (s_status) {
    case 1:
      s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_FAILED);
      strcpy(s_message, DIALOG_MESSAGE_FETCH_ERROR);
      break;
    case 200:
      s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_CONFIRMATION);
      strcpy(s_message, DIALOG_MESSAGE_REQUEST_SUCCESS);
      break;
    case 408:
      s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_FAILED);
      strcpy(s_message, DIALOG_MESSAGE_REQUEST_TIMEOUT);
      break;
    default:
      s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_FAILED);
      strcpy(s_message, DIALOG_MESSAGE_REQUEST_FAILURE);
      break;
  }
  
}

void dialog_message_window_push(int status) {
  s_status = status;
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = prv_window_load,
        .unload = prv_window_unload,
        .appear = prv_window_appear
    });
  }

  prv_set_icon_and_message();

  window_stack_push(s_window, true);
}
