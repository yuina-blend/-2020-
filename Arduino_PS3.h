#ifndef ARDUINO_PS3_H
#define ARDUINO_PS3_H
//bit,button
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

#define PI 3.141592654

class PS3 // : public Serial
{
public:
    PS3(); //(PinName TX, PinName RX); /* : Serial(TX,RX)
    void getdata();
    /*{
        if(Serial::readable())
        {
            for(int i = 0;i < 8;i++)
            *(PS3Data+i) = Serial::getc();
        }
    }*/
    void myattach();
    void addattach(void (*Func)());
    void nothingFunc();
    bool getButtonState(int button);
    bool getSELECTState();
    bool getSTARTState();
    int getRightJoystickXaxis();
    int getRightJoystickYaxis();
    int getLeftJoystickXaxis();
    int getLeftJoystickYaxis();
    /*{
        return (*(PS3Data+(button>>4)) >> (button & 0x0f)) & 1;
    }*/
    //double getRightJoystickAngle();
    /*{
        return atan2(double(PS3Data[6]*-1+64), double(PS3Data[5]-64))*double(180/PI);
    }*/
    //double getLeftJoystickAngle();
    /*{
        return atan2(double(PS3Data[4]*-1+64), double(PS3Data[3]-64))*double(180/PI);
    }*/
    //void printdata();

private:
    char PS3Data[8];
    void (*fpFunc)();
    bool addflag;
};

#endif