/* Error.cpp */

#include "Error.h"
#include <stdarg.h>
#include <stdio.h>

#include <Alert.h>


Error::Error(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);

	char msgStr[1024];
	vsprintf(msgStr, format, argList);
	msg = new char[strlen(msgStr) + 1];
	strcpy(msg, msgStr);

	va_end(argList);
}


Error::~Error()
{
	delete msg;
}


void Error::Display()
{
	BAlert* alert = new BAlert(NULL, msg, "OK");
	alert->Go(NULL);
}


