/* InfoView.cpp */

#include "InfoView.h"
#include "ClassWind.h"

const rgb_color InfoView::bgndColor = { 226, 226, 226, 255 };


InfoView::InfoView(BRect frame)
	: BView(frame, "InfoView", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	SetViewColor(bgndColor);
}


void InfoView::Draw(BRect updateRect)
{
	SetLowColor(bgndColor);
	FillRect(updateRect, B_SOLID_LOW);
}


void InfoView::MouseDown(BPoint point)
{
	float startY = point.y;
	uint32 buttons = 1;
	while (buttons) {
		// see if it moved
		if (point.y != startY)
			((ClassWind*) Window())->ResizePanes(point.y - startY);

		// go around again
		snooze(20000);	// BeBook-recommended snooze time
		GetMouse(&point, &buttons, true);
		}
}


