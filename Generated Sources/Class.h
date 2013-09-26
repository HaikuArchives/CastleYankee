/* Class.h */

#ifndef _H_Class_
#define _H_Class_

#include <string>

#include <Rect.h>
#include <Locker.h>

class ClassWind;
class TextFile;
class Project;
class Method;
class Variable;

class Class {
public:
	Class(Project* projectIn);
	~Class();
	void         	Dirtify();
	void         	Save();
	inline string	Name();
	void         	SetName(string newName);
	string       	Superclasses();
	void         	SetSuperclasses(string newCode);
	int32        	NumMethods();
	string       	MethodSignatureAt(int32 index);
	bool         	MethodIsUglyNotesHackAt(int32 index);
	Method*      	MethodAt(int32 index);
	void         	AddMethod(int32 index);
	void         	RemoveMethodAt(int32 index);
	void         	MoveMethod(int32 fromIndex, int32 toIndex);
	int32        	NumVariables();
	string       	VariableNameAt(int32 index);
	bool         	VariableIsUglyNotesHackAt(int32 index);
	Variable*    	VariableAt(int32 index);
	void         	AddVariable(int32 index);
	void         	RemoveVariableAt(int32 index);
	void         	MoveVariable(int32 fromIndex, int32 toIndex);
	void         	RemoveAllSubobjects();
	string       	HIncludes();
	void         	SetHIncludes(string newCode);
	string       	CPPIncludes();
	void         	SetCPPIncludes(string newCode);
	void         	GenerateH(BDirectory* directory);
	void         	GenerateCPP(BDirectory* directory);
	void         	Export(TextFile* exportFile);
	void         	Import(TextFile* importFile);
	void         	SetWindow(ClassWind* window);
	void         	MethodSignatureChanged(Method* method);
	void         	VariableNameChanged(Variable* variable);
	BRect        	WindFrame();
	void         	SetWindFrame(BRect newWindFrame);
	float        	ListViewsHeight();
	void         	SetListViewsHeight(float newListViewsHeight);
	void         	Clear();
	time_t       	DeepModTime();

protected:
	string    	name;
	Project*  	project;
	BList*    	methods;
	BList*    	variables;
	string    	hIncludes;
	string    	cppIncludes;
	string    	superclasses;
	time_t    	modTime;
	BRect     	windFrame;
	float     	listViewsHeight;
	ClassWind*	wind;
	BLocker   	lock;
	bool      	dirty;
};

inline string Class::Name()
{
	return name;
}




#endif
