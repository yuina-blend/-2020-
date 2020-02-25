#include <Wire.h>

void send(char, unsigned char);

void setup()
{
    Wire.begin();
}

void loop()
{
    for (char address = 0x10; address <= 0x40; address += 0x10)
    {
        send(address, 0xff);
    }
}

void send(char md_address, unsigned char send_data)
{
    delay(10);
    Wire.beginTransmission(address);
    Wire.write(0xff);
    Wire.endTransmission();
}
