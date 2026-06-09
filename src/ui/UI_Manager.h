#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "Globals.h"

void switchMapAndText(int index);
void update_battery_display(int percent);
void resetMapDotsToGray();
void initUiToGray();
void checkInHistory(int locationID);
void turnOnMapDot(int locationID);

#endif