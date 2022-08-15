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

#if !defined(__MII_H__)
#define __MII_H__

#if defined (HW_RVL)

#if defined(__cplusplus)
   extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include <wchar.h>

#define MII_ERROR_NONE		0
#define MII_ERROR_FILE		-1
#define MII_ERROR_VERSION	-2
#define MII_ERROR_MEMORY	-3
#define MII_ERROR_EMPTY		-4

#define MII_NAME_LENGTH		10
#define MII_CREATOR_LENGTH	10
#define MII_SIZE			74
#define MII_MAX				100
#define MII_HEADER			4

#define MII_COLOR_RED			0
#define MII_COLOR_ORANGE		1
#define MII_COLOR_YELLOW		2
#define MII_COLOR_LIGHT_GREEN	3
#define MII_COLOR_GREEN			4
#define MII_COLOR_BLUE			5
#define MII_COLOR_LIGHT_BLUE	6
#define MII_COLOR_PINK			7
#define MII_COLOR_PURPLE		8
#define MII_COLOR_BROWN			9
#define MII_COLOR_WHITE			10
#define MII_COLOR_BLACK			11

#define MII_SKIN_COLOR_PALE			0
#define MII_SKIN_COLOR_BEIGE		1
#define MII_SKIN_COLOR_TAN			2
#define MII_SKIN_COLOR_PINK			3
#define MII_SKIN_COLOR_BROWN		4
#define MII_SKIN_COLOR_DARK_BROWN	5

#define MII_HAIR_COLOR_BLACK		0
#define MII_HAIR_COLOR_BROWN		1
#define MII_HAIR_COLOR_RED			2
#define MII_HAIR_COLOR_AUBURN		3
#define MII_HAIR_COLOR_GRAY			4
#define MII_HAIR_COLOR_DIRTY_BROWN	5
#define MII_HAIR_COLOR_DIRTY_BLONDE	6
#define MII_HAIR_COLOR_BLONDE		7

#define MII_EYEBROW_COLOR_BLACK			0
#define MII_EYEBROW_COLOR_BROWN			1
#define MII_EYEBROW_COLOR_RED			2
#define MII_EYEBROW_COLOR_AUBURN		3
#define MII_EYEBROW_COLOR_GRAY			4
#define MII_EYEBROW_COLOR_DIRTY_BROWN	5
#define MII_EYEBROW_COLOR_DIRTY_BLONDE	6
#define MII_EYEBROW_COLOR_BLONDE		7

#define MII_EYE_COLOR_BLACK	0
#define MII_EYE_COLOR_GRAY	1
#define MII_EYE_COLOR_BROWN	2
#define MII_EYE_COLOR_HAZEL	3
#define MII_EYE_COLOR_BLUE	4
#define MII_EYE_COLOR_GREEN	5

#define MII_LIP_COLOR_NATURAL	0
#define MII_LIP_COLOR_RED		1
#define MII_LIP_COLOR_PINK		2

#define MII_GLASSES_COLOR_BLACK		0
#define MII_GLASSES_COLOR_BROWN		1
#define MII_GLASSES_COLOR_RED		2
#define MII_GLASSES_COLOR_BLUE		3
#define MII_GLASSES_COLOR_YELLOW	4
#define MII_GLASSES_COLOR_GRAY		5

#define MII_FACIAL_HAIR_COLOR_BLACK			0
#define MII_FACIAL_HAIR_COLOR_BROWN			1
#define MII_FACIAL_HAIR_COLOR_RED			2
#define MII_FACIAL_HAIR_COLOR_AUBURN		3
#define MII_FACIAL_HAIR_COLOR_GRAY			4
#define MII_FACIAL_HAIR_COLOR_DIRTY_BROWN	5
#define MII_FACIAL_HAIR_COLOR_DIRTY_BLONDE	6
#define MII_FACIAL_HAIR_COLOR_BLONDE		7

typedef struct _mii {
	bool invalid;            // doesn't seem to have any effect?
	bool female;
	int month;
	int day;
	int favorite_color;	// 0 - 11 (changing to 1111, along with setting the preceeding bit
						// results in a grey shirt, some values over 11 will crash the Wii
						// when trying to change the favorite color).
	bool favorite;

// addr: 0x02 through 0x15
	wchar_t name[MII_NAME_LENGTH];

// addr: 0x16
	double height;                 // 0.0 - 1.0

// addr: 0x17
	double weight;                 // 0.0 - 1.0

// addr: 0x18 - 0x1B
	int mii_id_1;                // Unique Mii identifiers. Seem to increment with time. Also can
	int mii_id_2;                // be used to change color of Mii Trousers
	int mii_id_3;
	int mii_id_4;

// addr: 0x1C through 0x1F
	int system_id_0;	           // Checksum8 of first 3 bytes of mac addr
	int system_id_1;	           // mac addr 3rd-to-last byte
	int system_id_2;	           // mac addr 2nd-to-last byte
	int system_id_3;	           // mac addr last byte

// addr: 0x20 & 0x21
	int face_shape;           // 0 - 7
	int skin_color;           // 0 - 5
	int facial_feature;       // 0 - 11
	//u16 unknown;             // Mii appears unaffected by changes to this data
	bool mingle;
	//u16 unknown;             // Mii appears unaffected by changes to this data
	bool downloaded;

// addr: 0x22 & 0x23
	int hair_type;            // 0 - 71, Value is non-sequential with regard to page, row and column
	int hair_color;           // 0 - 7
	bool hair_reversed;
	//u16 unknown;

// addr: 0x24 through 0x27
	int eyebrow_type;         // 0 - 23, Value is non-sequential with regard to page, row and column
	//u32 unknown;
	int eyebrow_rotation;     // 0 - 11, Default value varies based on eyebrow type
	//u32 unknown;
	int eyebrow_color;        // 0 - 7
	int eyebrow_size;	   // 0 - 8, Default = 4
	int eyebrow_height;   // 3 - 18, Default = 10
	int eyebrow_spacing;    // 0 - 12, Default = 2

// addr: 0x28 through 0x2B
	int eye_type;             // 0 - 47, Value is non-sequential with regard to page, row and column
	//u32 unknown;
	int eye_rotation;         // 0 - 7, Default value varies based on eye type
	int eye_height;          // 0 - 18, Default = 12
	int eye_color;            // 0 - 5
	//u32 unknown;
	int eye_size;             // 0 - 7, Default = 4
	int eye_spacing;     // 0 - 12, Default = 2
	//u32 unknown;

// addr: 0x2C & 0x2D
	int nose_type;            // 0 - 11, Value is non-sequential with regard to row and column
	int nose_size;            // 0 - 8, Default = 4
	int nose_height;          // 0 - 18, Default = 9
	//u16 unknown;

// addr: 0x2E & 2F
	int lip_type;             // 0 - 23, Value is non-sequential with regard to page, row and column
	int lip_color;            // 0 - 2
	int lip_size;             // 0 - 8, Default = 4
	int lip_height;          // 0 - 18, Default = 13

// addr: 0x30 & 0x31
	int glasses_type;         // 0 - 8
	int glasses_color;        // 0 - 5
	//int unknown;             // when turned on mii does not appear (use not known)
	int glasses_size;         // 0 - 7, Default = 4
	int glasses_height;      // 0 - 20, Default = 10

// addr: 0x32 & 33
	int mustache_type;        // 0 - 3
	int beard_type;           // 0 - 3
	int facial_hair_color;     // 0 - 7
	int mustache_size;        // 0 - 8, Default = 4
	int mustache_height;     // 0 - 16, Default = 10

// addr: 0x34 & 0x35
	bool mole;
	int mole_size;			// 0 - 8, Default = 4
	int mole_vertical; 		// 0 - 30, Default = 20
	int mole_horizontal;	// 0 - 16, Default = 2
	//u16 unknown;

// addr: 0x36 through 0x49
	wchar_t creator[MII_CREATOR_LENGTH];
} Mii;

int MII_Init(void); // Initialize from Mii data on NAND
int MII_InitMemory(char *buffer, unsigned int size); // Initialize from Mii data in memory
int MII_InitFile(const char *filename); // Initialize from Mii data from file

Mii MII_GetMii(unsigned int index); // Get Mii data from index
unsigned int MII_GetCount(void); // Get number of Miis

void MII_GetMiis(Mii *miis, unsigned int size); // Get Mii data from all Miis in an array

#if defined(__cplusplus)
   }
#endif // __cplusplus

#endif // HW_RVL

#endif // __MII_H__