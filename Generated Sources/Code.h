/* Code.h */

#ifndef _H_Code_
#define _H_Code_

#include <string>

#include <SupportDefs.h>

class Code {
public:
	Code();
	string	GetCode();
	void  	SetCode(string newCode);
	void  	GetSelection(int32* selStartOut, int32* selEndOut);
	void  	SetSelection(int32 newSelStart, int32 newSelEnd);
	void  	SetFromTextView(BTextView* view);

protected:
	string	code;
	int32 	selStart;
	int32 	selEnd;
};


#endif
