#include "stdafx.h"
#include "SunDay.h"

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

void GetLineHeadAndTail(char * pFileBuf,long nFileSize,unsigned char* pFindPos, unsigned char* &pLineHead,unsigned char* &pLineTail)
{
	// 定位行首，倒叙查找换行符号
	pLineHead = pFindPos;
	do 
	{
		if (pLineHead<(unsigned char*)pFileBuf)
		{
			pLineHead=NULL;
			break;
		}

		if (pLineHead[0] == '\n')
			break;
		pLineHead--;
	} while (1);
	//  行尾
	pLineTail = pFindPos;
	do 
	{
		if ((char*)pLineTail < pFileBuf+nFileSize)
		{
			if (pLineTail[0]=='\n') 
				break;
			pLineTail++;
		}
		else
		{
			pLineTail = NULL;
			break;
		}

	} while (1);

}
