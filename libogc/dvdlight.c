/*-------------------------------------------------------------

dvdlight.c -- Wii DVD light subsystem

Copyright (C) 2022
Michael Wiedenbauer (shagkur)
Dave Murphy (WinterMute)
Harrison (HTV04)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/


#if defined(HW_RVL)

#include <time.h>
#include <lwp.h>
#include "dvdlight.h"

#define DVDLIGHT_REGISTER	0xCD0000C0
#define DVDLIGHT_LED		0x20

static lwp_t dvdlight_thread = 0;
static u32 *dvdlight_register = (u32*) DVDLIGHT_REGISTER;
static bool dvdlight_on = false;
static u8 dvdlight_level = 0;

struct timespec dvdlight_timeOn;
struct timespec dvdlight_timeOff;

// The light needs to be turned on and off to simulate the light intensity.
static void *dvdlight_loop(void *arg)
{
	struct timespec timeOn;
	struct timespec timeOff;

	while (dvdlight_on == true) {
		if (dvdlight_level == 255) {
			*dvdlight_register |= DVDLIGHT_LED; // Turn light on
		} else if (dvdlight_level == 0) {
			*dvdlight_register &= ~DVDLIGHT_LED; // Turn light off
		} else {
			timeOn = dvdlight_timeOn;
			timeOff = dvdlight_timeOff;

			*dvdlight_register |= DVDLIGHT_LED; // Turn light on
			nanosleep(&timeOn, NULL);
			*dvdlight_register &= ~DVDLIGHT_LED; // Turn light off
			nanosleep(&timeOff, NULL);
		}
	}

	// Shut down thread
	*dvdlight_register &= ~DVDLIGHT_LED; // Turn light off
	dvdlight_thread = 0; // Disable thread

	return NULL;
}

void DVDLight_On(void)
{
	dvdlight_on = true;

	// Create thread if it is disabled
	if (dvdlight_thread == 0)
		LWP_CreateThread(&dvdlight_thread, dvdlight_loop, NULL, NULL, 0, 70);
}
void DVDLight_Off(void)
{
	dvdlight_on = false;
}
bool DVDLight_IsOn(void)
{
	return dvdlight_on;
}

void DVDLight_SetLevel(s32 level)
{
	if (level > 255)
		level = 255;
	else if (level < 0)
		level = 0;

	dvdlight_timeOn.tv_nsec = level * 40000;
	dvdlight_timeOff.tv_nsec = (level * -40000) + 10200000;

	dvdlight_level = level;
}
u8 DVDLight_GetLevel(void) { return dvdlight_level; }

#endif /* defined(HW_RVL) */
