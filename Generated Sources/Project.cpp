/* Project.cpp */

#include "Project.h"
#include "Class.h"
#include "ProjectWind.h"
#include "ClassWind.h"
#include "MessageWind.h"
#include "TextFile.h"
#include "ScriptBeIDE.h"
#include "ProgressMessage.h"
#include "CastleYankeeApp.h"
#include "XMLUtils.h"
#include "Messages.h"
#include "Prefs.h"
#include "qstring.h"
#include "Error.h"

#include <Entry.h>
#include <Autolock.h>
#include <Directory.h>
#include <Path.h>
#include <Volume.h>
#include <FindDirectory.h>
#include <NodeInfo.h>
#include <stdio.h>		// just for SEEK_SET

struct ClassInfo {
	string    	name;
	Class*    	theClass;
	ClassWind*	wind;

	ClassInfo() : theClass(NULL), wind(NULL) {}
	~ClassInfo() { delete theClass; }
};



Project::Project(const BEntry* entryIn)
	: entry(NULL), wind(NULL), dirty(false)
{
	classes = new BList();

	if (entryIn) {
		// set up "entry"
		entry = new entry_ref;
		entryIn->GetRef(entry);

		// read the file
		Load();
		}
}


Project::~Project()
{
	for (int32 i=classes->CountItems()-1; i >= 0; --i) {
		ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(i);
		if (classInfo->wind)
			classInfo->wind->Close();
		delete classInfo;
		}
	delete classes;

	delete entry;
}


void Project::Dirtify()
{
	dirty = true;
}


bool Project::IsDirty()
{
	return dirty;
}


void Project::Undirtify()
{
	dirty = false;
}


int32 Project::NumClasses()
{
	BAutolock locker(lock);

	return classes->CountItems();
}


string Project::ClassNameAt(int32 index)
{
	BAutolock locker(lock);

	ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(index);
	if (classInfo)
		return classInfo->name;
	else
		return NULL;
}


void Project::AddClass(int32 index, bool openWind)
{
	BAutolock locker(lock);

	// create the class object
	Class* newClass = new Class(this);

	// add the classInfo to our list
	ClassInfo* classInfo = new ClassInfo;
	classInfo->theClass = newClass;
	classes->AddItem(classInfo, index);
	Dirtify();

	// update display
	if (wind)
		wind->NewClassAdded(index, openWind);
}


void Project::OpenClassAt(int32 index)
{
	BAutolock locker(lock);

	ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(index);
	if (classInfo == NULL)
		return;
	if (classInfo->wind)
		classInfo->wind->Activate();
	else {
		// open the window
		ClassWind* classWind = new ClassWind(classInfo->theClass, this);
		classInfo->wind = classWind;
		}
}


void Project::ClassNameChanged(Class* theClass)
{
	BAutolock locker(lock);

	// find the class in the classes list
	ClassInfo* classInfo = NULL;
	for (int32 i = classes->CountItems() - 1; i >= 0; --i) {
		ClassInfo* curClassInfo = (ClassInfo*) classes->ItemAt(i);
		if (curClassInfo->theClass == theClass) {
			classInfo = curClassInfo;
			break;
			}
		}
	if (classInfo == NULL)
		return;

	// change the name
	classInfo->name = theClass->Name();
	if (classInfo->wind)
		classInfo->wind->SetTitle(classInfo->name.c_str());
	Dirtify();

	// update display
	if (wind)
		wind->ClassesChanged();
}


void Project::RemoveClassAt(int32 index)
{
	BAutolock locker(lock);
// that leads to deadlock when classInfo->wind->Close()

	ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(index);
	if (classInfo == NULL)
		return;
	if (classInfo->wind)
		classInfo->wind->Close();

	// get rid of the class (& its subojects)
	Class* theClass = classInfo->theClass;
	theClass->RemoveAllSubobjects();

	// remove from the list
	classes->RemoveItem(index);
	delete classInfo;
	Dirtify();
}


void Project::MoveClass(int32 fromIndex, int32 toIndex)
{
	BAutolock locker(lock);

	ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(fromIndex);
	if (classInfo == NULL)
		return;
	classes->RemoveItem(fromIndex);
	classes->AddItem(classInfo, toIndex);
	Dirtify();
}


BRect Project::WindFrame()
{
	return windFrame;
}


void Project::SetWindFrame(BRect newWindFrame)
{
	windFrame = newWindFrame;
	Dirtify();
}


void Project::MessageReceived(BMessage* message)
{
	TextFile* exportFile;

	switch (message->what) {
		case NewClassMessage:
			int32 index;
			if (message->FindInt32("classIndex", &index) != B_NO_ERROR)
				index = classes->CountItems();
			AddClass(index, true);
			break;

		case SaveMessage:
			Save();
			break;

		case GenerateCodeMessage:
			GenerateCode();
			break;

		case ExportProjectFileChosenMessage:
			exportFile = MakeExportFile(message);
			if (exportFile) {
				Export(exportFile);
				delete exportFile;
				}
			break;

		case ExportClassFileChosenMessage:
			exportFile = MakeExportFile(message);
			if (exportFile)
				ExportClassAt(exportFile, message->FindInt32("classIndex"));
			break;

		case ImportFileChosenMessage:
			ImportFiles(message);
			break;

		case BackupMessage:
			Backup();
			break;

		case PrepareUploadMessage:
			PrepareUpload();
			break;
		}
}


void Project::GenerateCode()
{
	BAutolock locker(lock);

	MessageWind* messageWind = new MessageWind("Generating...");

	// get the sources directory
	BDirectory sourcesDir;
	GetSourcesDirectory(&sourcesDir);

	// have all classes generate their files
	int32 numClasses = classes->CountItems();
	for (int i=0; i<numClasses; i++) {
		ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(i);
		if (classInfo->wind)
			classInfo->wind->SaveCode();
		if (!classInfo->name.empty() && classInfo->name[0] == '"')
			continue;	// skip dividers
		Class* theClass = classInfo->theClass;
		if (theClass == NULL)
			throw Error("Internal Error: Missing class #%d.", i);
		theClass->GenerateH(&sourcesDir);
		theClass->GenerateCPP(&sourcesDir);
		}

	messageWind->Quit();

	if (Prefs()->GetBoolPref("autoIDEMake", true))
		MakeBeIDEProject();
}


void Project::Export(TextFile* exportFile)
{
	// start tag
	exportFile->WriteString("<project");
	WriteRectAttribute(exportFile, "wind-frame", windFrame);
	exportFile->WriteString(">\n");

	int32 numClasses = classes->CountItems();
	for (int i=0; i<numClasses; i++) {
		ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(i);
		if (classInfo->wind)
			classInfo->wind->SaveCode();
		classInfo->theClass->Export(exportFile);
		}

	exportFile->WriteString("</project>\n");
}


void Project::Load()
{
	if (entry == NULL)
		return;

	TextFile file(entry, B_READ_ONLY);
	status_t result = file.InitCheck();
	if (result == B_ENTRY_NOT_FOUND) {
		// doesn't exist yet, but it will when we save it
		return;
		}
	else if (result != B_NO_ERROR)
		throw Error("Couldn't open the file!");

	ProgressMessage msg("Opening...");
	Import(&file);

	// the import dirtifies the project, but it's not really dirty
	dirty = false;
}


void Project::Save()
{
	status_t result;

	try {

	ProgressMessage msg("Saving...");

	// find out where to put the temporary file
	BPath tempPath;
	result = find_directory(B_COMMON_TEMP_DIRECTORY, &tempPath,
	                        true, &BVolume(entry->device));
	if (result != B_NO_ERROR)
		throw Error("Couldn't find the temporary directory.");
	tempPath.Append(entry->name);
	BEntry tempFileEntry(tempPath.Path(), true);

	// open the temp file and write it
	TextFile file(&tempFileEntry, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() != B_NO_ERROR)
		throw Error("Couldn't open the temporary file.");
	BNodeInfo(&file).SetType(projectFileMimeType);
	Export(&file);
	file.Unset();

	// now move the saved file into place
	BDirectory homeDir;
	BEntry fileEntry(entry);
	result = fileEntry.GetParent(&homeDir);
	if (result != B_NO_ERROR)
		throw Error("Couldn't find parent directory!");
	result = tempFileEntry.MoveTo(&homeDir, NULL, true);
	if (result != B_NO_ERROR)
		throw Error("Couldn't move the file into place!");

	// undirtify
	dirty = false;

	} catch (Error& e) {
		e.Display();
		}
}


void Project::ExportClassAt(TextFile* exportFile, int32 classIndex)
{
	ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(classIndex);
	if (classInfo == NULL)
		return;
	classInfo->theClass->Export(exportFile);
}


void Project::Import(TextFile* importFile)
{
	qstring line;

	while (!importFile->AtEOF()) {
		off_t lineStart = importFile->Position();
		importFile->ReadLine(&line);

		if (line.startsWith("<project")) {
			// read the attributes
			BRect windFrameAttr = GetRectAttribute(line.c_str(), "wind-frame");
			if (windFrameAttr.IsValid())
				windFrame = windFrameAttr;
			}

		else if (line.startsWith("<class")) {
			// create the class
			int32 newIndex = NumClasses();
			AddClass(newIndex, false);

			// open the class
			ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(newIndex);
			if (classInfo == NULL)
				return;		// shouldn't happen...
			importFile->Seek(lineStart, SEEK_SET);	// rewind the line--the class'll want it
			classInfo->theClass->Import(importFile);
			}
		}
}


void Project::Backup()
{
	// sanity clause--can't do it if we don't know where we are
	if (entry == NULL)
		return;

	// find/make the directory
	BDirectory backupsDir;
	GetBackupsDirectory(&backupsDir);

	// build the name prefix
	// date
	time_t curTime = time(NULL);
	char dateStr[64];
	strftime(dateStr, 64, " %Y.%m.%d.", localtime(&curTime));
	// prefix
	qstring namePrefix = entry->name;
	namePrefix += dateStr;

	// figure out what filename to use
	qstring fileName;
	char revChar = 'A';
	while (true) {
		// build the name
		fileName = namePrefix;
		fileName += revChar;

		// see if it exists already
		if (!backupsDir.Contains(fileName.c_str()))
			break;

		// name exists; try the next one
		revChar++;
		}

	/* make the file */
	TextFile* backupFile =
		new TextFile(&backupsDir, fileName.c_str(),
		             B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (backupFile->InitCheck() != B_NO_ERROR) {
		delete backupFile;
		return;
		}
	BNodeInfo(backupFile).SetType("text/plain");

	// backup
	MessageWind* messageWind = new MessageWind("Backing up...");
	Export(backupFile);
	delete backupFile;
	messageWind->Quit();
}


void Project::PrepareUpload()
{
	// sanity clause--can't do it if we don't know where we are
	if (entry == NULL)
		return;

	// find the directory
	BDirectory parentDir;
	BEntry dirEntry(entry);
	dirEntry.GetParent(&parentDir);

	// build the name
	qstring fileName = entry->name;
	fileName += ".xml";

	/* make the file */
	TextFile uploadFile(&parentDir, fileName.c_str(),
	                    B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (uploadFile.InitCheck() != B_NO_ERROR)
		return;
	BNodeInfo(&uploadFile).SetType("text/plain");

	// prepare upload
	MessageWind* messageWind = new MessageWind("Preparing upload...");
	Export(&uploadFile);
	messageWind->Quit();
}


void Project::SetEntry(const BEntry* newEntry)
{
	if (entry == NULL)
		entry = new entry_ref;
	newEntry->GetRef(entry);
}


void Project::SetWindow(ProjectWind* windIn)
{
	wind = windIn;
}


void Project::ClassWindowClosing(ClassWind* wind)
{
//***	BAutolock locker(lock);
// Sometimes we're the one closing the window, so locking here leads
// to deadlock.  The correct solution would be to have a lock just for the
// "classes" list, and be careful about when we're locking it.

	int32 numClasses = classes->CountItems();
	for (int32 i=0; i<numClasses; i++) {
		ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(i);
		if (classInfo->wind == wind) {
			classInfo->wind = NULL;
			break;
			}
		}
}


void Project::CloseAllClassWindows()
{
	for (int32 i=classes->CountItems()-1; i >= 0; --i) {
		ClassInfo* classInfo = (ClassInfo*) classes->ItemAt(i);
		if (classInfo->wind)
			classInfo->wind->Close();
		}
}


void Project::GetSourcesDirectory(BDirectory* sourcesDir)
{
	// create the sources directory if necessary
	BDirectory parentDir;
	BEntry entry(this->entry);
	entry.GetParent(&parentDir);
	parentDir.CreateDirectory("Generated Sources", sourcesDir);

	// get the sources directory
	sourcesDir->SetTo(&parentDir, "Generated Sources");	// CreateDirectory() won't set this right if it doesn't actually create it
}


void Project::GetBackupsDirectory(BDirectory* backupsDir)
{
	// create the backups directory if necessary
	BDirectory parentDir;
	BEntry entry(this->entry);
	entry.GetParent(&parentDir);
	parentDir.CreateDirectory("Backups", backupsDir);

	// get the sources directory
	backupsDir->SetTo(&parentDir, "Backups");	// CreateDirectory() won't set this right if it doesn't actually create it
}


TextFile* Project::MakeExportFile(BMessage* message)
{
	entry_ref dirRef;
	if (message->FindRef("directory", &dirRef) != B_NO_ERROR)
		return NULL;
	BDirectory directory(&dirRef);
	TextFile* exportFile =
		new TextFile(&directory, message->FindString("name"),
		             B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (exportFile->InitCheck() != B_NO_ERROR) {
		delete exportFile;
		return NULL;
		}
	BNodeInfo(exportFile).SetType("text/plain");
	return exportFile;
}


void Project::ImportFiles(BMessage* message)
{
	for (int fileIndex=0; ; fileIndex++) {
		entry_ref fileRef;
		if (message->FindRef("refs", fileIndex, &fileRef) != B_NO_ERROR)
			break;
		TextFile* importFile = new TextFile(&fileRef, B_READ_ONLY);
		if (importFile->InitCheck() == B_NO_ERROR)
			Import(importFile);
		delete importFile;
		}
}


