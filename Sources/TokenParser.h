/* TokenParser.h */

#ifndef _H_TokenParser_
#define _H_TokenParser_

#include <SupportDefs.h>

class TokenParser {
public:
	TokenParser(const char* text, uint32 textLen);
	TokenParser(BFile* file);
	~TokenParser();

	void	NextLine();
	void	GetToken(char* token);
	void	GetRestOfLine(char* str);

protected:
	const char*	text;
	const char*	p;
	const char* stopper;
	bool       	ownText, started;
};

#endif
