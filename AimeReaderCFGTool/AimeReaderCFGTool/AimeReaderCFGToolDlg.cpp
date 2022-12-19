
// AimeReaderCFGToolDlg.cpp: 实现文件
//
#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "AimeReaderCFGTool.h"
#include "AimeReaderCFGToolDlg.h"
#include "afxdialogex.h"
#include "SerialPort.h"

#include <SetupAPI.h>
#pragma comment(lib, "Setupapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
SerialPort com;
static bool serialOpenFlag = false;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_ABOUTBOX
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 支持

    // 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CAimeReaderCFGToolDlg 对话框

CAimeReaderCFGToolDlg::CAimeReaderCFGToolDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_AIMEREADERCFGTOOL_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAimeReaderCFGToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO1, cb1_v);
    DDX_Control(pDX, IDC_BUTTON2, m_connbutt);
    DDX_Control(pDX, IDC_TEXT, verstr_v);
    DDX_Control(pDX, IDC_SLIDER2, GAMELEDSLI_v);
    DDX_Control(pDX, IDC_EDIT1, GAMELEDINP_v);
    DDX_Control(pDX, IDC_CHECK1, rt_check_v);
}

BEGIN_MESSAGE_MAP(CAimeReaderCFGToolDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_EN_CHANGE(IDC_EDIT1, &CAimeReaderCFGToolDlg::OnEnChangeEdit1)
    ON_BN_CLICKED(IDC_BUTTON6, &CAimeReaderCFGToolDlg::OnBnClickedButton6)
    ON_BN_CLICKED(IDC_BUTTON1, &CAimeReaderCFGToolDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CAimeReaderCFGToolDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &CAimeReaderCFGToolDlg::OnBnClickedButton3)
    ON_STN_CLICKED(IDC_TEXT, &CAimeReaderCFGToolDlg::OnStnClickedText)
    ON_BN_CLICKED(IDC_BUTTON5, &CAimeReaderCFGToolDlg::OnBnClickedButton5)
    ON_BN_CLICKED(IDC_BUTTON4, &CAimeReaderCFGToolDlg::OnBnClickedButton4)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, &CAimeReaderCFGToolDlg::OnNMCustomdrawSlider2)
    ON_BN_CLICKED(IDC_BUTTON7, &CAimeReaderCFGToolDlg::OnBnClickedButton7)
    ON_BN_CLICKED(IDC_CHECK1, &CAimeReaderCFGToolDlg::OnBnClickedCheck1)
    ON_WM_HSCROLL()
END_MESSAGE_MAP()

// CAimeReaderCFGToolDlg 消息处理程序

BOOL CAimeReaderCFGToolDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);  // 设置大图标
    SetIcon(m_hIcon, FALSE); // 设置小图标

    // TODO: 在此添加额外的初始化代码
    GAMELEDSLI_v.SetRange(0, 255);
    GAMELEDSLI_v.SetTicFreq(16);
    GAMELEDSLI_v.SetPos(64);
    GAMELEDINP_v.SetWindowTextA("64");

    return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

void CAimeReaderCFGToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAimeReaderCFGToolDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

BOOL CAimeReaderCFGToolDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        switch (pMsg->wParam)
        {
        case VK_RETURN: //屏蔽回车键
            return TRUE;
        case VK_ESCAPE: //屏蔽ESC键
            return TRUE;
        default:
            break;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAimeReaderCFGToolDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CAimeReaderCFGToolDlg::OnEnChangeEdit1()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialogEx::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    // TODO:  在此添加控件通知处理程序代码
}

void CAimeReaderCFGToolDlg::OnBnClickedButton6()
{
    // TODO: 在此添加控件通知处理程序代码
    exit(0);
}

// 查找计算机可用串口并将串口号设置到Combo Box控件上
void FindCommPort(CComboBox* pComboBox)
{
    pComboBox->ResetContent();
    HKEY hKey;

    LONG nRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "Hardware\\DeviceMap\\SerialComm", NULL,
        KEY_READ, &hKey);
    if (nRetVal == ERROR_SUCCESS)
    {
        int i = 0;
        char portName[256], commName[256];
        DWORD dwLong, dwSize;
        while (1)
        {
            dwLong = dwSize = sizeof(portName);
            nRetVal = RegEnumValue(hKey, i, portName, &dwLong, NULL, NULL, (PUCHAR)commName, &dwSize);
            if (nRetVal == ERROR_NO_MORE_ITEMS) // 枚举串口
                break;

            CString strCommName;
            strCommName.Format("%s", commName);
            pComboBox->AddString(strCommName); // commName：串口名字
            i++;
        }
        if (pComboBox->GetCount() == 0)
        {
            AfxMessageBox("啊，没有找到串口欸!");
        }
        else
        {
            pComboBox->SetCurSel(0);
        }
        RegCloseKey(hKey);
    }
}

void CAimeReaderCFGToolDlg::OnBnClickedButton1()
{
    // TODO: 在此添加控件通知处理程序代码
    FindCommPort(&cb1_v);
}

void CAimeReaderCFGToolDlg::OnBnClickedButton2()
{
    // TODO: 在此添加控件通知处理程序代码

    if (cb1_v.GetCount() == 0)
    {
        AfxMessageBox("没有正确选择串口，操作失败");
        return;
    }

    CString text;
    int cindex = cb1_v.GetCurSel();
    cb1_v.GetLBText(cindex, text);

    if (!serialOpenFlag)
    {
        int comport = 0;
        for (int i = 0; i < text.GetLength(); i++)
        {
            if ('0' <= text[i] && text[i] <= '9')
            {
                comport *= 10;
                comport += text[i] - '0';
            }
        }
        if (com.OpenPort(comport))
        {
            com.SetPort(115200, 1, 8, 0);
            m_connbutt.SetWindowText("断开");
            serialOpenFlag = true;
            // send enter conf
            unsigned char tempsend[10] = { 0xCF };
            com.WriteData(tempsend, 1);
            Sleep(50);
            // send get version
            unsigned char tempsend2[10] = "GETVER";
            com.WriteData(tempsend2, 6);
            unsigned char tempread[100];
            DWORD readlen = 0;
            Sleep(50);
            if (!com.ReadData(tempread, readlen))
            {
                AfxMessageBox(_T("读卡器版本读取失败或非hwdk_aimereader"));
                verstr_v.SetWindowTextA("未读取到");
            }
            else
            {
                AfxMessageBox(_T("连接成功！请务必记住调整完毕后先保存再断开或退出！！"));
                CString tempconvert(tempread);
                verstr_v.SetWindowTextA(LPCTSTR(tempconvert.GetString()));
            }
        }
    }
    else
    {
        // send escape conf
        unsigned char tempsend[10] = "ESCCONF";
        com.WriteData(tempsend, 7);
        com.ClosePort();
        m_connbutt.SetWindowText("连接");
        verstr_v.SetWindowTextA("未连接");
        serialOpenFlag = false;
    }
}

int PVSerialPort(CString strVidPid)
{
    // 获取当前系统所有使用的设备
    int nPort = -1;
    int nStart = -1;
    int nEnd = -1;
    int i = 0;
    CString strTemp, strName;
    DWORD dwFlag = (DIGCF_ALLCLASSES | DIGCF_PRESENT);
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA sDevInfoData;
    TCHAR szDis[MAX_PATH] = { 0x00 }; // 存储设备实例ID
    TCHAR szFN[MAX_PATH] = { 0x00 };  // 存储设备实例属性
    DWORD nSize = 0;

    // 准备遍历所有设备查找USB
    hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, dwFlag);
    if (INVALID_HANDLE_VALUE == hDevInfo)
        goto STEP_END;

    // 开始遍历所有设备
    memset(&sDevInfoData, 0x00, sizeof(SP_DEVICE_INTERFACE_DATA));
    sDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &sDevInfoData); i++)
    {
        nSize = 0;

        // 无效设备
        if (!SetupDiGetDeviceInstanceId(hDevInfo, &sDevInfoData, szDis, sizeof(szDis), &nSize))
            goto STEP_END;

        // 根据设备信息寻找VID PID一致的设备
        strTemp.Format(_T("%s"), szDis);
        strTemp.MakeUpper();
        if (strTemp.Find(strVidPid, 0) == -1)
            continue;

        // 查找设备属性
        nSize = 0;
        SetupDiGetDeviceRegistryProperty(hDevInfo, &sDevInfoData,
            SPDRP_FRIENDLYNAME,
            0, (PBYTE)szFN,
            sizeof(szFN),
            &nSize);

        // "XXX Virtual Com Port (COM7)"
        strName.Format(_T("%s"), szFN);
        if (strName.IsEmpty())
            goto STEP_END;

        // 寻找串口信息
        nStart = strName.Find(_T("(COM"), 0);
        nEnd = strName.Find(_T(")"), 0);
        if (nStart == -1 || nEnd == -1)
            goto STEP_END;

        strTemp = strName.Mid(nStart + 4, nEnd - nStart - 2);
        nPort = atoi(strTemp);
    }

STEP_END:
    // 关闭设备信息集句柄
    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        hDevInfo = INVALID_HANDLE_VALUE;
    }

    return nPort;
}

void CAimeReaderCFGToolDlg::OnBnClickedButton3()
{
    // TODO: 在此添加控件通知处理程序代码

    FindCommPort(&cb1_v);
    int portnum = PVSerialPort(_T("VID_0483&PID_5740"));
    if (portnum < 0)
        AfxMessageBox("没有找到hwdk_aimereader设备");
    else
    {
        CString targetText, nowText;
        targetText.Format("COM%d", portnum);
        int cindex = cb1_v.GetCurSel();
        for (int i = 0; i < cb1_v.GetCount(); i++)
        {
            cb1_v.GetLBText(i, nowText);
            if (nowText == targetText)
            {
                cb1_v.SetCurSel(i);
            }
        }
    }
}

void CAimeReaderCFGToolDlg::OnStnClickedText()
{
    // TODO: 在此添加控件通知处理程序代码
}

void CAimeReaderCFGToolDlg::OnBnClickedButton5()
{
    // TODO: 在此添加控件通知处理程序代码
    unsigned char tempsend[10] = "READCONF";
    com.WriteData(tempsend, 8);
    // todo:读取的的conf写到控件里面
}

void CAimeReaderCFGToolDlg::OnBnClickedButton4()
{
    // TODO: 在此添加控件通知处理程序代码
    // SETGAMELED_xxx

    uint8_t m_int = GAMELEDSLI_v.GetPos();
    //可改为全配一个函数方便开发，无太大开销，暂时单独来，后续再改
    unsigned char tempsend[16];
    sprintf((char*)tempsend, "SETGAMELED_%3d", m_int);
    com.WriteData(tempsend, 14);
    Sleep(10);
    // SETNGAMELEDDIV_xxx

    sprintf((char*)tempsend, "SAVECONF");
    com.WriteData(tempsend, 8);
    // todo:roger判断
}

void CAimeReaderCFGToolDlg::OnNMCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;
}

void CAimeReaderCFGToolDlg::OnBnClickedButton7()
{
    // TODO: 在此添加控件通知处理程序代码
    unsigned char tempsend[10] = "INITCONF";
    com.WriteData(tempsend, 8);
    // todo:roger判断
    unsigned char tempsend2[10] = "READCONF";
    com.WriteData(tempsend2, 8);
    // todo:读配置到控件
}

void CAimeReaderCFGToolDlg::OnBnClickedCheck1()
{
    // TODO: 在此添加控件通知处理程序代码
    //开始实时预览需要发送全部当前状态到板子，方法一：并要求开启实时预览，方法二：或者采用设备方实时，软件方存本地配置表，发送消息的方式
    //或者实现单方预览
    //暂时实现设备实时全部预览
    // todo:点按时发送全部状态
    //难蚌，可能不好写，以后(24h内)再说
}

void CAimeReaderCFGToolDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (pScrollBar != NULL)
    {
        CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;

        if (pSlider->GetDlgCtrlID() == IDC_SLIDER2)
        {
            uint8_t m_int = GAMELEDSLI_v.GetPos();
            SetDlgItemInt(IDC_EDIT1, m_int);
            if (rt_check_v.GetCheck())
            {
                // rt，可改为全配一个函数方便开发，无太大开销，暂时单独来，后续再改
                unsigned char tempsend[16];
                sprintf((char*)tempsend, "SETGAMELED_%3d", m_int);
                com.WriteData(tempsend, 14);
                // todo:roger判断
                // todo:设置本地配置表
            }
            else
            {
                // nrt
                // todo:设置本地配置表
            }
        }
    }
    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
