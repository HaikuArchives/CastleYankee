/* Method.h */

#ifndef _H_Method_
#define _H_Method_

#include "ClassElement.h"
#include <string>

#include <SupportDefs.h>

class Code;
class TextFile;

class Method : public ClassElement {
public:
	Method(Class* classIn);
	virtual ~Method();
	string	Signature();
	Code* 	GetCode();
	void  	SetCode(string newCode);
	void  	SetCodeFromTextView(BTextView* view);
	bool  	IsInline();
	bool  	IsUglyNotesHack();
	int32 	HSpecLength();
	void  	GenerateH(TextFile* file, int32 specLength);
	void  	GenerateCPP(TextFile* file);
	void  	Export(TextFile* file);
	void  	CodeChanged();

protected:
	string	signature;
	Code* 	code;
	bool  	isInline;
	bool  	isPureVirtual;
	bool  	isUglyNotesHack;
};


#endif
