/* Variable.h */

#ifndef _H_Variable_
#define _H_Variable_

#include "ClassElement.h"
#include <string>

#include <SupportDefs.h>

class TextFile;
class Class;

class Variable : public ClassElement {
public:
	Variable(Class* classIn);
	virtual ~Variable() {}
	string	Name();
	string	Code();
	void  	SetCode(const char* newCode);
	bool  	IsUglyNotesHack();
	int32 	HSpecLength();
	void  	GenerateH(TextFile* file, int32 specLength);
	void  	GenerateCPP(TextFile* file);
	void  	Export(TextFile* file);

protected:
	string	name;
	string	code;
	bool  	isUglyNotesHack;
};


#endif
