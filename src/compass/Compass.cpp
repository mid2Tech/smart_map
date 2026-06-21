#include "Compass.h"
#include <Wire.h>
#include <math.h>

#define QMC5883P_ADDR 0x2C

void initHardwareQMC5883L()
{
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x29);
    Wire.write(0x06);
    Wire.endTransmission();
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x0B);
    Wire.write(0x08);
    Wire.endTransmission();
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x0A);
    Wire.write(0xCD);
    Wire.endTransmission();
}

int getCompassHeading()
{
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x09);
    Wire.endTransmission(false);
    Wire.requestFrom(QMC5883P_ADDR, 1);
    if (Wire.available())
    {
        if (Wire.read() & 0x01)
        {
            Wire.beginTransmission(QMC5883P_ADDR);
            Wire.write(0x01);
            Wire.endTransmission(false);
            Wire.requestFrom(QMC5883P_ADDR, 6);
            if (Wire.available() >= 6)
            {
                int16_t raw_x = Wire.read() | (Wire.read() << 8);
                int16_t raw_y = Wire.read() | (Wire.read() << 8);
                int16_t raw_z = Wire.read() | (Wire.read() << 8);
                if (raw_x == 0 && raw_y == 0)
                    return -1;
                float goc_rad = atan2(raw_y, raw_x);
                float goc_deg = goc_rad * 180.0 / M_PI;
                if (goc_deg < 0)
                    goc_deg += 360.0;
                return (int)goc_deg;
            }
        }
    }
    return -1;
}