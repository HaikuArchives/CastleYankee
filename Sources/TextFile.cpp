/* TextFile.cpp - extension of BFile with text handling */

#include "TextFile.h"
#include "qstring.h"
#include <string>

#include <stdio.h>	// just for SEEK_CUR


ssize_t TextFile::WriteString(const char* str)
{
	return Write(str, strlen(str));
}


ssize_t TextFile::WriteString(const string& str)
{
	return Write(str.data(), str.length());
}


ssize_t TextFile::WriteString(const qstring& str)
{
	return Write(str.data(), str.length());
}


qstring* TextFile::ReadLine()
{
	qstring* line = new qstring;
	ReadLine(line);
	return line;
}


void TextFile::ReadLine(qstring* line)
{
	static const int bufSize = 256;

	char buffer[bufSize];
	*line = "";
	while (true) {
		// get the buffer
		ssize_t bytesRead = Read(buffer, bufSize);
		if (bytesRead == 0)
			break;
		// try to find the end of the line
		const char* p = buffer;
		const char* stopper = &buffer[bytesRead];
		bool found = false;
		while (p < stopper) {
			if (*p++ == '\n') {
				found = true;
				break;
				}
			}
		// add to the line
		line->append(buffer, p - buffer - found);	// "- found" because we don't want to include the '\n'
		// if we found the EOL, finish up
		if (found) {
			// rewind the file
			Seek(-(stopper - p), SEEK_CUR);
			break;
			}
		}
}


bool TextFile::AtEOF()
{
	off_t size;
	GetSize(&size);
	return (Position() == size);
}


