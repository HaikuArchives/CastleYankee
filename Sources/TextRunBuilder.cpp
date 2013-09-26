/* TextRunBuilder.cpp */

#include "TextRunBuilder.h"

static const int32 RunChunkSize = 20;


TextRunBuilder::TextRunBuilder()
{
	runs = AllocRuns(RunChunkSize);
	runs->count = 0;
	runsAllocated = RunChunkSize;
}


TextRunBuilder::~TextRunBuilder()
{
	delete runs;
}


void TextRunBuilder::AddRun(int32 offset, const BFont* font, rgb_color color)
{
	// grow it if necessary
	if (runs->count == runsAllocated) {
		text_run_array* newRuns = AllocRuns(runsAllocated + RunChunkSize);
		newRuns->count = runs->count;
		for (int32 i=0; i<runs->count; i++)
			newRuns->runs[i] = runs->runs[i];
		delete runs;
		runs = newRuns;
		runsAllocated += RunChunkSize;
		}

	// add it
	text_run* run = &runs->runs[runs->count];
	run->offset = offset;
	run->font = font;
	run->color = color;
	runs->count += 1;
}


text_run_array* TextRunBuilder::RunArray()
{
	return runs;
}


text_run_array* TextRunBuilder::AllocRuns(int32 numRuns)
{
	return (text_run_array*) new char[sizeof(int32) + numRuns * sizeof(text_run)];
}


