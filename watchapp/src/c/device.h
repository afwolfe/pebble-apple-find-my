#pragma once
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

typedef enum {
    CHARGING,
    NOT_CHARGING,
    BATTERY_UNKNOWN,
} BatteryStatus;

typedef struct {
    uint8_t deviceId;
    char deviceName[MAX_NAME_LENGTH];
    DeviceClass deviceClass;
    uint16_t deviceStatus;
    char batteryLevel[4];
    BatteryStatus batteryStatus;
} DeviceSummary;

void add_device(uint8_t deviceId, char *deviceName, DeviceClass deviceClass, uint16_t deviceStatus, char *batteryLevel, BatteryStatus BatteryStatus);

DeviceSummary* get_device_at_index(int index);

void send_find_request(int deviceIndex);

int get_device_count();