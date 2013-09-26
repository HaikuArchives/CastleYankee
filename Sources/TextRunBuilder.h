/* TextRunBuilder.h */

#ifndef _H_TextRunBuilder_
#define _H_TextRunBuilder_

#include <Font.h>
#include <TextView.h>

class TextRunBuilder {
public:
	TextRunBuilder();
	~TextRunBuilder();

	void	AddRun(int32 offset, const BFont* font, rgb_color color);

	text_run_array*	RunArray();

protected:
	text_run_array*	runs;
	int32          	runsAllocated;

	text_run_array*	AllocRuns(int32 numRuns);
};

#endif
