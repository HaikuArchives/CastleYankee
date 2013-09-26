/* ClassElement.cpp */

#include "ClassElement.h"
#include "Class.h"
#include "TextFile.h"
#include "XMLUtils.h"
#include "qstring.h"



ClassElement::ClassElement(Class* classIn)
	: classs(classIn), accessType(NoAccessType), modTime(0)
{
}


AccessType ClassElement::Access()
{
	return accessType;
}


void ClassElement::SetAccess(AccessType newAccess)
{
	accessType = newAccess;
	Dirtify();
}


time_t ClassElement::ModTime()
{
	return modTime;
}


void ClassElement::Dirtify()
{
	modTime = real_time_clock();
	classs->Dirtify();
}


void ClassElement::WriteAttributes(TextFile* file)
{
	// access
	static const char* accessTypeNames[] = { NULL, "public", "protected", "private" };
	const char* accessTypeStr = accessTypeNames[Access()];
	if (accessTypeStr) {
		file->WriteString(" access=\"");
		file->WriteString(accessTypeStr);
		file->WriteString("\"");
		}

	// modTime
	WriteDateAttribute(file, "mod-time", modTime);
}


void ClassElement::ReadAttributes(const char* tag)
{
	// access
	qstring* accessAttr = GetTagAttribute(tag, "access");
	if (accessAttr) {
		accessType = StrToAccessType(accessAttr);
		delete accessAttr;
		}

	// modTime
	modTime = GetDateAttribute(tag, "mod-time");
}


