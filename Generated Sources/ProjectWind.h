/* ProjectWind.h */

#ifndef _H_ProjectWind_
#define _H_ProjectWind_

#include <Window.h>


class Project;
class ClassesView;

class ProjectWind : public BWindow {
public:
	ProjectWind(Project* projectIn, const char* name);
	~ProjectWind();
	void	MenusBeginning();
	void	MessageReceived(BMessage *message);
	void	FrameMoved(BPoint screenPoint);
	void	FrameResized(float newWidth, float newHeight);
	bool	QuitRequested();
	void	ClassesChanged();
	void	NewClassAdded(int32 index, bool openWind = true);

protected:
	Project*    	project;
	ClassesView*	classesView;
	BScrollBar* 	scrollBar;
};


#endif
