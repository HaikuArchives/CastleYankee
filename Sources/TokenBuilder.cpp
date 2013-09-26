/* TokenBuilder.cpp */

#include "TokenBuilder.h"


TokenBuilder::TokenBuilder()
	: lastChar('\0')
{
	str = new string;
}


TokenBuilder::~TokenBuilder()
{
	delete str;
}


void TokenBuilder::AddToken(const char* token)
{
	// add whitespace if necessary
	char c = token[0];
	bool tokenIsAlpha = ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
	bool lastWasWhitespace =
		(lastChar == ' ' || lastChar == '\t' ||
		 lastChar == '\n' || lastChar == '\r');
	bool lastWasAlpha =
		((lastChar >= 'a' && lastChar <= 'z') ||
		 (lastChar >= 'A' && lastChar <= 'Z') ||
		 (lastChar >= '0' && lastChar <= '9') ||
		 lastChar == '_' );
	if (tokenIsAlpha &&
			((!lastWasWhitespace && lastWasAlpha) ||	// hmm...redundancy: lastWasAlpha implies !lastWasWhitespace
			 (lastChar == '*' && (*str)[str->length() - 2] != '(')
			 || lastChar == '&' || lastChar == ','))
		*str += ' ';

	// add token
	*str += token;

	// keep lastChar
	lastChar = token[strlen(token) - 1];
}


void TokenBuilder::PadToLength(int32 len)
{
	int32 padLen = len - str->length();
	if (padLen > 0) {
		str->append(padLen, ' ');
		lastChar = ' ';
		}
}


void TokenBuilder::Clear()
{
	*str = "";
	lastChar = '\0';
}


const char* TokenBuilder::String()
{
	return str->c_str();
}


int32 TokenBuilder::Length()
{
	return str->length();
}


