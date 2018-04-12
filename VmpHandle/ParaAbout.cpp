// ParaAbout.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "ParaAbout.h"


// CParaAbout dialog

IMPLEMENT_DYNAMIC(CParaAbout, CDialog)

CParaAbout::CParaAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CParaAbout::IDD, pParent)
	, m_nParaAbout(0)
{

}

CParaAbout::~CParaAbout()
{
}

void CParaAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CParaAbout, CDialog)
END_MESSAGE_MAP()


// CParaAbout message handlers

int CParaAbout::GetParaValue(void)
{
	return m_nParaAbout;
}
