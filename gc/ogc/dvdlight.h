/*-------------------------------------------------------------

dvdlight.h -- Wii DVD light subsystem

Copyright (C) 2022
Michael Wiedenbauer (shagkur)
Dave Murphy (WinterMute)
Harrison (HTV04)

Additionally following copyrights apply for the patching system:
 * Copyright (C) 2005 The GameCube Linux Team
 * Copyright (C) 2005 Albert Herranz

Thanks alot guys for that incredible patch!!

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

#include <gctypes.h>

void DVDLight_On(void);
void DVDLight_Off(void);
bool DVDLight_IsOn(void);

void DVDLight_SetLevel(s32 level);
u8 DVDLight_GetLevel(void);

#endif /* defined(HW_RVL) */
