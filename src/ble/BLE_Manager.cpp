#include "ble/BLE_Manager.h" // Sửa lại đường dẫn nếu cần cho khớp với cấu trúc thư mục của ông
#include "ui/UI_Manager.h"
#include "ui/ui.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Các biến toàn cục dùng chung hệ thống
BLEScan *pBLEScan;
int rssiThreshold = -65; // Ngưỡng sóng để kích hoạt check-in
int detectedMajor = 0;
int detectedMinor = 0;
bool isBeaconFound = false;

// Địa chỉ MAC của các cục trạm phát (Đã đầy đủ 3 trạm)
String mac_tram_1 = "2b:80:3d:06:48:b1";
String mac_tram_2 = "2a:07:98:03:bf:a1";
String mac_tram_3 = "2a:07:98:03:52:59";

int targetLocID = 0;

// ========================================================
// HÀM CLICK CHỌN ĐIỂM ĐẾN TRÊN MÀN HÌNH
// ========================================================
void destination_click_cb(lv_event_t *e)
{
    int selectedID = (int)(uintptr_t)lv_event_get_user_data(e);
    targetLocID = selectedID;

    Serial.print("🎯 Nguoi dung chon diem den: ");
    Serial.println(targetLocID);
}

// ========================================================
// BỘ LỌC QUÉT SÓNG BLE BẰNG ĐIA CHỈ MAC (ĐÃ THÊM TRẠM 2 & 3)
// ========================================================
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // Ép kiểu địa chỉ MAC quét được về String chữ thường
        String currentMAC = advertisedDevice.getAddress().toString().c_str();
        int currentRSSI = advertisedDevice.getRSSI();

        // Kiểm tra sóng phải đủ khỏe trước khi check MAC để tránh quét nhầm sóng ảo
        if (currentRSSI >= rssiThreshold)
        {
            // Check Trạm 1
            if (currentMAC == mac_tram_1)
            {
                isBeaconFound = true;
                detectedMajor = 1; // Gán ID Trạm là 1
                detectedMinor = 0;
            }
            // Check Trạm 2
            else if (currentMAC == mac_tram_2)
            {
                isBeaconFound = true;
                detectedMajor = 2; // Gán ID Trạm là 2
                detectedMinor = 0;
            }
            // Check Trạm 3
            else if (currentMAC == mac_tram_3)
            {
                isBeaconFound = true;
                detectedMajor = 3; // Gán ID Trạm là 3
                detectedMinor = 0;
            }
        }
    }
};

// ========================================================
// KHỞI TẠO CẤU HÌNH BLE
// ========================================================
void setupBLE()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

// ========================================================
// HÀM XỬ LÝ QUÉT VÀ HIỂN THỊ CHẤM TRÒN BẢN ĐỒ
// ========================================================
void scanAndProcessBLE()
{
    isBeaconFound = false;
    detectedMajor = 0; // Reset lại biến sau mỗi chu kỳ quét

    pBLEScan->start(1, false); // Tiến hành quét sóng trong 1 giây
    pBLEScan->clearResults();  // Xóa bộ nhớ đệm để giải phóng RAM cho ESP32

    // Reset toàn bộ chấm trên bản đồ về màu xám mặc định
    resetMapDotsToGray();

    if (isBeaconFound)
    {
        int locID = 0;

        // KIỂM TRA BẢN ĐỒ SỐ 1
        if (currentMapIndex == 1)
        {
            if (detectedMajor == 1)
                locID = 1;
            else if (detectedMajor == 2)
                locID = 2;
            else if (detectedMajor == 3)
                locID = 3;
            else if (detectedMajor == 4)
                locID = 4;
            else if (detectedMajor == 5)
                locID = 5;
            else if (detectedMajor == 6)
                locID = 6;
        }
        // KIỂM TRA BẢN ĐỒ SỐ 2
        else if (currentMapIndex == 2)
        {
            if (detectedMajor == 7)
                locID = 7;
            else if (detectedMajor == 8)
                locID = 8;
            else if (detectedMajor == 9)
                locID = 9;
            else if (detectedMajor == 10)
                locID = 10;
            else if (detectedMajor == 11)
                locID = 11;
        }

        // Nếu xác định được tọa độ trạm hợp lệ, kích hoạt Check-in và sáng đèn màn hình
        if (locID > 0)
        {
            checkInHistory(locID);
            turnOnMapDot(locID);
        }
    }
}