#ifndef SUNDAY_H__
#define SUNDAY_H__ 1

unsigned char* SUNDAY(unsigned char *lpBase, unsigned char *lpCmp,DWORD len,DWORD maxSize);


void GetLineHeadAndTail(char * pFileBuf,long nFileSize,unsigned char* pFindPos, unsigned char* &pLineHead,unsigned char* &pLineTail);

#endif //SUNDAY_H__