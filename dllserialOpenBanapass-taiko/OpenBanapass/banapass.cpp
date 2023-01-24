#include "banapass.h"
#include "log.h"
#include "SerialInterface.h"
#include <filesystem>
#include <thread>
#include <chrono>
#include <string>
#include <cstdio>
#include "random.h"
constexpr auto BANA_API_VERSION = "Ver 1.6.1";

constexpr auto hexCharacterTable = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

bool readerActive = false;

SerialInterface serReader;
int serReaderStatus = 0;

using Random = effolkronium::random_static;

void randomHex(char str[], int length)
{
    // hexadecimal characters
    for (int i = 0; i < length; i++)
    {
        str[i] = Random::get(hexCharacterTable);
    }
    str[length] = 0;
}

void randomNumberString(char str[], int length)
{
    for (int i = 0; i < length; i++)
    {
        str[i] = Random::get('0', '9');
    }
    str[length] = 0;
}

std::string getProfileString(LPCSTR name, LPCSTR key, LPCSTR def, LPCSTR filename)
{
    char temp[1024];
    const DWORD result = GetPrivateProfileStringA(name, key, def, temp, sizeof(temp), filename);
    return std::string(temp, result);
}

void createCard()
{
    // use Serial CardReader, not Card.ini

    // if (std::filesystem::exists(".\\card.ini"))
    // {
    //     log("Card.ini found!\n");
    // }
    // else
    // {
    //     //std::string accessCode = "30764352518498791337";
    //     //std::string chipId = "7F5C9744F111111143262C3300040610";
    //     char generatedAccessCode[33] = "00000000000000000000000000000000";
    //     randomHex(generatedAccessCode, 32);
    //     WritePrivateProfileStringA("card", "accessCode", generatedAccessCode, ".\\card.ini");
    //     char generatedChipId[21] = "00000000000000000000";
    //     randomNumberString(generatedChipId, 20);
    //     WritePrivateProfileStringA("card", "chipId", generatedChipId, ".\\card.ini");
    //     log("New card generated\n");
    // }
}

void StartAttachThread(long (*callback)(long, long, long *), long *someStructPtr)
{
    // this is a really ugly hack, forgive me
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(50ms);
    // connect serial card reader
    if (!serReader.openSyn("COM1", CBR_9600, NOPARITY, 8, ONESTOPBIT))
    {
        log("serReader COM1 open fail!");
        serReaderStatus = 71;
        callback(7, 1, someStructPtr);
        return;
    }
    log("serReader opened COM1");
    serReader.setBufferSize(1024, 1024);
    if (!serReader.writeStr("BNRCONNINIT"))
    {
        log("serReader request BNRCONNINIT fail!");
        serReaderStatus = 72;
        callback(7, 1, someStructPtr);
        return;
    }
    log("serReader request BNRCONNINIT");

    std::this_thread::sleep_for(50ms);

    char readBuf[128];
    memset(readBuf, 0, sizeof(readBuf));
    DWORD readSize;
    serReader.readData(readBuf, readSize);
    readBuf[readSize-1] = '\0';

    if (strcmp(readBuf, "ROGER INITED OBPR") == 0)
    {
        log("serReader didn't receive INITOK message!");
        serReaderStatus = 73;
        callback(7, 1, someStructPtr);
        return;
    }
    // should be "ROGER INITED OBPR"

    log("serReader received INITOK message!");
    serReaderStatus = 0;
    callback(0, 0, someStructPtr);
    return;
}
void StartResetThread(long (*callback)(int, int, long *), long *someStructPtr)
{
    // this is a really ugly hack, forgive me
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    if (!serReader.isOpened())
    {
        callback(7,1, someStructPtr);
        return;
    }
    serReader.writeStr("RESETHW");
    callback(0, 0, someStructPtr);
}

void StartReqActionThread(void (*callback)(long, int, long *), long *someStructPtr)
{
    // this is a really ugly hack, forgive me
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    if (!serReader.isOpened())
    {
        callback(7, 1, someStructPtr);
        return;
    }
    serReader.writeStr("StartReq");
    callback(0, 0, someStructPtr);
    callback(0, 0, someStructPtr);
}



void StartReadThread(void (*callback)(int, int, void *, void *), void *cardStuctPtr)
{

    serReader.writeStr("StartRead");

    serReader.purgeBuff(PURGE_RXCLEAR);

    while (readerActive)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        if (!serReader.isOpened())continue;

        static char readBuf[128];
        memset(readBuf, 0, sizeof(readBuf));
        DWORD readSize;
        serReader.readData(readBuf, readSize);

        if (strcmp(string(readBuf).substr(0, 14).c_str(), "ReadNormalCard") == 0)
        {
            log("serReader read normal card success");

            // Raw card data and some other stuff, who cares
            unsigned char rawCardData[168] = {
                0x01, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x92, 0x2E, 0x58, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x7F, 0x5C, 0x97, 0x44, 0xF0, 0x88, 0x04, 0x00,
                0x43, 0x26, 0x2C, 0x33, 0x00, 0x04, 0x06, 0x10, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
                0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x4E, 0x42, 0x47, 0x49, 0x43, 0x36, 0x00, 0x00, 0xFA, 0xE9, 0x69, 0x00,
                0xF6, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //  string


            const auto accessCode = string(readBuf).substr(14, 20);
            const auto chipId = string(readBuf).substr(35, 32);

            memcpy(rawCardData + 0x50, accessCode.c_str(), accessCode.size() + 1);
            memcpy(rawCardData + 0x2C, chipId.c_str(), chipId.size() + 1);

            log("Callback from serRead card");
            callback(0, 0, rawCardData, cardStuctPtr);
        }
 
    }
}

extern "C"
{

    ULONGLONG BngRwAttach(UINT a1, char *a2, int a3, int a4, long (*callback)(long, long, long *), long *some_struct_ptr)
    {
        log("BngRwAttach(%i, %s, %d, %d, %p, %p)\n", a1, a2, a3, a4, callback, some_struct_ptr);
        createCard();

        std::thread t(StartAttachThread, callback, some_struct_ptr);
        t.detach();
        return 1;
    }

    long BngRwInit()
    {
        log("BngRwInit()\n");
        return 0;
    }

    ULONGLONG BngRwReqSetLedPower()
    {
        log("BngRwSetLedPower()\n");
        return 0;
    }

    int BngRwDevReset(UINT a, long (*callback)(int, int, long *), long *some_struct_ptr)
    {
        log("BngRwDevReset(%i, %p, %p)\n", a, callback, some_struct_ptr);

        std::thread t(StartResetThread, callback, some_struct_ptr);
        t.detach();
        return 1;
    }

    ULONGLONG BngRwExReadMifareAllBlock()
    {
        log("BngRwExReadMifareAllBlock()\n");
        return 0xffffff9c;
    }

    // Finalise?
    void BngRwFin()
    {
        log("BngRwFin()\n");
    }

    UINT BngRwGetFwVersion(UINT a)
    {
        log("BngRwGetFwVersion(%i)\n", a);
        return 0;
    }

    UINT BngRwGetStationID(UINT a)
    {
        log("BngRwGetStationID(%i)\n", a);
        return 0;
    }

    const char *BngRwGetVersion()
    {
        log("BngRwGetVersion()\n");
        return BANA_API_VERSION;
    }

    ULONGLONG BngRwIsCmdExec(UINT a)
    {
        log("BngRwIsCmdExec(%i)\n", a);
        return 0xFFFFFFFF;
    }

    UINT BngRwGetTotalRetryCount(UINT a)
    {
        log("BngRwGetTotalRetryCount(%i)\n", a);
        return 0;
    }

    int BngRwReqLed(UINT a, UINT b, ULONGLONG c, ULONGLONG d)
    {
        log("BngRwReqLed(%i, %i, %llu, %llu)\n", a, b, c, d);
        return 1;
    }

    int BngRwReqAction(UINT a, UINT b, void (*callback)(long, int, long *), long *some_struct_ptr)
    {
        log("BngRwReqAction(%i, %i, %p, %p)\n", a, b, callback, some_struct_ptr);

        std::thread t(StartReqActionThread, callback, some_struct_ptr);
        t.detach();
        if (b == 0)
        {
            return 1;
        }
        return -1;
    }

    int BngRwReqAiccAuth(UINT a, int b, UINT c, int *d, ULONGLONG e, ULONGLONG f, ULONGLONG *g)
    {
        log("BngRwReqAiccAuth(%i, %d, %i, %p, %llu, %llu, %p)\n", a, b, c, d, e, f, g);
        return 1;
    }

    int BngRwReqBeep(UINT a, UINT b, ULONGLONG c, ULONGLONG d)
    {
        log("BngRwReqBeep(%i, %i, %llu, %llu)\n", a, b, c, d);
        return 1;
    }

    int BngRwReqCancel(UINT a)
    {
        log("BngRwReqCancel(%i)\n", a);
        if (7 < a)
        {
            return -100;
        }
        return 1;
    }

    int BngRwReqFwCleanup(UINT a, ULONGLONG b, ULONGLONG c)
    {
        log("BngRwReqFwCleanup(%i, %llu, %llu)\n", a, b, c);
        return 1;
    }

    int BngRwReqFwVersionup(UINT a, ULONGLONG b, ULONGLONG c, ULONGLONG d)
    {
        log("BngRwReqFwVersionup(%i, %llu, %llu, %llu)\n", a, b, c, d);
        return 1;
    }

    int BngRwReqLatchID(UINT a, ULONGLONG b, ULONGLONG c)
    {
        log("BngRwReqLatchId(%i, %llu, %llu)\n", a, b, c);
        if (a < 8)
        {
            return -100;
        }
        return 1;
    }

    int BngRwReqSendMailTo(UINT a, int b, UINT c, int *d,
                           char *e, char *f, char *g, char *h, ULONGLONG i, ULONGLONG j)
    {
        log("BngRwReqSendMailTo(%i, %d, %i, %p, %s, %s, %s, %s, %llu, %llu)\n", a, b, c, d, e, f, g, h, i, j);
        if (7 < a)
        {
            return -100;
        }
        if (!e)
        {
            return -100;
        }
        return 1;
    }

    int BngRwReqSendUrlTo(UINT a, int b, UINT c, int *d,
                          char *e, char *f, ULONGLONG g, ULONGLONG h)
    {
        log("BngRwReqSendUrlTo(%i, %d, %i, %p, %s, %s, %llu, %llu)\n", a, b, c, d, e, f, g, h);
        if (7 < a)
        {
            return -100;
        }
        if (!e)
        {
            return -100;
        }
        return 1;
    }

    int BngRwReqWaitTouch(UINT a, int maxIntSomehow, UINT c, void (*callback)(int, int, void *, void *), void *card_struct_ptr)
    {
        log("BngRwReqWaitTouch(%i, %d, %i, %p, %p)\n", a, maxIntSomehow, c, callback, card_struct_ptr);

        // Hack to make sure previous threads have exited
        readerActive = false;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(250ms);

        readerActive = true;

        std::thread t(StartReadThread, callback, card_struct_ptr);
        t.detach();

        return 1;
    }
}