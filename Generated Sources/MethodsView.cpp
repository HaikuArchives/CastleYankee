/* MethodsView.cpp */

#include "MethodsView.h"
#include "Class.h"
#include "ClassWind.h"

#include <Alert.h>

// tweex
static const float MethodItemHeight = 12;
static const float Baseline = 2;
static const float NameX = 4;
static const float MethodIndent = 6;



MethodsView::MethodsView(BRect rect, Class* classIn)
	: ScrolledListView(rect, NULL, B_FOLLOW_ALL_SIDES,
	                   B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	  theClass(classIn)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}


int32 MethodsView::NumItems()
{
	return theClass->NumMethods();
}


float MethodsView::ItemHeight()
{
	return MethodItemHeight;
}


void MethodsView::DrawItem(int32 itemIndex, BRect itemRect, bool selected)
{
	static const rgb_color blackColor = { 0, 0, 0, 255 };
	static const rgb_color whiteColor = { 255, 255, 255, 255 };
	static const rgb_color greyColor = { 192, 192, 192, 255 };
	static const rgb_color darkGreyColor = { 128, 128, 128, 255 };
	static const rgb_color redColor = { 255, 0, 0, 255 };

	// hilite setup
	if (selected && IsActive()) {
		if (IsFocus()) {
			SetLowColor(blackColor);
			SetHighColor(whiteColor);
			}
		else {
			SetLowColor(greyColor);
			SetHighColor(blackColor);
			}
		}
	else {
		SetLowColor(whiteColor);
		SetHighColor(blackColor);
		}

	// clear background
	FillRect(itemRect, B_SOLID_LOW);

	// draw name
	string signature = theClass->MethodSignatureAt(itemIndex);
	bool italicize = false;
	BFont font(be_plain_font);
	bool isDivider = theClass->MethodIsUglyNotesHackAt(itemIndex);
	if (isDivider) {
		SetHighColor(darkGreyColor);
		italicize = true;
		}
	else if (signature.empty()) {
		signature = "Unnamed Method";
		SetHighColor(redColor);
		italicize = true;
		}
	if (italicize) {
		font_family fontFamily;
		font_style fontStyle;
		font.GetFamilyAndStyle(&fontFamily, &fontStyle);
		strcpy(fontStyle, "Italic");
		font.SetFamilyAndStyle(fontFamily, fontStyle);
		}
	SetFont(&font, B_FONT_FAMILY_AND_STYLE);
	MovePenTo(NameX + (isDivider ? 0 : MethodIndent), itemRect.bottom - Baseline);
	const char* sigStr = signature.c_str();
	const char* argsStr = strchr(sigStr, '(');
	if (argsStr) {
		DrawString(sigStr, argsStr - sigStr);
		SetHighColor(darkGreyColor);
		DrawString(argsStr);
		}
	else
		DrawString(sigStr);

	// divider
	if (isDivider) {
		MovePenBy(2, 0);
		StrokeLine(PenLocation() + BPoint(200, 0));
		}

	// hilite cleanup
	SetLowColor(whiteColor);
	SetHighColor(blackColor);
}


void MethodsView::RemoveSelectedItem(int32 selectedIndex)
{
	// confirm
	BAlert* alert =
		new BAlert("", "Are you sure you want to permanently remove this method?",
		           "Delete", "Cancel", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(1, B_ESCAPE);
	alert->SetShortcut(0, 'd');
	if (alert->Go() != 0)
		return;

	// remove
	theClass->RemoveMethodAt(selectedIndex);
	NumItemsChanged();
	((ClassWind*) Window())->SelectionRemoved();
	Select(-1);
}


void MethodsView::SelectionChanging(int32 newSelection, int32 oldSelection)
{
	((ClassWind*) Window())->MethodSelected(newSelection);
}


void MethodsView::ActivationChanging(bool toActive)
{
	if (toActive)
		((ClassWind*) Window())->MethodSelected(Selection());
}


bool MethodsView::CanRearrange()
{
	return true;
}


void MethodsView::ItemMoved(int32 oldIndex, int32 newIndex)
{
	theClass->MoveMethod(oldIndex, newIndex);
}


void MethodsView::MakeFocus(bool focused)
{
	ScrolledListView::MakeFocus(focused);
	Invalidate();
}


