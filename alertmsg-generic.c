#include <stdio.h>
#include "alertmsg.h"
//Generic AlertMsg for non-Win32 systems

#ifndef SHIMLIBS_PROVIDES_ALERTMSG
	#define SHIMLIBS_PROVIDES_ALERTMSG

	void AlertMsg (const char *Message, const char *Title)
	{
		printf("[%s]:\n\t%s", Title, Message);
		/*	Example:
				This is an example message.
		*/
	}
#endif