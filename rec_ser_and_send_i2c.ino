#include <Wire.h>

void send(char md_address, unsigned char send_data)
{
    delay(10);
    Wire.beginTransmission(md_address);
    Wire.write(send_data);
    Wire.endTransmission();
}

void forward(bool slow)
{
    for (char address = 0x10; address <= 0x30; address += 0x10)
    {
        send(address, slow ? 0x90 : 0xC0);
    }
}

void back(bool slow)
{
    for (char address = 0x10; address <= 0x30; address += 0x10)
    {
        send(address, slow ? 0x70 : 0x30);
    }
}

void stop()
{
    for (char address = 0x10; address <= 0x30; address += 0x10)
    {
        send(address, 0x80);
    }
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
        Serial.println(hoge);
        if (hoge == 'F')
        {
            forward(true);
        }
        else if (hoge == 'f')
        {
            forward(false);
        }
        else if (hoge == 'B')
        {
            back(true);
        }
        else if (hoge == 'b')
        {
            back(false);
        }
        else if (hoge == 'N')
        {
            stop();
            send(0x40, 0x80);
        }

       /*以下は移動と関係なし*/
        if (hoge == 'R')
        {
            send(0x40, 0xA0);
        }
        if (hoge == 'L')
        {
            send(0x40, 0x60);
        }
    }
}
