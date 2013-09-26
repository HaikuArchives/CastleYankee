/* ConditionalTextFile.h */

#ifndef _H_ConditionalTextFile_
#define _H_ConditionalTextFile_

#include "TextFile.h"

class ConditionalTextFile : public TextFile {
public:
	ConditionalTextFile(BDirectory *dir, const char *path, uint32 openMode);
	~ConditionalTextFile();
	ssize_t	Write(const void* buffer, size_t size);

protected:
	bool       	writing;
	char*      	text;
	const char*	p;
	const char*	stopper;
};


#endif
