#include "mbed.h"
#include "PS3.h"
#include "QEI.h"

/*
交ロボ
緊急停止信号　D13
i2c SDA D14
    SDL D15
ps3 RX D2
    TX D8
交ロボ２
ロリコン1（A層:PC_6,B層:PC_5）
ロリコン2（A層:PA_12,B層:PA_11)
ロリコン3（A層:PB_1,B層:PB_15)
ロリコン4（A層:PB_14,B層:PB_13)

address
    0x10: 足回り1
    0x20: 足回り2
    0x30: 足回り3
    0x40: 足回り4
    0x70: エアー
*/

DigitalOut Emergency_stop(D13); //5Vについてる非常停止ピン用
I2C i2c(D14, D15);
PS3 ps3(D8, D2);
Serial pc(USBTX, USBRX);

QEI rori_1(PC_6, PC_5, NC, 600, QEI::X4_ENCODING); //600じゃね？
QEI rori_2(PA_12, PA_11, NC, 600, QEI::X4_ENCODING);
QEI rori_3(PB_1, PB_15, NC, 600, QEI::X4_ENCODING);
QEI rori_angle(PB_14, PB_13, NC, 600, QEI::X4_ENCODING);

void get_rori_pulses(int *);
bool get_rori_difference(int *, int *);
void set_duty(unsigned char *, unsigned char, int, bool, int);
void Emergency_check();
void send(char, unsigned char);
void forward(unsigned char *);
void back(unsigned char *);
void stop();
void angle_left(unsigned char);
void angle_right(unsigned char);

int main()
{
    int controller_axis;
    int rori_pulses[4];
    int rori_difference;
    bool flag;
    unsigned char send_datas[3];
    Emergency_stop = 0;
    while (true)
    {
        controller_axis = ps3.getLeftJoystickYaxis();
        get_rori_pulses(rori_pulses);
        flag = get_rori_difference(rori_pulses, &rori_difference);
        Emergency_check();
        for (int i = 0; i < 3; i++)
        {
            pc.printf("\t%d",rori_pulses[i]);
        }
        pc.printf("\n");
        //足回り
        if (controller_axis >= 20)
        {
            set_duty(send_datas, 200, rori_difference, flag, 75);
            forward(send_datas);
        }
        else if (controller_axis <= -20)
        {
            // back();
        }
        else
        {
            stop();
        }
        //角度調整
        if (ps3.getButtonState(maru))
        {
            angle_left(0x70);
        }
        else if (ps3.getButtonState(sikaku))
        {
            angle_right(0x92);
        }
        else
        {
            send(0x40, 0x80);
        }
    }
}

void get_rori_pulses(int *rori)
{
    rori[0] = rori_1.getPulses();
    rori[1] = rori_2.getPulses();
    rori[2] = rori_3.getPulses();
    rori[3] = rori_angle.getPulses();
}

bool get_rori_difference(int *rori_pulses, int *difference)
{
    *difference = rori_pulses[0] - rori_pulses[1];
    if (*difference > 0)
    {
        return true;
    }
    else if (*difference < 0)
    {
        return false;
    }
}

void set_duty(unsigned char *datas, unsigned char target_value, int difference, bool mode, int pulse)
{
    if (mode)
    {
        for (int i = 0; i < 3; i++)
        {
            if (i == 1)
            {
                datas[i] = target_value + difference / pulse;
            }
            else
            {
                datas[i] = target_value;
            }
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                datas[i] = target_value + difference / pulse;
            }
            else
            {
                datas[i] = target_value;
            }
        }
    }
}

void Emergency_check()
{
    if (ps3.getSELECTState())
    {
        Emergency_stop = 1;
        wait(1.0);
        while (true)
        {
            if (ps3.getSTARTState())
            {
                Emergency_stop = 0;
                break;
            }
        }
    }
}

void send(char md_address, unsigned char send_data)
{
    wait_ms(10);
    i2c.start();
    i2c.write(md_address);
    i2c.write(send_data); //0x00(最大) ~ 0x7C(最小):逆転,  0x7D ~ 0x83:ショートブレーキ ,0x84(最小) ~ 0xFF(最大):正転
    i2c.stop();
}

void forward(unsigned char *datas)
{
    for (char address = 0x10; address <= 0x30; address += 0x10)
    {
        send(address, datas[address % 0x10 - 1]);
    }
}

void back(unsigned char *datas)
{
    for (char address = 0x10; address <= 0x30; address += 0x10)
    {
        send(address, datas[address % 0x10 - 1]);
    }
}

void stop()
{
    for (char address = 0x10; address <= 0x30; address += 0x10)
    {
        send(address, 0x80);
    }
}

void angle_left(unsigned char data)
{
    send(0x40, data);
}

void angle_right(unsigned char data)
{
    send(0x40, data);
}
