/* Project.h */

#ifndef _H_Project_
#define _H_Project_

#include <string>
#include <Locker.h>
#include <Rect.h>

class ProjectWind;
class ClassWind;
class TextFile;
class Class;

class Project {
public:
	Project(const BEntry* entryIn);
	~Project();
	void     	Dirtify();
	bool     	IsDirty();
	void     	Undirtify();
	int32    	NumClasses();
	string   	ClassNameAt(int32 index);
	void     	AddClass(int32 index, bool openWind = true);
	void     	OpenClassAt(int32 index);
	void     	ClassNameChanged(Class* theClass);
	void     	RemoveClassAt(int32 index);
	void     	MoveClass(int32 fromIndex, int32 toIndex);
	BRect    	WindFrame();
	void     	SetWindFrame(BRect newWindFrame);
	void     	MessageReceived(BMessage* message);
	void     	GenerateCode();
	void     	Export(TextFile* exportFile);
	void     	Load();
	void     	Save();
	void     	ExportClassAt(TextFile* exportFile, int32 classIndex);
	void     	Import(TextFile* importFile);
	void     	Backup();
	void     	PrepareUpload();
	void     	SetEntry(const BEntry* newEntry);
	void     	SetWindow(ProjectWind* windIn);
	void     	ClassWindowClosing(ClassWind* wind);
	void     	CloseAllClassWindows();
	void     	GetSourcesDirectory(BDirectory* sourcesDir);
	void     	GetBackupsDirectory(BDirectory* backupsDir);
	TextFile*	MakeExportFile(BMessage* message);
	void     	ImportFiles(BMessage* message);

protected:
	BList*      	classes;
	BRect       	windFrame;
	entry_ref*  	entry;
	ProjectWind*	wind;
	BLocker     	lock;
	bool        	dirty;
};


#endif
