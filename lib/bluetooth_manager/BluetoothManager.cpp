#include <Arduino.h>
#include <BluetoothManager.h>
#include <NimBLEDevice.h>

// Globally Static Variables
static NimBLECharacteristic *ble_bpm_characteristics;
static NimBLEServer *ble_server;
static NimBLEService *ble_sensor_service;

// BLE Callbacks
class ServerCallbacks : public NimBLEServerCallbacks
{
      void onConnect(NimBLEServer *ble_server, NimBLEConnInfo &ble_conn_info) override
      {
            Serial.printf("[Bluetooth] Connected client with address: %s\n", ble_conn_info.getAddress().toString().c_str());
      }

      void onDisconnect(NimBLEServer *ble_server, NimBLEConnInfo &ble_conn_info, int reason) override
      {
            Serial.printf("[Bluetooth] Disconnected client with address: %s\n", ble_conn_info.getAddress().toString().c_str());
            NimBLEDevice::startAdvertising();
      }

      uint32_t onPassKeyDisplay() override
      {
            Serial.printf("[Bluetooth] Server Passkey Display.\n");
            return ENV_BLE_PASSKEY;
      }

      void onConfirmPassKey(NimBLEConnInfo &connInfo, uint32_t pass_key) override
      {
            Serial.printf("[Bluetooth] The passkey for number: %" PRIu32 "\n", pass_key);

            /** Inject false if passkeys don't match. */
            NimBLEDevice::injectConfirmPasskey(connInfo, pass_key == ENV_BLE_PASSKEY);
      }

      void onAuthenticationComplete(NimBLEConnInfo &connInfo) override
      {
            /** Check that encryption was successful, if not we disconnect the client */
            if (!connInfo.isEncrypted())
            {
                  NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
                  Serial.print("[Bluetooth] Encrypt connection failed - disconnecting client\n");
                  return;
            }

            Serial.printf("[Bluetooth] Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
      }
} serverCallbacks;

void BluetoothManager::init()
{
      Serial.println("[Bluetooth] Initializing Bluetooth Server");

      // Initialize Basic Functions
      NimBLEDevice::init(BLUETOOTH_ADVERTISING_NAME); 
      NimBLEDevice::setMTU(BLUETOOTH_MTU_VALUE);

      // Initialize Server
      ble_server = NimBLEDevice::createServer();
      ble_server->setCallbacks(&serverCallbacks);

      // Initialize Service
      ble_sensor_service = ble_server->createService(BLUETOOTH_SENSOR_SERVICE_UUID);

      // Initialize Characteristics
      ble_bpm_characteristics = ble_sensor_service->createCharacteristic(BLUETOOTH_BPM_CHARACTERISTICS_UUID, BLUETOOTH_DEFAULT_BLE_PROPERTIES);

      // Starting service
      ble_sensor_service->start();

      // Initialize Advertiser
      NimBLEAdvertising *ble_advertising = NimBLEDevice::getAdvertising();
      ble_advertising->setName(BLUETOOTH_ADVERTISING_NAME);
      ble_advertising->addServiceUUID(ble_sensor_service->getUUID());
      ble_advertising->enableScanResponse(true);
      ble_advertising->start();
}

void BluetoothManager::update_bpm(uint8_t hr)
{
      // Set the value of BLE characteristic for heart rate
      ble_bpm_characteristics->setValue(String(hr).c_str());
      ble_bpm_characteristics->notify();
      Serial.println("[Bluetooth] Updating BPM value");
}
