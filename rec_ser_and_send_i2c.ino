#include <Wire.h>

void send(char md_address, unsigned char send_data)
{
    delay(10);
    Wire.beginTransmission(md_address);
    Wire.write(send_data);
    Wire.endTransmission();
}

void setup()
{
    Wire.begin();
    Serial.begin(9600);
}

char hoge;
void loop()
{
    if (Serial.available())
    {
        hoge = Serial.read();
        if (hoge == 'F')
        {
            Serial.print("F");
            for (char address=0x10; address<=0x40; address += 0x10)
            {
                send(address, 0x90);
            }
        }
        if (hoge == 'B')
        {
            Serial.print("B");
            for (char address=0x10; address<=0x40; address += 0x10)
            {
                send(address, 0x70);
            }
        }
        if (hoge == 'N')
        {
            Serial.print("N");
            for (char address=0x10; address<=0x40; address+=0x10)
            {
                send(address, 0x80);
            }
        }
    }
}
