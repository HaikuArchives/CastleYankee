/* ClassesView.h */

#ifndef _H_ClassesView_
#define _H_ClassesView_

#include "ScrolledListView.h"

class Project;

class ClassesView : public ScrolledListView {
public:
	ClassesView(BRect rect, Project* projectIn);
	int32	NumItems();
	float	ItemHeight();
	void 	DrawItem(int32 itemIndex, BRect itemRect, bool selected);
	void 	OpenSelectedItem(int32 selectedIndex);
	void 	RemoveSelectedItem(int32 selectedIndex);
	bool 	CanRearrange();
	void 	ItemMoved(int32 oldIndex, int32 newIndex);

protected:
	Project*	project;
};


#endif
