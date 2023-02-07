// CRWERDlg.cpp: 实现文件
//
#define _CRT_SECURE_NO_WARNINGS
#include "afxwin.h"
#include "SerialPort.h"
#include "AimeReaderCFGTool.h"
#include "afxdialogex.h"
#include "CRWERDlg.h"

extern SerialPort com;
// CRWERDlg 对话框

IMPLEMENT_DYNAMIC(CRWERDlg, CDialogEx)

CRWERDlg::CRWERDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_READWRITER, pParent)
{

}

CRWERDlg::~CRWERDlg()
{
}

void CRWERDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, rwStatEdit);
	DDX_Control(pDX, IDC_RADIO4, rwtpARad);
	DDX_Control(pDX, IDC_EDIT3, rwACEdit);
}


BEGIN_MESSAGE_MAP(CRWERDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CRWERDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON4, &CRWERDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON1, &CRWERDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CRWERDlg 消息处理程序


void CRWERDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}




BOOL CRWERDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	rwtpARad.SetCheck(1);
	rwACEdit_font.CreatePointFont(120, "Arial");//创建字体和大小
	rwACEdit.SetFont(&rwACEdit_font);
	rwACEdit.SetLimitText(20);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CRWERDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	unsigned char tempsend[64] ;
	CString AccessCodeTapd;
	rwACEdit.GetWindowText(AccessCodeTapd);
	sprintf((char*)tempsend, "WRITECARD %s", AccessCodeTapd.GetBuffer(AccessCodeTapd.GetLength()));
	AccessCodeTapd.ReleaseBuffer();
	com.WriteData(tempsend, 30);

	unsigned char tempread[100];
	DWORD readlen = 0;
	time_t begtim = clock();
	while (!com.ReadData(tempread, readlen) && clock()-begtim<CLOCKS_PER_SEC) {
		Sleep(50);
	}
	if (! readlen)
	{
		rwStatEdit.SetWindowTextA("读卡器无回复：读卡器通信失败或非正确版本固件");
		AfxMessageBox(_T("读卡器通信失败或非正确版本固件"));
	}
	else
	{
		// reply "ROGER WRITECARD RET %d" %d is 20
		if (tempread[20] == '0')
		{
			rwStatEdit.SetWindowTextA("写卡成功");
		}
		else if (tempread[20] == '6')
		{
			rwStatEdit.SetWindowTextA("写卡失败：卡号格式错误");
		}
		else if (tempread[20] == '4')
		{
			rwStatEdit.SetWindowTextA("写卡失败：未找到卡");
		}
		else if (tempread[20] == '2')
		{
			rwStatEdit.SetWindowTextA("写卡失败：卡密钥认证失败，此加密卡密钥非常见");
		}
		else if (tempread[20] == '3')
		{
			rwStatEdit.SetWindowTextA("写卡失败：写块错误，损坏或被取走或只读？不明原因");
		}
	}


}


BOOL CRWERDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

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


void CRWERDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	AfxMessageBox("本读写卡器组件程序旨在增加读卡器功能，方便用户写自制卡，\n\
为音游社团等低价获取为团员的游戏数据记录卡所用。\
减少手机写卡，导入，修改十六进制转储再写转储的麻烦。\n\
所需材料：\n\
M1(mifare 1 classic)（IC，uid，cuid）空白卡，密钥常见的无用M1卡（例如币用完的机厅卡），\
或手机NFC开一张白卡，felica卡片不支持。\n\
使用方法：\n\
单卡写卡，先输入目标卡号（20位纯数字，可自行编写），\
然后将实体卡或手机开的白卡放在读写卡器正确位置，点写卡即可。\n\
注意事项：\n\
并非所有旧卡都可写入，若原来卡已经用未知密钥加密或只读，会无法写入。\
已经内置密钥：Aime密钥，Bana密钥，新卡FF密钥，常用00密钥，NDEF密钥。\
并非所有二十位数字都可以使用，写卡后要到游戏里刷一下试试。\
写入已购买到卡的卡号，可做为兼容复制卡使用相同游戏记录数据。\
不锁卡的情况下，写入的卡号可多次更改，或者恢复到空卡。\
使用官方卡片卡号写卡可在官方机台上使用，自编卡号可能会错误。\
写出卡片为兼容卡，适用大部分机台游戏，未写入官方卡片校验位，\
读卡认卡特别严格的游戏，刷卡会出现失败，此时请使用官方卡片\n\
警告：\n\
部分非友好网络，请勿刷自写卡号卡，特别是滥用第一局免费，可能导致服主封禁机台。\
购买的官方卡片，一般为只读，不要尝试向内部写入数据，可能造成损坏。\
不要尝试向机厅币卡，门禁卡，饭卡，水卡中写入数据，\
可能导致卡原数据被破坏，造成原有币数，余额失效，\
与读卡器和软件作者无任何责任，后果请您自付!\
使用本读卡器，软件，破坏卡片，盗号，是违法违规行为，\
与读卡器，软件作者无关，后果请您自付\n\
后续更新，会增加批量写卡功能，复制官卡（带校验位，完全复制），\
自定义卡写入前密钥用于验证，锁卡使卡片只读等功能，敬请期待。");
}
