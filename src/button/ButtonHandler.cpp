#include "ButtonHandler.h"
#include <Arduino.h>
#include "Globals.h"

// Lấy 2 biến này từ file main.cpp qua để xài ké
extern int targetLocID;
extern bool isNavigating;

void xu_ly_nut_bam()
{
    static bool lastNextState = HIGH;
    static unsigned long nextPressedTime = 0;
    static bool nextLongPressed = false;

    bool currentNextState = digitalRead(PIN_BTN_NEXT);

    if (lastNextState == HIGH && currentNextState == LOW)
    {
        delay(20);
        if (digitalRead(PIN_BTN_NEXT) == LOW)
        {
            nextPressedTime = millis();
            nextLongPressed = false;
        }
    }
    else if (lastNextState == LOW && currentNextState == LOW)
    {
        if (!nextLongPressed && (millis() - nextPressedTime > 800))
        {
            nextLongPressed = true;
            isNavigating = true;
        }
    }
    else if (lastNextState == LOW && currentNextState == HIGH)
    {
        if (!nextLongPressed && (millis() - nextPressedTime > 20))
        {
            targetLocID++;
            if (targetLocID > 4)
                targetLocID = 1;
            isNavigating = false;
        }
    }
    lastNextState = currentNextState;

    static bool lastPrevState = HIGH;
    bool currentPrevState = digitalRead(PIN_BTN_PREV);
    if (lastPrevState == HIGH && currentPrevState == LOW)
    {
        delay(20);
        if (digitalRead(PIN_BTN_PREV) == LOW)
        {
            targetLocID--;
            if (targetLocID < 1)
                targetLocID = 4;
            isNavigating = false;
        }
    }
    lastPrevState = currentPrevState;
}