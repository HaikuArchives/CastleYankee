/* MenuBuilder.cpp */

#include "MenuBuilder.h"
#include "TokenParser.h"
#include "CastleYankeeApp.h"

#include <Resources.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <string.h>


MenuBuilder::MenuBuilder(const char* name)
	: rsrc(NULL), parser(NULL)
{
	// get the resource
	BResources* resources = ((CastleYankeeApp*) be_app)->Resources();
	if (resources == NULL)
		return;
	size_t length;
	char* rsrc = (char*) resources->FindResource('MENU', name, &length);
	if (rsrc == NULL)
		return;

	parser = new TokenParser(rsrc, length);
}


MenuBuilder::~MenuBuilder()
{
	delete parser;
	delete rsrc;
}


void MenuBuilder::BuildInto(BMenu* parentMenu)
{
	if (parser == NULL)
		return;

	while (true) {
		parser->NextLine();
		parser->GetToken(token);
		if (token[0] == 0)
			break;

		if (strcmp(token, "End") == 0)
			break;	// end of this submenu

		else if (strcmp(token, "Menu") == 0) {
			// new menu
			parser->GetToken(token);	// name
			BMenu* subMenu = new BMenu(token);
			BuildInto(subMenu);		// parse the submenu
			parentMenu->AddItem(subMenu);
			}

		else if (token[0] == '-') {
			// separator
			parentMenu->AddSeparatorItem();
			}

		else {
			// menu item
			char shortcut = 0;
			uint32 modifiers = 0;
			// read command
			BMessage* message = ReadCommand();
			// read shortcut
			if (message)
				ReadShortcut(&shortcut, &modifiers);
			// build & add
			parentMenu->AddItem(new BMenuItem(token, message, shortcut, modifiers));
			}
		}
}


void MenuBuilder::AppGetsMenuMessage(uint32 command, BMenuBar* menuBar)
{
	BMenuItem* item = menuBar->FindItem(command);
	if (item)
		item->SetTarget(be_app_messenger);
}


void MenuBuilder::AppGetsMenuMessage(const char* label, BMenuBar* menuBar)
{
	BMenuItem* item = menuBar->FindItem(label);
	if (item)
		item->SetTarget(be_app_messenger);
}


BMessage* MenuBuilder::ReadCommand()
{
	BMessage* message = NULL;

	char msgStr[256];
	parser->GetToken(msgStr);
	if (msgStr[0] != 0) {
		message = new BMessage(ntohl(*(uint32*) msgStr));
		}

	return message;
}


void MenuBuilder::ReadShortcut(char* key, uint32* modifiers)
{
	// defaults
	*key = 0;
	*modifiers = 0;

	char token[256];
	parser->GetToken(token);
	char* p = token;
	while (true) {
		if (strncmp(p, "Shift-", 6) == 0) {
			*modifiers |= B_SHIFT_KEY;
			p += 6;
			}
		else if (strncmp(p, "Option-", 7) == 0) {
			*modifiers |= B_OPTION_KEY;
			p += 7;
			}
		else if (strncmp(p, "Opt-", 4) == 0) {
			*modifiers |= B_OPTION_KEY;
			p += 4;
			}
		else if (strncmp(p, "Control-", 8) == 0) {
			*modifiers |= B_CONTROL_KEY;
			p += 8;
			}
		else if (strncmp(p, "Ctrl-", 5) == 0) {
			*modifiers |= B_CONTROL_KEY;
			p += 5;
			}
		else if (strncmp(p, "Command-", 8) == 0) {
			*modifiers |= B_COMMAND_KEY;
			p += 8;
			}
		else if (strncmp(p, "Cmd-", 4) == 0) {
			*modifiers |= B_COMMAND_KEY;
			p += 4;
			}
		else {
			// no more modifiers, we're at the key itself
			*key = *p;
			break;
			}
		}
}


