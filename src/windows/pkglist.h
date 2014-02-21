#pragma once

void pkglist_init(void);
void pkglist_destroy(void);
void pkglist_in_received_handler(DictionaryIterator *iter);
void pkglist_in_dropped_handler(AppMessageResult reason);
void pkglist_out_sent_handler(DictionaryIterator *sent);
void pkglist_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool pkglist_is_on_top();
