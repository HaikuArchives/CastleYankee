/* VariablesView.cpp */

#include "VariablesView.h"
#include "Class.h"
#include "ClassWind.h"

#include <Alert.h>

// tweex
static const float VariableItemHeight = 12;
static const float Baseline = 2;
static const float NameX = 4;
static const float VariableIndent = 6;



VariablesView::VariablesView(BRect rect, Class* classIn)
	: ScrolledListView(rect, NULL, B_FOLLOW_ALL_SIDES,
	                   B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	  theClass(classIn)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}


int32 VariablesView::NumItems()
{
	return theClass->NumVariables();
}


float VariablesView::ItemHeight()
{
	return VariableItemHeight;
}


void VariablesView::DrawItem(int32 itemIndex, BRect itemRect, bool selected)
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
	string varName = theClass->VariableNameAt(itemIndex);
	BFont font(be_plain_font);
	bool italicize = false;
	bool isDivider = theClass->VariableIsUglyNotesHackAt(itemIndex);
	if (isDivider) {
		SetHighColor(darkGreyColor);
		italicize = true;
		}
	else if (varName.empty()) {
		varName = "Unnamed Variable";
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
	MovePenTo(NameX + (isDivider ? 0 : VariableIndent), itemRect.bottom - Baseline);
	DrawString(varName.begin(), varName.length());

	// hilite cleanup
	SetLowColor(whiteColor);
	SetHighColor(blackColor);
}


void VariablesView::RemoveSelectedItem(int32 selectedIndex)
{
	// confirm
	BAlert* alert =
		new BAlert("", "Are you sure you want to permanently remove this variable?",
		           "Delete", "Cancel", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(1, B_ESCAPE);
	alert->SetShortcut(0, 'd');
	if (alert->Go() != 0)
		return;

	// remove
	theClass->RemoveVariableAt(selectedIndex);
	NumItemsChanged();
	((ClassWind*) Window())->SelectionRemoved();
	Select(-1);
}


void VariablesView::SelectionChanging(int32 newSelection, int32 oldSelection)
{
	((ClassWind*) Window())->VariableSelected(newSelection);
}


void VariablesView::ActivationChanging(bool toActive)
{
	if (toActive)
		((ClassWind*) Window())->VariableSelected(Selection());
}


bool VariablesView::CanRearrange()
{
	return true;
}


void VariablesView::ItemMoved(int32 oldIndex, int32 newIndex)
{
	theClass->MoveVariable(oldIndex, newIndex);
}


void VariablesView::MakeFocus(bool focused)
{
	ScrolledListView::MakeFocus(focused);
	Invalidate();
}


