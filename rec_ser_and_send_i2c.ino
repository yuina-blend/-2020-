#include <Wire.h>

/* I2C系の関数 */
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

/* エアー関係の関数 */
void up_close()
{
    send(0x38, 0b0100);
}

void up_open()
{
    send(0x38, 0b0110);
}

void down_close()
{
    send(0x38, 0b0000);
}

void down_open()
{
    send(0x38, 0b0010);
}


/* main */
void setup()
{
    Wire.begin();
    Serial.begin(9600);
}

char hoge;
bool down_now=true, close_now=true;


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
        else if (hoge == 'L')
        {
            send(0x40, 0x60);
        }


        if (hoge == 'U')
        {
            if (down_now&&close_now)
            {
                up_close();
            }
            else if (down_now&&!close_now)
            {
                up_open();
            }
            else if (!down_now&&close_now)
            {
                down_close();
            }
            else
            {
                down_open();
            }
            down_now = !down_now;
            delay(500);
        }
        if (hoge == 'O')
        {
            if (down_now&&close_now)
            {
                down_open();
            }
            else if (down_now&&!close_now)
            {
                down_close();
            }
            else if (!down_now&&close_now)
            {
                up_open();
            }
            else
            {
                up_close();
            }
            close_now = !close_now;
            delay(500);
        }
    }
}
