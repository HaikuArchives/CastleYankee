/* MethodsView.h */

#ifndef _H_MethodsView_
#define _H_MethodsView_

#include "ScrolledListView.h"

class Class;

class MethodsView : public ScrolledListView {
public:
	MethodsView(BRect rect, Class* classIn);
	int32	NumItems();
	float	ItemHeight();
	void 	DrawItem(int32 itemIndex, BRect itemRect, bool selected);
	void 	RemoveSelectedItem(int32 selectedIndex);
	void 	SelectionChanging(int32 newSelection, int32 oldSelection);
	void 	ActivationChanging(bool toActive);
	bool 	CanRearrange();
	void 	ItemMoved(int32 oldIndex, int32 newIndex);
	void 	MakeFocus(bool focused = true);

protected:
	Class*	theClass;
};


#endif
