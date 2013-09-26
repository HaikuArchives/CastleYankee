/* AccessType.cpp */

#include "AccessType.h"
#include "qstring.h"


AccessType StrToAccessType(qstring* str)
{
	if (*str == "public")
		return Public;
	else if (*str == "protected")
		return Protected;
	else if (*str == "private")
		return Private;
	else
		return NoAccessType;
}

