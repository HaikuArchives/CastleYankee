/* PrefsChangeClient.h */

#ifndef _H_PrefsChangeClient_
#define _H_PrefsChangeClient_

class PrefsChangeClient {
public:
	PrefsChangeClient();
	virtual ~PrefsChangeClient();
	virtual void	PrefChanged(const char* prefName);
};


#endif
