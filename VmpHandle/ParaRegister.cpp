// ParaRegister.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "ParaRegister.h"
#include "SunDay.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////
//子线程函数  
unsigned int __stdcall ThreadSearchFun(PVOID pM)
{
	CParaRegister* pWmpDlg = (CParaRegister*)pM;
	pWmpDlg->SearchData();
	OutputDebugString(_T("搜索线程退出了\n"));
	SetEvent(pWmpDlg->m_hEventForThread);
	_endthreadex(0);
	return 0;
}

bool SortByAddress(const SearchResult*  pS1, const SearchResult* pS2)  
{  
	if (pS1==NULL ||pS2 ==NULL)
		return false;
	long nValue1 = strtol(pS1->szAddress,NULL,16);
	long nValue2 = strtol(pS2->szAddress,NULL,16);
	return nValue1<nValue2;
}  

bool AddressIsEqual(const SearchResult*  pS1, const SearchResult* pS2)  
{  
	if (pS1==NULL ||pS2 ==NULL)
		return false;
	long nValue1 = strtol(pS1->szAddress,NULL,16);
	long nValue2 = strtol(pS2->szAddress,NULL,16);
	if (nValue1 == nValue2)
		return true;
	return false;
}  

void DeleteVec(SearchResult* pSearchdata)
{
	delete pSearchdata;
	pSearchdata = NULL;
}

int  uniqueCmp(const SearchResult*  pS1, const SearchResult* pS2)  
{
	long nValue1 = strtol(pS1->szAddress,NULL,16);
	long nValue2 = strtol(pS2->szAddress,NULL,16);
	if(nValue1==nValue2)
		return 1;
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

// CParaRegister dialog

IMPLEMENT_DYNAMIC(CParaRegister, CDialog)

CParaRegister::CParaRegister(CWnd* pParent /*=NULL*/)
	: CDialog(CParaRegister::IDD, pParent)
	, m_nParaReg(0)
{
	m_hEventForThread = CreateEvent(NULL,FALSE,TRUE,NULL);
}



CParaRegister::~CParaRegister()
{
	if(m_hEventForThread)
	{
		// 若线程在执行，等待线程退出
		WaitForSingleObject(m_hEventForThread,INFINITE);
		CloseHandle(m_hEventForThread);
	}
	std::for_each(m_pVec.begin(),m_pVec.end(),DeleteVec);
	m_pVec.clear();
}

void CParaRegister::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT1, m_editKeyWord);
}

BOOL CParaRegister::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0,_T("ID"),LVCFMT_CENTER,40);
	m_list.InsertColumn(1,_T("Offset"),LVCFMT_CENTER,60);
	m_list.InsertColumn(2,_T("Address"),LVCFMT_CENTER,80);
	m_list.InsertColumn(3,_T("Thread"),LVCFMT_CENTER,50);
	m_list.InsertColumn(4,_T("Command"),LVCFMT_CENTER,220);
	m_list.InsertColumn(5,_T("Register"),LVCFMT_CENTER,100);
	m_editKeyWord.SetWindowText(_T("EAX=00000040"));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CParaRegister, CDialog)
	ON_BN_CLICKED(IDC_BTN_SEARCH, &CParaRegister::OnBnClickedBtnSearch)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST1, &CParaRegister::OnLvnGetdispinfoList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CParaRegister::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BTN_UNIQUE, &CParaRegister::OnBnClickedBtnUnique)
END_MESSAGE_MAP()


// CParaRegister message handlers

int CParaRegister::GetParaValue(void)
{
	return m_nParaReg;
}

int CParaRegister::SetFileName(CString szFileName)
{
	m_szFileName = szFileName;
	return 0;
}

int CParaRegister::SearchData(void)
{
	do 
	{
		char szkeyWord[MAX_PATH]={0};
		::GetWindowTextA(m_editKeyWord.m_hWnd,szkeyWord,MAX_PATH);
		int nSearchLen = strlen(szkeyWord);
		if (nSearchLen<=0)
		{
			AfxMessageBox(_T("请输入搜索内容！"));
			break;
		}
		if (nSearchLen > 12)
		{
			AfxMessageBox(_T("搜索内容非法！"));
			break;
		}

		if (!PathFileExists(m_szFileName)) break;
		FILE *pFile = _tfopen(m_szFileName.GetBuffer(),_T("rb"));
		if (!pFile) break;

		fseek(pFile, 0, SEEK_END);
		size_t nLen = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		char * pbuf = (char*)malloc(nLen + 1024); // 越界保护
		if (!pbuf)
		{
			fclose(pFile);
			break;
		}
		memset(pbuf, 0, nLen+1024);
		nLen = fread(pbuf, 1, nLen, pFile);

		unsigned char* szFindStart = (unsigned char*)pbuf;
		unsigned char* szFindRegResult; 
		int nMaxLen = nLen;

		std::for_each(m_pVec.begin(),m_pVec.end(),DeleteVec);
		m_pVec.clear();
	
		do
		{
			// 最大内存越界 < pbuf + nLen + 1024
			szFindRegResult = SUNDAY(szFindStart,(unsigned char*)szkeyWord,nSearchLen,nMaxLen);
			if (!szFindRegResult) break;
			
			// 倒叙查找换行符号
			int nMaxFind = 80;
			unsigned char* pLineAddress = szFindStart;
			int nCount = 0;
			char* pCom = NULL;
			char* pComEnd = NULL;
			do 
			{
				if (pLineAddress==NULL) break;
				if (nCount>nMaxFind) break;
				if (pLineAddress<(unsigned char*)pbuf) break;
				if (pLineAddress[0] == '\n')
				{
					SearchResult*  pResultData = new SearchResult;
					memset(pResultData,0,sizeof(SearchResult));
					memcpy(pResultData->szAddress,pLineAddress+1,8);
					// 命令前一定有四个空格
					pCom = strstr((char*)(pLineAddress+8+1),"     ");
					memcpy(pResultData->szThread,pLineAddress+8+1,pCom - ((char*)pLineAddress+8+1));
	
					// 寄存器前面至少有两个空格
					pComEnd = strstr(pCom+4,"  ");
					memcpy(pResultData->szCom,pCom+4,pComEnd-pCom-2-1);
					memcpy(pResultData->szReg,szFindRegResult,12);
					m_pVec.push_back(pResultData);
					break;
				}
				nCount++;
				pLineAddress--;
			} while (1);
			// 准备下一次搜索
			nMaxLen = nLen - (szFindRegResult-(unsigned char*)pbuf);
			szFindStart = szFindRegResult+12;
		}while(1);
		free(pbuf);
		fclose(pFile);

		if (!m_list)  break;
		m_list.SetItemCount(m_pVec.size());

	} while (0);
	return 0;
}

void CParaRegister::OnBnClickedBtnSearch()
{
	// TODO: Add your control notification handler code here
	if (WaitForSingleObject(m_hEventForThread,300) == WAIT_OBJECT_0)
	{
		m_list.DeleteAllItems();
		unsigned threadID;
		HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, ThreadSearchFun, this, 0, &threadID);
		CloseHandle(handle);
	}
	else
	{
		OutputDebugString(_T("等待超时\n"));
		::MessageBox(m_hWnd, _T("任务已经在加载，请稍后"), _T("提示"), MB_OK);
	}
}

void CParaRegister::OnLvnGetdispinfoList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: Add your control notification handler code here
	//插入列表数据
	m_list.InsertItemDataCallBack(pNMHDR,m_pVec);
	*pResult = 0;
}

void CParaRegister::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_list.CopyItemTextToClipboard(pNMHDR,FALSE);
	*pResult = 0;
}

void CParaRegister::OnBnClickedBtnUnique()
{
	// 先排序  
	std::sort(m_pVec.begin(),m_pVec.end(),SortByAddress);
	// 去除重复项   
	for (std::vector<SearchResult*>::iterator it= m_pVec.begin();it!=m_pVec.end();)
	{
		std::vector<SearchResult*>::iterator itFront = it+1;
		if (itFront==m_pVec.end())
			break;
		bool bRet = AddressIsEqual(*it,*itFront);
		if (bRet)
		{
			delete *itFront;
			m_pVec.erase(itFront);
		}
		else
		{
			it++;
		}
	}
	m_list.SetItemCount(m_pVec.size());
}

void CParaRegister::OnOK()
{
	// 屏蔽回车关闭窗口
}