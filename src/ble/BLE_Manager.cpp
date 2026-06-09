#include "BLE_Manager.h"
#include "ui/UI_Manager.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan *pBLEScan;
int rssiThreshold = -65;
int detectedMajor = 0;
int detectedMinor = 0;
bool isBeaconFound = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.haveManufacturerData())
        {
            std::string strManufacturerData = advertisedDevice.getManufacturerData();
            uint8_t *cManufacturerData = (uint8_t *)strManufacturerData.data();

            if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)
            {
                int major = (cManufacturerData[20] << 8) + cManufacturerData[21];
                int minor = (cManufacturerData[22] << 8) + cManufacturerData[23];
                int currentRSSI = advertisedDevice.getRSSI();

                if (currentRSSI >= rssiThreshold)
                {
                    isBeaconFound = true;
                    detectedMajor = major;
                    detectedMinor = minor;
                }
            }
        }
    }
};

void setupBLE()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void scanAndProcessBLE()
{
    isBeaconFound = false;
    pBLEScan->start(1, false); // Quét 1 giây
    pBLEScan->clearResults();

    resetMapDotsToGray();

    if (isBeaconFound)
    {
        int locID = 0;
        if (currentMapIndex == 1)
        {
            if (detectedMajor == 1 && detectedMinor == 1)
                locID = 1;
            else if (detectedMajor == 2 && detectedMinor == 2)
                locID = 2;
            else if (detectedMajor == 3 && detectedMinor == 3)
                locID = 3;
            else if (detectedMajor == 4 && detectedMinor == 4)
                locID = 4;
            else if (detectedMajor == 5 && detectedMinor == 5)
                locID = 5;
            else if (detectedMajor == 6 && detectedMinor == 6)
                locID = 6;
        }
        else if (currentMapIndex == 2)
        {
            if (detectedMajor == 7 && detectedMinor == 7)
                locID = 7;
            else if (detectedMajor == 8 && detectedMinor == 8)
                locID = 8;
            else if (detectedMajor == 9 && detectedMinor == 9)
                locID = 9;
            else if (detectedMajor == 10 && detectedMinor == 10)
                locID = 10;
            else if (detectedMajor == 11 && detectedMinor == 11)
                locID = 11;
        }

        if (locID > 0)
        {
            checkInHistory(locID);
            turnOnMapDot(locID);
        }
    }
}