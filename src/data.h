#include <Arduino.h>

bool compareArrays(const uint8_t *arr1, const uint8_t *arr2, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr1[i] != arr2[i])
        {
            return false;
        }
    }
    return true;
}
void copyArray(const uint8_t *source, uint8_t *destination, int size)
{
    for (int i = 0; i < size; i++)
    {
        destination[i] = source[i];
    }
}

uint8_t mapFloatToInteger(float value)
{
    return (127 * value) / 10;
}

float mapIntegerToFloat(int value)
{
    // Scale the integer value to the range 0-1
    return (float)(10 * value) / 127;
}

class bleData
{
public:
    uint8_t macAddress[6][6];
    uint8_t data[6][3];
    // data Structure :
    //  [0] = Packet Status; || 0=>Test Packet; 1=>Distances Pakcet; 2=>Change Gateway permission ; 3=> Missing Packet;
    //  [1] = Distances only for packetStatus = 1(Distances Packet)
    int size;

    void dataProcessInput(const uint8_t *macAddr, const uint8_t *data, int dataLen) // used in callback when receive data from another node
    {
        bool newNode = true;
        for (int i = 1; i < this->size; i++)
        {
            if (compareArrays(macAddr, this->macAddress[i], 6))
            {
                newNode = false;
                this->data[i][0] = data[0];
                this->data[i][1] = data[1];
                printData(i);
                break;
            }
        }
        if (newNode)
        {
            if (this->size == 6)
            {
                Serial.println("Node Storage Full");
                return;
            }
            else
            {
                copyArray(data, this->data[this->size], dataLen);
                copyArray(macAddr, this->macAddress[this->size], 6);
                printData(this->size);
                this->size += 1;
            }
        }
    }

    void dataProcessInputforThisNode(float distances)
    {
        this->data[0][1] = mapFloatToInteger(distances);
    }

    int findNearest(){
        // if(this->data[0][1] == 0) return 0;
        int nearest[2] = {0, this->data[0][1]};
        for(int i=1;i<this->size;i++){
            if(this->data[i][1] < nearest[1]) {
                nearest[0] = i;
                nearest[1] = this->data[i][1];
            } 
        }
        return nearest[0];
    }

    void printData(int indx)
    {
        Serial.println("\nDataBaseData");
        Serial.println(this->data[indx][0]);
        Serial.println(this->data[indx][1]);
        Serial.print("Distances: ");
        Serial.println(mapIntegerToFloat((int)this->data[indx][1]));
    }
};

bleData BLEData;