#include <math.h>
#include <iostream>

double RSSItoDistance(int RSSI)
{
    const float rssi1M = -85, n = 0.47712;
    return pow(10.0, (rssi1M - RSSI) / 10 * n);
}

uint8_t mapFloatToInteger(float value)
{
    return (127*value) / 10;
}

float mapIntegerToFloat(int value) {
    // Scale the integer value to the range 0-1
    return (float)(10 * value) / 127;
}
int main()
{
    int RSSId0 = -85, RSSId1 = -88;
    
    float x = RSSItoDistance(RSSId1);
    uint8_t y = mapFloatToInteger(x);
    float z = mapIntegerToFloat((int) y);
    std::cout << x << " - " << (int)y << " - " << z << std::endl;

    return 0;
}