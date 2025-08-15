#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Chars starting with 0xE0-0xFE are a 2 byte sequence
#define MULTIBYTE_BEGIN 0xE0
#define MULTIBYTE_END 0xFE

#define SYM_EVENT '#'
#define SYM_COMMENT '-'
#define SYM_COMMAND '<'
#define SYM_PARAM_SEP ':'

enum {
	LANG_EN = 0x00,
	LANG_ES,
	LANG_PT,
	LANG_FR,
	LANG_IT,
	LANG_DE,
	LANG_BR,
	LANG_FI,

	LANG_JA = 0x20,
	LANG_ZH,
	LANG_KO,
	LANG_TW,

	LANG_RU = 0x30,
	LANG_UA,

	LANG_INVALID = 0xFF,
};

enum {
	CT_COMMAND,
	CT_EVENT,
	CT_ASCII,
	CT_EXTEND,
	CT_KANJI,
	CT_SKIP,
	CT_SKIP2BYTE,
	CT_INVALID,
	CT_INVALID2BYTE,
};

static uint16_t *kanji = NULL;
static uint16_t kanjiCount = 0;

static uint16_t language = LANG_EN;

static bool is_ascii(char c) {
	static const char *ValidChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
									" /,.?=!@$%^&*()[]{}|_-+:;'~\"\n";
	for(size_t i = 0; i < strlen(ValidChars); i++) {
		if(c == ValidChars[i])
			return true;
	}
	return false;
}

static bool is_extended(uint8_t c) {
	static const uint8_t ExtendedChars[] = {
		0xA1, 0xBF, /* Upside-down exclamation/question */
		0xC0, 0xC8, 0xCC, 0xD2, 0xD9, 0xE0, 0xE8, 0xEC, 0xF2, 0xF9, /* Grave */
		0xC7, 0xE7,                                                 /* Cedilla */
		0xC1, 0xC9, 0xCD, 0xD3, 0xDA, 0xE1, 0xE9, 0xED, 0xF3, 0xFA, /* Acute */
		0xC2, 0xCA, 0xCE, 0xD4, 0xDB, 0xE2, 0xEA, 0xEE, 0xF4, 0xFB, /* Circumflex */
		0xC4, 0xCB, 0xCF, 0xD6, 0xDC, 0xE4, 0xEB, 0xEF, 0xF6, 0xFC, /* Umlaut */
		0xC3, 0xD5, 0xE3, 0xF5,                                     /* Squiggly vowels */
		0xD1, 0xF1,                                                 /* Squiggly N */
		0x8C, 0x9C,                                                 /* OE */
		0xC6, 0xE6,                                                 /* AE */
		0xBA,                                                       /* Ordinal */
		0xDF,                                                       /* Sharp S */
	};
	for(int i = 0; i < 56; i++) {
		if(c == ExtendedChars[i])
			return true;
	}
	return false;
}

static void decode_extended(FILE *f, uint8_t c) {
    if(language >= LANG_RU && language <= LANG_UA) {
        static const char mark = 0x01;
        static const char extmap[] = {
            0, 8, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 0,  0, 8, 0, 0, 0, 0, 2, 0,
            0, 0, 0, 6, 0, 0, 4, 5, 9, 0, 0, 0,  1,  12, 3, 0, 0, 0, 0, 7,
        };
        char mychr = extmap[c - 0x80];
        fwrite(&mark, 1, 1, f);
        fwrite(&mychr, 1, 1, f);
    } else if(c == 0x8C) {
        static const char mystr[2] = "OE";
        fwrite(&mystr, 1, 2, f);
    } else if(c == 0x9C) {
        static const char mystr[2] = "oe";
        fwrite(&mystr, 1, 2, f);
    } else if(c == 0xC6) {
        static const char mystr[2] = "AE";
        fwrite(&mystr, 1, 2, f);
    } else if(c == 0xE6) {
        static const char mystr[2] = "ae";
        fwrite(&mystr, 1, 2, f);
    } else {
        static const char mark = 0x01;
        static const char extmap[] = {
            0, 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0, 0,  0,  0,  0,  0,  27, 0,  0,  0,  0,  2,  3,  3,  3,  3,  3,  3,  0,  15,
            4, 4,  4,  4,  5,  5,  5,  5,  0,  6,  7,  7,  7,  7,  7,  0,  0,  8,  8,  8,
            8, 0,  0,  30, 9,  9,  17, 28, 22, 22, 0,  16, 10, 10, 18, 23, 11, 11, 19, 24,
            0, 12, 13, 13, 20, 29, 25, 0,  0,  14, 14, 21, 26, 0,  0,  0,
        };
        char mychr = extmap[c - 160];
        fwrite(&mark, 1, 1, f);
        fwrite(&mychr, 1, 1, f);
    }
}

static bool decode_kanji(FILE *f, uint16_t wc) {
    int k;
    for(k = 0; k < kanjiCount; k++) {
        if(wc == kanji[k])
            break;
    }
    if(k == kanjiCount) {
        printf("WARN: Unknown kanji: 0x%04hx\n", wc);
        fwrite("x", 1, 1, f);
		return false;
    } else {
        // Index by appearance in the list, and fit that number into banks
        // of 0x60 for each 'page'
        uint8_t page = k / 0x60;
        uint8_t word = k % 0x60;
        uint8_t b = MULTIBYTE_BEGIN + page; // First byte
        fwrite(&b, 1, 1, f);
        b = word + 0x20; // Second byte
        fwrite(&b, 1, 1, f);
		return true;
    }
}

static bool load_kanji_list(void) {
    static const char kanjifn[4][80] = {
        "tools/tscomp/kanjimap.txt",
        "tools/tscomp/kanjimap_zh.txt",
        "tools/tscomp/kanjimap_ko.txt",
        "tools/tscomp/kanjimap_zh.txt",
    };
    // Load the kanji list
    FILE *kfile = fopen(kanjifn[language - LANG_JA], "rb");
    if(!kfile) {
        printf("ERROR: Failed to open '%s'.\n", "kanjimap.txt");
        return false;
    }
    fseek(kfile, 0, SEEK_END);
    kanjiCount = ftell(kfile) / 2; // Each iteration is 2 bytes
    fseek(kfile, 0, SEEK_SET);
    kanji = malloc(kanjiCount * 2);
    fread(kanji, 1, kanjiCount * 2, kfile);
    fclose(kfile);
    // Endianness is a bitch
    for(uint16_t k = 0; k < kanjiCount; k++) {
        uint8_t hi = kanji[k] >> 8;
        uint8_t lo = kanji[k] & 0xFF;
        kanji[k] = (lo << 8) | hi;
    }
    return true;
}

static uint8_t get_char_type(uint8_t c) {
	// Command symbol '<'
	if(c == SYM_COMMAND)
		return CT_COMMAND;
	// Event symbol '#'
	if(c == SYM_EVENT)
		return CT_EVENT;
	// Skip '\r'
	if(c == '\r')
		return CT_SKIP;
	if(language >= LANG_RU && language <= LANG_UA) {
		// TODO: KOI8-R/U support
		if(c >= 0xC0)
			c -= 0x80;
		if(c < 0x80)
			return CT_ASCII;
		return CT_EXTEND;
	} else {
		// First check the valid ASCII chars list
		if(is_ascii(c))
			return CT_ASCII;
		if(language > LANG_EN && language < LANG_JA) {
			// Check if it's a supported ISO-8859-1 / Windows-1252 char
			if(is_extended(c))
				return CT_EXTEND;
		} else if(language >= LANG_JA && language < LANG_RU) {
			// Double byte char?
			if((c >= 0x81 && c <= 0xFE)) {
				return CT_KANJI;
			}
		}
	}
	return CT_INVALID;
}

static uint16_t read_langcode(const char *str) {
	char code[4] = {toupper(str[0]), toupper(str[1]), 0, 0};
	if(strcmp("EN", code) == 0)
		return LANG_EN;
	else if(strcmp("JA", code) == 0)
		return LANG_JA;
	else if(strcmp("ES", code) == 0)
		return LANG_ES;
	else if(strcmp("FR", code) == 0)
		return LANG_FR;
	else if(strcmp("DE", code) == 0)
		return LANG_DE;
	else if(strcmp("PT", code) == 0)
		return LANG_PT;
	else if(strcmp("IT", code) == 0)
		return LANG_IT;
	else if(strcmp("BR", code) == 0)
		return LANG_BR;
	else if(strcmp("FI", code) == 0)
		return LANG_FI;
	else if(strcmp("ZH", code) == 0)
		return LANG_ZH;
	else if(strcmp("KO", code) == 0)
		return LANG_KO;
	else if(strcmp("RU", code) == 0)
		return LANG_RU;
	else if(strcmp("UA", code) == 0)
		return LANG_UA;
    else if(strcmp("TW", code) == 0)
		return LANG_TW;
	else
		return LANG_INVALID;
}
