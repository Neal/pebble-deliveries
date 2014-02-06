#pragma once

typedef struct {
	int index;
	char title[90];
	char subtitle[30];
} Package;

enum {
	KEY_INDEX = 0x0,
	KEY_TITLE = 0x1,
	KEY_SUBTITLE = 0x2,
	KEY_STATUS = 0x3,
};
