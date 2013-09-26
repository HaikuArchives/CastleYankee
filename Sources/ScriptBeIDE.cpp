/* ScriptBeIDE.cpp */

#include "ScriptBeIDE.h"
#include <AppDefs.h>	// must come before "Scripting.h"
#include <SupportDefs.h>
#include <Messenger.h>
#include "Scripting.h"


void MakeBeIDEProject()
{
	// BeIDE uses its own scripting protocol, not the standard Be one
	BMessage message('MMak');
	PropertyItem property;
	strcpy(property.property, "project");
	property.form = formDirect;
	message.AddData("target", PROPERTY_TYPE, &property, sizeof(property));

	BMessenger messenger("application/x-mw-BeIDE");
	messenger.SendMessage(&message);
}


