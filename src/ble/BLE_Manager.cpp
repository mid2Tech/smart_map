#include "ble/BLE_Manager.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan *pBLEScan;
int rssiThreshold = -105; // Hạ ngưỡng để bắt sóng xa 5-8 mét
int detectedMajor = 0;
int bestRSSI = -100;
unsigned long lastBeaconTime = 0;

String mac_tuyen_sinh = "2a:07:98:03:7b:38";
String mac_it = "2a:07:98:03:52:59";
String mac_thu_vien = "2a:07:98:03:bf:a1";
String mac_wc = "2a:07:98:03:54:6b";

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        String currentMAC = advertisedDevice.getAddress().toString().c_str();
        int currentRSSI = advertisedDevice.getRSSI();

        if (currentRSSI >= rssiThreshold)
        {
            // Định kỳ reset mức sóng để bắt trạm mới khi di chuyển
            if (millis() - lastBeaconTime > 5000)
            {
                detectedMajor = 0;
                bestRSSI = -100;
            }

            if (currentRSSI > bestRSSI)
            {
                bestRSSI = currentRSSI;
                lastBeaconTime = millis();
                if (currentMAC == mac_tuyen_sinh)
                    detectedMajor = 1;
                else if (currentMAC == mac_it)
                    detectedMajor = 2;
                else if (currentMAC == mac_thu_vien)
                    detectedMajor = 3;
                else if (currentMAC == mac_wc)
                    detectedMajor = 4;
            }
        }
    }
};

void setupBLE()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();

    // Tham số 'true' cho phép lưu lại các sóng đã quét
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);

    // BÍ QUYẾT 1: Quét thụ động (Chỉ nghe, không hỏi) -> Ổn định tín hiệu tuyệt đối
    pBLEScan->setActiveScan(false);

    // BÍ QUYẾT 2: Quét liên tục 100% thời gian (Window = Interval)
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(100);

    // Bắt đầu quét ngầm vô thời hạn
    pBLEScan->start(0, nullptr, false);
}

void scanAndProcessBLE()
{
    // Nếu đi ra khỏi vùng phủ sóng quá 3 giây, tàng hình vị trí
    if (millis() - lastBeaconTime > 3000)
    {
        detectedMajor = 0;
        bestRSSI = -100;
    }
}