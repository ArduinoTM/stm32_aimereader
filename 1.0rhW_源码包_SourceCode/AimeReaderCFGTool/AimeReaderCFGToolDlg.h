
// AimeReaderCFGToolDlg.h: 头文件
//

#pragma once


// CAimeReaderCFGToolDlg 对话框
class CAimeReaderCFGToolDlg : public CDialogEx
{
// 构造
public:
	CAimeReaderCFGToolDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AIMEREADERCFGTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit1();
	CComboBox cb1_v;
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CButton m_connbutt;
	afx_msg void OnBnClickedButton3();

	CStatic verstr_v;
	afx_msg void OnStnClickedText();
	CSliderCtrl GAMELEDSLI_v;
	CEdit GAMELEDINP_v;
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnNMCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CButton rt_check_v;
	afx_msg void OnBnClickedButton8();
};
//static CAimeReaderCFGToolDlg* pCAimeReaderCFGToolDlg;