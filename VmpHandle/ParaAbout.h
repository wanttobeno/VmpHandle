#pragma once


// CParaAbout dialog

class CParaAbout : public CDialog
{
	DECLARE_DYNAMIC(CParaAbout)

public:
	CParaAbout(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParaAbout();

// Dialog Data
	enum { IDD = IDD_DLG_ABOUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nParaAbout;
	int GetParaValue(void);
};
