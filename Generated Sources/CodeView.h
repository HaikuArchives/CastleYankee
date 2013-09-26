/* CodeView.h */

#ifndef _H_CodeView_
#define _H_CodeView_

#include "PrefsChangeClient.h"
#include <string>

#include <TextView.h>

class Code;

class CodeView : public BTextView, public PrefsChangeClient {
public:
	CodeView(BRect frameRect, const char* name);
	inline bool	IsDirty();
	inline void	Undirtify();
	void       	KeyDown(const char *bytes, int32 numBytes);
	void       	MessageReceived(BMessage* message);
	void       	InsertText(const char *text, int32 length, int32 offset, const text_run_array *runs);
	void       	DeleteText(int32 start, int32 finish);
	void       	FindWord(int32 offset, int32 *start, int32 *finish);
	void       	SetTextTo(const char* newText);
	void       	SetTextTo(string newText);
	void       	SetCode(Code* newCode);
	void       	SelectEnd();
	void       	Indent();
	void       	Unindent();
	void       	PrefChanged(const char* prefName);
	void       	ColorSyntax(bool allText = false);
	void       	SelectCurrentLines();

protected:
	bool	dirty;
};

inline bool CodeView::IsDirty()
{
	return dirty;
}



inline void CodeView::Undirtify()
{
	dirty = false;
}




#endif
