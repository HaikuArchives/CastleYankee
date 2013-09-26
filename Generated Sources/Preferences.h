/* Preferences.h */

#ifndef _H_Preferences_
#define _H_Preferences_

#include <SupportDefs.h>

class PrefsChangeClient;

class Preferences {
public:
	Preferences(const char* prefsFileName);
	~Preferences();
	status_t           	InitCheck();
	void               	Save();
	bool               	GetBoolPref(const char* prefName, bool defaultVal = false);
	void               	SetBoolPref(const char* prefName, bool value);
	int32              	GetInt32Pref(const char* prefName, int32 defaultVal = 0);
	void               	SetInt32Pref(const char* prefName, int32 value);
	const char*        	GetStringPref(const char* prefName, const char* defaultVal = "");
	void               	SetStringPref(const char* prefName, const char* value);
	static Preferences*	GetPreferences(const char* prefsFileName);
	void               	AddChangeClient(PrefsChangeClient* client);
	void               	RemoveChangeClient(PrefsChangeClient* client);
	void               	PrefChanged(const char* prefName);

protected:
	BMessage*          	prefs;
	BFile*             	file;
	BList*             	changeClients;
	status_t           	initStatus;
	static Preferences*	preferences;
};


#endif
