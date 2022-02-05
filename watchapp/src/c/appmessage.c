#include <pebble.h>
#include "libs/pebble-assist.h"
#include "find-my.h"
#include "appmessage.h"
#include "commands.h"
#include "device.h"
#include "dialog-message.h"

static int outbound_size;
static int inbound_size;

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  find_my_inbox_received_handler(iter);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  ERROR("Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  ERROR("Outbox send failed!");
  dialog_message_window_push(0);
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  ERROR("Outbox send success!");
}

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

void appmessage_init() {
  app_message_register_inbox_received(inbox_received_callback);

  in_out_size_calc();
  app_message_open(inbound_size, outbound_size);

  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}