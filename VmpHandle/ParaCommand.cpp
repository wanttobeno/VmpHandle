// ParaRegister.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "ParaCommand.h"
#include "SunDay.h"
#include <algorithm>
#include "FileHelper.h"

//子线程函数  
unsigned int __stdcall ThreadSearchComFun(PVOID pM)
{
	CParaCommand* pWmpDlg = (CParaCommand*)pM;
	pWmpDlg->SearchData();
	OutputDebugString(_T("搜索线程退出了\n"));
	SetEvent(pWmpDlg->m_hEventForThread);
	_endthreadex(0);
	return 0;
}

//去除尾部空格  
char *rtrim(char *str)  
{  
	if (str == NULL || *str == '\0')  
	{  
		return str;  
	}  

	int len = strlen(str);  
	char *p = str + len - 1;  
	while (p >= str  && isspace(*p))  
	{  
		*p = '\0';  
		--p;  
	}  
	return str;  
}  


/*

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

*/

void DeleteData(SearchResult* pSearchdata)
{
	delete pSearchdata;
	pSearchdata = NULL;
}


// ParaCommand dialog

IMPLEMENT_DYNAMIC(CParaCommand, CDialog)

CParaCommand::CParaCommand(CWnd* pParent /*=NULL*/)
	: CDialog(CParaCommand::IDD, pParent)
	, m_nParaReg(0)
	, m_bNoSysAddress(FALSE)
{
	m_hEventForThread = CreateEvent(NULL,FALSE,TRUE,NULL);
}



CParaCommand::~CParaCommand()
{
	if(m_hEventForThread)
	{
		// 若线程在执行，等待线程退出
		WaitForSingleObject(m_hEventForThread,INFINITE);
		CloseHandle(m_hEventForThread);
	}
	std::for_each(m_pVec.begin(),m_pVec.end(),DeleteData);
	m_pVec.clear();
}

void CParaCommand::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT1, m_editKeyWord);
	DDX_Check(pDX, IDC_CHECK_NO_SYS_ADDRESS, m_bNoSysAddress);
}

BOOL CParaCommand::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0,_T("ID"),LVCFMT_CENTER,40);
	m_list.InsertColumn(1,_T("Offset"),LVCFMT_CENTER,60);
	m_list.InsertColumn(2,_T("Address"),LVCFMT_CENTER,80);
	m_list.InsertColumn(3,_T("Thread"),LVCFMT_CENTER,50);
	m_list.InsertColumn(4,_T("Command"),LVCFMT_CENTER,220);
	m_list.InsertColumn(5,_T("Register"),LVCFMT_CENTER,100);
	m_editKeyWord.SetWindowText(_T("add edx,0x"));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CParaCommand, CDialog)
	ON_BN_CLICKED(IDC_BTN_SEARCH, &CParaCommand::OnBnClickedBtnSearch)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST1, &CParaCommand::OnLvnGetdispinfoList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CParaCommand::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BTN_UNIQUE, &CParaCommand::OnBnClickedBtnUnique)
END_MESSAGE_MAP()


// ParaCommand message handlers

int CParaCommand::GetParaValue(void)
{
	return m_nParaReg;
}

int CParaCommand::SetFileName(CString szFileName)
{
	m_szFileName = szFileName;
	return 0;
}

int CParaCommand::SearchData(void)
{
	bool bNoSysAddress = m_bNoSysAddress;
	long nPreFileOffset = 0;
	long nOffset = 0;
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
		if (nSearchLen<3)
		{
			AfxMessageBox(_T("至少输入3个字节"));
			break;
		}
		
		char* pFileBuf = NULL;
		size_t nFileSize =0;
		ReadFileBuf(m_szFileName.GetBuffer(),&pFileBuf,&nFileSize);
		if (pFileBuf == NULL) break;

		unsigned char* szFindStart = (unsigned char*)pFileBuf;
		unsigned char* pFindPos = NULL; 
		int nMaxLen = nFileSize;

		std::for_each(m_pVec.begin(),m_pVec.end(),DeleteData);
		m_pVec.clear();
	
		// 通用算法，参数可以是行内的任意关键字
		do
		{
			if ((char*)szFindStart >= pFileBuf+nFileSize)  break;
			
			pFindPos = SUNDAY(szFindStart,(unsigned char*)szkeyWord,nSearchLen,nMaxLen);
			if (!pFindPos) break;
			
			int nCount = 0;
			int nMaxFind = 80;
			unsigned char* pLineHead = NULL;
			unsigned char* pLineTail = NULL;
			// 定位行首和行尾
			GetLineHeadAndTail(pFileBuf,nFileSize,pFindPos,pLineHead,pLineTail);
			if(pLineHead ==NULL || pLineTail == NULL)
				break;
			nOffset= (char*)pLineHead - pFileBuf - nPreFileOffset;
			nPreFileOffset = (char*)pLineHead - pFileBuf;

			do
			{
				// 无效行
				// 例如 MessageBoxTimeout>mov edi,edi
				// api的头一个一定是字符
				if (pLineHead[1]<'0') break;
				if (pLineHead[1]>'F') break;
				if (pLineHead[1]>'9'&&pLineHead[1]<'A') break;
				// 排除系统空间
				if (bNoSysAddress)
					if (pLineHead[1] >='7') 
						break;

				SearchResult*  pResultData = new SearchResult;
				pResultData->nOffset = nOffset;
				memcpy(pResultData->szAddress,pLineHead+1,8);
				// 模块名,必定有5个空格，否则无效行
				char* pThreadEnd = strstr((char*)pLineHead,"     ");
				char* pComStart = pThreadEnd + 5;
				memcpy(pResultData->szThread,pLineHead+10,pThreadEnd-(char*)pLineHead-10);

				// 判断下有无寄存器注释
				char *pRegflag = strstr(pComStart,";");
				BOOL bReg = FALSE;
				if (pRegflag)
				{
					if (pRegflag <(char*)pLineTail) 
						bReg = TRUE;
				}
				if (bReg)
				{
					memcpy(pResultData->szCom,pComStart,pRegflag-(char*)pComStart);
					memcpy(pResultData->szReg,pRegflag+2,12);
				}
				else
				{
					int nLen = (char*)pLineTail-(char*)pComStart-1;
					memcpy(pResultData->szCom,pComStart,nLen);
				}
				// 去除空格
				rtrim(&pResultData->szCom[0]);
				m_pVec.push_back(pResultData);
			}while(0);
			// 准备下一次搜索
			nMaxLen = nFileSize - ((char*)pLineTail-(char*)pFileBuf);
			szFindStart = pLineTail+1;
			if (!szFindStart) break;
		}while(1);
		free(pFileBuf);
		if (!m_list)  break;
		m_list.SetItemCount(m_pVec.size());
	} while (0);
	return 0;
}

void CParaCommand::OnBnClickedBtnSearch()
{
	// TODO: Add your control notification handler code here
	if (WaitForSingleObject(m_hEventForThread,300) == WAIT_OBJECT_0)
	{
		UpdateData();
		m_list.DeleteAllItems();
		unsigned threadID;
		HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, ThreadSearchComFun, this, 0, &threadID);
		CloseHandle(handle);
	}
	else
	{
		OutputDebugString(_T("等待超时\n"));
		::MessageBox(m_hWnd, _T("任务已经在加载，请稍后"), _T("提示"), MB_OK);
	}
}

void CParaCommand::OnLvnGetdispinfoList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: Add your control notification handler code here
	//插入列表数据
	m_list.InsertItemDataCallBack(pNMHDR,m_pVec);
	*pResult = 0;
}

void CParaCommand::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_list.CopyItemTextToClipboard(pNMHDR,FALSE);
	*pResult = 0;
}

void CParaCommand::OnBnClickedBtnUnique()
{
	// 先排序  
	//std::sort(m_pVec.begin(),m_pVec.end(),SortByAddress);
	//// 去除重复项   
	//for (std::vector<SearchResult*>::iterator it= m_pVec.begin();it!=m_pVec.end();)
	//{
	//	std::vector<SearchResult*>::iterator itFront = it+1;
	//	if (itFront==m_pVec.end())
	//		break;
	//	bool bRet = AddressIsEqual(*it,*itFront);
	//	if (bRet)
	//	{
	//		delete *itFront;
	//		m_pVec.erase(itFront);
	//	}
	//	else
	//	{
	//		it++;
	//	}
	//}
	//m_list.SetItemCount(m_pVec.size());
}

void CParaCommand::OnOK()
{
	// TODO: Add your control notification handler code here
}
