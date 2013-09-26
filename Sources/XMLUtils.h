/* XMLUtils.h */

#ifndef _H_XMLUtils_
#define _H_XMLUtils_

#include <time.h>

class qstring;
class TextFile;

extern qstring* GetTagAttribute(const char* tag, const char* attribName);
extern time_t GetDateAttribute(const char* tag, const char* attribName);
extern void WriteDateAttribute(TextFile* file, const char* attribName, time_t date);
extern BRect GetRectAttribute(const char* tag, const char* attribName);
extern void WriteRectAttribute(TextFile* file, const char* attribName, BRect rect);
extern void WriteFloatAttribute(TextFile* file, const char* attribName, float value);
extern void WriteAttribute(TextFile* file, const char* attribName, const char* value);

#endif
