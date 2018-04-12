// ParaHandle.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "ParaHandle.h"


// CParaHandle dialog

IMPLEMENT_DYNAMIC(CParaHandle, CDialog)

CParaHandle::CParaHandle(CWnd* pParent /*=NULL*/)
	: CDialog(CParaHandle::IDD, pParent)
	, m_nParaHandle(0)
{

}

CParaHandle::~CParaHandle()
{
}

void CParaHandle::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CParaHandle, CDialog)
END_MESSAGE_MAP()


// CParaHandle message handlers

int CParaHandle::GetParaValue(void)
{
	return m_nParaHandle;
}
