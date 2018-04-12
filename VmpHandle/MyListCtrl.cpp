// MyListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "MyListCtrl.h"

#include "ClipboardHelper.h"

// CMyListCtrl

IMPLEMENT_DYNAMIC(CMyListCtrl, CListCtrl)

CMyListCtrl::CMyListCtrl()
{
	
}

CMyListCtrl::~CMyListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
END_MESSAGE_MAP()



// CMyListCtrl message handlers


bool CMyListCtrl::CopyItemTextToClipboard(NMHDR* pNMHDR, bool bAppendOnly)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	TCHAR buf[MAX_PATH+1]={0};
	GetItemText(pNMItemActivate->iItem,pNMItemActivate->iSubItem,buf,MAX_PATH);
	CopyTextToClipboard(buf,FALSE);
	return false;
}

