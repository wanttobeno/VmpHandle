#pragma once

#include "dragedit.h"
#include "afxcmn.h"
#include "afxwin.h"

#include "ParaHandle.h"
#include "VmpHandle.h"
#include "ParaCommand.h"
#include "ParaRegister.h"
#include "ParaAbout.h"

// CMainDlg dialog

class CVmpHandleDlg : public CDialog
{
	DECLARE_DYNAMIC(CVmpHandleDlg)

public:
	CVmpHandleDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVmpHandleDlg();

// Dialog Data
	enum { IDD = IDD_VMPHANDLE_DIALOG };

// 实现
protected:
	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tab;
	CParaHandle m_paraHandle;
	CParaCommand m_paraCom;
	CParaRegister m_paraReg;
	CParaAbout m_paraAbout;
	
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	CDragEdit m_editFilePath;
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnEnChangeEdit1();
	virtual void OnOK();
};
