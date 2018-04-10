
// VmpHandleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VmpHandle.h"
#include "VmpHandleDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVmpHandleDlg 对话框


//子线程函数  
unsigned int __stdcall ThreadFun(PVOID pM)
{
	CVmpHandleDlg* pWmpDlg = (CVmpHandleDlg*)pM;
	pWmpDlg->LoadTraceData();
	OutputDebugString(_T("线程退出了\n"));
	SetEvent(pWmpDlg->m_hEventForThread);
	return 0;
}


CVmpHandleDlg::CVmpHandleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVmpHandleDlg::IDD, pParent)
{
	m_hEventForThread = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hEventForThread = CreateEvent(NULL,FALSE,TRUE,NULL);
}

CVmpHandleDlg::~CVmpHandleDlg()
{
	if(m_hEventForThread)
	{
		// 若线程在执行，等待线程退出
		WaitForSingleObject(m_hEventForThread,INFINITE);
		CloseHandle(m_hEventForThread);
	}
}

void CVmpHandleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_EditFile);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT2, m_editAdd);
	DDX_Control(pDX, IDC_STATIC_TIP, m_tip);
}

BEGIN_MESSAGE_MAP(CVmpHandleDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CVmpHandleDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CVmpHandleDlg::OnBnClickedBtnSave)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CVmpHandleDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BTN_OFFSET, &CVmpHandleDlg::OnBnClickedCalcOffset)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CVmpHandleDlg::OnBnClickedBtnOpen)
END_MESSAGE_MAP()


// CVmpHandleDlg 消息处理程序

BOOL CVmpHandleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	char str[MAX_PATH+1] ={0};
	GetWindowTextA(m_hWnd,str,MAX_PATH);
	strcat(str," 编译时间： ");
	strcat(str,__DATE__);
	strcat(str," ");
	strcat(str,__TIME__);
	strcat(str," By：ZhanYue");
	CString title;
	title = str;
	SetWindowText(title);

	// TODO: 在此添加额外的初始化代码
	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0,_T("ID"),LVCFMT_CENTER,50);
	m_list.InsertColumn(1,_T("Dispatch"),LVCFMT_CENTER,100);
	m_list.InsertColumn(2,_T("Address"),LVCFMT_CENTER,100);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVmpHandleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVmpHandleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool sort_token(const DWORD& s1, const DWORD& s2)  
{  
	return s1< s2;  
}  

void CVmpHandleDlg::OnBnClickedOk()
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
	
void CVmpHandleDlg::OnBnClickedBtnSave()
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


unsigned char* SUNDAY(unsigned char *lpBase, unsigned char *lpCmp,DWORD len,DWORD maxSize)
{
	size_t temp[256]; 
	size_t *shift = temp; 
	size_t i, patt_size = len, text_size = maxSize; 

	for( i=0; i < 256; i++ ) 
		*(shift+i) = patt_size+1; 

	for( i=0; i < patt_size; i++ ) 
		*(shift+unsigned char(*(lpCmp+i))) = patt_size-i; 
	size_t limit = text_size-patt_size+1; 
	for( i=0; i < limit; i += shift[ lpBase[i+patt_size] ] ) 
	{ 
		if( lpBase[i] == *lpCmp ) 
		{ 
			unsigned char *match_text = lpBase+i+1; 
			size_t match_size = 1; 
			do 
			{
				if( match_size == patt_size )
				{
					return lpBase+i;
				}
			} 
			while( (*match_text++) == lpCmp[match_size++] ); 
		} 
	} 
	return NULL;
}


void CVmpHandleDlg::LoadTraceData()
{
	CString szFile;
	m_EditFile.GetWindowText(szFile);
	do 
	{
		if (!PathFileExists(szFile)) break;
		FILE *pFile = _tfopen(szFile.GetBuffer(),_T("rb"));
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


bool CopyTextToClipboard(const TCHAR *text, bool appendOnly) 
{
	if (!text)
		return false;

	if (!appendOnly) {
		if (!OpenClipboard(NULL))
			return false;
		EmptyClipboard();
	}

	size_t n = _tcslen(text) + 1;
	HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, n * sizeof(TCHAR));
	if (hMen) {
		TCHAR *globalText = (TCHAR *)GlobalLock(hMen);
		if (globalText) {
			memcpy(globalText,text, n *sizeof(TCHAR));
		}
		GlobalUnlock(hMen);

		// 注意这里
		SetClipboardData(CF_UNICODETEXT, hMen);
	}

	if (!appendOnly)
		CloseClipboard();

	return hMen != NULL;
}

void CVmpHandleDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	TCHAR buf[MAX_PATH+1]={0};
	m_list.GetItemText(pNMItemActivate->iItem,pNMItemActivate->iSubItem,buf,MAX_PATH);
	CopyTextToClipboard(buf,FALSE);

	*pResult = 0;
}

int CVmpHandleDlg::UpProcessState(ProState Sate)
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

void CVmpHandleDlg::OnBnClickedCalcOffset()
{
	// TODO: Add your control notification handler code here
	InsertListControl();
}

void CVmpHandleDlg::InsertListControl()
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

void CVmpHandleDlg::OnBnClickedBtnOpen()
{
	// TODO: Add your control notification handler code here
	CFileDialog fileDlg(TRUE, _T("txt"), _T("log"));     
	fileDlg.m_ofn.lpstrTitle = _T("Open File");    
	fileDlg.m_ofn.lpstrFilter = _T("Text Files(*txt)\0*.txt\0All Files(*.*)\0*.*\0\0");  
	if(fileDlg.DoModal() == IDOK )
	{
		CString szStr;
		szStr = fileDlg.GetPathName();
		m_EditFile.SetWindowText(szStr);
	}
}
