/* ConditionalTextFile.cpp */

#include "ConditionalTextFile.h"

#include <stdio.h>	// just for SEEK_SET



ConditionalTextFile::ConditionalTextFile(BDirectory* dir, const char* path, uint32 openMode)
	: TextFile(dir, path, openMode), writing(true), text(NULL)
{
	if (InitCheck() != B_NO_ERROR)
		return;

	// read the file
	off_t size = 0;
	GetSize(&size);
	text = new char[size];
	ssize_t result = ReadAt(0, text, size);
	if (result < B_NO_ERROR)
		return;
	p = text;
	stopper = text + size;
	writing = false;
}


ConditionalTextFile::~ConditionalTextFile()
{
	// need to set the size
	if (writing || (p < stopper)) {
		off_t newSize = (writing ? Position() : (p - text));
		SetSize(newSize);
		}

	delete text;
}


ssize_t ConditionalTextFile::Write(const void* buffer, size_t size)
{
	if (writing)
		return TextFile::Write(buffer, size);

	// see if the file is the same up to this point
	if (size > stopper - p) {
		// writing beyond the end
		writing = true;
		}
	else if (memcmp(buffer, p, size) == 0) {
		// they're the same, don't bother to write.  just bump "p"
		p += size;
		}
	else {
		// they're not the same, we need to start writing
		writing = true;
		}

	if (writing) {
		// set up to start writing, and do it
		off_t result = Seek(p - text, SEEK_SET);
		if (result < B_NO_ERROR)
			return result;
		delete text;
		text = NULL;
		return TextFile::Write(buffer, size);
		}
	else
		return B_NO_ERROR;
}


