/* ScrolledListView.h */

#ifndef _H_ScrolledListView_
#define _H_ScrolledListView_

#include <View.h>

class Project;

class ScrolledListView : public BView {
public:
	ScrolledListView(BRect frame, const char *name,
	                 uint32 resizingMode, uint32 flags);

	// to be overridden by subclasses
	virtual int32	NumItems() = 0;
	virtual float	ItemHeight() = 0;	// real height *in pixels*.
	virtual void 	DrawItem(int32 itemIndex, BRect itemRect, bool selected) = 0;

	// hook methods
	virtual void	OpenSelectedItem(int32 selectedIndex) {}
	virtual void	RemoveSelectedItem(int32 selectedIndex) {}
	virtual void	SelectionChanging(int32 newSelection, int32 oldSelection) {}
	virtual void	ActivationChanging(bool toActive) {}
	virtual bool	CanRearrange() { return false; }
	virtual void	ItemMoved(int32 oldIndex, int32 newIndex) {}

	int32	Selection() { return selection; }	// returns -1 if no selection
	void 	Select(int32 index);
	void 	ScrollToSelection();

	void	Activate();
	void	Deactivate();
	bool	IsActive() { return active; }

	void	Draw(BRect updateRect);
	void	MouseDown(BPoint point);
	void	KeyDown(const char *bytes, int32 numBytes);
	void	MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	void	TargetedByScrollView(BScrollView* scroller);
	void	AttachedToWindow();
	void	FrameResized(float width, float height);

	void	NumItemsChanged();

protected:
	int32       	selection;
	BScrollView*	scroller;
	bool        	active;

	void	UpdateScrollBar();
	void	TrackRearrangement(BPoint point);
	void	DrawItemAt(int32 index);

private:
	typedef BView inherited;
};

#endif
