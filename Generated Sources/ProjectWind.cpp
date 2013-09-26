/* ProjectWind.cpp */

#include "ProjectWind.h"
#include "Project.h"
#include "ClassesView.h"
#include "CastleYankeeApp.h"
#include "MenuBuilder.h"
#include "Messages.h"

#include <MenuBar.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <FilePanel.h>
#include <Alert.h>

// tweex
static const float XPos = 20;
static const float YPos = 50;
static const float WindWidth = 200;
static const float WindHeight = 500;
static const float MenuBarHeight = 18;



ProjectWind::ProjectWind(Project* projectIn, const char* name)
	: BWindow(BRect(XPos, YPos, XPos + WindWidth, YPos + WindHeight),
	          name, B_TITLED_WINDOW, 0),
	  project(projectIn)
{
	// move to saved position
	BRect savedFrame = project->WindFrame();
	if (savedFrame.IsValid()) {
		MoveTo(savedFrame.LeftTop());
		ResizeTo(savedFrame.Width(), savedFrame.Height());
		}
	BRect bounds = Bounds();

	// menu bar
	BMenuBar* menuBar = new BMenuBar(BRect(0, 0, bounds.right, MenuBarHeight), NULL);
	MenuBuilder menuBuilder("Project");
	menuBuilder.BuildInto(menuBar);
	MenuBuilder::AppGetsMenuMessage("Quit", menuBar);
	MenuBuilder::AppGetsMenuMessage(NewProjectMessage, menuBar);
	MenuBuilder::AppGetsMenuMessage(OpenProjectMessage, menuBar);
	AddChild(menuBar);

	// classes view
	BRect classesViewRect(0, MenuBarHeight + 1,
	                      bounds.right - B_V_SCROLL_BAR_WIDTH, bounds.bottom);
	classesView = new ClassesView(classesViewRect, project);
	BScrollView* classesScroller =
		new BScrollView(NULL, classesView, B_FOLLOW_ALL_SIDES, B_NAVIGABLE,
		                false, true, B_NO_BORDER);
	AddChild(classesScroller);
	classesView->MakeFocus();

	project->SetWindow(this);

	Show();

	// moving the window (which really happened during Show()) made it dirty,
	// though it's really not
	project->Undirtify();
}


ProjectWind::~ProjectWind()
{
	project->CloseAllClassWindows();

	((CastleYankeeApp*) be_app)->WindowClosing(this);
}


void ProjectWind::MenusBeginning()
{
	BMenuBar* menuBar = KeyMenuBar();

	menuBar->FindItem(SaveMessage)->SetEnabled(project->IsDirty());

	bool classSelected = (classesView->Selection() >= 0);
	menuBar->FindItem(ExportClassMessage)->SetEnabled(classSelected);
}


void ProjectWind::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case NewClassMessage:
			if (classesView->Selection() >= 0)
				message->AddInt32("classIndex", classesView->Selection() + 1);
		case SaveMessage:
		case GenerateCodeMessage:
		case ExportProjectFileChosenMessage:
		case ExportClassFileChosenMessage:
		case ImportFileChosenMessage:
		case BackupMessage:
		case PrepareUploadMessage:
			// project object handles these itself
			project->MessageReceived(message);
			break;

		case ExportProjectMessage:
		case ExportClassMessage:
			{
			BMessage panelMessage(message->what == ExportProjectMessage ?
			                      ExportProjectFileChosenMessage :
			                      ExportClassFileChosenMessage);
			panelMessage.AddInt32("classIndex", classesView->Selection());
			BFilePanel* exportPanel = ((CastleYankeeApp*) be_app)->GetExportPanel();
			exportPanel->SetMessage(&panelMessage);
			exportPanel->SetTarget(BMessenger(this));
			exportPanel->Window()->SetTitle(message->what == ExportProjectMessage ?
			                                "Export Project" : "Export Class");
			exportPanel->Show();
			}
			break;

		case ImportMessage:
			{
			BFilePanel* openPanel = ((CastleYankeeApp*) be_app)->GetOpenPanel();
			openPanel->SetMessage(&BMessage(ImportFileChosenMessage));
			openPanel->SetTarget(BMessenger(this));
			openPanel->Window()->SetTitle("Castle Yankee: Import");
			openPanel->Show();
			}
			break;

		default:
			inherited::MessageReceived(message);
			break;
		}
}


void ProjectWind::FrameMoved(BPoint screenPoint)
{
	project->SetWindFrame(Frame());
}


void ProjectWind::FrameResized(float newWidth, float newHeight)
{
	project->SetWindFrame(Frame());
}


bool ProjectWind::QuitRequested()
{
	if (!project->IsDirty())
		return true;

	// prompt for save
	BAlert* alert =
		new BAlert("", "Save changes to the project?", "Cancel", "Don't Save", "Save",
	               B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	alert->SetShortcut(1, 'd');
	int32 choice = alert->Go();
	if (choice == 0)
		return false;
	else if (choice == 2)
		project->Save();
	return true;
}


void ProjectWind::ClassesChanged()
{
	Lock();
	classesView->NumItemsChanged();
	UpdateIfNeeded();	// especially good when importing many classes
	Unlock();
}


void ProjectWind::NewClassAdded(int32 index, bool openWind)
{
	// update display
	Lock();
	classesView->NumItemsChanged();
	classesView->Select(index);
	Unlock();

	// auto-open
	if (openWind)
		project->OpenClassAt(index);
}


