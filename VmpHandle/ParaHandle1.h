#pragma once


// CParaHandle dialog

class CParaHandle : public CDialog
{
	DECLARE_DYNAMIC(CParaHandle)

public:
	CParaHandle(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParaHandle();

// Dialog Data
	enum { IDD = IDD_DLG_HANDLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nParaHandle;
	int GetParaValue(void);
};
