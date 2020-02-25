#define sikaku 16  //00010000
#define sankaku 36 //00100100
#define batu 37    //00100101
#define maru 38    //00100110
#define ue 32      //00100000
#define sita 33    //00100001
#define migi 34    //00100010
#define hidari 35  //00100011
#define L1 17      //00010001
#define L2 18      //00010010
#define R1 19      //00010011
#define R2 20      //00010100

char PS3Data[8];

bool getButtonState(int button);
bool getSELECTState();
bool getSTARTState();
int getRightJoystickXaxis();
int getRightJoystickYaxis();
int getLeftJoystickXaxis();
int getLeftJoystickYaxis();

void setup()
{
    Serial.begin(115200);
    PS3Data[0] = 128;
    PS3Data[1] = 0;
    PS3Data[2] = 0;
    PS3Data[3] = 64;
    PS3Data[4] = 64;
    PS3Data[5] = 64;
    PS3Data[6] = 64;
    PS3Data[7] = 0;
}

void loop()
{
    getdata();
    Serial.print(getLeftJoystickYaxis());
    Serial.print("\n");
}

void getdata()  //受信割り込みで呼び出し
{
    if (Serial.available())
    {
        while (Serial.read() != 128)
        {
        }
        for (int i = 1; i < 8; i++)
        {
            *(PS3Data + i) = Serial.read();
        }
    }
}

bool getButtonState(int button)
{
    return (*(PS3Data + (button >> 4)) >> (button & 0x0f)) & 1;
}

bool getSELECTState()
{
    return getButtonState(migi) & getButtonState(hidari);
}

bool getSTARTState()
{
    return getButtonState(ue) & getButtonState(sita);
}

int getRightJoystickXaxis()
{
    return (int)PS3Data[5] - 64;
}

int getRightJoystickYaxis()
{
    return (int)PS3Data[6] * -1 + 64;
}

int getLeftJoystickXaxis()
{
    return (int)PS3Data[3] - 64;
}

int getLeftJoystickYaxis()
{
    return (int)PS3Data[4] * -1 + 64;
}