
/*
Copyright (c) 2010 Peter Barrett

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mbed.h"
#include "Utils.h"
#include "USBHost.h"
#include "hci.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
DigitalOut em_pin(p21);

//Serial mypc(USBTX, USBRX);
Serial wii(p9, p10);

char send_data;

void printf(const BD_ADDR *addr)
{
    const u8 *a = addr->addr;
    printf("%02X:%02X:%02X:%02X:%02X:%02X", a[5], a[4], a[3], a[2], a[1], a[0]);
}

#define MAX_HCL_SIZE 260
#define MAX_ACL_SIZE 400

class HCITransportUSB : public HCITransport
{
    int _device;
    u8 *_hciBuffer;
    u8 *_aclBuffer;

public:
    void Open(int device, u8 *hciBuffer, u8 *aclBuffer)
    {
        _device = device;
        _hciBuffer = hciBuffer;
        _aclBuffer = aclBuffer;
        USBInterruptTransfer(_device, 0x81, _hciBuffer, MAX_HCL_SIZE, HciCallback, this);
        USBBulkTransfer(_device, 0x82, _aclBuffer, MAX_ACL_SIZE, AclCallback, this);
    }

    static void HciCallback(int device, int endpoint, int status, u8 *data, int len, void *userData)
    {
        HCI *t = ((HCITransportUSB *)userData)->_target;
        if (t)
            t->HCIRecv(data, len);
        USBInterruptTransfer(device, 0x81, data, MAX_HCL_SIZE, HciCallback, userData);
    }

    static void AclCallback(int device, int endpoint, int status, u8 *data, int len, void *userData)
    {
        HCI *t = ((HCITransportUSB *)userData)->_target;
        if (t)
            t->ACLRecv(data, len);
        USBBulkTransfer(device, 0x82, data, MAX_ACL_SIZE, AclCallback, userData);
    }

    virtual void HCISend(const u8 *data, int len)
    {
        USBControlTransfer(_device, REQUEST_TYPE_CLASS, 0, 0, 0, (u8 *)data, len);
    }

    virtual void ACLSend(const u8 *data, int len)
    {
        USBBulkTransfer(_device, 0x02, (u8 *)data, len);
    }
};

#define WII_REMOTE 0x042500

class HIDBluetooth
{
    int _control; // Sockets for control (out) and interrupt (in)
    int _interrupt;
    int _devClass;
    BD_ADDR _addr;
    u8 _pad[2]; // Struct align

public:
    HIDBluetooth() : _control(0), _interrupt(0), _devClass(0){};

    bool InUse()
    {
        return _control != 0;
    }

    static void OnHidInterrupt(int socket, SocketState state, const u8 *data, int len, void *userData)
    {
        HIDBluetooth *t = (HIDBluetooth *)userData;
        if (data)
        {
            if (t->_devClass == WII_REMOTE && data[1] == 0x30)
            {
                printf("================wii====================\n");
                t->Led();
                t->Hid(0x30); // ask for accelerometer
                t->_devClass = 0;
            }

            const u8 *d = data;
            switch (d[1])
            {
            case 0x02:
            {
                int x = (signed char)d[3];
                int y = (signed char)d[4];
                printf("Mouse %2X dx:%d dy:%d\n", d[2], x, y);
            }
            break;
            case 0x30:
            {
                int pad = (d[2] & 0x9F) | ((d[3] & 0x9F) << 8);
                printf("Wii %4X\n", pad);
                int button_up = (pad & 2);    //反時計回り L
                int button_down = (pad & 1);  //時計回り R
                int button_right = (pad & 4); //up U
                int button_left = (pad & 8);  //open O
                int button_plus = (pad & 16);
                int button_2 = (pad & 256); //後進 B
                int button_1 = (pad & 512); //前進 F
                int button_B = (pad & 1024);
                int button_A = (pad & 2048); //投射 S
                int button_minus = (pad & 4096);
                int button_home = (pad & 32768); //緊急停止 E

                if (button_1 && button_B)
                {
                    send_data = 'f';
                    wii.putc(send_data);
                }
                else if (button_2 && button_B)
                {
                    send_data = 'b';
                    wii.putc(send_data);
                }
                else if (button_home)
                {
                    em_pin = !em_pin;
                    wait_ms(500);
                }
                else if (button_1)
                {
                    send_data = 'F';
                    wii.putc(send_data);
                }
                else if (button_2)
                {
                    send_data = 'B';
                    wii.putc(send_data);
                }
                else if (button_left)
                {
                    send_data = 'L';
                    wii.putc(send_data);
                }
                else if (button_right)
                {
                    send_data = 'R';
                    wii.putc(send_data);
                }
                else if (button_up)
                {
                    send_data = 'U';
                    wii.putc(send_data);
                }
                else if (button_down)
                {
                    send_data = 'O';
                    wii.putc(send_data);
                }
                else
                {
                    send_data = 'N';
                    wii.putc(send_data);
                }
            }
            break;

            case 0x37: // Accelerometer http://wiki.wiimoteproject.com/Reports
            {
                int pad = (d[2] & 0x9F) | ((d[3] & 0x9F) << 8);
                int x = (d[2] & 0x60) >> 5 | d[4] << 2;
                int y = (d[3] & 0x20) >> 4 | d[5] << 2;
                int z = (d[3] & 0x40) >> 5 | d[6] << 2;
                printf("WII %04X %d %d %d\n", pad, x, y, z);
            }
            break;
            default:
                printHex(data, len);
            }
        }
    }

    static void OnHidControl(int socket, SocketState state, const u8 *data, int len, void *userData)
    {
        printf("OnHidControl\n");
        if (data)
            printHex(data, len);
    }

    void Open(BD_ADDR *bdAddr, inquiry_info *info)
    {
        printf("L2CAPAddr size %d\n", sizeof(L2CAPAddr));
        _addr = *bdAddr;
        L2CAPAddr sockAddr;
        sockAddr.bdaddr = _addr;
        sockAddr.psm = L2CAP_PSM_HID_INTR;
        printf("Socket_Open size %d\n", sizeof(L2CAPAddr));
        _interrupt = Socket_Open(SOCKET_L2CAP, &sockAddr.hdr, OnHidInterrupt, this);
        sockAddr.psm = L2CAP_PSM_HID_CNTL;
        _control = Socket_Open(SOCKET_L2CAP, &sockAddr.hdr, OnHidControl, this);

        printfBytes("OPEN DEVICE CLASS", info->dev_class, 3);
        _devClass = (info->dev_class[0] << 16) | (info->dev_class[1] << 8) | info->dev_class[2];
    }

    void Close()
    {
        if (_control)
            Socket_Close(_control);
        if (_interrupt)
            Socket_Close(_interrupt);
        _control = _interrupt = 0;
    }

    void Led(int id = 0x10)
    {
        u8 led[3] = {0x52, 0x11, id};
        if (_control)
            Socket_Send(_control, led, 3);
    }

    void Hid(int report = 0x37)
    {
        u8 hid[4] = {0x52, 0x12, 0x00, report};
        if (_control != -1)
            Socket_Send(_control, hid, 4);
    }
};

HCI *gHCI = 0;

#define MAX_HID_DEVICES 8

int GetConsoleChar();
class ShellApp
{
    char _line[64];
    HIDBluetooth _hids[MAX_HID_DEVICES];

public:
    void Ready()
    {
        printf("HIDBluetooth %d\n", sizeof(HIDBluetooth));
        memset(_hids, 0, sizeof(_hids));
        Inquiry();
    }

    //  We have connected to a device
    void ConnectionComplete(HCI *hci, connection_info *info)
    {
        printf("ConnectionComplete ");
        BD_ADDR *a = &info->bdaddr;
        printf(a);
        BTDevice *bt = hci->Find(a);
        HIDBluetooth *hid = NewHIDBluetooth();
        printf("%08x %08x\n", bt, hid);
        if (hid)
            hid->Open(a, &bt->_info);
    }

    HIDBluetooth *NewHIDBluetooth()
    {
        for (int i = 0; i < MAX_HID_DEVICES; i++)
            if (!_hids[i].InUse())
                return _hids + i;
        return 0;
    }

    void ConnectDevices()
    {
        BTDevice *devs[8];
        int count = gHCI->GetDevices(devs, 8);
        for (int i = 0; i < count; i++)
        {
            printfBytes("DEVICE CLASS", devs[i]->_info.dev_class, 3);
            if (devs[i]->_handle == 0)
            {
                BD_ADDR *bd = &devs[i]->_info.bdaddr;
                printf("Connecting to ");
                printf(bd);
                printf("\n");
                gHCI->CreateConnection(bd);
            }
        }
    }

    const char *ReadLine()
    {
        int i;
        for (i = 0; i < 255;)
        {
            USBLoop();
            int c = GetConsoleChar();
            if (c == -1)
                continue;
            if (c == '\n' || c == 13)
                break;
            _line[i++] = c;
        }
        _line[i] = 0;
        return _line;
    }

    void Inquiry()
    {
        printf("Inquiry..\n");
        gHCI->Inquiry();
    }

    void List()
    {
#if 0
        printf("%d devices\n",_deviceCount);
        for (int i = 0; i < _deviceCount; i++)
        {
            printf(&_devices[i].info.bdaddr);
            printf("\n");
        }
#endif
    }

    void Connect()
    {
        ConnectDevices();
    }

    void Disconnect()
    {
        gHCI->DisconnectAll();
    }

    void CloseMouse()
    {
    }

    void Quit()
    {
        CloseMouse();
    }

    void Run()
    {
        for (;;)
        {
            const char *cmd = ReadLine();
            if (strcmp(cmd, "scan") == 0 || strcmp(cmd, "inquiry") == 0)
                Inquiry();
            else if (strcmp(cmd, "ls") == 0)
                List();
            else if (strcmp(cmd, "connect") == 0)
                Connect();
            else if (strcmp(cmd, "disconnect") == 0)
                Disconnect();
            else if (strcmp(cmd, "q") == 0)
            {
                Quit();
                break;
            }
            else
            {
                printf("eh? %s\n", cmd);
            }
        }
    }
};

//  Instance
ShellApp gApp;

static int HciCallback(HCI *hci, HCI_CALLBACK_EVENT evt, const u8 *data, int len)
{
    switch (evt)
    {
    case CALLBACK_READY:
        printf("CALLBACK_READY\n");
        gApp.Ready();
        break;

    case CALLBACK_INQUIRY_RESULT:
        printf("CALLBACK_INQUIRY_RESULT ");
        printf((BD_ADDR *)data);
        printf("\n");
        break;

    case CALLBACK_INQUIRY_DONE:
        printf("CALLBACK_INQUIRY_DONE\n");
        gApp.ConnectDevices();
        break;

    case CALLBACK_REMOTE_NAME:
    {
        BD_ADDR *addr = (BD_ADDR *)data;
        const char *name = (const char *)(data + 6);
        printf(addr);
        printf(" % s\n", name);
    }
    break;

    case CALLBACK_CONNECTION_COMPLETE:
        gApp.ConnectionComplete(hci, (connection_info *)data);
        break;
    };
    return 0;
}

//  these should be placed in the DMA SRAM
typedef struct
{
    u8 _hciBuffer[MAX_HCL_SIZE];
    u8 _aclBuffer[MAX_ACL_SIZE];
} SRAMPlacement;

HCITransportUSB _HCITransportUSB;
HCI _HCI;

u8 *USBGetBuffer(u32 *len);
int OnBluetoothInsert(int device)
{
    printf("Bluetooth inserted of %d\n", device);
    u32 sramLen;
    u8 *sram = USBGetBuffer(&sramLen);
    sram = (u8 *)(((u32)sram + 1023) & ~1023);
    SRAMPlacement *s = (SRAMPlacement *)sram;
    _HCITransportUSB.Open(device, s->_hciBuffer, s->_aclBuffer);
    _HCI.Open(&_HCITransportUSB, HciCallback);
    RegisterSocketHandler(SOCKET_L2CAP, &_HCI);
    gHCI = &_HCI;
    gApp.Inquiry();
    return 0;
}

void TestShell()
{
    USBInit();
    gApp.Run();
}
