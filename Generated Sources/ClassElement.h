/* ClassElement.h */

#ifndef _H_ClassElement_
#define _H_ClassElement_

#include "AccessType.h"

#include <time.h>

class Class;
class TextFile;
class qstring;

class ClassElement {
public:
	ClassElement(Class* classIn);
	AccessType  	Access();
	void        	SetAccess(AccessType newAccess);
	time_t      	ModTime();
	void        	Dirtify();
	virtual void	WriteAttributes(TextFile* file);
	virtual void	ReadAttributes(const char* tag);

protected:
	Class*    	classs;
	AccessType	accessType;
	time_t    	modTime;
};


#endif
