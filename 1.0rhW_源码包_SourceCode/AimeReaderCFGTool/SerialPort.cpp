
#include "SerialPort.h"


SerialPort::SerialPort()
{
	this->bOpenCom = false;
}
SerialPort::~SerialPort()
{
	this->ClosePort();
}

bool SerialPort::OpenPort(int port)
{
	if (bOpenCom)
	{
		this->ClosePort();
		bOpenCom = false;
	}
	char szport[100];
	CString portname;
	if (port < 10) {
		sprintf_s(szport, "COM%d", port);
	}
	else {
		sprintf_s(szport, "\\\\.\\COM%d", port);
	}
	portname = szport;
	hCom = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		CString mess;
		mess = _T("连接")+portname+_T("失败");
		AfxMessageBox(mess);
		return FALSE;
	}
	else
	{
		CString mess;
		mess = _T("连接") + portname + _T("成功");
		return TRUE;
	}

}
void SerialPort::ClosePort()
{
	if (!bOpenCom)
		return;

	CloseHandle(hCom);
	hCom = NULL;

	CloseHandle(ReadovReady.hEvent);
	CloseHandle(WriteovReady.hEvent);
	ReadovReady.hEvent = NULL;
	WriteovReady.hEvent = NULL;
}
bool SerialPort::SetPort(DWORD baud, BYTE stop, BYTE data, BYTE parity)
{
	if (!SetupComm(hCom, 1024, 1024))//输入缓冲区和输出缓冲区的大小都是1024
		return false;

	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	if (!SetCommTimeouts(hCom, &TimeOuts)) //设置超时
		return false;

	memset(&ReadovReady, 0, sizeof(OVERLAPPED));
	memset(&WriteovReady, 0, sizeof(OVERLAPPED));
	ReadovReady.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	WriteovReady.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	DCB dcb;
	GetCommState(hCom, &dcb);
	//dcb.BaudRate = 115200; //波特率为9600
	//dcb.ByteSize = 8; //每个字节有8位
	//dcb.Parity = NOPARITY; //无奇偶校验位
	//dcb.StopBits = TWOSTOPBITS; //两个停止位
	dcb.BaudRate = baud;
	dcb.ByteSize = data;
	dcb.Parity = parity;
	dcb.StopBits = stop;
	if (!SetCommState(hCom, &dcb))
		return false;

	bOpenCom = true;
	return bOpenCom;

}
bool SerialPort::ReadData(unsigned char* receive, DWORD& receivelen)
{

	if (!bOpenCom)
		return false;
	/*if (ReadovReady.hEvent == NULL)
		return false;*/
	DWORD dwErrorFlags;
	COMSTAT ComStat;
	receivelen = 0;
	ClearCommError(hCom, &dwErrorFlags, &ComStat);
	if (ReadFile(hCom, receive, ComStat.cbInQue, &receivelen, &ReadovReady) == FALSE)
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			return false;
		}
	}

	if (receivelen == 0)
	{
		return false;
	}

	//receive[receivelen] = 0;

	DWORD dwRead;
	DWORD dwRes = WaitForSingleObject(ReadovReady.hEvent, 2000);
	switch (dwRes)
	{
	case WAIT_OBJECT_0:
		if (!GetOverlappedResult(hCom, &ReadovReady, &dwRead, FALSE))
			return false;
		break;

	case WAIT_TIMEOUT:
		break;

	default:
		break;
	}
	return true;

}

bool SerialPort::WriteData(unsigned char* send, int sendlen)
{
	if (!bOpenCom)    return false;

	DWORD    BytesSent;
	DWORD    resD;

	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	BytesSent = 0;
	BOOL hr = WriteFile(hCom,       // Handle to COMM Port
		send,                       // Pointer to message buffer in calling finction
		sendlen,                    // Length of message to send
		&BytesSent,                 // Where to store the number of bytes sent
		&WriteovReady);             // Overlapped structure
	if (!hr)
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			return false;
		}
		else
		{
			resD = WaitForSingleObject(WriteovReady.hEvent, INFINITE);
		}
		switch (resD)
		{
		case WAIT_OBJECT_0:
		{
			if (!GetOverlappedResult(hCom, &WriteovReady, &BytesSent, false))
				return false;
			else
				return true;

		}
		default:
			return false;
			break;
		}
	}
	return true;

}
