#pragma once
#include "afxdialogex.h"


// CRWERDlg 对话框

class CRWERDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRWERDlg)

public:
	CRWERDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CRWERDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_READWRITER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	CEdit rwStatEdit;
	CBrush m_brush;
	CButton rwtpARad;
	virtual BOOL OnInitDialog();
	CEdit rwACEdit;
private:
	CFont rwACEdit_font;


public:
	afx_msg void OnBnClickedButton4();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButton1();
};
