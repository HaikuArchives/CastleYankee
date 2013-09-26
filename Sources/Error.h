/* Error.h */

#ifndef _H_Error_
#define _H_Error_

class Error {
public:
//***	Error(const char* message) : msg(message) {}
	Error(const char* format, ...);
	~Error();

	void	Display();

protected:
	char*	msg;
};

#endif
