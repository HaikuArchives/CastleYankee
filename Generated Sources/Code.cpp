/* Code.cpp */

#include "Code.h"

#include <TextView.h>


Code::Code()
	: selStart(0), selEnd(0)
{
}


string Code::GetCode()
{
	return code;
}


void Code::SetCode(string newCode)
{
	code = newCode;
	selStart = selEnd = 0;
}


void Code::GetSelection(int32* selStartOut, int32* selEndOut)
{
	if (selStartOut)
		*selStartOut = selStart;
	if (selEndOut)
		*selEndOut = selEnd;
}


void Code::SetSelection(int32 newSelStart, int32 newSelEnd)
{
	selStart = newSelStart;
	selEnd = newSelEnd;
}


void Code::SetFromTextView(BTextView* view)
{
	code = view->Text();
	view->GetSelection(&selStart, &selEnd);
}


