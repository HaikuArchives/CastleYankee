/* TokenParser.cpp */

#include "TokenParser.h"

#include <File.h>


TokenParser::TokenParser(const char* textIn, uint32 textLen)
	: text(textIn), p(text), ownText(false), started(false)
{
	stopper = text + textLen;
}


TokenParser::TokenParser(BFile* file)
	: started(false)
{
	off_t fileLen;
	file->GetSize(&fileLen);
	char* fileText = new char[fileLen];
	file->ReadAt(0, fileText, fileLen);
	text = fileText;
	ownText = true;
	p = text;
	stopper = text + fileLen;
}


TokenParser::~TokenParser()
{
	if (ownText)
		delete text;
}


void TokenParser::NextLine()
{
	if (started) {
		// skip to next line
		while (p < stopper && *p != '\n' && *p != '\r')
			p++;
		}
	started = true;

	while (true) {
		// skip initial whitespace and blank lines
		while (p < stopper &&
		       (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
			p++;
		if (p >= stopper)
			break;

		// if not a comment, we've found the token start
		if (*p != '/' && *p != ';' && *p != '#')
			break;
		}
}


void TokenParser::GetToken(char* token)
{
	// skip whitespace
	while (p < stopper && (*p == ' ' || *p == '\t'))
		p++;

	// copy token (quoted)
	if (p < stopper && *p == '\"') {
		p++;
		while (p < stopper) {
			if (*p == '\"') {
				// done with quoted string
				p++;
				break;
				}
			else if (*p == '\\') {
				// quote next character
				p++;
				if (p < stopper)
					*token++ = *p++;
				}
			else
				*token++ = *p++;
			}
		}

	// copy token (unquoted)
	else {
		while (p < stopper && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r')
			*token++ = *p++;
		}

	// finish off token
	*token = '\0';
}


void TokenParser::GetRestOfLine(char* str)
{
	// skip whitespace
	while (p < stopper && (*p == ' ' || *p == '\t') && *p != '\n' && *p != '\r')
		p++;

	// copy rest of line
	while (p < stopper && *p != '\n' && *p != '\r')
		*str++ = *p++;

	// finish off str
	*str = '\0';
}


