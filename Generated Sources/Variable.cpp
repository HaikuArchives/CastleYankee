/* Variable.cpp */

#include "Variable.h"
#include "Class.h"
#include "TextFile.h"
#include "CPPParser.h"
#include "TokenLookahead.h"
#include "TokenBuilder.h"



Variable::Variable(Class* classIn)
	: ClassElement(classIn), isUglyNotesHack(false)
{
	// default access type for variables is "protected"
	accessType = Protected;
}


string Variable::Name()
{
	return name;
}


string Variable::Code()
{
	return code;
}


void Variable::SetCode(const char* newCode)
{
	// set the code
	code = newCode;

	// find the name
	const char* newName = "";
	isUglyNotesHack = false;
	CPPParser parser(newCode);
	TokenLookahead tokens(&parser);
	while (true) {
		// get the current token and stop if done
		const char* curToken = tokens.CurToken(false);
		if (curToken[0] == 0)
			break;

		// ugly notes hack
		if (curToken[0] == '"') {
			isUglyNotesHack = true;
			char strippedName[256];
			strcpy(strippedName, &curToken[1]);
			strippedName[strlen(strippedName) - 1] = 0;
			newName = strippedName;
			break;
			}

		// get the next token
		const char* nextToken = tokens.NextToken(false);
		if (nextToken[0] == 0)
			break;

		// if the next token is ';', '[', '=', or ')', the current token
		// is the name
		if (nextToken[0] == ';' || nextToken[0] == '[' ||
		    	strcmp(nextToken, "=") == 0 || nextToken[0] == ')') {
		    newName = curToken;
		    break;
		    }

		tokens.ConsumeToken();
		}
	name = newName;

	Dirtify();

	// notify the class
	if (classs)
		classs->VariableNameChanged(this);
}


bool Variable::IsUglyNotesHack()
{
	return isUglyNotesHack;
}


int32 Variable::HSpecLength()
{
	TokenBuilder spec;
	const char* codeStr = code.c_str();	// shouldn't need this, but we do...
	CPPParser parser(codeStr);
	TokenLookahead tokens(&parser);
	for (;; tokens.ConsumeToken()) {
		const char* curToken = tokens.CurToken();
		if (curToken[0] == 0)
			break;

		if (curToken[0] == '(') {
			// it's a function pointer--spec portion is now complete
			break;
			}

		const char* nextToken = tokens.NextToken();

		if (nextToken[0] == ';' || nextToken[0] == '=' || nextToken[0] == '[') {
			// curToken is the variable name--don't add it or anything after
			break;
			}

		// pass thru all other tokens
		spec.AddToken(curToken);
		}
	return spec.Length();
}


void Variable::GenerateH(TextFile* file, int32 specLength)
{
	if (isUglyNotesHack)
		return;

	file->WriteString("\t");

	TokenBuilder line;
	const char* codeStr = code.c_str();	// shouldn't need this, but we do...
	CPPParser parser(codeStr);
	TokenLookahead tokens(&parser);

	// write the spec portion
	for (;; tokens.ConsumeToken()) {
		const char* curToken = tokens.CurToken();
		if (curToken[0] == 0)
			break;

		if (curToken[0] == '(') {
			// it's a function pointer--spec portion is now complete
			break;
			}

		const char* nextToken = tokens.NextToken();

		if (nextToken[0] == ';' || nextToken[0] == '=' || nextToken[0] == '[') {
			// curToken is the variable name--spec portion is now complete
			break;
			}

		// pass thru all other tokens
		line.AddToken(curToken);
		}
	line.PadToLength(specLength);
	line.AddToken("\t");
	line.AddToken(tokens.CurToken());	// add first token after spec
	tokens.ConsumeToken();           	// consume that token

	// add the non-spec portion
	for (;; tokens.ConsumeToken()) {
		const char* token = tokens.CurToken();
		if (token[0] == 0)
			break;

		if (token[0] == ';' || token[0] == '=') {
			// end of the declaration
			line.AddToken(";\n");
			break;
			}

		// pass all other tokens thru
		line.AddToken(token);
		}

	file->WriteString(line.String());
}


void Variable::GenerateCPP(TextFile* file)
{
	if (isUglyNotesHack)
		return;

	bool needsDef = false;
	TokenBuilder line;
	const char* codeStr = code.c_str();	// shouldn't need this, but we do...
	CPPParser parser(codeStr);
	TokenLookahead tokens(&parser);
	for (;; tokens.ConsumeToken()) {
		const char* curToken = tokens.CurToken();
		if (curToken[0] == 0)
			break;

		// omit "static" keyword, but it signals that we need this def
		if (strcmp(curToken, "static") == 0) {
			needsDef = true;
			continue;
			}

		const char* nextToken = tokens.NextToken();
		if (nextToken[0] == 0)
			break;

		// if the next token is ';', '[', or '=', the current token is the name
		if (nextToken[0] == ';' || nextToken[0] == '[' ||
		    	strcmp(nextToken, "=") == 0) {
			// insert Class:: before the method name
			line.AddToken(classs->Name());
			line.AddToken("::");
			line.AddToken(curToken);
			if (nextToken[0] == '=')
				line.AddToken(" =");	// we want that extra space!
			else
				line.AddToken(nextToken);
			tokens.ConsumeToken();	// consume method name
			tokens.ConsumeToken();	// consume ;[=
			break;
			}

		// pass thru other tokens
		else
			line.AddToken(curToken);
		}

	if (needsDef) {
		file->WriteString(line.String());
		file->WriteString(parser.RestOfText());
		file->WriteString("\n");
		}
}


void Variable::Export(TextFile* file)
{
	file->WriteString("<variable");
	WriteAttributes(file);
	file->WriteString("><![CDATA[\n");

	// write the code
	file->WriteString(code);

	// make sure there's a newline at the end
	if (*(code.end() - 1) != '\n')
		file->WriteString("\n");

	file->WriteString("]]></variable>\n");
}


