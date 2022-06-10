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

struct timespec __dvdlight_timeon;
struct timespec __dvdlight_timeoff;

static lwp_t __dvdlight_thread = 0;
static u32 *__dvdlight_register = (u32*)DVDLIGHT_REGISTER;
static bool __dvdlight_status = false;
static u8 __dvdlight_level = 0;

// The light needs to be turned on and off to simulate the light intensity.
static void *__dvdlight_loop(void *arg)
{
	struct timespec timeon;
	struct timespec timeoff;

	while (__dvdlight_status == true) {
		timeon = __dvdlight_timeon;
		timeoff = __dvdlight_timeoff;

		// Turn on the light
		*__dvdlight_register |= DVDLIGHT_LED;
		nanosleep(&timeon, NULL);

		// Turn off the light
		if (timeoff.tv_nsec > 0)
			*__dvdlight_register &= ~DVDLIGHT_LED;
		nanosleep(&timeoff, NULL);
	}

	// Turn off the light
	*__dvdlight_register &= ~DVDLIGHT_LED;

	// Disable the thread
	__dvdlight_thread = 0;

	return NULL;
}

void DVDLight_On(void)
{
	__dvdlight_status = true;

	// Create thread if it is disabled
	if (__dvdlight_thread == 0)
		LWP_CreateThread(&__dvdlight_thread, __dvdlight_loop, NULL, NULL, 0, 70);
}
void DVDLight_Off(void)
{
	__dvdlight_status = false;
}
bool DVDLight_GetStatus(void)
{
	return __dvdlight_status;
}

void DVDLight_SetLevel(s32 level)
{
	if (level >= 100)
		level = 100;
	else if (level <= 0)
		level = 0;

	__dvdlight_timeon.tv_nsec = level * 40000;
	__dvdlight_timeoff.tv_nsec = level * -40000 + 10200000;

	__dvdlight_level = level;
}
u8 DVDLight_GetLevel(void) { return __dvdlight_level; }

#endif /* defined(HW_RVL) */
