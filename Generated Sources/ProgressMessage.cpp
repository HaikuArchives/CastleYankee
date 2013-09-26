/* ProgressMessage.cpp */

#include "ProgressMessage.h"
#include "MessageWind.h"



ProgressMessage::ProgressMessage(const char* message)
{
	wind = new MessageWind(message);
}


ProgressMessage::~ProgressMessage()
{
	wind->Close();
}


