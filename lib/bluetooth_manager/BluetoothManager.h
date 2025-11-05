#include <Arduino.h>
#include <env.h>

#ifndef ENV_DEVICE_NAME
#error "Must specified device name!"
#endif

#define BLUETOOTH_BPM_CHARACTERISTICS_UUID "209afaca-46c9-4aa0-8f1a-540e7266ea72"
#define BLUETOOTH_SENSOR_SERVICE_UUID "6edda78e-092b-47d9-8eb8-3199598c5515"
#define BLUETOOTH_ADVERTISING_NAME ENV_DEVICE_NAME
#define BLUETOOTH_MTU_VALUE 512

#define BLUETOOTH_DEFAULT_BLE_PROPERTIES NIMBLE_PROPERTY::READ |\
                                         NIMBLE_PROPERTY::WRITE |\
                                         NIMBLE_PROPERTY::NOTIFY

class BluetoothManager
{
public:
      static void init();
      static void update_bpm(uint8_t hr);
};

