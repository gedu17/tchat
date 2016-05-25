#pragma once
#include "stdafx.h"
#include "Settings.h"
#include "structs.h"
#ifndef UTILS_H
#define	UTILS_H

namespace utils {

    /* Key */
    is import_key_from_keyserver(string homedir, string keyserver, string key);
    void export_key_to_keyserver(string homedir, string keyserver, string key);

    /* Gpgme */
    void init_gpgme(gpgme_ctx_t *ctx, string gpgdir);

    /* Settings*/
    void read_settings(string sett, gpgme_ctx_t ctx, gpgme_key_t *the_key, string *alias, string *fingerprint, vector<string> *key_servers, vector<string> *trackers);
    void update_settings(string sett, vector<string> key_servers, vector<string> trackers, string alias, string fingerprint);
    /* Tracker queue*/
    vector<tqs> get_tq(int action);
    void insert_tq(tqs tq);
    void remove_tq(int id);
    bool update_tq(string transaction_id, vector<ss> scrape_queue);
    bool update_tq(string transaction_id, vector<ads> announce_queue, int seeders, int leechers);

    /* Tests */
    void test_import(string homedir, string keyserver, string key);
    void test_search(string homedir, string keyserver, string key);

}
#endif

