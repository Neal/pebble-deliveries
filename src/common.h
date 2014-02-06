#pragma once

typedef struct {
	int index;
	char title[96];
	char subtitle[32];
} Package;

enum {
	KEY_INDEX = 0x0,
	KEY_TITLE = 0x1,
	KEY_SUBTITLE = 0x2,
	KEY_STATUS = 0x3,
};
