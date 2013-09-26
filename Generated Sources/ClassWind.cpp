/* ClassWind.cpp */

#include "ClassWind.h"
#include "Class.h"
#include "Method.h"
#include "Variable.h"
#include "Project.h"
#include "Code.h"
#include "MethodsView.h"
#include "VariablesView.h"
#include "CodeView.h"
#include "InfoView.h"
#include "MenuBuilder.h"
#include "CPPParser.h"
#include "TokenBuilder.h"
#include "Messages.h"
#include "Prefs.h"
#include "Error.h"
#include <string>

#include <MenuBar.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <ScrollView.h>
#include <PopUpMenu.h>
#include <Alert.h>

enum CodeType {
	NoSelection,
	MethodCode,
	VariableCode,
	ClassName,
	Superclasses,
	HIncludes,
	CPPIncludes
};

struct SelectedCode {
	CodeType     	type;
	ClassElement*	classElement;

	SelectedCode() : type(NoSelection) {}
	SelectedCode(CodeType typeIn) : type(typeIn) {}
	SelectedCode(Method* methodIn) : type(MethodCode), classElement(methodIn) {}
	SelectedCode(Variable* variableIn) : type(VariableCode), classElement(variableIn) {}
};

// tweex
static const float XPos = 100;
static const float YPos = 60;
static const float WindWidth = 640;
static const float WindHeight = 530;
static const float MenuBarHeight = 18;
static const float ListViewsHeight = 120;
static const float VariablesViewWidth = 200;
static const float InfoViewHeight = 20;

// calcs
inline float MethodViewWidth(float windWidth)
	{ return windWidth - VariablesViewWidth - 2; }
inline float InfoViewTop(float listViewsHeight)
	{ return MenuBarHeight + listViewsHeight + 2; }
inline float CodeViewTop(float listViewsHeight)
	{ return InfoViewTop(listViewsHeight) + InfoViewHeight + 2; }



ClassWind::ClassWind(Class* classIn, Project* projectIn)
	: BWindow(BRect(XPos, YPos, XPos + WindWidth, YPos + WindHeight),
	          "", B_DOCUMENT_WINDOW, 0),
	  theClass(classIn), project(projectIn), selection(new SelectedCode())
{
	theClass->SetWindow(this);

	// set name
	string className = theClass->Name();
	string windowName = className;
	if (windowName.empty())
		windowName = "Unnamed Class";
	SetTitle(windowName.c_str());

	// move to saved position
	BRect savedFrame = theClass->WindFrame();
	if (savedFrame.IsValid()) {
		MoveTo(savedFrame.LeftTop());
		ResizeTo(savedFrame.Width(), savedFrame.Height());
		}
	lastFrame = Frame();
	float listViewsHeight = theClass->ListViewsHeight();
	if (listViewsHeight == 0)
		listViewsHeight = ListViewsHeight;
	BRect bounds = Bounds();

	// menu bar
	BMenuBar* menuBar = new BMenuBar(BRect(0, 0, bounds.right, MenuBarHeight), NULL);
	MenuBuilder menuBuilder("Class");
	menuBuilder.BuildInto(menuBar);
	MenuBuilder::AppGetsMenuMessage("Quit", menuBar);
	MenuBuilder::AppGetsMenuMessage(NewProjectMessage, menuBar);
	MenuBuilder::AppGetsMenuMessage(OpenProjectMessage, menuBar);
	AddChild(menuBar);

	// methods view
	BRect methodsViewRect(0, MenuBarHeight + 1,
	                      MethodViewWidth(bounds.Width()) - B_V_SCROLL_BAR_WIDTH,
	                      MenuBarHeight + listViewsHeight);
	methodsView = new MethodsView(methodsViewRect, theClass);
	BScrollView* methodsScroller =
		new BScrollView("Methods Scroller", methodsView,
		                B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
		                B_FRAME_EVENTS, false, true, B_PLAIN_BORDER);
	AddChild(methodsScroller);
	methodsView->MakeFocus();

	// variables view
	BRect variablesViewRect = methodsViewRect;
	variablesViewRect.left = variablesViewRect.right + 2 + B_V_SCROLL_BAR_WIDTH;
	variablesViewRect.right = bounds.right - B_V_SCROLL_BAR_WIDTH;
	variablesView = new VariablesView(variablesViewRect, theClass);
	BScrollView* variablesScroller =
		new BScrollView("Variables Scroller", variablesView,
		                B_FOLLOW_RIGHT | B_FOLLOW_TOP,
		                B_FRAME_EVENTS, false, true, B_PLAIN_BORDER);
	AddChild(variablesScroller);

	// code view
	BRect codeViewRect;
	codeViewRect.left = 0;
	codeViewRect.right = bounds.right - B_V_SCROLL_BAR_WIDTH;
	codeViewRect.top = CodeViewTop(listViewsHeight);
	codeViewRect.bottom = bounds.bottom - B_H_SCROLL_BAR_HEIGHT;
	codeView = new CodeView(codeViewRect, "Code View");
	BScrollView* codeScroller =
		new BScrollView("Code Scroller", codeView, B_FOLLOW_ALL, B_FRAME_EVENTS,
		                true, true, B_PLAIN_BORDER);
	AddChild(codeScroller);

	// info view
	BRect infoViewRect(0, InfoViewTop(listViewsHeight),
	                   bounds.right, InfoViewTop(listViewsHeight) + InfoViewHeight);
	infoView = new InfoView(infoViewRect);
	AddChild(infoView);

	// access menu
	BPopUpMenu* accessPopupMenu = new BPopUpMenu("");
	MenuBuilder accessMenuBuilder("Access");
	accessMenuBuilder.BuildInto(accessPopupMenu);
	BMenuItem* publicItem = accessPopupMenu->FindItem("public");
	if (publicItem) {
		// we have to check if we actually found it, because sometimes it doesn't.
		// I have no idea why not, but at least this will prevent crashing.
		publicItem->SetMarked(true);
		}
	else {
		// put up a warning dialog to help debug this problem
		BAlert* alert =
			new BAlert("", "Internal error:\n  The \"public\" menu item is missing!\nThis error is non-fatal.", "OK");
		alert->Go();
		}
	BRect accessMenuRect(0, 0, 82, 12);
	accessMenu = new BMenuField(accessMenuRect, "Access Menu", "", accessPopupMenu);
	accessMenu->SetDivider(0);
	accessMenu->SetEnabled(false);
	infoView->AddChild(accessMenu);

	if (className.empty())
		SetViewedCode(SelectedCode(ClassName));
	else {
		// select .cpp Includes or .h Includes, as appropriate
		CodeType newSelection = CPPIncludes;
		string cppIncludes = theClass->CPPIncludes();
		string hIncludes = theClass->HIncludes();
		if (cppIncludes.empty() && !hIncludes.empty())
			newSelection = HIncludes;
		SetViewedCode(SelectedCode(newSelection));
		}

	Show();
}


ClassWind::~ClassWind()
{
	SaveCode();
	project->ClassWindowClosing(this);
	delete selection;
}


void ClassWind::MenusBeginning()
{
	BMenuBar* menuBar = KeyMenuBar();
	if (menuBar == NULL)
		return;

	menuBar->FindItem(SaveMessage)->SetEnabled(project->IsDirty());

	CodeType selectionType = selection->type;
	menuBar->FindItem(NameMessage)->SetMarked(selectionType == ClassName);
	menuBar->FindItem(HIncludesMessage)->SetMarked(selectionType == HIncludes);
	menuBar->FindItem(CPPIncludesMessage)->SetMarked(selectionType == CPPIncludes);

	bool codeViewIsFocus = (CurrentFocus() == codeView);
	bool hasSelection = false;
	if (codeViewIsFocus) {
		int32 selStart, selEnd;
		codeView->GetSelection(&selStart, &selEnd);
		hasSelection = (selStart != selEnd);
		}
	menuBar->FindItem(B_UNDO)->SetEnabled(codeViewIsFocus);
	menuBar->FindItem(B_CUT)->SetEnabled(hasSelection);
	menuBar->FindItem(B_COPY)->SetEnabled(hasSelection);
	menuBar->FindItem(B_PASTE)->SetEnabled(codeViewIsFocus);
	menuBar->FindItem(B_SELECT_ALL)->SetEnabled(codeViewIsFocus);
	menuBar->FindItem(IndentMessage)->SetEnabled(codeViewIsFocus);
	menuBar->FindItem(UnindentMessage)->SetEnabled(codeViewIsFocus);

	menuBar->FindItem(ColorSyntaxMessage)->SetMarked(Prefs()->GetBoolPref("colorSyntax", true));
	menuBar->FindItem(AutoBlockMessage)->SetMarked(Prefs()->GetBoolPref("autoBlock", true));

	// undo
	if (codeViewIsFocus) {
		BMenuItem* undoItem = menuBar->FindItem(B_UNDO);
		bool isRedo;
		undo_state actionType = codeView->UndoState(&isRedo);
		const char* label = "Undo";
		switch (actionType) {
			case B_UNDO_UNAVAILABLE:
				undoItem->SetEnabled(false);
				break;
			case B_UNDO_TYPING:
				label = (isRedo ? "Redo Typing" : "Undo Typing");
				break;
			case B_UNDO_CUT:
				label = (isRedo ? "Redo Cut" : "Undo Cut");
				break;
			case B_UNDO_PASTE:
				label = (isRedo ? "Redo Paste" : "Undo Paste");
				break;
			case B_UNDO_CLEAR:
				label = (isRedo ? "Redo Clear" : "Undo Clear");
				break;
			case B_UNDO_DROP:
				label = (isRedo ? "Redo Drop" : "Undo Drop");
				break;
			default:
				label = (isRedo ? "Redo" : "Undo");
				break;
			}
		undoItem->SetLabel(label);
		}
}


void ClassWind::MessageReceived(BMessage* message)
{
	uint32       	newItem;
	ClassElement*	curClassElement;
	Preferences* 	prefs;
	AccessType   	newAccessType;

	try {

	switch (message->what) {
		case NewMethodMessage:
			newItem = methodsView->Selection() + 1;
			if (newItem <= 0)
				newItem = theClass->NumMethods();
			theClass->AddMethod(newItem);
			methodsView->NumItemsChanged();
			methodsView->Select(newItem);
			methodsView->ScrollToSelection();
			SetViewedCode(SelectedCode(theClass->MethodAt(newItem)));
			codeView->MakeFocus();
			break;

		case NewVariableMessage:
			newItem = variablesView->Selection() + 1;
			if (newItem <= 0)
				newItem = theClass->NumVariables();
			theClass->AddVariable(newItem);
			variablesView->NumItemsChanged();
			variablesView->Select(newItem);
			variablesView->ScrollToSelection();
			SetViewedCode(SelectedCode(theClass->VariableAt(newItem)));
			codeView->MakeFocus();
			break;

		case SaveMessage:
			SaveCode();
			project->Save();
			break;

		case NameMessage:
			SetViewedCode(SelectedCode(ClassName));
			break;

		case HIncludesMessage:
			SetViewedCode(SelectedCode(HIncludes));
			break;

		case CPPIncludesMessage:
			SetViewedCode(SelectedCode(CPPIncludes));
			break;

		case B_UNDO:
		case B_CUT:
		case B_COPY:
		case B_PASTE:
		case B_SELECT_ALL:
			// since the MenuBuilder doesn't set the target of these menu
			// items, the default target is this window.  Just pass it on.
			{
			BView* focusView = CurrentFocus();
			if (focusView)
				focusView->MessageReceived(message);
			}
			break;

		case NewClassMessage:
		case GenerateCodeMessage:
			// project object handles these itself--but make sure to save changes
			// first (especially for GenerateCodeMessage)
			SaveCode();
			project->MessageReceived(message);
			break;

		case IndentMessage:
			if (CurrentFocus() == codeView)
				codeView->Indent();
			break;

		case UnindentMessage:
			if (CurrentFocus() == codeView)
				codeView->Unindent();
			break;

		case ColorSyntaxMessage:
			prefs = Prefs();
			prefs->SetBoolPref("colorSyntax", !prefs->GetBoolPref("colorSyntax", true));
			break;

		case AutoBlockMessage:
			prefs = Prefs();
			prefs->SetBoolPref("autoBlock", !prefs->GetBoolPref("autoBlock", true));
			break;

		case PublicMessage:
			newAccessType = Public;
			goto setAccessType;
		case ProtectedMessage:
			newAccessType = Protected;
			goto setAccessType;
		case PrivateMessage:
			newAccessType = Private;
			goto setAccessType;
		setAccessType:
			if (selection->type != MethodCode && selection->type != VariableCode)
				break;
			curClassElement = selection->classElement;
			if (curClassElement)
				curClassElement->SetAccess(newAccessType);
			break;

		default:
			inherited::MessageReceived(message);
			break;
		}

	} catch (Error error) {
		error.Display();
		}
}


void ClassWind::DispatchMessage(BMessage* message, BHandler* handler)
{
	bool passMessage = true;
	uint32 what = message->what;
	if (what == B_KEY_DOWN) {
		BView* focusView;
		uint32 modifiers = message->FindInt32("modifiers");
		int8 c;
		message->FindInt8("byte", &c);
		if (c == '\n' && (modifiers & B_OPTION_KEY) != 0) {
			// we want to handle option-return ourselves, but something
			// in inherited::DispatchMessage() sucks it up without passing
			// it on to the focus view.  So we have to do that here.
			focusView = CurrentFocus();
			if (focusView)
				focusView->KeyDown("\n", 1);
			passMessage = false;
			}
		else if ((c == B_LEFT_ARROW || c == B_RIGHT_ARROW) &&
		         (modifiers & B_COMMAND_KEY) != 0) {
		    // it'll take these as menu commands unless we deal with them here
			focusView = CurrentFocus();
			if (focusView) {
				char charStr[4];
				charStr[0] = c;
				charStr[1] = 0;
				focusView->KeyDown(charStr, 1);
				}
			passMessage = false;
			}
		}

	if (passMessage)
		BWindow::DispatchMessage(message, handler);

	// automatically dirtify when the codeView is dirty
	if (codeView->IsDirty())
		project->Dirtify();
}


void ClassWind::FrameMoved(BPoint screenPoint)
{
	BRect newFrame = Frame();
	if (newFrame != lastFrame) {
		theClass->SetWindFrame(newFrame);
		lastFrame = newFrame;
		}
}


void ClassWind::FrameResized(float newWidth, float newHeight)
{
	BRect newFrame = Frame();
	if (newFrame != lastFrame) {
		theClass->SetWindFrame(newFrame);
		lastFrame = newFrame;
		}
}


void ClassWind::MethodSelected(int32 methodIndex)
{
	if (methodIndex < 0) {
		SetViewedCode(SelectedCode(NoSelection));
		return;
		}
	Method* method = theClass->MethodAt(methodIndex);
	if (method == NULL) {
		SetViewedCode(SelectedCode(NoSelection));
		return;
		}
	SetViewedCode(SelectedCode(method));
}


void ClassWind::VariableSelected(int32 variableIndex)
{
	if (variableIndex < 0) {
		SetViewedCode(SelectedCode(NoSelection));
		return;
		}
	Variable* variable = theClass->VariableAt(variableIndex);
	if (variable == NULL) {
		SetViewedCode(SelectedCode(NoSelection));
		return;
		}
	SetViewedCode(SelectedCode(variable));
}


void ClassWind::InvalidateMethods()
{
	Lock();
	methodsView->Invalidate();
	Unlock();
}


void ClassWind::InvalidateVariables()
{
	Lock();
	variablesView->Invalidate();
	Unlock();
}


void ClassWind::SelectionRemoved()
{
	codeView->Undirtify();		// make sure it doesn't try to SaveCode
	SetViewedCode(SelectedCode(NoSelection));
}


void ClassWind::ResizePanes(float yDelta)
{
	FindView("Methods Scroller")->ResizeBy(0, yDelta);
	FindView("Variables Scroller")->ResizeBy(0, yDelta);
	infoView->MoveBy(0, yDelta);
	BView* codeScroller = FindView("Code Scroller");
	codeScroller->MoveBy(0, yDelta);
	codeScroller->ResizeBy(0, -yDelta);

	theClass->SetListViewsHeight(FindView("Methods Scroller")->Bounds().Height());
}


void ClassWind::SaveCode()
{
	if (!codeView->IsDirty())
		return;

	switch (selection->type) {
		case ClassName:
			SaveNameAndSuperclasses();
			break;

		case HIncludes:
			theClass->SetHIncludes(codeView->Text());
			break;

		case CPPIncludes:
			theClass->SetCPPIncludes(codeView->Text());
			break;

		case MethodCode:
			{
			Method* method = (Method*) selection->classElement;
			if (method == NULL)
				break;
			method->SetCodeFromTextView(codeView);
			}
			break;

		case VariableCode:
			{
			Variable* variable = (Variable*) selection->classElement;
			if (variable == NULL)
				break;
			variable->SetCode(codeView->Text());
			}
			break;
		}

	codeView->Undirtify();
}


void ClassWind::SetViewedCode(const SelectedCode& newSelection)
{
	AccessType	accessType;

	static const uint32 accessMessages[] = { 0, PublicMessage, ProtectedMessage, PrivateMessage };

	SaveCode();

	*selection = newSelection;
	switch (selection->type) {
		case ClassName:
			LoadNameAndSuperclasses();
			methodsView->Deactivate();
			variablesView->Deactivate();
			accessMenu->SetEnabled(false);
			codeView->MakeFocus();
			break;

		case HIncludes:
			codeView->SetTextTo(theClass->HIncludes());
			codeView->SelectEnd();
			methodsView->Deactivate();
			variablesView->Deactivate();
			accessMenu->SetEnabled(false);
			codeView->MakeFocus();
			break;


		case CPPIncludes:
			codeView->SetTextTo(theClass->CPPIncludes());
			codeView->SelectEnd();
			methodsView->Deactivate();
			variablesView->Deactivate();
			accessMenu->SetEnabled(false);
			codeView->MakeFocus();
			break;

		case MethodCode:
			{
			Method* method = (Method*) selection->classElement;
			if (method == NULL)
				break;
			Code* methodCode = method->GetCode();
			if (methodCode->GetCode().empty()) {
				// new method--show a template
				codeView->SetTextTo("void MethodName()\n{\n\t/***/\n}\n");
				codeView->Select(0, 15);
				}
			else
				codeView->SetCode(methodCode);
			accessType = method->Access();

			methodsView->Activate();
			variablesView->Deactivate();
			accessMenu->SetEnabled(true);
			accessMenu->Menu()->FindItem(accessMessages[accessType])->SetMarked(true);
			}
			break;

		case VariableCode:
			{
			Variable* variable = (Variable*) selection->classElement;
			if (variable == NULL)
				break;
			string varCode = variable->Code();
			if (varCode.empty()) {
				// new variable--show a template
				codeView->SetTextTo("int variableName;");
				codeView->Select(0, 16);
				}
			else {
				codeView->SetTextTo(varCode);
				codeView->SelectEnd();
				}
			accessType = variable->Access();

			variablesView->Activate();
			methodsView->Deactivate();
			accessMenu->SetEnabled(true);
			accessMenu->Menu()->FindItem(accessMessages[accessType])->SetMarked(true);
			}
			break;

		default:
			codeView->SetTextTo("");
			accessMenu->SetEnabled(false);
			break;
		}
}


void ClassWind::LoadNameAndSuperclasses()
{
	string line;

	// class name
	string className = theClass->Name();
	if (className.empty())
		line = "ClassName";
	else
		line = className;

	// superclasses
	string superclasses = theClass->Superclasses();
	if (!superclasses.empty()) {
		line += " : ";
		line += superclasses;
		}

	// selection
	codeView->SetTextTo(line);
	if (className.empty())
		codeView->Select(0, 9);
	else
		codeView->SelectEnd();
}


void ClassWind::SaveNameAndSuperclasses()
{
	char token[256];

	CPPParser parser(codeView->Text());

	// first token is class name
	parser.GetToken(token);
	theClass->SetName(token);

	// next token must be ':'; ignore the rest if it's not
	parser.GetToken(token);
	if (token[0] != ':' || token[1] != 0) {
		theClass->SetSuperclasses("");
		return;
		}

	// superclasses
	TokenBuilder superclassesBuilder;
	parser.GetToken(token);		// start with first token
	superclassesBuilder.AddToken(token);
	superclassesBuilder.AddToken(parser.RestOfText());
	theClass->SetSuperclasses(superclassesBuilder.String());
}


