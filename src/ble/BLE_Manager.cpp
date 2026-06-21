#include "ble/BLE_Manager.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan *pBLEScan;
int rssiThreshold = -85;
int detectedMajor = 0;
int bestRSSI = -200;
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
            int mac_id = 0;
            if (currentMAC == mac_tuyen_sinh)
                mac_id = 1;
            else if (currentMAC == mac_it)
                mac_id = 2;
            else if (currentMAC == mac_thu_vien)
                mac_id = 3;
            else if (currentMAC == mac_wc)
                mac_id = 4;

            if (mac_id > 0)
            {
                // BÍ QUYẾT ĐỂ NHẬN NHANH: Luôn cập nhật mức sóng nếu là trạm đang đứng,
                // hoặc nếu gặp trạm mới có sóng mạnh hơn. Không bị "kẹt" số ảo nữa.
                if (currentRSSI > bestRSSI || mac_id == detectedMajor)
                {
                    bestRSSI = currentRSSI;
                    detectedMajor = mac_id;
                    lastBeaconTime = millis();
                }
            }
        }
    }
};

void scanAndProcessBLE()
{
    // Giảm thời gian reset sóng từ 3000ms xuống 1500ms để nhạy hơn khi rời đi
    if (millis() - lastBeaconTime > 1500)
    {
        detectedMajor = 0;
        bestRSSI = -100;
    }
}

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
