#include "stdafx.h"
#include "ClipboardHelper.h"

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

		// ◊¢“‚’‚¿Ô
		SetClipboardData(CF_UNICODETEXT, hMen);
	}

	if (!appendOnly)
		CloseClipboard();

	return hMen != NULL;
}
