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
static void mii_utf16(const unsigned short *input, wchar_t *output, int size) {
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

	if (buffer[start+0x01] & 2) mii->favorite_color += 1;
	if (buffer[start+0x01] & 4) mii->favorite_color += 2;
	if (buffer[start+0x01] & 8) mii->favorite_color += 4;
	if (buffer[start+0x01] & 16) mii->favorite_color += 8;

	if (buffer[start+0x01] & 32) mii->day += 1;
	if (buffer[start+0x01] & 64) mii->day += 2;
	if (buffer[start+0x01] & 128) mii->day += 4;

	mii->height = buffer[start + 0x16] / 127.0;
	mii->weight = buffer[start + 0x17] / 127.0;
	mii->mii_id_1 = buffer[start + 0x18];
	mii->mii_id_2 = buffer[start + 0x19];
	mii->mii_id_3 = buffer[start + 0x1A];
	mii->mii_id_4 = buffer[start + 0x1B];
	mii->system_id_0 = buffer[start + 0x1C];
	mii->system_id_1 = buffer[start + 0x1D];
	mii->system_id_2 = buffer[start + 0x1E];
	mii->system_id_3 = buffer[start + 0x1F];

	if (buffer[start+0x20] & 1) mii->face_shape += 1;
	if (buffer[start+0x20] & 2) mii->face_shape += 2;
	if (buffer[start+0x20] & 4) mii->face_shape += 4;

	if (buffer[start+0x20] & 8) mii->skin_color += 1;
	if (buffer[start+0x20] & 16) mii->skin_color += 2;
	if (buffer[start+0x20] & 32) mii->skin_color += 4;

	if (buffer[start+0x20] & 64) mii->facial_feature += 1;
	if (buffer[start+0x20] & 128) mii->facial_feature += 2;
	if (buffer[start+0x21] & 1) mii->facial_feature += 4;
	if (buffer[start+0x21] & 2) mii->facial_feature += 8;

	mii->mingle = !(buffer[start+0x21] & 8);
	mii->downloaded = buffer[start+0x21] & 32;

	if (buffer[start+0x22] & 1) mii->hair_type += 1;
	if (buffer[start+0x22] & 2) mii->hair_type += 2;
	if (buffer[start+0x22] & 4) mii->hair_type += 4;
	if (buffer[start+0x22] & 8) mii->hair_type += 8;
	if (buffer[start+0x22] & 16) mii->hair_type += 16;
	if (buffer[start+0x22] & 32) mii->hair_type += 32;
	if (buffer[start+0x22] & 64) mii->hair_type += 64;

	if (buffer[start+0x22] & 128) mii->hair_color += 1;
	if (buffer[start+0x23] & 1) mii->hair_color += 2;
	if (buffer[start+0x23] & 2) mii->hair_color += 4;

	mii->hair_reversed = buffer[start+0x23] & 128;

	if (buffer[start+0x24] & 1) mii->eyebrow_type += 1;
	if (buffer[start+0x24] & 2) mii->eyebrow_type += 2;
	if (buffer[start+0x24] & 4) mii->eyebrow_type += 4;
	if (buffer[start+0x24] & 8) mii->eyebrow_type += 8;
	if (buffer[start+0x24] & 16) mii->eyebrow_type += 16;

	if (buffer[start+0x24] & 64) mii->eyebrow_type += 1;
	if (buffer[start+0x24] & 128) mii->eyebrow_type += 2;
	if (buffer[start+0x25] & 1) mii->eyebrow_type += 4;
	if (buffer[start+0x25] & 2) mii->eyebrow_type += 8;

	if (buffer[start+0x26] & 1) mii->eyebrow_color += 1;
	if (buffer[start+0x26] & 2) mii->eyebrow_color += 2;
	if (buffer[start+0x26] & 4) mii->eyebrow_color += 4;

	if (buffer[start+0x26] & 8) mii->eyebrow_size += 1;
	if (buffer[start+0x26] & 16) mii->eyebrow_size += 2;
	if (buffer[start+0x26] & 32) mii->eyebrow_size += 4;
	if (buffer[start+0x26] & 64) mii->eyebrow_size += 8;

	if (buffer[start+0x26] & 128) mii->eyebrow_height += 1;
	if (buffer[start+0x27] & 1) mii->eyebrow_height += 2;
	if (buffer[start+0x27] & 2) mii->eyebrow_height += 4;
	if (buffer[start+0x27] & 4) mii->eyebrow_height += 8;
	if (buffer[start+0x27] & 8) mii->eyebrow_height += 16;

	if (buffer[start+0x27] & 16) mii->eyebrow_spacing += 1;
	if (buffer[start+0x27] & 32) mii->eyebrow_spacing += 2;
	if (buffer[start+0x27] & 64) mii->eyebrow_spacing += 4;
	if (buffer[start+0x27] & 128) mii->eyebrow_spacing += 8;

	if (buffer[start+0x28] & 1) mii->eye_type += 1;
	if (buffer[start+0x28] & 2) mii->eye_type += 2;
	if (buffer[start+0x28] & 4) mii->eye_type += 4;
	if (buffer[start+0x28] & 8) mii->eye_type += 8;
	if (buffer[start+0x28] & 16) mii->eye_type += 16;
	if (buffer[start+0x28] & 32) mii->eye_type += 32;

	if (buffer[start+0x29] & 1) mii->eye_rotation += 1;
	if (buffer[start+0x29] & 2) mii->eye_rotation += 2;
	if (buffer[start+0x29] & 4) mii->eye_rotation += 4;

	if (buffer[start+0x29] & 8) mii->eye_height += 1;
	if (buffer[start+0x29] & 16) mii->eye_height += 2;
	if (buffer[start+0x29] & 32) mii->eye_height += 4;
	if (buffer[start+0x29] & 64) mii->eye_height += 8;
	if (buffer[start+0x29] & 128) mii->eye_height += 16;

	if (buffer[start+0x2A] & 1) mii->eye_color += 1;
	if (buffer[start+0x2A] & 2) mii->eye_color += 2;
	if (buffer[start+0x2A] & 4) mii->eye_color += 4;

	if (buffer[start+0x2A] & 16) mii->eye_size += 1;
	if (buffer[start+0x2A] & 32) mii->eye_size += 2;
	if (buffer[start+0x2A] & 64) mii->eye_size += 4;

	// These are key values for checking if a Mii exists, if they are all 0, then the Mii doesn't exist (or is potentially invalid)
	if((mii->eyebrow_type == 0) && (mii->eyebrow_rotation == 0) && (mii->eye_type == 0) && (mii->eye_color == 0) && (mii->eye_size == 0)) return false;

	if (buffer[start+0x2A] & 128) mii->eye_spacing += 1;
	if (buffer[start+0x2B] & 1) mii->eye_spacing += 2;
	if (buffer[start+0x2B] & 2) mii->eye_spacing += 4;
	if (buffer[start+0x2B] & 4) mii->eye_spacing += 8;

	if (buffer[start+0x2C] & 1) mii->nose_type += 1;
	if (buffer[start+0x2C] & 2) mii->nose_type += 2;
	if (buffer[start+0x2C] & 4) mii->nose_type += 4;
	if (buffer[start+0x2C] & 8) mii->nose_type += 8;

	if (buffer[start+0x2C] & 1) mii->nose_size += 1;
	if (buffer[start+0x2C] & 2) mii->nose_size += 2;
	if (buffer[start+0x2C] & 4) mii->nose_size += 4;
	if (buffer[start+0x2C] & 8) mii->nose_size += 8;

	if (buffer[start+0x2C] & 16) mii->nose_height += 1;
	if (buffer[start+0x2C] & 32) mii->nose_height += 2;
	if (buffer[start+0x2C] & 64) mii->nose_height += 4;
	if (buffer[start+0x2C] & 128) mii->nose_height += 8;

	if (buffer[start+0x2E] & 1) mii->lip_type += 1;
	if (buffer[start+0x2E] & 2) mii->lip_type += 2;
	if (buffer[start+0x2E] & 4) mii->lip_type += 4;
	if (buffer[start+0x2E] & 8) mii->lip_type += 8;
	if (buffer[start+0x2E] & 16) mii->lip_type += 16;

	if (buffer[start+0x2E] & 32) mii->lip_color += 1;
	if (buffer[start+0x2E] & 64) mii->lip_color += 2;

	if (buffer[start+0x2E] & 128) mii->lip_size += 1;
	if (buffer[start+0x2F] & 1) mii->lip_size += 2;
	if (buffer[start+0x2F] & 2) mii->lip_size += 4;
	if (buffer[start+0x2F] & 4) mii->lip_size += 8;

	if (buffer[start+0x2F] & 8) mii->lip_height += 1;
	if (buffer[start+0x2F] & 16) mii->lip_height += 2;
	if (buffer[start+0x2F] & 32) mii->lip_height += 4;
	if (buffer[start+0x2F] & 64) mii->lip_height += 8;
	if (buffer[start+0x2F] & 128) mii->lip_height += 16;

	if (buffer[start+0x30] & 1) mii->glasses_type += 1;
	if (buffer[start+0x30] & 2) mii->glasses_type += 2;
	if (buffer[start+0x30] & 4) mii->glasses_type += 4;
	if (buffer[start+0x30] & 8) mii->glasses_type += 8;

	if (buffer[start+0x30] & 16) mii->glasses_color += 1;
	if (buffer[start+0x30] & 32) mii->glasses_color += 2;
	if (buffer[start+0x30] & 64) mii->glasses_color += 4;

	if (buffer[start+0x30] & 1) mii->glasses_size += 1;
	if (buffer[start+0x31] & 2) mii->glasses_size += 2;
	if (buffer[start+0x31] & 4) mii->glasses_size += 4;

	if (buffer[start+0x31] & 8) mii->glasses_height += 1;
	if (buffer[start+0x31] & 16) mii->glasses_height += 2;
	if (buffer[start+0x31] & 32) mii->glasses_height += 4;
	if (buffer[start+0x31] & 64) mii->glasses_height += 8;
	if (buffer[start+0x31] & 128) mii->glasses_height += 16;

	if (buffer[start+0x32] & 1) mii->mustache_type += 1;
	if (buffer[start+0x32] & 2) mii->mustache_type += 2;

	if (buffer[start+0x32] & 4) mii->beard_type += 1;
	if (buffer[start+0x32] & 8) mii->beard_type += 2;

	if (buffer[start+0x32] & 16) mii->facial_hair_color += 1;
	if (buffer[start+0x32] & 32) mii->facial_hair_color += 2;
	if (buffer[start+0x32] & 64) mii->facial_hair_color += 4;

	if (buffer[start+0x32] & 128) mii->mustache_size += 1;
	if (buffer[start+0x33] & 1) mii->mustache_size += 2;
	if (buffer[start+0x33] & 2) mii->mustache_size += 4;
	if (buffer[start+0x33] & 4) mii->mustache_size += 8;

	if (buffer[start+0x33] & 8) mii->mustache_height += 1;
	if (buffer[start+0x33] & 16) mii->mustache_height += 2;
	if (buffer[start+0x33] & 32) mii->mustache_height += 4;
	if (buffer[start+0x33] & 64) mii->mustache_height += 8;
	if (buffer[start+0x33] & 128) mii->mustache_height += 16;

	mii->mole = buffer[start+0x34] & 1;
	if (buffer[start+0x34] & 2) mii->mole_size += 1;
	if (buffer[start+0x34] & 4) mii->mole_size += 2;
	if (buffer[start+0x34] & 8) mii->mole_size += 4;
	if (buffer[start+0x34] & 16) mii->mole_size += 8;

	if (buffer[start+0x34] & 32) mii->mole_vertical += 1;
	if (buffer[start+0x34] & 64) mii->mole_vertical += 2;
	if (buffer[start+0x34] & 128) mii->mole_vertical += 4;
	if (buffer[start+0x35] & 1) mii->mole_vertical += 8;
	if (buffer[start+0x35] & 2) mii->mole_vertical += 16;

	if (buffer[start+0x35] & 4) mii->mole_horizontal += 1;
	if (buffer[start+0x35] & 8) mii->mole_horizontal += 2;
	if (buffer[start+0x35] & 16) mii->mole_horizontal += 4;
	if (buffer[start+0x35] & 32) mii->mole_horizontal += 8;
	if (buffer[start+0x35] & 64) mii->mole_horizontal += 16;

	return true;
}

static int mii_init_common(char *buffer) {
	// Verify Mii data version
	if (memcmp(buffer, "RNOD", 4) != 0) return MII_ERROR_VERSION;

	// Parse and store Mii data into an internal array
	mii_miis = (Mii *) malloc(sizeof(Mii) * MII_MAX);
	if (mii_miis == NULL) {
		free(buffer);

		return MII_ERROR_MEMORY;
	}

	mii_count = 0;

	for (int n = 0; n < MII_MAX; n++){
		if (mii_load(buffer, (n * MII_SIZE) + 4, &mii_miis[mii_count]) == true) mii_count++;
	}

	if (mii_count == 0) {
		free(mii_miis);

		return MII_ERROR_EMPTY;
	} else if (mii_count < MII_MAX) {
		Mii *temp = (Mii *) realloc(mii_miis, sizeof(Mii) * mii_count);

		if (temp == NULL) {
			free(mii_miis);

			return MII_ERROR_MEMORY;
		} else {
			mii_miis = temp;
		}
	}

	return MII_ERROR_NONE;
}

int MII_Init(void) {
	char *buffer;

	int fd;
	int ret;

	if (mii_inited == true) mii_inited = false;

	// Allocate memory for the Mii data
	buffer = (char *) memalign(32, sizeof(char) * MII_FILE_SIZE);
	if (buffer == NULL) return MII_ERROR_MEMORY;

	// Load Mii data via IPC
	fd = IOS_Open(mii_file, 1);
	if(fd < IPC_OK) {
		free(buffer);

		return MII_ERROR_FILE;
	}
	ret = IOS_Read(fd, buffer, MII_FILE_SIZE);
	if ((IOS_Close(fd) < IPC_OK) || (ret != MII_FILE_SIZE)) {
		free(buffer);

		return MII_ERROR_FILE;
	}

	ret = mii_init_common(buffer);
	if (ret != MII_ERROR_NONE) {
		free(buffer);

		return ret;
	}

	free(buffer);

	mii_inited = true;

	return MII_ERROR_NONE;
}
int MII_InitMemory(char *buffer, unsigned int size) {
	int ret;

	if (mii_inited == true) mii_inited = false;

	if (size != MII_FILE_SIZE) return MII_ERROR_FILE;

	ret = mii_init_common(buffer);
	if (ret != MII_ERROR_NONE) return ret;

	mii_inited = true;

	return MII_ERROR_NONE;
}
int MII_InitFile(const char *filename) {
	char *buffer;

	FILE *fp;
	int ret;

	if (mii_inited == true) mii_inited = false;

	// Allocate memory for the Mii data
	buffer = (char *) malloc(sizeof(char) * MII_FILE_SIZE);
	if (buffer == NULL) return MII_ERROR_MEMORY;

	// Load Mii data from file
	fp = fopen(filename, "rb");
	if (fp == NULL) return MII_ERROR_FILE;
	ret = fread(buffer, 1, MII_FILE_SIZE, fp);
	if ((fclose(fp) == EOF) || (ret != MII_FILE_SIZE)) {
		free(buffer);

		return MII_ERROR_FILE;
	}

	ret = mii_init_common(buffer);
	if (ret != MII_ERROR_NONE) {
		free(buffer);

		return ret;
	}

	free(buffer);

	mii_inited = true;

	return MII_ERROR_NONE;
}

Mii MII_GetMii(unsigned int index) {
	if ((index >= mii_count) || (mii_inited == false))
		return mii_empty_mii;

	return mii_miis[index];
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
