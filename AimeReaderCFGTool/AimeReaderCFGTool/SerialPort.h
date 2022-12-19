#pragma once
#include "framework.h"
#include "AimeReaderCFGTool.h"
#include "AimeReaderCFGToolDlg.h"
#include "afxdialogex.h"

class SerialPort
{
public:
    int m_Port;
    char szCurPath[256];
    SerialPort();
    ~SerialPort();
    bool OpenPort(int port);
    bool SetPort(DWORD baud, BYTE stop, BYTE data, BYTE parity);
    void ClosePort();
    bool ReadData(unsigned char* receive, DWORD& receivelen);
    bool WriteData(unsigned char* send, int sendlen);
private:
    OVERLAPPED ReadovReady, WriteovReady;
    HANDLE hCom;
    bool bOpenCom;
};
