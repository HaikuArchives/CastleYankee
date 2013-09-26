/* ClassWind.h */

#ifndef _H_ClassWind_
#define _H_ClassWind_

#include <Window.h>

class Class;
class Project;
class MethodsView;
class VariablesView;
class CodeView;
class InfoView;
class SelectedCode;

class ClassWind : public BWindow {
public:
	ClassWind(Class* classIn, Project* projectIn);
	~ClassWind();
	void	MenusBeginning();
	void	MessageReceived(BMessage* message);
	void	DispatchMessage(BMessage *message, BHandler *handler);
	void	FrameMoved(BPoint screenPoint);
	void	FrameResized(float newWidth, float newHeight);
	void	MethodSelected(int32 methodIndex);
	void	VariableSelected(int32 variableIndex);
	void	InvalidateMethods();
	void	InvalidateVariables();
	void	SelectionRemoved();
	void	ResizePanes(float yDelta);
	void	SaveCode();
	void	SetViewedCode(const SelectedCode& newSelection);
	void	LoadNameAndSuperclasses();
	void	SaveNameAndSuperclasses();

protected:
	Class*        	theClass;
	Project*      	project;
	MethodsView*  	methodsView;
	VariablesView*	variablesView;
	CodeView*     	codeView;
	InfoView*     	infoView;
	BMenuField*   	accessMenu;
	SelectedCode* 	selection;
	BRect         	lastFrame;
};


#endif
