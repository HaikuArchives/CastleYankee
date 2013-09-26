/* MessageWind.cpp */

#include "MessageWind.h"

#include <StringView.h>
#include <Screen.h>

static const float WindWidth = 200;
static const float WindHeight = 40;



MessageWind::MessageWind(const char* message)
	: BWindow(BRect(0, 0, WindWidth, WindHeight), "Message", B_MODAL_WINDOW, B_NOT_RESIZABLE)
{
	// center the window
	BRect screenRect = BScreen(this).Frame();
	MoveTo((screenRect.left + screenRect.right - WindWidth) / 2,
	       (screenRect.top + screenRect.bottom - WindHeight) / 2);

	// add the messageView
	messageView = new BStringView(BRect(0, 0, WindWidth, WindHeight), "Message View",
	                              message);
	messageView->SetAlignment(B_ALIGN_CENTER);
	messageView->SetFontSize(24);
	AddChild(messageView);

	Show();
	Lock();
	UpdateIfNeeded();
	Flush();
	Unlock();
}


