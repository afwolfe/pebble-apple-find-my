
#define MAX_DEVICES 10
#define DEVICE_ID_LENGTH 96
#define MAX_NAME_LENGTH 32

typedef enum {
    PHONE,
    LAPTOP,
    TABLET,
    DESKTOP,
    WATCH,
    UNKNOWN
} DeviceClass;

typedef struct {
    uint8_t deviceId;
    char deviceName[MAX_NAME_LENGTH];
    DeviceClass deviceClass; 
} DeviceSummary;

static DeviceSummary devices[MAX_DEVICES];

static int device_count = 0;

static void add_device(uint8_t deviceId, char *deviceName, DeviceClass deviceClass) {

    // Don't do anything if device already exists.
    for (int i=0; i<device_count; i++) {
        if (devices[i].deviceId == deviceId) {
            return;
		}
	}
    if (device_count >= MAX_DEVICES) {
		return;
	}
	
    devices[device_count].deviceId = deviceId;
	strcpy(devices[device_count].deviceName, deviceName);
	devices[device_count].deviceClass = deviceClass;
	device_count++;
		
}

static DeviceSummary* get_device_at_index(int index) {
  if (index < 0 || index >= MAX_DEVICES) {
		return NULL;
	}
	
	return &devices[index];
}

static void send_find_request(int deviceIndex) {

    DictionaryIterator *iter;
    if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
        return;
    }
    if (dict_write_uint8(iter, MESSAGE_KEY_Command, COMMAND_FIND) != DICT_OK) {
        return;
    }
    if (dict_write_uint8(iter, MESSAGE_KEY_DeviceId, get_device_at_index(deviceIndex)->deviceId) != DICT_OK) {
        return;
    }
    app_message_outbox_send();
}