/* TokenLookahead.cpp */

#include "TokenLookahead.h"
#include "CPPParser.h"


TokenLookahead::TokenLookahead(CPPParser* parserIn)
	: parser(parserIn), curToken(tok1), nextToken(tok2),
	  curComment(NULL), nextComment(NULL),
	  haveCurToken(false), haveNextToken(false)
{
}


const char* TokenLookahead::CurToken(bool acceptComment)
{
	// get the token if we haven't got it already
	if (!haveCurToken) {
		do {
			parser->GetToken(curToken);
		} while (!acceptComment && CPPParser::TokenIsComment(curToken) && curToken[0] != 0);
		delete curComment;
		curComment = NULL;
		if (acceptComment && CPPParser::TokenIsComment(curToken))
			curComment = parser->LastComment();
		haveCurToken = true;
		}

	return curToken;
}


const char* TokenLookahead::NextToken(bool acceptComment)
{
	// make sure we actually have the current token
	CurToken(acceptComment);

	// get the next token
	if (!haveNextToken) {
		do {
			parser->GetToken(nextToken);
		} while (!acceptComment && CPPParser::TokenIsComment(nextToken) && nextToken[0] != 0);
		delete nextComment;
		nextComment = NULL;
		if (acceptComment && CPPParser::TokenIsComment(nextToken))
			nextComment = parser->LastComment();
		haveNextToken = true;
		}

	return nextToken;
}


const char* TokenLookahead::CurComment()
{
	return curComment;
}


const char* TokenLookahead::NextComment()
{
	return nextComment;
}


void TokenLookahead::ConsumeToken()
{
	if (!haveCurToken) {
		// must have token before we can consume it
		parser->GetToken(curToken);
		}

	// shift nextToken to curToken (swapping buffers)
	char* temp = curToken;
	curToken = nextToken;
	nextToken = temp;
	// shift comment
	delete curComment;
	curComment = nextComment;
	nextComment = NULL;
	// shift "have"
	haveCurToken = haveNextToken;
	haveNextToken = false;
}


