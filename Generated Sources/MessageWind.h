/* MessageWind.h */

#ifndef _H_MessageWind_
#define _H_MessageWind_

#include <Window.h>

class BStringView;

class MessageWind : public BWindow {
public:
	MessageWind(const char* message);

protected:
	BStringView*	messageView;
};


#endif
