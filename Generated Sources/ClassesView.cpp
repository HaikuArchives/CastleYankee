/* ClassesView.cpp */

#include "ClassesView.h"
#include "Project.h"
#include "string_slice.h"

#include <Alert.h>

// tweex
static const float ClassItemHeight = 12;
static const float Baseline = 2;
static const float NameX = 4;



ClassesView::ClassesView(BRect rect, Project* projectIn)
	: ScrolledListView(rect, NULL, B_FOLLOW_ALL_SIDES,
	                   B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	  project(projectIn)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}


int32 ClassesView::NumItems()
{
	return project->NumClasses();
}


float ClassesView::ItemHeight()
{
	return ClassItemHeight;
}


void ClassesView::DrawItem(int32 itemIndex, BRect itemRect, bool selected)
{
	static const rgb_color blackColor = { 0, 0, 0, 255 };
	static const rgb_color whiteColor = { 255, 255, 255, 255 };
	static const rgb_color redColor = { 255, 0, 0, 255 };
	static const rgb_color greyColor = { 64, 64, 64, 255 };

	// hilite setup
	if (selected) {
		SetLowColor(blackColor);
		SetHighColor(whiteColor);
		}
	else {
		SetLowColor(whiteColor);
		SetHighColor(blackColor);
		}

	// clear background
	FillRect(itemRect, B_SOLID_LOW);

	// draw name
	string className = project->ClassNameAt(itemIndex);
	string_slice classNameStr(className.begin(), className.end());
	BFont font(be_plain_font);
	bool isDivider = false;
	if (className.empty()) {
		classNameStr = "Unnamed Class";
		font_family fontFamily;
		font_style fontStyle;
		font.GetFamilyAndStyle(&fontFamily, &fontStyle);
		strcpy(fontStyle, "Italic");
		font.SetFamilyAndStyle(fontFamily, fontStyle);
		SetHighColor(redColor);
		}
	else if (className[0] == '"') {
		// trim quotes
		classNameStr = classNameStr.substr(1, classNameStr.length() - 2);

		// special font
		font.SetFace(B_ITALIC_FACE);
		SetHighColor(greyColor);
		isDivider = true;
		}
	SetFont(&font, B_FONT_FAMILY_AND_STYLE);
	MovePenTo(NameX, itemRect.bottom - Baseline);
	DrawString(classNameStr.begin(), classNameStr.length());

	// divider
	if (isDivider) {
		MovePenBy(2, 0);
		StrokeLine(BPoint(itemRect.right - 12, itemRect.bottom - Baseline));
		}

	// hilite cleanup
	SetLowColor(whiteColor);
	SetHighColor(blackColor);
}


void ClassesView::OpenSelectedItem(int32 selectedIndex)
{
	project->OpenClassAt(selectedIndex);
}


void ClassesView::RemoveSelectedItem(int32 selectedIndex)
{
	// confirm
	BAlert* alert =
		new BAlert("", "Are you sure you want to permanently remove this class?",
		           "Delete", "Cancel", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(1, B_ESCAPE);
	if (alert->Go() != 0)
		return;

	// remove
	project->RemoveClassAt(selectedIndex);
	NumItemsChanged();
	Select(-1);
}


bool ClassesView::CanRearrange()
{
	return true;
}


void ClassesView::ItemMoved(int32 oldIndex, int32 newIndex)
{
	project->MoveClass(oldIndex, newIndex);
}


