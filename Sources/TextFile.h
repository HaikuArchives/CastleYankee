/* TextFile.h - extension of BFile with text handling */

#ifndef _H_TextFile_
#define _H_TextFile_

#include <string>
#include <File.h>

class qstring;

class TextFile : public BFile {
public:
	TextFile() : BFile() {}
	TextFile(const BFile& file) : BFile(file) {}
	TextFile(const entry_ref *ref, uint32 openMode) : BFile(ref, openMode) {}
	TextFile(const BEntry *entry, uint32 openMode) : BFile(entry, openMode) {}
	TextFile(const char *path, uint32 openMode) : BFile(path, openMode) {}
	TextFile(BDirectory *dir, const char *path, uint32 openMode)
		: BFile(dir, path, openMode) {}

	ssize_t	WriteString(const char* str);
	ssize_t	WriteString(const string& str);
	ssize_t	WriteString(const qstring& str);

	qstring*	ReadLine();
	void    	ReadLine(qstring* line);
	bool    	AtEOF();
};

#endif
