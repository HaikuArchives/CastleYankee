/* TokenBuilder.h */

#ifndef _H_TokenBuilder_
#define _H_TokenBuilder_

#include <string>

#include <SupportDefs.h>

class TokenBuilder {
public:
	TokenBuilder();
	~TokenBuilder();

	void       	AddToken(const char* token);
	void       	AddToken(string token) { AddToken(token.c_str()); }
	void       	PadToLength(int32 len);
	void       	Clear();
	const char*	String();
	int32      	Length();

protected:
	string*	str;		// I would have done this as "string" rt "string*"
	            		// except that necessitates dragging in <string>
	            		// and everything it #includes.  In future classes
	            		// I might just bite the bullet on that, since it'll
	            		// become so ubiquitous.  It does seem to be a
	            		// significant compile hit though.
	            		// 1999.8.7:  I had to do it to compile under the
	            		// new CW; there's no longer an implicit
	            		// "class string;" forward decl, and an explicit
	            		// one doesn't work because "string" is now a
	            		// typedef, not a class.  Just another way CW's
	            		// "string" implementation sucks.
	char   	lastChar;
};

#endif
