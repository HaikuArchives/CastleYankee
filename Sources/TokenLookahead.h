/* TokenLookahead.h */

#ifndef _H_TokenLookahead_
#define _H_TokenLookahead_

class CPPParser;

class TokenLookahead {
public:
	TokenLookahead(CPPParser* parser);	// does not take ownership of "parser"

	const char*	CurToken(bool acceptComment = false);
	const char*	NextToken(bool acceptComment = false);
	const char*	CurComment();
	const char*	NextComment();
	void       	ConsumeToken();

protected:
	CPPParser*	parser;
	char      	tok1[256], tok2[256];
	char*     	curToken;
	char*     	nextToken;
	char*     	curComment;
	char*     	nextComment;
	bool      	haveCurToken, haveNextToken;
};

#endif
