#pragma once
#include "mylistctrl.h"
#include "afxwin.h"
#include "VirtualList.h"
#include <vector>

// CParaRegister dialog

class CParaRegister : public CDialog
{
	DECLARE_DYNAMIC(CParaRegister)

public:
	CParaRegister(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParaRegister();

// Dialog Data
	enum { IDD = IDD_DLG_REGISTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	std::vector<SearchResult*> m_pVec;
	HANDLE  m_hEventForThread;
	int m_nParaReg;
	int GetParaValue(void);
	CString m_szFileName;
	int SetFileName(CString szFileName);
	CVirtualList m_list;
	int SearchData(void);
	CEdit m_editKeyWord;
	afx_msg void OnBnClickedBtnSearch();
	afx_msg void OnLvnGetdispinfoList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnUnique();
};
