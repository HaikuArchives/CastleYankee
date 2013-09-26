/* AccessType.h */

#ifndef _H_AccessType_
#define _H_AccessType_

enum AccessType {
	NoAccessType,
	Public,
	Protected,
	Private
};

class qstring;

extern AccessType StrToAccessType(qstring* str);

#endif
