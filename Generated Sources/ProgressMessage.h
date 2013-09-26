/* ProgressMessage.h */

#ifndef _H_ProgressMessage_
#define _H_ProgressMessage_

#include <string>

class MessageWind;

class ProgressMessage {
public:
	ProgressMessage(const char* message);
	~ProgressMessage();

protected:
	MessageWind*	wind;
};


#endif
