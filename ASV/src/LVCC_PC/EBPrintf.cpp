#include "stdafx.h"

//	Edit Debug Configuration
#define STRING_LEN			4096
#define EDIT_BUF_SIZE		(0x60000)   
#define EDIT_BUF_DEC_SIZE	(0x2000)

//8: \b
//13: \r
// 1)about EM_SETSEL message arguments
//   start_position,end_position
//   The selected character is from start_position to end_position-1;
//   So, We should do +1 to the position of last selected character.
// 2)The char index start from 0.
// 3)WM_CLEAR,WM_CUT doesn't operate on EM_SETSEL chars.
void EB_Printf(CEdit *pEdt, TCHAR *fmt,...)
{
	va_list ap;
	int i,slen,lineIdx;
	int txtRepStart,txtRepEnd,txtSelEnd;
	static int wasCr=0; //should be static type.
	TCHAR string[STRING_LEN]; //margin for '\b'
	TCHAR string2[STRING_LEN]; //margin for '\n'->'\r\n'
	static int prevBSCnt=0;
	int str2Pt=0;
	CEdit edtDebug;

	txtRepStart = pEdt->GetWindowTextLength();
	txtRepEnd = txtRepStart-1;

	va_start(ap,fmt);
	_vsntprintf(string2,STRING_LEN-1,fmt,ap);
	va_end(ap);

	string2[STRING_LEN-1]=TEXT('\0');

	//for better look of BS(backspace) char.,
	//the BS in the end of the string will be processed next time.
	for(i=0;i<prevBSCnt;i++) //process the previous BS char.
		string[i]=TEXT('\b');
	string[prevBSCnt]=TEXT('\0');
	lstrcat(string,string2);
	string2[0]=TEXT('\0');

	slen=lstrlen(string);
	for(i=0;i<slen;i++)
		if(string[slen-i-1]!=TEXT('\b'))break;

	prevBSCnt=i; // These BSs will be processed next time.
	slen=slen-prevBSCnt;

	if(slen==0)
	{
		return;
	}

	for(i=0;i<slen;i++)
	{
		if( (string[i]==TEXT('\n')))
		{
			string2[str2Pt++]=TEXT('\r');txtRepEnd++;
			string2[str2Pt++]=TEXT('\n');txtRepEnd++;
			wasCr=0;
			continue;
		}
		if( (string[i]!=TEXT('\n')) && (wasCr==1) )
		{
			string2[str2Pt++]=TEXT('\r');txtRepEnd++;
			string2[str2Pt++]=TEXT('\n');txtRepEnd++;
			wasCr=0;
		}
		if(string[i]==TEXT('\r'))
		{
			wasCr=1;
			continue;
		}

		if(string[i]==TEXT('\b'))
		{
			//flush string2
			if(str2Pt>0)
			{
				string2[--str2Pt]=TEXT('\0');
				txtRepEnd--;
				continue;
			}
			//str2Pt==0;	    
			if(txtRepStart>0)
			{
				txtRepStart--;
			}
			continue;
		}
		string2[str2Pt++]=string[i];
		txtRepEnd++;
		// if(str2Pt>256-3)break; //why needed? 2001.1.26
	}

	string2[str2Pt]=TEXT('\0');
	if(str2Pt>0)
	{
		pEdt->SetSel(txtRepStart,txtRepEnd+1);
		pEdt->ReplaceSel(string2);
	}
	else
	{
		if(txtRepStart<=txtRepEnd)
		{
			pEdt->SetSel(txtRepStart,txtRepEnd+1);
			pEdt->ReplaceSel(TEXT(""));
		}
	}


	//If edit buffer is over EDIT_BUF_SIZE,
	//the size of buffer must be decreased by EDIT_BUF_DEC_SIZE.
	if(txtRepEnd>EDIT_BUF_SIZE)
	{
		lineIdx = pEdt->LineFromChar(EDIT_BUF_DEC_SIZE);
		txtSelEnd = pEdt->LineIndex( lineIdx ) - 1;
		pEdt->SetSel(0,txtSelEnd+1);
		pEdt->ReplaceSel(TEXT(""));

		//make the end of the text shown.
		txtRepEnd = pEdt->GetWindowTextLength() - 1;
		pEdt->SetSel(txtRepEnd+1,txtRepEnd+2);
		pEdt->ReplaceSel(TEXT(" "));
		pEdt->SetSel(txtRepEnd+1,txtRepEnd+2);
		pEdt->ReplaceSel(TEXT(""));
	}
}


