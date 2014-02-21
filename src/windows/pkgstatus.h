#pragma once
#include "../common.h"

void pkgstatus_init(Package pkg);
void pkgstatus_destroy(void);
void pkgstatus_in_received_handler(DictionaryIterator *iter);
void pkgstatus_in_dropped_handler(AppMessageResult reason);
void pkgstatus_out_sent_handler(DictionaryIterator *sent);
void pkgstatus_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool pkgstatus_is_on_top();
