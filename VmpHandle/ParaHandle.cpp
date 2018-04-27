
// ParaHandleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "ParaHandle.h"
#include "SunDay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVmpHandleDlg 对话框


//子线程函数  
unsigned int __stdcall ThreadFun(PVOID pM)
{
	CParaHandle* pWmpDlg = (CParaHandle*)pM;
	pWmpDlg->LoadTraceData();
	OutputDebugString(_T("线程退出了\n"));
	SetEvent(pWmpDlg->m_hEventForThread);
	_endthreadex(0);
	return 0;
}

bool sort_token(const DWORD& s1, const DWORD& s2)  
{  
	return s1< s2;  
}  

CParaHandle::CParaHandle(CWnd* pParent /*=NULL*/)
	: CDialog(CParaHandle::IDD, pParent)
	,m_nParaHandle(0)
{
	m_hEventForThread = CreateEvent(NULL,FALSE,TRUE,NULL);
}

CParaHandle::~CParaHandle()
{
	if(m_hEventForThread)
	{
		// 若线程在执行，等待线程退出
		WaitForSingleObject(m_hEventForThread,INFINITE);
		CloseHandle(m_hEventForThread);
	}
}

void CParaHandle::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT2, m_editAdd);
	DDX_Control(pDX, IDC_STATIC_TIP, m_tip);

}

BEGIN_MESSAGE_MAP(CParaHandle, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CParaHandle::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CParaHandle::OnBnClickedBtnSave)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CParaHandle::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BTN_OFFSET, &CParaHandle::OnBnClickedCalcOffset)
END_MESSAGE_MAP()


// CVmpHandleDlg 消息处理程序

BOOL CParaHandle::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: 在此添加额外的初始化代码
	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0,_T("ID"),LVCFMT_CENTER,40);
	m_list.InsertColumn(1,_T("Dispatch"),LVCFMT_CENTER,80);
	m_list.InsertColumn(2,_T("Address"),LVCFMT_CENTER,80);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CParaHandle::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if (WaitForSingleObject(m_hEventForThread,300) == WAIT_OBJECT_0)
	{
		UpProcessState(Start);
		m_list.DeleteAllItems();
		unsigned threadID;
		HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, this, 0, &threadID);
		CloseHandle(handle);
	}
	else
	{
		OutputDebugString(_T("等待超时\n"));
		::MessageBox(m_hWnd, _T("任务已经在加载，请稍后"), _T("提示"), MB_OK);
	}
}



void CParaHandle::LoadTraceData()
{
	do 
	{
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
		unsigned char* szFindRet; 
		int nMaxLen = nLen;
		int nFindLen1 = strlen("[eax*4+");
		int nFindLen2 = strlen("EDX=");
		char szAddress[10]={0};
		DWORD dwDispathAddress=0;
		char* str;
		vec.clear();
		do
		{
			// 最大内存越界 < pbuf + nLen + 1024
			szFindRet = SUNDAY(szFindStart,(unsigned char*)"[eax*4+",nFindLen1,nMaxLen);
			if (!szFindRet) break;
			szFindStart = szFindRet;
			nMaxLen = nLen - (szFindRet-(unsigned char*)pbuf);
			szFindRet = SUNDAY(szFindStart,(unsigned char*)"EDX=",nFindLen2,50);
			if (szFindRet)
			{
				szFindStart = szFindRet;
				nMaxLen = nLen - (szFindRet-(unsigned char*)pbuf);
				memcpy(szAddress,szFindRet+4,8);
				dwDispathAddress = strtol(szAddress,&str,16);
				StlInsert(vec,dwDispathAddress);
			}
			else
			{
				szFindStart += 40;
			}
		}while(1);
		free(pbuf);
		fclose(pFile);
		std::sort(vec.begin(),vec.end(),sort_token); // 先排序  
		// 去除重复项  
		StlCotainer::iterator unque_it  = std::unique(vec.begin(), vec.end());  
		vec.erase(unque_it, vec.end());  
		InsertListControl();
		UpProcessState(Finish);
	} while (0);
}
	
void CParaHandle::OnBnClickedBtnSave()
{
	// TODO: Add your control notification handler code here
	CFileDialog fileDlg(FALSE, _T("txt"), _T("VmpHandle"));     
	fileDlg.m_ofn.lpstrTitle = _T("Save File");    
	fileDlg.m_ofn.lpstrFilter = _T("Text Files(*txt)\0*.txt\0All Files(*.*)\0*.*\0\0");  

	do 
	{
		if(fileDlg.DoModal() != IDOK )
			break;
		CString szFile = fileDlg.GetPathName();
		FILE *pFile = _tfopen(szFile.GetBuffer(),_T("wb"));
		if (!pFile) break;
		
        TCHAR  lpBuffer[MAX_PATH+1];
		TCHAR szTemp[1000] ={0};
		int nWrite = 0;
		int nWriteCount=0;
		_stprintf(szTemp,_T("Wmp Handle\n"));
		fwrite(szTemp,sizeof(TCHAR),_tcslen(szTemp),pFile);


		CHeaderCtrl* pHeaderCtrl = m_list.GetHeaderCtrl(); 
		int  nColumnCount = pHeaderCtrl->GetItemCount();  
	
		for (int i = 0; i < nColumnCount; i++)  
		{  
			HDITEM pHeadItem;         
			pHeadItem.mask = HDI_TEXT;   
			pHeadItem.pszText = lpBuffer;  
			pHeadItem.cchTextMax = MAX_PATH;  
			pHeaderCtrl->GetItem(i,&pHeadItem);  
			if (i==0)
			{
				nWrite =_stprintf(szTemp+nWriteCount,_T("%s\t\t"),pHeadItem.pszText);
			}
			else
			{
				nWrite =_stprintf(szTemp+nWriteCount,_T("%8s\t\t"),pHeadItem.pszText);
			}
			
			nWriteCount = nWriteCount + nWrite;
		}     
		nWrite = _stprintf(szTemp+nWriteCount,_T("\n"));
		fwrite(szTemp,sizeof(TCHAR),_tcslen(szTemp),pFile);


		nWriteCount = 0;
		nWrite = 0;
		int nCount = m_list.GetItemCount();
		TCHAR* pBuf = new TCHAR[nCount*200];
		if (!pBuf)
		{
			fclose(pFile);
			break;
		}
		ZeroMemory(pBuf,sizeof(TCHAR)*nCount*200);
		for (int i=0;i<nCount;i++)
		{
			for(int j=0;j<nColumnCount;j++)
			{
				m_list.GetItemText(i,j,lpBuffer,MAX_PATH);
				if (j==0)
				{
					nWrite = _stprintf(pBuf+nWriteCount,_T("%s\t\t"),lpBuffer);
				}
				else
				{
					nWrite = _stprintf(pBuf+nWriteCount,_T("%8s\t\t"),lpBuffer);
				}
				nWriteCount = nWriteCount+nWrite;
			}
			nWrite = _stprintf(pBuf+nWriteCount,_T("\n"));
			nWriteCount = nWriteCount+nWrite;
		}
		fwrite(pBuf,sizeof(TCHAR),nWriteCount,pFile);
		delete[] pBuf;
		fclose(pFile);
	} while (0);	
}

void CParaHandle::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_list.CopyItemTextToClipboard(pNMHDR);
	*pResult = 0;
}

int CParaHandle::UpProcessState(ProState Sate)
{
	switch(Sate)
	{
	case Start:
			m_tip.SetWindowText(_T("正在处理，请稍后"));
			break;
	case Finish:
		m_tip.SetWindowText(_T("处理完毕"));
		break;
	case None:
	default:
		m_tip.SetWindowText(_T(""));
			break;
	}
	return 0;
}

void CParaHandle::OnBnClickedCalcOffset()
{
	// TODO: Add your control notification handler code here
	InsertListControl();
}

void CParaHandle::InsertListControl()
{
	CString szFile,szAddValue;
	m_editAdd.GetWindowText(szAddValue);

	TCHAR* pStr;
	DWORD dwOffset = _tcstol(szAddValue,&pStr,16);
	int nCount = 0;
	TCHAR bufID[32] ={0};
	int nIndex = 0;
	DWORD dwAddress = 0;
	if (m_list)
	{
		m_list.DeleteAllItems();
		for (StlCotainer::iterator it = vec.begin();it!=vec.end();it++)
		{
			_itot(nCount+1,bufID,10);
			nIndex = m_list.InsertItem(nCount,bufID);
			bufID[0]=_T('0');
			bufID[1]=_T('x');
			_itot(*it,bufID+2,16);
			m_list.SetItemText(nIndex,1,bufID);

			dwAddress = *it - 1 + dwOffset;
			_itot(dwAddress,bufID+2,16);
			m_list.SetItemText(nIndex,2,bufID);
			nCount++;
		}
	}
}

int CParaHandle::SetFileName(CString szFileName)
{
	m_szFileName = szFileName;
	return 0;
}

void CParaHandle::OnOK()
{
	// 屏蔽回车关闭窗口
}