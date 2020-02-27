#include <Wire.h>

void send(char, unsigned char);

void setup()
{
    Wire.begin();
}

void loop()
{
    for (char address = 0x20; address <= 0x80; address += 0x20)
    {
        send(address, 0xff);
    }
}

void send(char md_address, unsigned char send_data)
{
    delay(10);
    Wire.beginTransmission(md_address);
    Wire.write(0xff);
    Wire.endTransmission();
}
