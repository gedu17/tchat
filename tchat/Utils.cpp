#include "stdafx.h"
#include "Utils.h"
#include "Log.h"

namespace utils {
    Log log = Log::get_instance();

    mutex tq_lock;
    vector<tqs> tracker_queue;

    /* Key */

    is import_key_from_keyserver(string homedir, string keyserver, string key) {
        is ret;
        string res;
        string cmd = "gpg2 --batch --homedir " + homedir + " --keyserver " + keyserver +
            " --expert --recv-key " + key + " 2>&1";
        #ifdef __linux__
            FILE *pipe = popen(cmd.c_str(), "r");
        #endif
        #ifdef _WIN32
            FILE *pipe = _popen(cmd.c_str(), "r");
        #endif
        
        if (!pipe) {
            log.write("Failed to open process for importing keys", 2);
            return ret;
        }
        else {
            char tmp[128];
            while (!feof(pipe)) {
                if (fgets(tmp, 128, pipe) != NULL) {
                    res += tmp;
                }
            }

        #ifdef __linux__
            pclose(pipe);
        #endif
        #ifdef _WIN32
            _pclose(pipe);
        #endif

            istringstream iss(res);
            string tempstr;
            while (getline(iss, tempstr)) {
                if (tempstr.substr(0, 9) == "gpg: key ") {
                    string tmp1 = tempstr.substr(tempstr.find("\"") + 1, (tempstr.find(">\"", (tempstr.find("\"") + 1))));
                    ks keystruct;
                    keystruct.email = tmp1.substr(tmp1.find(" <") + 2, tmp1.find(">", tmp1.find(" <")) - (tmp1.find(" <") + 2));
                    keystruct.key = tempstr.substr(9, 8);
                    keystruct.name = tmp1.substr(0, tmp1.find(" <"));
                    ret.keys.push_back(keystruct);
                }

                if (tempstr.substr(0, 6) == "gpg: T") {
                    ret.processed = stoi(tempstr.substr(tempstr.find(":", 4) + 2, string::npos));
                }

                if (tempstr.substr(0, 6) == "gpg:  ") {
                    if (tempstr.substr(tempstr.find(":", 4) - 9, 2) == "un") {
                        ret.unchanged = stoi(tempstr.substr(tempstr.find(":", 4) + 2, string::npos));
                    }
                    else {
                        ret.imported = stoi(tempstr.substr(tempstr.find(":", 4) + 2, string::npos));
                    }
                }
            }            
            return ret;
        }
    }

    void export_key_to_keyserver(string homedir, string keyserver, string key) {
        string res;
        string cmd = "gpg2 --batch --homedir " + homedir + " --keyserver " + keyserver +
            " --expert --send-key " + key + " 2>&1";
        #ifdef __linux__
            FILE *pipe = popen(cmd.c_str(), "r");
        #endif
        #ifdef _WIN32
            FILE *pipe = _popen(cmd.c_str(), "r");
        #endif
        if (!pipe) {
            log.write("Failed to open process for exporting keys", 2);
            return;
        }
        else {
            #ifdef __linux__
                pclose(pipe);
            #endif
            #ifdef _WIN32
                _pclose(pipe);
            #endif
        }
    }

    /* Gpgme */

    void init_gpgme(gpgme_ctx_t *ctx, string gpgdir) {
        gpgme_check_version(NULL);
        gpgme_error_t err;

        setlocale(LC_ALL, "");
        gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));
        err = gpgme_new(ctx);
        if (err) {
            log.write("Failed to get gpgme context.", 2);
            string tmp1 = "Error code: ";
            tmp1 += gpgme_err_code(err);
            log.write(tmp1, 2);
            tmp1.clear();
            tmp1 += "Error string: ";
            tmp1 += gpgme_strerror(err);
            log.write(tmp1, 2);
            return;
        }
        gpgme_engine_info_t eng = gpgme_ctx_get_engine_info(*ctx);
        gpgme_protocol_t proto = eng->protocol;
        const char *filename = eng->file_name;
        err = gpgme_ctx_set_engine_info(*ctx, proto, filename, gpgdir.c_str());
        if (err) {
            log.write("Failed to set gpgme engine info.", 2);
            return;
        }
    }

    /* Settings */

    void read_settings(string sett, gpgme_ctx_t ctx, gpgme_key_t *the_key, string *alias, string *fingerprint, vector<string> *key_servers, vector<string> *trackers) {
        gpgme_key_t key;
        gpgme_error_t err;
        vector<gpgme_key_t> temp_keystore;
        settings set(sett);
        if (set.read_settings()) {
            *alias = set.get_alias();
            string def_key = set.get_key();
            err = gpgme_op_keylist_start(ctx, NULL, 1);
            bool found = false;
            while (1) {
                err = gpgme_op_keylist_next(ctx, &key);
                if (err) {
                    break;
                }
                bool matches = true;
                for (uint32_t i = 0; i < def_key.size(); i++) {
                    if (key->subkeys->keyid[i] != def_key.at(i)) {
                        matches = false;
                        break;
                    }
                }

                if (matches) {
                    *the_key = key;
                    *fingerprint = key->subkeys->fpr;
                    found = true;
                    break;
                }
                gpgme_key_release(key);
            }

            *key_servers = set.get_key_servers();
            *trackers = set.get_trackers();

            if (!found) {
                log.write("Couldn't find default key in keychain.", 2);
                gpgme_release(ctx);
                return;
            }
        }
        else {
            *alias = DEFAULT_ALIAS;
            set.set_alias(*alias);

            err = gpgme_op_keylist_start(ctx, NULL, 1);
            int counter = 0;
            while (1) {
                err = gpgme_op_keylist_next(ctx, &key);
                if (err) {
                    break;
                }
                cout << counter << ". " << key->subkeys->keyid << " (" << key->subkeys->fpr << ")";
                if (key->uids && key->uids->name) {
                    cout << " " << key->uids->name;
                }
                if (key->uids && key->uids->email) {
                    cout << " <" << key->uids->email << ">";
                }
                cout << endl;
                counter++;
                temp_keystore.push_back(key);
            }

            if (counter == 0) {
                log.write("No secret keys found!", 2);
                log.write("Generate key using gpg2 --gen-key and try again.", 2);
                gpgme_release(ctx);
                return;
            }
            else {
                log.write("Which key would you like to use ? (Finger Print in brackets will be used as a identifier)");
                cout << "Key number: ";
                string keynum;
                cin >> keynum;
                if ((uint32_t)stoi(keynum) > temp_keystore.size() - 1) {
                    while (1) {
                        log.write("Invalid number.", 2);
                        cout << "Key number: ";
                        cin >> keynum;
                        if ((uint32_t)stoi(keynum) <= temp_keystore.size() - 1) {
                            break;
                        }
                    }
                }
                *the_key = temp_keystore.at(stoi(keynum));
                *fingerprint = temp_keystore.at(stoi(keynum))->subkeys->fpr;
                string sel = "Selected ";
                sel += (*the_key)->subkeys->keyid;
                log.write(sel);
                set.set_key((*the_key)->subkeys->keyid);
                set.set_alias((*the_key)->uids->name);
                *alias = (*the_key)->uids->name;

                for (uint32_t i = 0; i < temp_keystore.size(); i++) {
                    if (i != stoi(keynum)) {
                        gpgme_key_release(temp_keystore.at(i));
                    }
                }

                temp_keystore.clear();

            }
            set.create_settings();
        }
    }

    void update_settings(string sett, vector<string> key_servers, vector<string> trackers, string alias, string fingerprint) {
        settings set(sett);

        set.set_key(fingerprint);
        set.set_alias(alias);

        for (uint i = 0; i < key_servers.size(); i++) {
            set.add_key_server(key_servers.at(i));
        }

        for (uint i = 0; i < trackers.size(); i++) {
            set.add_tracker(trackers.at(i));
        }

        set.update_settings();
    }

    /* Tracker queue */

    //Get all items which match action and are finished
    vector<tqs> get_tq(int action) {
        vector<uint32_t> delete_queue;
        tq_lock.lock();
        vector<tqs> ret;
        for (uint32_t i = 0; i < tracker_queue.size(); i++) {
            if (tracker_queue.at(i).finished && tracker_queue.at(i).action == action) {
                ret.push_back(tracker_queue.at(i));
                delete_queue.push_back(i);
            }
        }
        tq_lock.unlock();
        for (uint32_t i = 0; i < delete_queue.size(); i++) {
            remove_tq(delete_queue.at(i));
        }
        return ret;
    }

    void insert_tq(tqs tq) {
        tq_lock.lock();
        tracker_queue.push_back(tq);
        tq_lock.unlock();
    }

    void remove_tq(int id) {
        tq_lock.lock();
        if ((uint32_t)id <= tracker_queue.size() - 1) {
            tracker_queue.erase(tracker_queue.begin() + id);
        }
        else {
            log.write("Invalid id for remove_tq", 2);
        }
        tq_lock.unlock();
    }

    bool update_tq(string transaction_id, vector<ads> announce_queue, int seeders, int leechers) {
        tq_lock.lock();
        bool found = false;
        for (uint32_t i = 0; i < tracker_queue.size(); i++) {
            if (tracker_queue.at(i).transaction_id == transaction_id) {
                tracker_queue.at(i).announce_queue = announce_queue;
                tracker_queue.at(i).seeders = seeders;
                tracker_queue.at(i).leechers = leechers;
                tracker_queue.at(i).finished = true;
                found = true;
                break;
            }
        }
        tq_lock.unlock();
        return found;
    }

    bool update_tq(string transaction_id, vector<ss> scrape_queue) {
        tq_lock.lock();
        bool found = false;
        for (uint32_t i = 0; i < tracker_queue.size(); i++) {
            if (tracker_queue.at(i).transaction_id == transaction_id) {
                tracker_queue.at(i).scrape_queue = scrape_queue;
                tracker_queue.at(i).finished = true;
                found = true;
                break;
            }
        }
        tq_lock.unlock();
        return found;
    }

    /* Tests */

    void test_import(string homedir, string keyserver, string key) {
        log.write("test_import start", 3);
        is test = import_key_from_keyserver(homedir, keyserver, key);
        string vs = "Key vector size: ";
        vs += to_string(test.keys.size());
        log.write(vs, 4);
        for (uint32_t i = 0; i < test.keys.size(); i++) {
            string c = to_string(i) + ". " + test.keys.at(i).name + " // " + test.keys.at(i).email +
                " // " + test.keys.at(i).key;
            log.write(c, 4);
        }
        string proc = "Processed: " + to_string(test.processed);
        log.write(proc, 4);
        string imp = "Imported: " + to_string(test.imported);
        log.write(imp, 4);
        string unc = "Unchanged: " + to_string(test.unchanged);
        log.write(unc, 4);
        log.write("test_import end", 3);
    }

    void test_search(string homedir, string keyserver, string key) {
        log.write("test_search start", 3);
        is test = import_key_from_keyserver(homedir, keyserver, key);

        if (test.processed == 0) {
            log.write("No results.", 4);
        }

        for (uint32_t i = 0; i < test.keys.size(); i++) {
            string c = to_string(i+1) + ". " + test.keys.at(i).name + " // " + test.keys.at(i).email +
                " // " + test.keys.at(i).key;
            log.write(c, 4);
        }
        log.write("test_search end", 3);
    }
}