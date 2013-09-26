/* Method.cpp */

#include "Method.h"
#include "Code.h"
#include "Class.h"
#include "CPPParser.h"
#include "TokenLookahead.h"
#include "TokenBuilder.h"
#include "TextFile.h"



Method::Method(Class* classIn)
	: ClassElement(classIn), code(new Code),
	  isInline(false), isPureVirtual(false), isUglyNotesHack(false)
{
	// default access type for methods is "public"
	accessType = Public;
}


Method::~Method()
{
	delete code;
}


string Method::Signature()
{
	return signature;
}


Code* Method::GetCode()
{
	return code;
}


void Method::SetCode(string newCode)
{
	code->SetCode(newCode);
	CodeChanged();
}


void Method::SetCodeFromTextView(BTextView* view)
{
	code->SetFromTextView(view);
	CodeChanged();
}


bool Method::IsInline()
{
	return isInline;
}


bool Method::IsUglyNotesHack()
{
	return isUglyNotesHack;
}


int32 Method::HSpecLength()
{
	CPPParser parser(code->GetCode());
	TokenLookahead tokens(&parser);
	TokenBuilder builder;
	for (;; tokens.ConsumeToken()) {
		const char* curToken = tokens.CurToken(false);
		if (curToken[0] == 0)
			break;

		if (curToken[0] == '~') {
			// found the method name--it's a destructor
			break;
			}

		else if (strcmp(curToken, "operator") == 0) {
			// found the method name--it's an operator
			break;
			}

		const char* nextToken = tokens.NextToken(false);
		if (nextToken[0] == 0)
			break;

		if (nextToken[0] == '(') {
			// found the method name
			break;
			}

		// all other tokens pass thru
		builder.AddToken(curToken);
		}
	return builder.Length();
}


void Method::GenerateH(TextFile* file, int32 specLength)
{
	if (isUglyNotesHack)
		return;

	string line = "\t";
	CPPParser parser(code->GetCode());

	// handle initial tokens (up to "(")
	TokenBuilder builder;
	TokenLookahead tokens(&parser);
	bool isCtor = true;		// until we determine otherwise
	for (;; tokens.ConsumeToken()) {
		const char* curToken = tokens.CurToken(false);
		if (curToken[0] == 0)
			break;

		if (curToken[0] == '~') {
			// found the method name--it's a destructor
			builder.AddToken(curToken);				// add the '~'
			tokens.ConsumeToken();					// consume the "~"
			builder.AddToken(tokens.CurToken());	// add the method name
			tokens.ConsumeToken();					// consume the method name
			builder.AddToken(tokens.CurToken());	// add the '('
			tokens.ConsumeToken();					// consume the '('
			break;
			}

		else if (strcmp(curToken, "operator") == 0) {
			// found the method name--it's an operator
			builder.PadToLength(specLength);
			builder.AddToken("\t");
			builder.AddToken(curToken);				// add "operator"
			tokens.ConsumeToken();					// consume "operator"
			const char* op = tokens.CurToken();
			bool doubleTokenOp = (op[0] == '[' || op[0] == '(');
			builder.AddToken(op);					// add the operator
			tokens.ConsumeToken();					// consume the operator
			if (doubleTokenOp) {
				// these include the next token as part of the operator name
				builder.AddToken(tokens.CurToken());
				tokens.ConsumeToken();
				}
			builder.AddToken(tokens.CurToken());	// add the '('
			tokens.ConsumeToken();					// consume the '('
			break;
			}

		const char* nextToken = tokens.NextToken(false);
		if (nextToken[0] == '(') {
			// found the method name
			if (!isCtor) {
				builder.PadToLength(specLength);
				builder.AddToken("\t");
				}
			builder.AddToken(curToken); 	// add the method name
			tokens.ConsumeToken();      	// consume the method name
			builder.AddToken(nextToken);	// add the '('
			tokens.ConsumeToken();      	// consume the '('
			break;
			}

		// pass thru other tokens
		builder.AddToken(curToken);
		isCtor = false;		// can't be a ctor
		}
	line += builder.String();

	// write the args
	const char *argsStr = parser.RestOfText();
	uint32 parenLevel = 0;
	while (*argsStr) {
		char c = *argsStr++;
		if (c == 0)
			break;
		line += c;
		if (c == ')') {
			if (parenLevel == 0)
				break;
			else
				--parenLevel;
			}
		else if (c == '(')
			parenLevel++;
		}

	// check for "const"
	CPPParser constParser(argsStr);
	TokenLookahead constTokens(&constParser);	// don't really need to lookahead, but it allocates a buffer for the token...
	if (strcmp(constTokens.CurToken(false), "const") == 0)
		line += " const";

	// handle pure virtual
	if (isPureVirtual)
		line += " = 0";

	// finish it
	line += ";\n";

	file->WriteString(line);
}


void Method::GenerateCPP(TextFile* file)
{
	if (isPureVirtual || isUglyNotesHack)
		return;

	CPPParser parser(code->GetCode());
	TokenLookahead tokens(&parser);

	// copy up to the method name
	TokenBuilder specBuilder;
	for (;; tokens.ConsumeToken()) {
		// get the next token
		const char* curToken = tokens.CurToken(true);
		if (curToken[0] == 0)
			break;
		if (CPPParser::TokenIsComment(curToken)) {
			specBuilder.AddToken(tokens.CurComment());
			continue;
			}

		// ignore certain keywords
		if (strcmp(curToken, "virtual") == 0 ||
		    strcmp(curToken, "friend") == 0 ||
		    strcmp(curToken, "static") == 0)
		    	continue;

		// "~" signals start of destructor name
		if (curToken[0] == '~') {
			specBuilder.AddToken(classs->Name());
			specBuilder.AddToken("::~");
			tokens.ConsumeToken();	// consume the '~' before we break
			break;
			}

		// handle "operator"
		if (strcmp(curToken, "operator") == 0) {
			// write "<class.name>::operator"
			specBuilder.AddToken(classs->Name());
			specBuilder.AddToken("::");
			specBuilder.AddToken("operator");
			// write the actual operator
			tokens.ConsumeToken();	// consume "operator"
			const char* op = tokens.CurToken(false);
			bool doubleTokenOp = (op[0] == '[' || op[0] == '(');
			specBuilder.AddToken(op);
			tokens.ConsumeToken();	// consume the operator
			if (doubleTokenOp) {
				// these include the next token as part of the operator name
				specBuilder.AddToken(tokens.CurToken(false));
				tokens.ConsumeToken();
				}
			// write the '('
			specBuilder.AddToken("(");
			tokens.ConsumeToken();	// consume the '(' (it had better be that!)
			break;
			}

		// get the next token
		const char* nextToken = tokens.NextToken(false);

		// method name is last token before the '('
		if (nextToken[0] == '(') {
			specBuilder.AddToken(classs->Name());
			specBuilder.AddToken("::");
			specBuilder.AddToken(curToken);
			specBuilder.AddToken("(");
			tokens.ConsumeToken();	// consume the method name
			tokens.ConsumeToken();	// consume the '(' too before we break
			break;
			}

		// add all other tokens
		else
			specBuilder.AddToken(curToken);
		}
	file->WriteString(specBuilder.String());

	// write the args, stripping defaults
	specBuilder.Clear();
	for (;; tokens.ConsumeToken()) {
		const char* token = tokens.CurToken();
		if (token[0] == 0)
			break;

		// check for end
		if (token[0] == ')') {
			specBuilder.AddToken(token);
			break;
			}

		// strip default arguments
		else if (token[0] == '=') {
			int32 parenLevel = 0;
			bool endedArgs = false;
			tokens.ConsumeToken();
			for (;; tokens.ConsumeToken()) {
				token = tokens.CurToken();
				if (token[0] == 0)
					break;	// ran out of text...shouldn't happen

				if (token[0] == ',' && parenLevel == 0) {
					// end of argument
					specBuilder.AddToken(token);
					break;
					}
				else if (token[0] == '(')
					parenLevel += 1;
				else if (token[0] == ')') {
					if (parenLevel == 0) {
						// end of all arguments
						specBuilder.AddToken(token);
						endedArgs = true;
						break;
						}
					else
						parenLevel -= 1;
					}
				}
			if (endedArgs) {
				// need to break out of the args "for" loop
				break;
				}
			}

		// normal tokens--pass thru
		else
			specBuilder.AddToken(token);
		}
	file->WriteString(specBuilder.String());

	// copy the method body
	const char* methodBody = parser.RestOfText();
	file->WriteString(methodBody);

	// make sure there are three newlines at the end
	const char* p = methodBody;
	while (*p)
		p++;		// run it to the end
	int32 numNewlines = 0;
	while (*--p == '\n')
		numNewlines++;
	char newLines[16];
	char* pp = newLines;
	while (numNewlines < 3) {
		*pp++ = '\n';
		numNewlines++;
		}
	*pp = 0;
	file->WriteString(newLines);
}


void Method::Export(TextFile* file)
{
	// write the start tag
	file->WriteString("<method");
	WriteAttributes(file);
	file->WriteString("><![CDATA[\n");

	// write the code
	string codeText = code->GetCode();
	file->WriteString(codeText);

	// make sure there's a newline at the end
	const char* p = codeText.end() - 1;
	if (*p != '\n')
		file->WriteString("\n");

	file->WriteString("]]></method>\n");
}


void Method::CodeChanged()
{
	// recalculate the signature
	TokenBuilder sigBuilder;
	CPPParser parser(code->GetCode());
	TokenLookahead tokens(&parser);
	isInline = isPureVirtual = isUglyNotesHack = false;	// until we determine otherwise
	// find the method name
	for (;; tokens.ConsumeToken()) {
		// get the next two tokens, quitting if at end
		const char* curToken = tokens.CurToken(false);
		if (curToken[0] == 0)
			break;
		const char* nextToken = tokens.NextToken(false);

		// ugly notes hack
		if (curToken[0] == '"') {
			// strip quotes and add it directly without param list
			string topicName(&curToken[1]);	// strip initial quote...
			topicName.resize(topicName.size() - 1);	// strip final quote
			sigBuilder.AddToken(topicName.c_str());
			tokens.ConsumeToken();	// consume the string
			isUglyNotesHack = true;
			break;
			}

		// method name is last token before the '('
		if (nextToken[0] == '(') {
			sigBuilder.AddToken(curToken);
			sigBuilder.AddToken("(");
			tokens.ConsumeToken();	// consume the method name
			tokens.ConsumeToken();	// consume the '(' too
			break;
			}

		// destructor looks a little different, starting with '~'
		else if (curToken[0] == '~') {
			sigBuilder.AddToken(curToken);
			sigBuilder.AddToken(nextToken);
			sigBuilder.AddToken("(");
			tokens.ConsumeToken();	// consume the '~'
			tokens.ConsumeToken();	// consume the method name
			tokens.ConsumeToken();	// consume '(' that inevitably follows
			break;
			}

		// handle operator
		else if (strcmp(curToken, "operator") == 0) {
			sigBuilder.AddToken(curToken);
			sigBuilder.AddToken(nextToken);
			bool doubleTokenOp = (nextToken[0] == '[' || nextToken[0] == ')');
			tokens.ConsumeToken();	// consume "operator"
			tokens.ConsumeToken();	// consume the operator
			if (doubleTokenOp) {
				sigBuilder.AddToken(tokens.CurToken(false));
				tokens.ConsumeToken();
				}
			sigBuilder.AddToken("(");
			tokens.ConsumeToken();	// consume the '(' that inevitably follows
			break;
			}

		else if (strcmp(curToken, "inline") == 0)
			isInline = true;
		}
	// add in the arguments
	int32 parenLevel = 0;
	if (!isUglyNotesHack) {
		for (;; tokens.ConsumeToken()) {
			const char* token = tokens.CurToken(false);
			if (token[0] == 0 || (token[0] == ')' && parenLevel == 0)) {
				sigBuilder.AddToken(")");
				tokens.ConsumeToken();
				break;
				}
			else {
				if (token[0] == '(')
					parenLevel += 1;
				else if (token[0] == ')')
					parenLevel -= 1;
				sigBuilder.AddToken(token);
				}
			}
		}
	// skip "const"
	if (strcmp(tokens.CurToken(false), "const") == 0)
		tokens.ConsumeToken();
	// check for pure virtual
	if (strcmp(tokens.CurToken(false), "=") == 0 && strcmp(tokens.NextToken(false), "0") == 0)
		isPureVirtual = true;

	// change the signature
	signature = sigBuilder.String();

	Dirtify();

	// notify the class
	if (classs)
		classs->MethodSignatureChanged(this);
}


