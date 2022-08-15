// libmii (libogc-mod adaptation)
// Modified by HTV04

/*
MIT License

Copyright (c) 2021 Matthew Bauer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#if defined(HW_RVL)

#include <gcutil.h>
#include <ogc/ipc.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>

#include "mii.h"

#define MII_FILE_SIZE ((MII_MAX * MII_SIZE) + MII_HEADER)

static const char mii_file[] ATTRIBUTE_ALIGN(32) = "/shared2/menu/FaceLib/RFL_DB.dat";
static const Mii mii_empty_mii = {0};

static bool mii_inited = false;

static Mii *mii_miis;
static unsigned int mii_count;

// Convert a UTF-16 string to wchar_t
static void mii_utf16(const unsigned short *input, wchar_t *output, int size)
{
	for (int i = 0; i < size; i++) {
		if ((input[i] - 0xD800U) >= 2048U) {
			output[i] = input[i];
		} else {
			if (((input[i] & 0xFFFFFC00) == 0xD800) && (i <= size) && ((input[i + 1] & 0xFFFFFC00) == 0xDC00)) {
                output[i] = ((input[i] << 10) + input[i + 1]) - 0x35FDC00;

				i++;
			} else {
				output[i] = 0xFFFD;
			}
		}
	}
}

// Parse Mii data
static bool mii_load(char *buffer, int start, Mii *mii) {
	*mii = mii_empty_mii;

	mii_utf16((unsigned short *) (buffer + start + 0x02), mii->name, MII_NAME_LENGTH);
	mii_utf16((unsigned short *) (buffer + start + 0x36), mii->creator, MII_CREATOR_LENGTH);

	if (buffer[start+0x00] & 1) mii->day += 8;
	if (buffer[start+0x00] & 2) mii->day += 16;

	if (buffer[start+0x00] & 4) mii->month += 1;
	if (buffer[start+0x00] & 8) mii->month += 2;
	if (buffer[start+0x00] & 16) mii->month += 4;
	if (buffer[start+0x00] & 32) mii->month += 8;

	mii->female = buffer[start+0x00] & 64;
	mii->invalid = buffer[start+0x00] & 128;
	mii->favorite = buffer[start+0x01] & 1;

	if (buffer[start+0x01] & 2) mii->favoriteColor += 1;
	if (buffer[start+0x01] & 4) mii->favoriteColor += 2;
	if (buffer[start+0x01] & 8) mii->favoriteColor += 4;
	if (buffer[start+0x01] & 16) mii->favoriteColor += 8;

	if (buffer[start+0x01] & 32) mii->day += 1;
	if (buffer[start+0x01] & 64) mii->day += 2;
	if (buffer[start+0x01] & 128) mii->day += 4;

	mii->height = buffer[start + 0x16] / 127.0;
	mii->weight = buffer[start + 0x17] / 127.0;
	mii->miiID1 = buffer[start + 0x18];
	mii->miiID2 = buffer[start + 0x19];
	mii->miiID3 = buffer[start + 0x1A];
	mii->miiID4 = buffer[start + 0x1B];
	mii->systemID0 = buffer[start + 0x1C];
	mii->systemID1 = buffer[start + 0x1D];
	mii->systemID2 = buffer[start + 0x1E];
	mii->systemID3 = buffer[start + 0x1F];

	if (buffer[start+0x20] & 1) mii->faceShape += 1;
	if (buffer[start+0x20] & 2) mii->faceShape += 2;
	if (buffer[start+0x20] & 4) mii->faceShape += 4;

	if (buffer[start+0x20] & 8) mii->skinColor += 1;
	if (buffer[start+0x20] & 16) mii->skinColor += 2;
	if (buffer[start+0x20] & 32) mii->skinColor += 4;

	if (buffer[start+0x20] & 64) mii->facialFeature += 1;
	if (buffer[start+0x20] & 128) mii->facialFeature += 2;
	if (buffer[start+0x21] & 1) mii->facialFeature += 4;
	if (buffer[start+0x21] & 2) mii->facialFeature += 8;

	mii->mingle = !(buffer[start+0x21] & 8);
	mii->downloaded = buffer[start+0x21] & 32;

	if (buffer[start+0x22] & 1) mii->hairType += 1;
	if (buffer[start+0x22] & 2) mii->hairType += 2;
	if (buffer[start+0x22] & 4) mii->hairType += 4;
	if (buffer[start+0x22] & 8) mii->hairType += 8;
	if (buffer[start+0x22] & 16) mii->hairType += 16;
	if (buffer[start+0x22] & 32) mii->hairType += 32;
	if (buffer[start+0x22] & 64) mii->hairType += 64;

	if (buffer[start+0x22] & 128) mii->hairColor += 1;
	if (buffer[start+0x23] & 1) mii->hairColor += 2;
	if (buffer[start+0x23] & 2) mii->hairColor += 4;

	mii->hairReversed = buffer[start+0x23] & 128;

	if (buffer[start+0x24] & 1) mii->eyebrowType += 1;
	if (buffer[start+0x24] & 2) mii->eyebrowType += 2;
	if (buffer[start+0x24] & 4) mii->eyebrowType += 4;
	if (buffer[start+0x24] & 8) mii->eyebrowType += 8;
	if (buffer[start+0x24] & 16) mii->eyebrowType += 16;

	if (buffer[start+0x24] & 64) mii->eyebrowType += 1;
	if (buffer[start+0x24] & 128) mii->eyebrowType += 2;
	if (buffer[start+0x25] & 1) mii->eyebrowType += 4;
	if (buffer[start+0x25] & 2) mii->eyebrowType += 8;

	if (buffer[start+0x26] & 1) mii->eyebrowColor += 1;
	if (buffer[start+0x26] & 2) mii->eyebrowColor += 2;
	if (buffer[start+0x26] & 4) mii->eyebrowColor += 4;

	if (buffer[start+0x26] & 8) mii->eyebrowSize += 1;
	if (buffer[start+0x26] & 16) mii->eyebrowSize += 2;
	if (buffer[start+0x26] & 32) mii->eyebrowSize += 4;
	if (buffer[start+0x26] & 64) mii->eyebrowSize += 8;

	if (buffer[start+0x26] & 128) mii->eyebrowHeight += 1;
	if (buffer[start+0x27] & 1) mii->eyebrowHeight += 2;
	if (buffer[start+0x27] & 2) mii->eyebrowHeight += 4;
	if (buffer[start+0x27] & 4) mii->eyebrowHeight += 8;
	if (buffer[start+0x27] & 8) mii->eyebrowHeight += 16;

	if (buffer[start+0x27] & 16) mii->eyebrowSpacing += 1;
	if (buffer[start+0x27] & 32) mii->eyebrowSpacing += 2;
	if (buffer[start+0x27] & 64) mii->eyebrowSpacing += 4;
	if (buffer[start+0x27] & 128) mii->eyebrowSpacing += 8;

	if (buffer[start+0x28] & 1) mii->eyeType += 1;
	if (buffer[start+0x28] & 2) mii->eyeType += 2;
	if (buffer[start+0x28] & 4) mii->eyeType += 4;
	if (buffer[start+0x28] & 8) mii->eyeType += 8;
	if (buffer[start+0x28] & 16) mii->eyeType += 16;
	if (buffer[start+0x28] & 32) mii->eyeType += 32;

	if (buffer[start+0x29] & 1) mii->eyeRotation += 1;
	if (buffer[start+0x29] & 2) mii->eyeRotation += 2;
	if (buffer[start+0x29] & 4) mii->eyeRotation += 4;

	if (buffer[start+0x29] & 8) mii->eyeHeight += 1;
	if (buffer[start+0x29] & 16) mii->eyeHeight += 2;
	if (buffer[start+0x29] & 32) mii->eyeHeight += 4;
	if (buffer[start+0x29] & 64) mii->eyeHeight += 8;
	if (buffer[start+0x29] & 128) mii->eyeHeight += 16;

	if (buffer[start+0x2A] & 1) mii->eyeColor += 1;
	if (buffer[start+0x2A] & 2) mii->eyeColor += 2;
	if (buffer[start+0x2A] & 4) mii->eyeColor += 4;

	if (buffer[start+0x2A] & 16) mii->eyeSize += 1;
	if (buffer[start+0x2A] & 32) mii->eyeSize += 2;
	if (buffer[start+0x2A] & 64) mii->eyeSize += 4;

	// These are key values for checking if a Mii exists, if they are all 0, then the Mii doesn't exist (or is potentially invalid)
	if((mii->eyebrowType == 0) && (mii->eyebrowRotation == 0) && (mii->eyeType == 0) && (mii->eyeColor == 0) && (mii->eyeSize == 0)) return false;

	if (buffer[start+0x2A] & 128) mii->eyeSpacing += 1;
	if (buffer[start+0x2B] & 1) mii->eyeSpacing += 2;
	if (buffer[start+0x2B] & 2) mii->eyeSpacing += 4;
	if (buffer[start+0x2B] & 4) mii->eyeSpacing += 8;

	if (buffer[start+0x2C] & 1) mii->noseType += 1;
	if (buffer[start+0x2C] & 2) mii->noseType += 2;
	if (buffer[start+0x2C] & 4) mii->noseType += 4;
	if (buffer[start+0x2C] & 8) mii->noseType += 8;

	if (buffer[start+0x2C] & 1) mii->noseSize += 1;
	if (buffer[start+0x2C] & 2) mii->noseSize += 2;
	if (buffer[start+0x2C] & 4) mii->noseSize += 4;
	if (buffer[start+0x2C] & 8) mii->noseSize += 8;

	if (buffer[start+0x2C] & 16) mii->noseHeight += 1;
	if (buffer[start+0x2C] & 32) mii->noseHeight += 2;
	if (buffer[start+0x2C] & 64) mii->noseHeight += 4;
	if (buffer[start+0x2C] & 128) mii->noseHeight += 8;

	if (buffer[start+0x2E] & 1) mii->lipType += 1;
	if (buffer[start+0x2E] & 2) mii->lipType += 2;
	if (buffer[start+0x2E] & 4) mii->lipType += 4;
	if (buffer[start+0x2E] & 8) mii->lipType += 8;
	if (buffer[start+0x2E] & 16) mii->lipType += 16;

	if (buffer[start+0x2E] & 32) mii->lipColor += 1;
	if (buffer[start+0x2E] & 64) mii->lipColor += 2;

	if (buffer[start+0x2E] & 128) mii->lipSize += 1;
	if (buffer[start+0x2F] & 1) mii->lipSize += 2;
	if (buffer[start+0x2F] & 2) mii->lipSize += 4;
	if (buffer[start+0x2F] & 4) mii->lipSize += 8;

	if (buffer[start+0x2F] & 8) mii->lipHeight += 1;
	if (buffer[start+0x2F] & 16) mii->lipHeight += 2;
	if (buffer[start+0x2F] & 32) mii->lipHeight += 4;
	if (buffer[start+0x2F] & 64) mii->lipHeight += 8;
	if (buffer[start+0x2F] & 128) mii->lipHeight += 16;

	if (buffer[start+0x30] & 1) mii->glassesType += 1;
	if (buffer[start+0x30] & 2) mii->glassesType += 2;
	if (buffer[start+0x30] & 4) mii->glassesType += 4;
	if (buffer[start+0x30] & 8) mii->glassesType += 8;

	if (buffer[start+0x30] & 16) mii->glassesColor += 1;
	if (buffer[start+0x30] & 32) mii->glassesColor += 2;
	if (buffer[start+0x30] & 64) mii->glassesColor += 4;

	if (buffer[start+0x30] & 1) mii->glassesSize += 1;
	if (buffer[start+0x31] & 2) mii->glassesSize += 2;
	if (buffer[start+0x31] & 4) mii->glassesSize += 4;

	if (buffer[start+0x31] & 8) mii->glassesHeight += 1;
	if (buffer[start+0x31] & 16) mii->glassesHeight += 2;
	if (buffer[start+0x31] & 32) mii->glassesHeight += 4;
	if (buffer[start+0x31] & 64) mii->glassesHeight += 8;
	if (buffer[start+0x31] & 128) mii->glassesHeight += 16;

	if (buffer[start+0x32] & 1) mii->mustacheType += 1;
	if (buffer[start+0x32] & 2) mii->mustacheType += 2;

	if (buffer[start+0x32] & 4) mii->beardType += 1;
	if (buffer[start+0x32] & 8) mii->beardType += 2;

	if (buffer[start+0x32] & 16) mii->facialHairColor += 1;
	if (buffer[start+0x32] & 32) mii->facialHairColor += 2;
	if (buffer[start+0x32] & 64) mii->facialHairColor += 4;

	if (buffer[start+0x32] & 128) mii->mustacheSize += 1;
	if (buffer[start+0x33] & 1) mii->mustacheSize += 2;
	if (buffer[start+0x33] & 2) mii->mustacheSize += 4;
	if (buffer[start+0x33] & 4) mii->mustacheSize += 8;

	if (buffer[start+0x33] & 8) mii->mustacheHeight += 1;
	if (buffer[start+0x33] & 16) mii->mustacheHeight += 2;
	if (buffer[start+0x33] & 32) mii->mustacheHeight += 4;
	if (buffer[start+0x33] & 64) mii->mustacheHeight += 8;
	if (buffer[start+0x33] & 128) mii->mustacheHeight += 16;

	mii->mole = buffer[start+0x34] & 1;
	if (buffer[start+0x34] & 2) mii->moleSize += 1;
	if (buffer[start+0x34] & 4) mii->moleSize += 2;
	if (buffer[start+0x34] & 8) mii->moleSize += 4;
	if (buffer[start+0x34] & 16) mii->moleSize += 8;

	if (buffer[start+0x34] & 32) mii->moleVertical += 1;
	if (buffer[start+0x34] & 64) mii->moleVertical += 2;
	if (buffer[start+0x34] & 128) mii->moleVertical += 4;
	if (buffer[start+0x35] & 1) mii->moleVertical += 8;
	if (buffer[start+0x35] & 2) mii->moleVertical += 16;

	if (buffer[start+0x35] & 4) mii->moleHoriziontal += 1;
	if (buffer[start+0x35] & 8) mii->moleHoriziontal += 2;
	if (buffer[start+0x35] & 16) mii->moleHoriziontal += 4;
	if (buffer[start+0x35] & 32) mii->moleHoriziontal += 8;
	if (buffer[start+0x35] & 64) mii->moleHoriziontal += 16;

	return true;
}

int MII_Init(void) {
	char *buffer;

	int fd;
	int ret;

	if (mii_inited == true) return 0;

	// Allocate memory for the Mii data
	buffer = (char*) memalign(32, MII_FILE_SIZE);
	if (buffer == NULL) return MII_ERROR_MEMORY;

	// Load Mii data via IPC
	fd = IOS_Open(mii_file, 1);
	if(fd < 0) {
		free(buffer);

		return MII_ERROR_FILE;
	}
	ret = IOS_Read(fd, buffer, MII_FILE_SIZE);
	IOS_Close(fd);
	if (ret != MII_FILE_SIZE) {
		free(buffer);

		return MII_ERROR_FILE;
	}

	// Verify Mii data version
	if (memcmp(buffer, "RNOD", 4) != 0) {
		free(buffer);

		return MII_ERROR_VERSION;
	}

	// Parse and store Mii data into an internal array
	mii_miis = (Mii *) malloc(sizeof(Mii));
	if (mii_miis == NULL) {
		free(buffer);

		return MII_ERROR_MEMORY;
	}
	mii_count = 0;
	for (int n = 0; n < MII_MAX; n++){
		if (mii_load(buffer, (n * MII_SIZE) + 4, &mii_miis[mii_count]) == true) {
			mii_count++;

			mii_miis = (Mii *) realloc(mii_miis, sizeof(Mii) * (mii_count + 1));
		}
	}
	if (mii_count == 0) {
		free(buffer);
		free(mii_miis);

		return MII_ERROR_EMPTY;
	}

	mii_inited = true;

	return 0;
}

Mii MII_GetMii(unsigned int n) {
	if ((n >= mii_count) || (mii_inited == false))
		return mii_empty_mii;

	return mii_miis[n];
}
unsigned int MII_GetCount(void) {
	return mii_count;
}

void MII_GetMiis(Mii *miis, unsigned int size) {
	if ((miis == NULL) || (mii_inited == false)) return;

	for (int n = 0; (n < size) || (n < mii_count); n++) {
		miis[n] = mii_miis[n];
	}
}

#endif // HW_RVL
