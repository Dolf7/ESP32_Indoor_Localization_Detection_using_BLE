#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <esp_now.h>
#include <math.h>
#include "data.h"

const char *ssid = "TPA Darul Hikmah Bawah";
const char *password = "1sampe10";
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t spesificAddress[] = {0x34, 0x94, 0x54, 0x24, 0x86, 0xC0};

typedef struct struct_message
{
  uint8_t a[6]; // 1Packet Status | 4 Distances |
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

// 0 Gateway{1=G | 0=N} | 1 Standby{1=STNDBY|0=SCAN} |
int nodeStatus[3] = {1, 1, 0};
// put function declarations here:
void connectWifi();
void BLE_SET();
void BLE_SCAN();
void ESP_SET();
float RSSItoDistance(int);
// void espNowSent(uint8_t *, uint8_t *);
void packingData(uint8_t, int);
void onDataReceived(const uint8_t *, const uint8_t *, int);

void setup()
{
  Serial.begin(115200);
  // put your setup code here, to run once:
  if (nodeStatus[0] == 1)
  {
    connectWifi();
  }
  BLE_SET();
  ESP_SET();
}

void loop()
{
  // put your main code here, to run repeatedly:
  // BLE_SCAN();
  // delay(500);
}

// put function definitions here:
void connectWifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\n Connect to %s \n", ssid);
  WiFi.mode(WIFI_STA);
}

// FOR BLE SECTION
int scanTime = 5; // In seconds
int bleRSSI = 0;  //
BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    // Serial.printf("Address : %s \n", advertisedDevice.getAddress().toString().c_str());
    String address = advertisedDevice.getAddress().toString().c_str();
    if (address == "f0:a9:71:17:57:77")
    {
      // Serial.printf("FOUND Address : %s \n", advertisedDevice.getAddress().toString().c_str());
      // Serial.printf("RSSI : %d \n ", advertisedDevice.getRSSI());
      BLEData.dataProcessInputforThisNode(RSSItoDistance(advertisedDevice.getRSSI()));
      // Serial.println(BLEData.data[0][1]);
      if (nodeStatus[0] == 0)
      {
        // esp_now_send(spesificAddress, packingData(1, advertisedDevice.getRSSI()), 6);
        // esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
        packingData(1, advertisedDevice.getRSSI());
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
        if (result == ESP_OK)
          Serial.println("DATA SENT");
        else
          Serial.println("DATA FAILED TO SENT");
      }
    }
  }
};

void BLE_SET()
{
  // Serial.println("Scanning");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void BLE_SCAN()
{
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}

float RSSItoDistance(int RSSI)
{
  const float rssi1M = -85, n = 0.47712;
  float x = pow(10.0, (rssi1M - RSSI) / 10 * n);
  // Serial.print("FUNCTION D : ");
  // Serial.println(x);
  return x;
}

void packingData(uint8_t packetType, int RSSI)
{
  myData.a[0] = packetType;
  myData.a[1] = mapFloatToInteger(RSSItoDistance(RSSI));
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void onDataReceived(const uint8_t *macAddr, const uint8_t *incomingData, int dataLen)
{
  // Process the received data
  // Example: Print the received data
  char macStr[18];
  Serial.print("Received data from ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  for(int i=0;i<3;i++){
    Serial.print((int)myData.a[i]);
  }
  BLEData.dataProcessInput(macAddr, incomingData, 3);
}

void ESP_SET()
{
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  
  // esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(onDataReceived);
}
