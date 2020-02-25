#include "Arduino.h"
#include "Arduino_PS3.h"

PS3::PS3()
{
    PS3Data[0] = 128;
    PS3Data[1] = 0;
    PS3Data[2] = 0;
    PS3Data[3] = 64;
    PS3Data[4] = 64;
    PS3Data[5] = 64;
    PS3Data[6] = 64;
    PS3Data[7] = 0;
    addflag = 0;
    baud(115200);
    Serial::attach(this, &PS3::getdata, Serial::RxIrq);
}
void PS3::getdata()
{
    if (Serial::readable())
    {
        while (Serial::getc() != 128)
        {
        }
        for (int i = 1; i < 8; i++)
        {
            *(PS3Data + i) = Serial::getc();
        }
    }
    if (addflag)
        (*fpFunc)();
}
void PS3::myattach()
{
    Serial::attach(this, &PS3::getdata, Serial::RxIrq);
}

void PS3::addattach(void (*Func)())
{
    fpFunc = Func;
    addflag = 1;
}

void PS3::nothingFunc()
{
}

bool PS3::getButtonState(int button)
{
    return (*(PS3Data + (button >> 4)) >> (button & 0x0f)) & 1;
}
bool PS3::getSELECTState()
{
    return getButtonState(migi) & getButtonState(hidari);
}
bool PS3::getSTARTState()
{
    return getButtonState(ue) & getButtonState(sita);
}
int PS3::getRightJoystickXaxis()
{
    return (int)PS3Data[5] - 64;
}
int PS3::getRightJoystickYaxis()
{
    return (int)PS3Data[6] * -1 + 64;
}
int PS3::getLeftJoystickXaxis()
{
    return (int)PS3Data[3] - 64;
}
int PS3::getLeftJoystickYaxis()
{
    return (int)PS3Data[4] * -1 + 64;
}
double PS3::getRightJoystickAngle()
{
    return atan2(double(PS3Data[6] * -1 + 64), double(PS3Data[5] - 64)) * double(180 / PI);
}
double PS3::getLeftJoystickAngle()
{
    return atan2(double(PS3Data[4] * -1 + 64), double(PS3Data[3] - 64)) * double(180 / PI);
}
void PS3::printdata()
{
    Serial pc(USBTX, USBRX);
    for (int i = 0; i < 8; i++)
    {
        pc.printf("%4d", PS3Data[i]);
    }
}
