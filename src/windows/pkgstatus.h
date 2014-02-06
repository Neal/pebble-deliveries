#include "../common.h"

#pragma once

void pkgstatus_init(Package pkg);
void pkgstatus_destroy(void);
void pkgstatus_in_received_handler(DictionaryIterator *iter);
