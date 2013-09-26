/* Class.cpp */

#include "Class.h"
#include "Method.h"
#include "Variable.h"
#include "Project.h"
#include "ClassWind.h"
#include "ConditionalTextFile.h"
#include "XMLUtils.h"
#include "qstring.h"
#include "Error.h"

#include <List.h>
#include <Autolock.h>
#include <Entry.h>
#include <Directory.h>
#include <stdio.h>


Class::Class(Project* projectIn)
	: project(projectIn), modTime(0), listViewsHeight(0),
	  wind(NULL)
{
	methods = new BList();
	variables = new BList();
}


Class::~Class()
{
	Clear();
}


void Class::Dirtify()
{
	modTime = real_time_clock();
	dirty = true;
	project->Dirtify();
}


void Class::Save()
{
	if (dirty)
		project->Save();
}


void Class::SetName(string newName)
{
	BAutolock locker(lock);

	// change the name
	name = newName;
	Dirtify();

	// notify the project
	if (project)
		project->ClassNameChanged(this);
}


string Class::Superclasses()
{
	BAutolock locker(lock);

	return superclasses;
}


void Class::SetSuperclasses(string newCode)
{
	BAutolock locker(lock);

	superclasses = newCode;
	Dirtify();
}


int32 Class::NumMethods()
{
	BAutolock locker(lock);

	return methods->CountItems();
}


string Class::MethodSignatureAt(int32 index)
{
	BAutolock locker(lock);

	Method* method = (Method*) methods->ItemAt(index);
	if (method == NULL)
		return "";
	return method->Signature();
}


bool Class::MethodIsUglyNotesHackAt(int32 index)
{
	BAutolock locker(lock);

	Method* method = (Method*) methods->ItemAt(index);
	if (method == NULL)
		return false;
	return method->IsUglyNotesHack();
}


Method* Class::MethodAt(int32 index)
{
	return (Method*) methods->ItemAt(index);
}


void Class::AddMethod(int32 index)
{
	BAutolock locker(lock);

	Method* method = new Method(this);
	methods->AddItem(method, index);
	Dirtify();
}


void Class::RemoveMethodAt(int32 index)
{
	BAutolock locker(lock);

	Method* method = (Method*) methods->ItemAt(index);
	if (method == NULL)
		return;
	methods->RemoveItem(method);
	delete method;
	Dirtify();
}


void Class::MoveMethod(int32 fromIndex, int32 toIndex)
{
	BAutolock locker(lock);

	Method* method = (Method*) methods->ItemAt(fromIndex);
	if (method == NULL)
		return;
	methods->RemoveItem(fromIndex);
	methods->AddItem(method, toIndex);
	Dirtify();
}


int32 Class::NumVariables()
{
	BAutolock locker(lock);

	return variables->CountItems();
}


string Class::VariableNameAt(int32 index)
{
	BAutolock locker(lock);

	Variable* var = (Variable*) variables->ItemAt(index);
	if (var == NULL)
		return NULL;
	return var->Name();
}


bool Class::VariableIsUglyNotesHackAt(int32 index)
{
	BAutolock locker(lock);

	Variable* var = (Variable*) variables->ItemAt(index);
	if (var == NULL)
		return false;
	return var->IsUglyNotesHack();
}


Variable* Class::VariableAt(int32 index)
{
	return (Variable*) variables->ItemAt(index);
}


void Class::AddVariable(int32 index)
{
	BAutolock locker(lock);

	Variable* variable = new Variable(this);
	variables->AddItem(variable, index);
	Dirtify();
}


void Class::RemoveVariableAt(int32 index)
{
	BAutolock locker(lock);

	Variable* variable = (Variable*) variables->ItemAt(index);
	if (variable == NULL)
		return;
	variables->RemoveItem(variable);
	delete variable;
	Dirtify();
}


void Class::MoveVariable(int32 fromIndex, int32 toIndex)
{
	BAutolock locker(lock);

	Variable* variable = (Variable*) variables->ItemAt(fromIndex);
	if (variable == NULL)
		return;
	variables->RemoveItem(fromIndex);
	variables->AddItem(variable, toIndex);
	Dirtify();
}


void Class::RemoveAllSubobjects()
{
	int	i;

	BAutolock locker(lock);

	// methods
	int32 numMethods = methods->CountItems();
	for (i=0; i<numMethods; i++) {
		Method* method = (Method*) methods->ItemAt(i);
		delete method;
		}
	methods->MakeEmpty();

	// variables
	int32 numVariables = variables->CountItems();
	for (i=0; i<numVariables; i++) {
		Variable* variable = (Variable*) variables->ItemAt(i);
		delete variable;
		}
	variables->MakeEmpty();

	Dirtify();
}


string Class::HIncludes()
{
	BAutolock locker(lock);

	return hIncludes;
}


void Class::SetHIncludes(string newCode)
{
	BAutolock locker(lock);

	hIncludes = newCode;
	Dirtify();
}


string Class::CPPIncludes()
{
	BAutolock locker(lock);

	return cppIncludes;
}


void Class::SetCPPIncludes(string newCode)
{
	BAutolock locker(lock);

	cppIncludes = newCode;
	Dirtify();
}


void Class::GenerateH(BDirectory* directory)
{
	int	i;

	static const char* accessTypeLines[] = { "", "public:\n", "protected:\n", "private:\n" };

	BAutolock locker(lock);

	// see if we need to update the file
	string fileName = name;
	fileName += ".h";
	BEntry entry;
	directory->FindEntry(fileName.c_str(), &entry);
	if (entry.InitCheck() == B_NO_ERROR) {
		// the file exists.  Was it updated since the last modTime?
		time_t fileModTime = 0;
		entry.GetModificationTime(&fileModTime);
		if (fileModTime >= DeepModTime()) {
			// no mods since last update; no need to write the file
			return;
			}
		}

	// create the file
	ConditionalTextFile file(directory, fileName.c_str(),
	                         B_READ_WRITE | B_CREATE_FILE);
	if (file.InitCheck() != B_NO_ERROR)
		throw Error("Couldn't create \"%s\":  Error %d.", fileName.c_str(), file.InitCheck());

	// write the file
	// name comment
	char line[256];
	sprintf(line, "/* %s.h */\n\n", name.c_str());
	file.WriteString(line);

	// recursion control prefix
	sprintf(line, "#ifndef _H_%s_\n", name.c_str());
	file.WriteString(line);
	sprintf(line, "#define _H_%s_\n\n", name.c_str());
	file.WriteString(line);

	// .h includes
	file.WriteString(hIncludes);
	int32 includesLen = hIncludes.length();
	if (includesLen > 0) {
		if (hIncludes[includesLen - 1] != '\n')
			file.WriteString("\n");
		file.WriteString("\n");
		}

	int32 numMethods = methods->CountItems();
	int32 numVariables = variables->CountItems();

	// class prefix
	if (numMethods > 0 || numVariables > 0) {
		sprintf(line, "class %s ", name.c_str());
		file.WriteString(line);
		if (superclasses.length() > 0) {
			file.WriteString(": ");
			file.WriteString(superclasses);
			file.WriteString(" ");
			}
		file.WriteString("{\n");
		}

	AccessType curAccessType = NoAccessType;
	bool needSectionDivider = false;

	// method decls
	Method* method;
	// calc the spec length
	int32 specLength = 0;
	for (i=0; i<numMethods; i++) {
		method = (Method*) methods->ItemAt(i);
		if (method == NULL)
			throw Error("Internal Error: Missing method in GenerateH.");
		if (!method->IsUglyNotesHack()) {
			int32 curSpecLength = method->HSpecLength();
			if (curSpecLength > specLength)
				specLength = curSpecLength;
			}
		}
	// generate
	for (i=0; i<numMethods; i++) {
		method = (Method*) methods->ItemAt(i);
		if (method->IsUglyNotesHack())
			continue;

		// change the access type if necessary
		AccessType methodAccess = method->Access();
		if (methodAccess != curAccessType && methodAccess != NoAccessType) {
			if (needSectionDivider) {
				file.WriteString("\n");
				needSectionDivider = false;
				}
			file.WriteString(accessTypeLines[methodAccess]);
			curAccessType = methodAccess;
			}

		// write the method decl
		method->GenerateH(&file, specLength);
		needSectionDivider = true;
		}

	// variable decls
	bool sectionStarted = false;
	Variable* variable;
	// calc the spec length
	specLength = 0;
	for (i=0; i<numVariables; i++) {
		variable = (Variable*) variables->ItemAt(i);
		if (variable == NULL)
			throw Error("Internal Error: Missing variable in GenerateH.");
		if (!variable->IsUglyNotesHack()) {
			int32 curSpecLength = variable->HSpecLength();
			if (curSpecLength > specLength)
				specLength = curSpecLength;
			}
		}
	// generate
	for (i=0; i<numVariables; i++) {
		variable = (Variable*) variables->ItemAt(i);
		if (variable->IsUglyNotesHack())
			continue;

		// change the access type if necessary
		AccessType varAccess = variable->Access();
		if (varAccess != curAccessType && varAccess != NoAccessType) {
			if (needSectionDivider) {
				file.WriteString("\n");
				needSectionDivider = false;
				}
			file.WriteString(accessTypeLines[varAccess]);
			curAccessType = varAccess;
			}

		// divide from the methods section with a blank line
		if (!sectionStarted && needSectionDivider)
			file.WriteString("\n");

		// write the variable decl
		variable->GenerateH(&file, specLength);
		sectionStarted = true;
		needSectionDivider = true;
		}

	// class suffix
	if (numMethods > 0 || numVariables > 0)
		file.WriteString("};\n\n");

	// inline method definitions
	for (i=0; i<numMethods; i++) {
		method = (Method*) methods->ItemAt(i);
		if (method->IsInline()) {
			method->GenerateCPP(&file);
			file.WriteString("\n");
			}
		}

	// recursion control suffix
	file.WriteString("\n#endif\n");
}


void Class::GenerateCPP(BDirectory* directory)
{
	int	i;

	BAutolock locker(lock);

	// see if we need to update the file
	string fileName = name;
	fileName += ".cpp";
	BEntry entry;
	directory->FindEntry(fileName.c_str(), &entry);
	if (entry.InitCheck() == B_NO_ERROR) {
		// the file exists.  Was it updated since the last modTime?
		time_t fileModTime = 0;
		entry.GetModificationTime(&fileModTime);
		if (fileModTime >= DeepModTime()) {
			// no mods since last update; no need to write the file
			return;
			}
		}

	// create the file
	TextFile file;
	directory->CreateFile(fileName.c_str(), &file, false);
	if (file.InitCheck() != B_NO_ERROR)
		throw Error("Couldn't create \"%s\":  Error %d.", fileName.c_str(), file.InitCheck());

	// write the file
	// name comment
	char line[256];
	sprintf(line, "/* %s.cpp */\n\n", name.c_str());
	file.WriteString(line);

	// #includes
	sprintf(line, "#include \"%s.h\"\n", name.c_str());
	file.WriteString(line);
	file.WriteString(cppIncludes);
	int32 includesLen = cppIncludes.length();
	if (includesLen > 0 && cppIncludes[includesLen - 1] != '\n')
		file.WriteString("\n");
	file.WriteString("\n");

	// variable decls
	int32 numVariables = variables->CountItems();
	for (i=0; i<numVariables; i++) {
		Variable* variable = (Variable*) variables->ItemAt(i);
		if (variable == NULL)
			throw Error("Internal Error: Missing variable in GenerateCPP.");
		variable->GenerateCPP(&file);
		}

	// method decls
	int32 numMethods = methods->CountItems();
	if (numMethods > 0 && numVariables > 0)
		file.WriteString("\n");		// need a divider line
	file.WriteString("\n");			// always want one more divider
	for (i=0; i<numMethods; i++) {
		Method* method = (Method*) methods->ItemAt(i);
		if (method == NULL)
			throw Error("Internal Error: Missing method in GenerateCPP.");
		if (!method->IsInline())
			method->GenerateCPP(&file);
		}
}


void Class::Export(TextFile* exportFile)
{
	int i;

	// start tag and attributes
	exportFile->WriteString("<class");
	WriteDateAttribute(exportFile, "mod-time", modTime);
	WriteRectAttribute(exportFile, "wind-frame", windFrame);
	WriteFloatAttribute(exportFile, "list-views-height", listViewsHeight);
	exportFile->WriteString(">\n");

	// name
	exportFile->WriteString("<name>\n");
	exportFile->WriteString(name);
	exportFile->WriteString("\n</name>\n");

	// superclasses
	if (superclasses.length() > 0) {
		exportFile->WriteString("<superclasses>\n");
		exportFile->WriteString(superclasses);
		exportFile->WriteString("\n");
		exportFile->WriteString("</superclasses>\n");
		}

	// hIncludes
	int codeLen = hIncludes.length();
	if (codeLen > 0) {
		exportFile->WriteString("<hIncludes><![CDATA[\n");
		exportFile->WriteString(hIncludes);
		if (hIncludes[codeLen-1] != '\n')
			exportFile->WriteString("\n");	// make sure it ends with a newline
		exportFile->WriteString("]]></hIncludes>\n");
		}

	// cppIncludes
	codeLen = cppIncludes.length();
	if (codeLen > 0) {
		exportFile->WriteString("<cppIncludes><![CDATA[\n");
		exportFile->WriteString(cppIncludes);
		if (cppIncludes[codeLen-1] != '\n')
			exportFile->WriteString("\n");	// make sure it ends with a newline
		exportFile->WriteString("]]></cppIncludes>\n");
		}

	// methods
	int32 numMethods = methods->CountItems();
	for (i=0; i<numMethods; i++) {
		Method* method = (Method*) methods->ItemAt(i);
		method->Export(exportFile);
		exportFile->WriteString("\n");
		}

	// variables
	int32 numVariables = variables->CountItems();
	for (i=0; i<numVariables; i++) {
		Variable* variable = (Variable*) variables->ItemAt(i);
		variable->Export(exportFile);
		exportFile->WriteString("\n");
		}

	exportFile->WriteString("</class>\n\n\n");
}


void Class::Import(TextFile* importFile)
{
	qstring line;
	qstring code;

	// search for class start tag
	bool foundStart = false;
	while (!foundStart && !importFile->AtEOF()) {
		importFile->ReadLine(&line);
		if (line.startsWith("<class"))
			foundStart = true;
		}
	if (!foundStart)
		return;
	qstring classStartTag = line;

	// handle rest of the tags
	bool done = false;
	while (!done && !importFile->AtEOF()) {
		importFile->ReadLine(&line);

		if (line.startsWith("</class"))
			done = true;

		else if (line.startsWith("<name")) {
			qstring newName;
			while (!importFile->AtEOF()) {
				importFile->ReadLine(&line);
				if (line.startsWith("</name"))
					break;
				newName += line;
				}
			SetName(newName.c_str());
			}

		else if (line.startsWith("<superclasses")) {
			qstring newSuperclasses;
			bool started = false;
			while (!importFile->AtEOF()) {
				importFile->ReadLine(&line);
				if (line.startsWith("</superclasses"))
					break;
				if (started)
					newSuperclasses += '\n';
				newSuperclasses += line;
				started = true;
				}
			SetSuperclasses(newSuperclasses.c_str());
			}

		else if (line.startsWith("<hIncludes")) {
			code = "";
			while (!importFile->AtEOF()) {
				importFile->ReadLine(&line);
				if (line.startsWith("</hIncludes") || line.startsWith("]]></hIncludes"))
					break;
				code += line;
				code += '\n';
				}
			if (code.length() > 0)
				SetHIncludes(code.c_str());
			}

		else if (line.startsWith("<cppIncludes")) {
			code = "";
			while (!importFile->AtEOF()) {
				importFile->ReadLine(&line);
				if (line.startsWith("</cppIncludes") || line.startsWith("]]></cppIncludes"))
					break;
				code += line;
				code += '\n';
				}
			if (code.length() > 0)
				SetCPPIncludes(code.c_str());
			}

		else if (line.startsWith("<method")) {
			qstring startTag = line;
			code = "";
			while (!importFile->AtEOF()) {
				importFile->ReadLine(&line);
				if (line.startsWith("</method") || line.startsWith("]]></method"))
					break;
				code += line;
				code += '\n';
				}
			if (code.length() > 0) {
				int32 newIndex = NumMethods();
				AddMethod(newIndex);
				Method* method = (Method*) methods->ItemAt(newIndex);
				method->SetCode(code.c_str());
				method->ReadAttributes(startTag.c_str());
				}
			}

		else if (line.startsWith("<variable")) {
			qstring startTag = line;
			code = "";
			bool started = false;
			while (!importFile->AtEOF()) {
				importFile->ReadLine(&line);
				if (line.startsWith("</variable") || line.startsWith("]]></variable"))
					break;
				if (started)
					code += '\n';
				code += line;
				started = true;
				}
			if (code.length() > 0) {
				int32 newIndex = NumVariables();
				AddVariable(newIndex);
				Variable* variable = (Variable*) variables->ItemAt(newIndex);
				variable->SetCode(code.c_str());
				variable->ReadAttributes(startTag.c_str());
				}
			}
		}

	// finally, read the attributes from the start tag
	// do this last so the "modTime" attribute doesn't get clobbered
	time_t modTimeAttr = GetDateAttribute(classStartTag.c_str(), "mod-time");
	if (modTimeAttr != 0)
		modTime = modTimeAttr;
	BRect windFrameAttr = GetRectAttribute(classStartTag.c_str(), "wind-frame");
	if (windFrameAttr.IsValid())
		windFrame = windFrameAttr;
	qstring* listViewsHeightAttr = GetTagAttribute(classStartTag.c_str(), "list-views-height");
	if (listViewsHeightAttr) {
		listViewsHeight = atof(listViewsHeightAttr->c_str());
		delete listViewsHeightAttr;
		}
}


void Class::SetWindow(ClassWind* window)
{
	wind = window;
}


void Class::MethodSignatureChanged(Method* method)
{
	BAutolock locker(lock);

	Dirtify();

	// update display
	if (wind)
		wind->InvalidateMethods();
}


void Class::VariableNameChanged(Variable* variable)
{
	BAutolock locker(lock);

	Dirtify();

	// update display
	if (wind)
		wind->InvalidateVariables();
}


BRect Class::WindFrame()
{
	return windFrame;
}


void Class::SetWindFrame(BRect newWindFrame)
{
	windFrame = newWindFrame;
	Dirtify();
}


float Class::ListViewsHeight()
{
	return listViewsHeight;
}


void Class::SetListViewsHeight(float newListViewsHeight)
{
	listViewsHeight = newListViewsHeight;
	Dirtify();
}


void Class::Clear()
{
	int32 i;

	// methods
	for (i = methods->CountItems() - 1; i >= 0; --i) {
		Method* method = (Method*) methods->ItemAt(i);
		delete method;
		}
	methods->MakeEmpty();

	// variables
	for (i = variables->CountItems() - 1; i >= 0; --i) {
		Variable* var = (Variable*) variables->ItemAt(i);
		delete var;
		}
	variables->MakeEmpty();
}


time_t Class::DeepModTime()
{
	int32	i;

	time_t deepModTime = modTime;

	// check methods
	int32 numMethods = methods->CountItems();
	for (i=0; i<numMethods; i++) {
		Method* method = (Method*) methods->ItemAt(i);
		if (method == NULL)
			continue;
		time_t methodModTime = method->ModTime();
		if (methodModTime > deepModTime)
			deepModTime = methodModTime;
		}

	// check variables
	int32 numVariables = variables->CountItems();
	for (i=0; i<numVariables; i++) {
		Variable* variable = (Variable*) variables->ItemAt(i);
		if (variable == NULL)
			continue;
		time_t varModTime = variable->ModTime();
		if (varModTime > deepModTime)
			deepModTime = varModTime;
		}

	return deepModTime;
}


