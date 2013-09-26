/* CastleYankeeApp.h */

#ifndef _H_CastleYankeeApp_
#define _H_CastleYankeeApp_

#include <Application.h>


extern const char* projectFileMimeType;
extern const char* appSignature;

class CastleYankeeApp : public BApplication {
public:
	CastleYankeeApp();
	~CastleYankeeApp();
	void       	RefsReceived(BMessage* message);
	void       	ReadyToRun();
	void       	MessageReceived(BMessage* message);
	void       	NewProject();
	void       	OpenProject();
	void       	WindowClosing(BWindow* window);
	BFilePanel*	GetOpenPanel();
	BFilePanel*	GetExportPanel();
	BResources*	Resources();
	void       	OpenProjectFile(BFile* file, const BEntry* entry);
	void       	NewProjectFile(BMessage* message);
	int32      	CountAppWindows();
	void       	InitialSetup();

protected:
	bool       	haveOpenedFiles;
	BFilePanel*	newProjectPanel;
	BFilePanel*	openPanel;
	BFilePanel*	exportPanel;
};


#endif
