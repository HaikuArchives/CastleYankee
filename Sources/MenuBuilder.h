/* MenuBuilder.h */

#ifndef _H_MenuBuilder_
#define _H_MenuBuilder_

#include <Menu.h>

class TokenParser;

class MenuBuilder {
public:
	MenuBuilder(const char* name);
	~MenuBuilder();

	void	BuildInto(BMenu* parentMenu);

	// handy util functions
	static void	AppGetsMenuMessage(uint32 command, BMenuBar* menuBar);
	static void	AppGetsMenuMessage(const char* label, BMenuBar* menuBar);

protected:
	char*       	rsrc;
	TokenParser*	parser;
	char        	token[256];	// for convenience

	BMessage*	ReadCommand();
	void     	ReadShortcut(char* key, uint32* modifiers);
};

#endif
