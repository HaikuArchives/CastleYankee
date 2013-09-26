/* CPPParser.h */

#ifndef _H_CPPParser_
#define _H_CPPParser_

#include <string>

#include <SupportDefs.h>

class CPPParser {
public:
	CPPParser(const char* text);
	CPPParser(string textIn);
	~CPPParser();

	void       	GetToken(char* token);
	const char*	RestOfText();
	char*      	LastComment();	// only valid right after GetToken(),
	           	              	// caller owns result
	void       	SkipWhitespace();
	void       	GetTokenLocation(int32* start, int32* end);

	static bool	TokenIsReserved(const char* token);
	static bool	TokenIsComment(const char* token);

protected:
	const char*	p;
	const char*	text;
	int32      	whitespaceStart, tokenStart, tokenEnd;
};

#endif
