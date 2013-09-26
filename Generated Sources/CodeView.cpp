/* CodeView.cpp */

#include "CodeView.h"
#include "Code.h"
#include "CPPParser.h"
#include "TextRunBuilder.h"
#include "Prefs.h"

#include <Window.h>
#include <Message.h>

// tweex
static const float MaxTextWidth = 2000;

// inline helper functions
inline bool IsCIdentChar(int c) { return (isalnum(c) || c == '_'); }



CodeView::CodeView(BRect frameRect, const char* name)
	: BTextView(frameRect, name, frameRect, B_FOLLOW_ALL_SIDES,
	            B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS),
	  dirty(false)
{
	SetFontAndColor(be_fixed_font);
	SetWordWrap(false);
	SetAutoindent(true);
	SetStylable(true);
	SetDoesUndo(true);
	BRect textRect = frameRect;
	textRect.OffsetTo(2, 0);
	textRect.right = MaxTextWidth;
	SetTextRect(textRect);
}


void CodeView::KeyDown(const char* bytes, int32 numBytes)
{
	BMessage*	message;
	int32    	modifiers;
	int32    	newSelection;
	int32    	oldStart, oldEnd;
	char     	c;

	bool passKey = true;
	bool recolorsSyntax = true;

	switch (bytes[0]) {
		case B_ENTER:
			message = Window()->CurrentMessage();
			modifiers = message->FindInt32("modifiers");
			if (modifiers & (B_OPTION_KEY | B_CONTROL_KEY)) {
				// non-indenting return
				Delete();
				Insert("\n");
				passKey = false;
				}
			break;

		case B_TAB:
			message = Window()->CurrentMessage(); 
			modifiers = message->FindInt32("modifiers");
			if (modifiers & B_CONTROL_KEY ) {
				// switch focus
				BView::KeyDown(bytes, numBytes);
				passKey = false;
				recolorsSyntax = false;
				}
			break;

		case B_LEFT_ARROW:
			message = Window()->CurrentMessage(); 
			modifiers = message->FindInt32("modifiers");
			GetSelection(&oldStart, &oldEnd);
			if (modifiers & B_COMMAND_KEY) {
				// beginning of line
				newSelection = OffsetAt(LineAt(oldStart));
				passKey = false;
				}
			else if (modifiers & B_OPTION_KEY) {
				// beginning of word
				int32 wordStart, wordEnd;
				FindWord(oldStart, &wordStart, &wordEnd);
				if (oldStart == wordStart || wordStart == wordEnd) {
					// we started at the beginning of a word, and we're still there,
					// OR we're at a non-word.
					// just go back until we find the previous word
					do {
						FindWord(wordStart - 1, &wordStart, &wordEnd);
					} while (wordStart == wordEnd && wordStart > 0);
					}
				newSelection = wordStart;
				passKey = false;
				}
			if (!passKey) {
				// we're gonna do it
				if (modifiers & B_SHIFT_KEY) {
					// extend selection
					Select(newSelection, oldEnd);
					}
				else {
					// move selection
					Select(newSelection, newSelection);
					}
				}
			recolorsSyntax = false;
			break;

		case B_RIGHT_ARROW:
			message = Window()->CurrentMessage(); 
			modifiers = message->FindInt32("modifiers");
			GetSelection(&oldStart, &oldEnd);
			if (modifiers & B_COMMAND_KEY) {
				// end of line
				int32 line = LineAt(oldEnd);
				if (line >= CountLines() - 1) {
					// last line needs special treatment
					newSelection = TextLength();
					}
				else
					newSelection = OffsetAt(line + 1) - 1;
				passKey = false;
				}
			else if (modifiers & B_OPTION_KEY) {
				// end of word
				int32 wordStart, wordEnd;
				FindWord(oldEnd, &wordStart, &wordEnd);
				if (oldEnd == wordEnd || wordStart == wordEnd) {
					// we started at the end of a word, and we're still there
					// OR we're at a non-word.
					// just go forward until we find the next word
					do {
						FindWord(wordEnd + 1, &wordStart, &wordEnd);
					} while (wordStart == wordEnd && wordEnd < TextLength());
					}
				newSelection = wordEnd;
				passKey = false;
				}
			if (!passKey) {
				// we're gonna do it
				if (modifiers & B_SHIFT_KEY) {
					// extend selection
					Select(oldStart, newSelection);
					}
				else {
					// move selection
					Select(newSelection, newSelection);
					}
				}
			recolorsSyntax = false;
			break;

		case B_UP_ARROW:
		case B_DOWN_ARROW:
		case B_PAGE_UP:
		case B_PAGE_DOWN:
		case B_HOME:
		case B_END:
			recolorsSyntax = false;
			break;

		case '{':
			// auto insert of block template
			if (!Prefs()->GetBoolPref("autoBlock", true))
				break;
			// get current indent
			string indent = "";
			GetSelection(&oldStart, &oldEnd);
			int32 offset = OffsetAt(LineAt(oldStart));
			while ((c = ByteAt(offset)) == '\t' || c == ' ') {
				indent += c;
				offset++;
				}
			indent += '\t';		// indent one more
			// create template
			string blockTemplate = "{\n";
			blockTemplate += indent;
			blockTemplate += "\n";
			blockTemplate += indent;
			blockTemplate += "}";
			// insert template
			Delete();
			Insert(blockTemplate.data(), blockTemplate.length());
			// select interior of block
			newSelection = oldStart + 2 + indent.length();
			Select(newSelection, newSelection);
			passKey = false;
			break;
		}

	if (passKey)
		BTextView::KeyDown(bytes, numBytes);

	if (recolorsSyntax)
		ColorSyntax();
}


void CodeView::MessageReceived(BMessage* message)
{
	bool recolorsSyntax = false;
	switch (message->what) {
		case B_CUT:
		case B_PASTE:
		case B_UNDO:
			recolorsSyntax = true;
			break;
		default:
			if (message->WasDropped())
				recolorsSyntax = true;
			break;
		}

	BTextView::MessageReceived(message);

	if (recolorsSyntax)
		ColorSyntax(true);
}


void CodeView::InsertText(const char* text, int32 length, int32 offset, const text_run_array* runs)
{
	BTextView::InsertText(text, length, offset, runs);
	dirty = true;
}


void CodeView::DeleteText(int32 start, int32 finish)
{
	BTextView::DeleteText(start, finish);
	dirty = true;
}


void CodeView::FindWord(int32 offset, int32* start, int32* finish)
{
	*start = offset;	// default: no selection
	*finish = offset;
	if (!IsCIdentChar(ByteAt(offset)))
		return;

	// search backwards
	int32 backOffset;
	for (backOffset = offset - 1;
	     backOffset >= 0 && IsCIdentChar(ByteAt(backOffset));
	     --backOffset) ;
	*start = backOffset + 1;

	// search forwards
	int32 length = TextLength();
	int32 forwardOffset;
	for (forwardOffset = offset + 1;
	     forwardOffset < length && IsCIdentChar(ByteAt(forwardOffset));
	     ++forwardOffset) ;
	*finish = forwardOffset;
}


void CodeView::SetTextTo(const char* newText)
{
	SetText(newText);
	ColorSyntax(true);
	dirty = false;
}


void CodeView::SetTextTo(string newText)
{
	SetText(newText.data(), newText.length());
	ColorSyntax(true);
	dirty = false;
}


void CodeView::SetCode(Code* newCode)
{
	string code = newCode->GetCode();
	SetText(code.data(), code.length());
	int32 selStart, selEnd;
	newCode->GetSelection(&selStart, &selEnd);
	Select(selStart, selEnd);
	ScrollToSelection();

	ColorSyntax(true);
	dirty = false;
}


void CodeView::SelectEnd()
{
	int32 length = TextLength();
	Select(length, length);
}


void CodeView::Indent()
{
	SelectCurrentLines();
	int32 selStart, selEnd;
	GetSelection(&selStart, &selEnd);
	int32 offset = selStart;
	do {
		// add the tab, but only if the line isn't blank
		if (ByteAt(offset) != '\n') {
			Insert(offset, "\t", 1);
			offset += 1;	// account for inserted tab
			selEnd += 1;	// ditto
			}
		// find the start of the next line
		while (offset < selEnd && ByteAt(offset) != '\n')
			++offset;
		offset += 1;	// skip past the '\n'
	} while (offset < selEnd);
	Select(selStart, selEnd);
}


void CodeView::Unindent()
{
	SelectCurrentLines();
	int32 selStart, selEnd;
	GetSelection(&selStart, &selEnd);
	int32 offset = selStart;
	do {
		// remove the tab or space, but only if the line isn't blank
		char c = ByteAt(offset);
		if (c == '\t' || c == ' ') {
			Delete(offset, offset + 1);
			selEnd -= 1;	// account for deleted character
			}
		// find the start of the next line
		while (offset < selEnd && ByteAt(offset) != '\n')
			++offset;
		offset += 1;	// skip past the '\n'
	} while (offset < selEnd);
	Select(selStart, selEnd);
}


void CodeView::PrefChanged(const char* prefName)
{
	if (strcmp(prefName, "colorSyntax") == 0) {
		Window()->Lock();
		if (Prefs()->GetBoolPref("colorSyntax"))
			ColorSyntax(true);
		else {
			// uncolor
			text_run_array runArray;
			runArray.count = 1;
			runArray.runs[0].offset = 0;
			runArray.runs[0].font = be_fixed_font;
			rgb_color blackColor = { 0, 0, 0, 255 };
			runArray.runs[0].color = blackColor;
			SetRunArray(0, TextLength(), &runArray);
			}
		Window()->Unlock();
		}
}


void CodeView::ColorSyntax(bool allText)	// just from selection on if !allText
{
	int32 tokenStart, tokenEnd;

	static const rgb_color blackColor = { 0, 0, 0, 255 };
	static const rgb_color redColor = { 255, 0, 0, 255 };
	static const rgb_color blueColor = { 0, 0, 255, 255 };
	static const rgb_color greyColor = { 128, 128, 128, 255 };

	if (!Prefs()->GetBoolPref("colorSyntax", true))
		return;

	int32 startChar = 0;
	if (!allText) {
		// update from current selection on
		int32 selectionEnd;
		GetSelection(&startChar, &selectionEnd);
		// because "reinterpret_cast" is 16 characters, back up 16
		startChar -= 16;
		if (startChar < 0)
			startChar = 0;
		}

	char* token = new char[2048];
	TextRunBuilder runBuilder;
	CPPParser parser(Text());
	const rgb_color* lastColor = NULL;
	bool isUglyNotesHack = false;
	while (true) {
		parser.GetToken(token);
		if (token[0] == 0)
			break;	// all done
		parser.GetTokenLocation(&tokenStart, &tokenEnd);

		// token color
		const rgb_color* tokenColor = &blackColor;
		if (CPPParser::TokenIsComment(token))
			tokenColor = &redColor;
		else if (CPPParser::TokenIsReserved(token) || token[0] == '#')
			tokenColor = &blueColor;
		else if (token[0] == '"') {
			tokenColor = &greyColor;
			if (tokenStart == 0)
				isUglyNotesHack = true;
			}
		if (tokenColor != lastColor) {
			// add a new run--but only if we've reached the part we're updating
			if (tokenEnd >= startChar) {
				tokenStart -= startChar;
				if (tokenStart < 0)
					tokenStart = 0;
				runBuilder.AddRun(tokenStart, be_fixed_font, *tokenColor);
				lastColor = tokenColor;
				}
			}

		//** part of ugly hack for taking notes
		if (isUglyNotesHack) {
			runBuilder.AddRun(tokenEnd, be_fixed_font, blackColor);
			break;
			}
		}
	delete token;
	SetRunArray(startChar, TextLength(), runBuilder.RunArray());
	Window()->UpdateIfNeeded();		// hmm, didn't help
}


void CodeView::SelectCurrentLines()
{
	// extends current selection to line start and line end

	int32 selStart, selEnd;
	GetSelection(&selStart, &selEnd);

	// search backward for line start
	for (selStart -= 1; selStart > 0 && ByteAt(selStart) != '\n'; --selStart) ;
	selStart += 1;

	// search forward for line end (if we're not there already)
	if (selEnd > 0 && ByteAt(selEnd - 1) != '\n') {
		int32 length = TextLength();
		for (; selEnd < length && ByteAt(selEnd) != '\n'; ++selEnd) ;
		selEnd += 1;
		}

	Select(selStart, selEnd);
}


