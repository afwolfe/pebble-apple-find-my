#pragma once

#define DIALOG_MESSAGE_REQUEST_SUCCESS "Request sent successfully."
#define DIALOG_MESSAGE_REQUEST_FAILURE "Error sending request."
#define DIALOG_MESSAGE_REQUEST_TIMEOUT "Timed out sending request. Check your connection."
#define DIALOG_MESSAGE_FETCH_ERROR "Error getting devices. Check config/connection."

#define DIALOG_MESSAGE_WINDOW_MARGIN   10

#define DIALOG_MESSAGE_MAX_LENGTH 50


void dialog_message_window_push(int status);
