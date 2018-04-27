// VirtualList.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "VirtualList.h"
#include "ClipboardHelper.h"

// CVirtualList

IMPLEMENT_DYNAMIC(CVirtualList, CListCtrl)

CVirtualList::CVirtualList()
{

}

CVirtualList::~CVirtualList()
{
}

BEGIN_MESSAGE_MAP(CVirtualList, CListCtrl)
END_MESSAGE_MAP()



// CVirtualList message handlers

enum ListID
{
	L_ID=0,
	L_Offset,
	L_Address,
	L_Thread,
	L_Command,
	L_Reg
};

int CVirtualList::InsertItemDataCallBack(NMHDR* pNMHDR,std::vector<SearchResult*>& pVec)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItemIndx= pItem->iItem;
	if (pItem->mask & LVIF_TEXT) //字符串缓冲区有效
	{
		CString szTmp;
		switch(pItem->iSubItem){
		case L_ID: //填充数据项的名字
			{
				TCHAR bufId[20]={0};
				_itot(pItem->iItem+1,bufId,10);
				szTmp = bufId;
			}
			break;
		case L_Offset:
			{
				TCHAR bufId[20]={0};
				_itot(pVec[iItemIndx]->nOffset,bufId,10);
				szTmp = bufId;
			}
			break;
		case L_Address: //填充子项2
			szTmp = pVec[iItemIndx]->szAddress;
			break;
		case L_Thread: //填充子项3
			szTmp = pVec[iItemIndx]->szThread;
			break;
		case L_Command: //填充子项4
			szTmp = pVec[iItemIndx]->szCom;
			break;
		case L_Reg: //填充子项5
			szTmp = pVec[iItemIndx]->szReg;
			break;
		}
		lstrcpy(pItem->pszText,szTmp);
	}
	return 0;
}

bool CVirtualList::CopyItemTextToClipboard(NMHDR* pNMHDR, bool bAppendOnly)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	TCHAR buf[MAX_PATH+1]={0};
	GetItemText(pNMItemActivate->iItem,pNMItemActivate->iSubItem,buf,MAX_PATH);
	CopyTextToClipboard(buf,FALSE);
	return false;
}