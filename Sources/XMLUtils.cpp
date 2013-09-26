/* XMLUtils.cpp */

#include "XMLUtils.h"
#include "TextFile.h"
#include "qstring.h"
#include <parsedate.h>

#include <Rect.h>
#include <stdio.h>

qstring* GetTagAttribute(const char* tag, const char* attribName)
{
	const char* p = tag;
	if (*p++ != '<')
		return NULL;	// not a tag

	// skip the tag name
	char c;
	while (true) {
		c = *p++;
		if (c == '>' || c == 0)
			return NULL;	// no attributes
		if (c == ' ' || c == '\t')
			break;
		}

	// parse attributes
	while (true) {
		// skip whitespace
		while (*p == ' ' || *p == '\t')
			p++;

		// check for end of tag
		if (*p == '>' || *p == 0)
			break;

		// get the attrib name
		const char* attribNameStart = p;
		while (true) {
			c = *p;
			if (c == '=' || c == ' ' || c == '\t' || c == '>' || c == 0)
				break;
			p++;
			}

		// see if it's the attribute we're looking for.
		// I don't trust strnxxx() functions after having difficulties
		// with strncpy()
		bool isRequestedAttribute = false;
		const char* tagNameP = attribNameStart;
		const char* reqP = attribName;
		while (true) {
			// check for end of either string
			if (*reqP == 0) {
				isRequestedAttribute = (tagNameP == p);
				break;
				}
			else if (tagNameP == p)
				break;
			// test
			if (*tagNameP++ != *reqP++)
				break;
			}

		// find the attribute's value
		const char* attribValueStart = p;
		const char* attribValueEnd = p;
		if (*p == '=') {		// null value otherwise
			// skip past the '='
			p++;
			attribValueStart++;
			// quoted--currently handles neither C-style nor Internet-style
			// escapes
			if (*p == '"') {
				// skip past the '"'
				p++;
				attribValueStart++;
				// get the string
				while (*p != '"' && *p != 0)
					p++;
				attribValueEnd = p;
				// skip the trailing '"'
				p++;
				}
			// unquoted
			else {
				while ((c = *p) != 0 && c != '>' && c != ' ' && c != '\t')
					p++;
				attribValueEnd = p;
				}
			}

		// if this is the attribute, return its value
		if (isRequestedAttribute) {
			qstring* value = new qstring;
			value->append(attribValueStart, attribValueEnd - attribValueStart);
			return value;
			}
		}
	return NULL;	// didn't find it
}


time_t GetDateAttribute(const char* tag, const char* attribName)
{
	qstring* attr = GetTagAttribute(tag, attribName);
	if (attr == NULL)
		return 0;
	time_t result = parsedate(attr->c_str(), -1);
	delete attr;
	return result;
}


void WriteDateAttribute(TextFile* file, const char* attribName, time_t date)
{
	char dateStr[128];
	struct tm* time = localtime(&date);
	strftime(dateStr, 128, "%a, %d %b %Y %H:%M:%S %Z", time);
	WriteAttribute(file, attribName, dateStr);
}


BRect GetRectAttribute(const char* tag, const char* attribName)
{
	qstring* attr = GetTagAttribute(tag, attribName);
	if (attr == NULL)
		return BRect();
	BRect result;
	string_slice specStr = *attr;
	result.left = specStr.separate(",", &specStr).asDouble();
	result.top = specStr.separate(",", &specStr).asDouble();
	result.right = specStr.separate(",", &specStr).asDouble();
	result.bottom = specStr.separate(",", &specStr).asDouble();
	return result;
}


void WriteRectAttribute(TextFile* file, const char* attribName, BRect rect)
{
	char str[128];
	sprintf(str, "%f,%f,%f,%f", rect.left, rect.top, rect.right, rect.bottom);
	WriteAttribute(file, attribName, str);
}


void WriteFloatAttribute(TextFile* file, const char* attribName, float value)
{
	char str[128];
	sprintf(str, "%f", value);
	WriteAttribute(file, attribName, str);
}


void WriteAttribute(TextFile* file, const char* attribName, const char* value)
{
	file->WriteString(" ");
	file->WriteString(attribName);
	file->WriteString("=\"");
	file->WriteString(value);
	file->WriteString("\"");
}



