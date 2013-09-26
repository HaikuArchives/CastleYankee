/* CPPParser.cpp */

#include "CPPParser.h"
#include <string>

static const char* const keywords[] = {
	"and", "and_eq", "asm", "auto", "bitand", "bitor",
	"bool", "break", "case", "catch", "char", "class",
	"compl", "const", "const_cast", "continue", "default", "delete",
	"do", "double", "dynamic_cast", "else", "enum", "explicit",
	"export", "extern", "false", "float", "for", "friend",
	"goto", "if", "inline", "int", "long", "mutable",
	"namespace", "new", "not", "not_eq", "operator", "or",
	"or_eq", "private", "protected", "public", "register", "reinterpret_cast",
	"return", "short", "signed", "sizeof", "static", "static_cast",
	"struct", "switch", "template", "this", "throw", "true",
	"try", "typedef", "typeid", "typename", "union", "unsigned",
	"using", "virtual", "void", "volatile", "wchar_t", "while",
	"xor", "xor_eq", NULL
};

#define MaxStringLen	250


CPPParser::CPPParser(const char* textIn)
	: p(textIn), text(textIn)
{
}


CPPParser::CPPParser(string textIn)
{
	text = textIn.c_str();
	p = text;
}


CPPParser::~CPPParser()
{
}


void CPPParser::GetToken(char* token)
{
	SkipWhitespace();
	tokenStart = p - text;

	char c = *token++ = *p++;
	switch (c) {
		case '/':
			c = *p;
			if (c == '/') {
				// C++-style comment
				*token++ = c;	// return minimal comment token
				// skip until end of line
				while ((c = *p) != '\n' && c != 0)
					p++;
				// skip additional lines
				while (((c = *p) != 0) && ((c == '\n') || (c == '\r')))
					p++;
				}
			else if (c == '*') {
				// C-style comment
				*token++ = c;	// return minimal comment token
				// skip until the end of the comment
				if (*p == 0)
					break;
				while (true) {
					c = *p++;
					if (c == 0)
						break;
					if (c == '*' && *p == '/') {
						p++;	// consume '/'
						break;
						}
					}
				// skip trailing whitespace
				while (((c = *p) != 0) &&
				       (c == '\n' || c == '\r' || c == ' ' || c == '\t'))
				    p++;
				}
			else if (*p == '=')
				*token++ = *p++;	// /= operator
			// otherwise, it was just a plain '/'
			break;

		case '\'':
			while (*p != 0) {
				c = *token++ = *p++;
				if (c == '\\' && *p != 0)
					*token++ = *p++;
				if (c == '\'')
					break;
				}
			break;

		case '\"':
			{
			int stringLen = 0;
			while (*p != 0) {
				c = *token++ = *p++;

				// don't exceed MaxStringLen for the return value
				if (++stringLen > MaxStringLen-2) {
					*token++ = '"';		// stop right there, but return it as a complete string
					// now just scan to the end
					while (*p != 0) {
						c = *p++;
						if (c == '\\' && p[1] != 0)
							p += 2;
						else if (c == '\"')
							break;
						}
					break;
					}

				if (c == '\\' && *p != 0)
					*token++ = *p++;	// the quoted character
				else if (c == '\"')
					break;
				}
			}
			break;

		case '-':
			{
			c = *p;
			if (c == '>') {
				*token++ = *p++;
				if (*p == '*')
					*token++ = *p++;
				}
			else if (c == '-' || c == '=')
				*token++ = *p++;
			else if (c >= '0' && c <= '9')
				goto numberToken;
			}
			break;

		case '+':
			{
			if ((c = *p) == '=' || c == '+')
				*token++ = *p++;
			else if (c >= '0' && c <= '9')
				goto numberToken;
			}
			break;

		numberToken:
		case '0':
			if ((c = *p) == 'x' || c == 'X') {
				// hex number
				*token++ = *p++;
				while ((c = *p) != 0 &&
				       ((c >= '0' && c <= '9') ||
				        (c >= 'A' && c <= 'F') ||
				        (c >= 'a' && c <= 'f'))) {
					*token++ = *p++;
					}
				break;
				}
			// otherwise fall thru
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			while ((c = *p) != 0 && c >= '0' && c <= '9')
				*token++ = *p++;
			if (*p == '.') {
				*token++ = *p++;
				while ((c = *p) != 0 && c >= '0' && c <= '9')
					*token++ = *p++;
				}
			c = *p;
			if (c == 'e' || c == 'E') {
				*token++ = *p++;
				if ((c = *p) == '-' || c == '+')
					*token++ = *p++;
				while ((c = *p) != 0 && c >= '0' && c <= '9')
					*token++ = *p++;
				if ((c = *p) == 'f' || c == 'l' || c == 'F' || c == 'L')
					*token++ = *p++;
				}
			else if (c == 'u' || c == 'U' || c == 'l' || c == 'L')
				*token++ = *p++;
			break;

		case '<':
			if (*p == '<') {
				*token++ = *p++;
				if (*p == '=')
					*token++ = *p++;
				}
			else if ((c = *p) == '=' || c == ':' || c == '%')
				*token++ = *p++;
			break;

		case '>':
			if (*p == '>') {
				*token++ = *p++;
				if (*p == '=')
					*token++ = *p++;
				}
			else if (*p == '=')
				*token++ = *p++;
			break;

		case '%':
			if (*p == ':') {
				*token++ = *p++;
				if (*p == '%' && p[1] == ':') {
					*token++ = *p++;
					*token++ = *p++;
					}
				}
			else if ((c = *p) == '=' || c == '>')
				*token++ = *p++;
			break;

		case '&':
		case '|':
			// can be doubled, or op='ed
			if (*p == c || *p == '=')
				*token++ = *p++;
			break;

		case '*':
		case '^':
		case '=':
		case '!':
			// all these have a op= possibility too
			if (*p == '=')
				*token++ = *p++;
			break;

		case '.':
			c = *p;
			if (c == '*')
				*token++ = *p++;
			else if (c == '.' && p[1] == '.') {
				*token++ = *p++;
				*token++ = *p++;
				}
			else if (c >= '0' && c <= '9')
				goto numberToken;
			break;

		case '{':
		case '}':
		case '[':
		case ']':
		case '(':
		case ')':
		case ';':
		case '?':
		case '~':
		case ',':
			// all these can only be a single character, so the token is done
			break;

		case 0:
			// all done with the whole thing
			--p;	// back it back up so it still points at the null byte
			break;

		default:
			while (((c = *p) >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
			       (c >= '0' && c <= '9') || c == '_') {
				*token++ = *p++;
				}
			break;
		}

	*token = 0;
	tokenEnd = p - text;
}


const char* CPPParser::RestOfText()
{
	return p;
}


char* CPPParser::LastComment()
{
	int commentLen = tokenEnd - whitespaceStart;
	char* str = new char[commentLen + 1];
	strncpy(str, &text[whitespaceStart], commentLen);
	str[commentLen] = 0;	// probly not needed
	return str;
}


void CPPParser::SkipWhitespace()
{
	whitespaceStart = p - text;
	char c;
	while ((c = *p) != 0 && (c == ' ' || c == '\t' || c == '\n' || c == '\r'))
		p++;
}


void CPPParser::GetTokenLocation(int32* start, int32* end)
{
	if (start)
		*start = tokenStart;
	if (end)
		*end = tokenEnd;
}


bool CPPParser::TokenIsReserved(const char* token)
{
	const char* const * keyword = keywords;
	while (*keyword != NULL) {
		if (strcmp(*keyword, token) == 0)
			return true;
		keyword++;
		}
	return false;
}


bool CPPParser::TokenIsComment(const char* token)
{
	return ((token[0] == '/' && (token[1] == '/' || token[1] == '*')) ||
	        (token[0] == ' ' || token[0] == '\t' || token[0] == '\n' ||
	         token[0] == '\r'));
}


