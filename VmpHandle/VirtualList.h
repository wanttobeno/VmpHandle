#pragma once

#include <vector>

struct SearchResult
{
	char szAddress[10];
	char szThread[20];
	char szCom[60];
	char szReg[16];
	SearchResult()
	{
		ZeroMemory(this, 0, sizeof(SearchResult));
	}
};

// CVirtualList

class CVirtualList : public CListCtrl
{
	DECLARE_DYNAMIC(CVirtualList)

public:
	CVirtualList();
	virtual ~CVirtualList();
	int InsertItemDataCallBack(NMHDR* pNMHDR,std::vector<SearchResult*>& pVec);
	bool CopyItemTextToClipboard(NMHDR* pNMHDR, bool bAppendOnly);
protected:
	DECLARE_MESSAGE_MAP()
};


