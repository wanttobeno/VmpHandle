
// VmpHandleDlg.h : 头文件
//

#pragma once
#include "dragedit.h"
#include "afxcmn.h"
#include "afxwin.h"

#include <string>
#include <vector>
#include <set>
#include <stdio.h>
#include <algorithm>

#define  StlCotainer std::vector<DWORD>
#define  StlInsert(vec,Value) vec.push_back(Value)

// CVmpHandleDlg 对话框
class CVmpHandleDlg : public CDialog
{
// 构造
public:
	CVmpHandleDlg(CWnd* pParent = NULL);	// 标准构造函数

	virtual ~CVmpHandleDlg();
// 对话框数据
	enum { IDD = IDD_VMPHANDLE_DIALOG };
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
	CEdit m_editAdd;
	CStatic m_tip;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CDragEdit m_EditFile;
	CListCtrl m_list;
	StlCotainer vec;
	HANDLE  m_hEventForThread;
public:
	afx_msg void OnBnClickedOk();
	void LoadTraceData();
	void InsertListControl();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	enum ProState{None,Start,Finish};
	int UpProcessState(ProState Sate);
	afx_msg void OnBnClickedCalcOffset();
	afx_msg void OnBnClickedBtnOpen();
};
