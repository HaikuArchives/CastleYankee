/* CastleYankeeApp.cpp */

#include "CastleYankeeApp.h"
#include "Project.h"
#include "Class.h"
#include "Method.h"
#include "Variable.h"
#include "ProjectWind.h"
#include "Messages.h"
#include "Prefs.h"
#include <File.h>
#include <FilePanel.h>
#include <Roster.h>
#include <Resources.h>
#include <NodeInfo.h>

#if __profile__
#include "libprof.h"
#endif

const char* projectFileMimeType = "text/x-vnd.Steve-Folta.Castle-Yankee-2-project";
const char* appSignature = "application/x-vnd.Steve-Folta.Castle-Yankee-2";

int main()
{
#if __profile__
	PROFILE_INIT(100);
#endif

	CastleYankeeApp* app = new CastleYankeeApp();
	app->Run();
	delete app;

#if __profile__
	PROFILE_DUMP("/boot/Development/Castle Yankee 2.0/profile.dump");
#endif

	return 0;
}



CastleYankeeApp::CastleYankeeApp()
	: BApplication(appSignature),
	  haveOpenedFiles(false), newProjectPanel(NULL), openPanel(NULL),
	  exportPanel(NULL)
{
	// do initial setup the first time it's run
	if (!Prefs()->GetBoolPref("have-run-once"))
		InitialSetup();
}


CastleYankeeApp::~CastleYankeeApp()
{
	delete Prefs();

	delete newProjectPanel;
	delete openPanel;
	delete exportPanel;
}


void CastleYankeeApp::RefsReceived(BMessage* message)
{
	// sanity clauses
	if (message == NULL)
		return;
	uint32 type;
	int32 count;
	message->GetInfo("refs", &type, &count);
	if ( type != B_REF_TYPE )
		return;

	// process the files
	for (long i = count - 1; i >= 0; i--) {
		// get the ref & make sure it's a file
		entry_ref ref;
		if (message->FindRef("refs", i, &ref) != B_OK )
			continue;
		BEntry entry;
		entry.SetTo(&ref);
		if (!entry.IsFile())
			continue;

		// create the file & project
		BFile* file = new BFile();
		if (file->SetTo(&ref, B_READ_WRITE) != B_OK) {
			delete file;
			continue;
			}
		OpenProjectFile(file, &entry);
		haveOpenedFiles = true;
		}
}


void CastleYankeeApp::ReadyToRun()
{
	if (CountAppWindows() == 0)
		NewProject();
}


void CastleYankeeApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case NewProjectMessage:
			NewProject();
			break;

		case OpenProjectMessage:
			OpenProject();
			break;

		case B_SAVE_REQUESTED:
			NewProjectFile(message);
			break;

		case B_CANCEL:
			// user cancelled the newProjectPanel
			if (CountAppWindows() == 0)
				Quit();		// from startup
			break;
		}
}


void CastleYankeeApp::NewProject()
{
	if (newProjectPanel == NULL) {
		newProjectPanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL, 0, false);
		newProjectPanel->Window()->SetTitle("Castle Yankee: New Project");
		}
	newProjectPanel->Show();
}


void CastleYankeeApp::OpenProject()
{
	GetOpenPanel();
	openPanel->SetTarget(BMessenger(this));
	openPanel->SetMessage(&BMessage(B_REFS_RECEIVED));
	openPanel->Window()->SetTitle("Castle Yankee: Open Project");
	openPanel->Show();
}


void CastleYankeeApp::WindowClosing(BWindow* window)
{
	if (CountAppWindows() == 1)	// last window
		Quit();
}


BFilePanel* CastleYankeeApp::GetOpenPanel()
{
	if (openPanel == NULL)
		openPanel = new BFilePanel();
	return openPanel;
}


BFilePanel* CastleYankeeApp::GetExportPanel()
{
	if (exportPanel == NULL)
		exportPanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL, 0, false);
	return exportPanel;
}


BResources* CastleYankeeApp::Resources()
{
	app_info info;
	if (GetAppInfo(&info) != B_OK)
		return NULL;
	BFile rsrcFile(&info.ref, B_READ_ONLY);
	if (rsrcFile.InitCheck() != B_OK)
		return NULL;
	BResources* rsrcs = new BResources();
	if (rsrcs->SetTo(&rsrcFile) != B_OK) {
		delete rsrcs;
		rsrcs = NULL;
		}
	return rsrcs;
}


void CastleYankeeApp::OpenProjectFile(BFile* file, const BEntry* entry)
{
	// make the project
	Project* project = new Project(entry);

	// open the window
	char fileName[B_FILE_NAME_LENGTH];
	entry->GetName(fileName);
	new ProjectWind(project, fileName);
}


void CastleYankeeApp::NewProjectFile(BMessage* message)
{
	// find out where the file goes
	entry_ref dirRef;
	message->FindRef("directory", &dirRef);
	const char* name = message->FindString("name");

	// create the file
	BDirectory directory(&dirRef);	// why did Be make us use this extra step?
	                              	// either BFile ctor should directly use
	                              	// what B_SAVE_REQUESTED gives us, or
	                              	// B_SAVE_REQUESTED message should give
	                              	// us something that can be directly used
	                              	// to construct a BFile.
	BFile* file = new BFile(&directory, name, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	// set the MIME type
	BNodeInfo(file).SetType(projectFileMimeType);

	// open it
	BEntry fileEntry(&directory, name, true);
	OpenProjectFile(file, &fileEntry);
}


int32 CastleYankeeApp::CountAppWindows()
{
	// like CountWindows(), but ignores file panels
	return CountWindows() -
		(newProjectPanel != NULL) - (openPanel != NULL) - (exportPanel != NULL);
}


void CastleYankeeApp::InitialSetup()
{
	// register ourself as the owner of our files
	BMimeType mimeType(projectFileMimeType);
	mimeType.SetPreferredApp(appSignature);

	// mark that we've done this stuff
	Prefs()->SetBoolPref("have-run-once", true);
}


