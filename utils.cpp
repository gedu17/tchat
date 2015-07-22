/* 
 * 
 * Author: Gediminas Dulskas <dulskasg@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "utils.h"
#include "log.h"

namespace utils {
    Log log = Log::getInstance();
    
    mutex tq_lock;
    vector<tqs> tracker_queue;
    
    /* Key */
    
    is import_key_from_keyserver(string homedir, string keyserver, string key) {
        is ret;
        string res;
        string cmd = "gpg2 --batch --homedir ";
        cmd += homedir;
        cmd += " --keyserver ";
        cmd += keyserver;
        cmd += " --expert --recv-key ";
        cmd += key;
        cmd += " 2>&1";
        
        FILE *pipe = popen(cmd.c_str(), "r");
        if(!pipe) {
            log.write("Failed to open process for importing keys", 2);
        } else {
            char tmp[128];
            while(!feof(pipe)) {
                if(fgets(tmp, 128, pipe) != NULL) {
                    res += tmp;
                }
            }
            pclose(pipe);

            istringstream iss(res);
            string tempstr;

            while(getline(iss, tempstr)) {
                if(tempstr.substr(0,9) == "gpg: key ") {
                    string tmp1 = tempstr.substr(tempstr.find("\"")+1, (tempstr.find(">\"", (tempstr.find("\"")+1))));
                    ks keystruct;
                    keystruct.email = tmp1.substr(tmp1.find(" <")+2, tmp1.find(">", tmp1.find(" <"))-(tmp1.find(" <")+2));
                    keystruct.key = tempstr.substr(9, 8);
                    keystruct.name = tmp1.substr(0, tmp1.find(" <"));
                    ret.keys.push_back(keystruct);
                }

                if(tempstr.substr(0,6) == "gpg: T") {
                    ret.processed = stoi(tempstr.substr(tempstr.find(":", 4)+2, string::npos));
                }

                if(tempstr.substr(0,6) == "gpg:  ") {
                    if(tempstr.substr(tempstr.find(":", 4)-9, 2) == "un") {
                        ret.unchanged = stoi(tempstr.substr(tempstr.find(":", 4)+2, string::npos));
                    } else {
                        ret.imported = stoi(tempstr.substr(tempstr.find(":", 4)+2, string::npos));
                    }
                }
            }
            return ret;
        }
    }
    
    vector<ks> search_keyserver(string homedir, string keyserver, string key) {
        vector<ks> ret;
        string res;
        string cmd = "gpg2 --batch --no-tty --homedir ";
        cmd += homedir;
        cmd += " --keyserver ";
        cmd += keyserver;
        cmd += " --expert --search-key ";
        cmd += key;
        cmd += " 2>&1";
        
        FILE *pipe = popen(cmd.c_str(), "r");
        if(!pipe) {
            log.write("Failed to open process for searching keys", 2);
        } else {
            char tmp[128];
            while(!feof(pipe)) {
                if(fgets(tmp, 128, pipe) != NULL) {
                    res += tmp;
                }
            }
            pclose(pipe);

            istringstream iss(res);
            string tempstr;
            bool new_entry = false;

            while(getline(iss, tempstr)) {
                if(tempstr.substr(0,1) == "(") {
                    string tmp1 = tempstr.substr(tempstr.find(")")+2, string::npos);
                    ks key;
                    key.name = tmp1.substr(0, tmp1.find(" <"));
                    key.email = tmp1.substr(tmp1.find(" <")+2, tmp1.find(">", tmp1.find(" <"))-(tmp1.find(" <")+2));
                    key.key = "";
                    ret.push_back(key);
                    new_entry = true;
                } else if(new_entry) {
                    ret.at(ret.size()-1).key = tempstr.substr(tempstr.find("key")+4, 8);
                    new_entry = false;
                }
            }
        }
        return ret;
    }    
    
    /* Tracker queue */
    
    //Get all items which match action and are finished
    vector<tqs> get_tq(int action) {
        vector<uint> delete_queue;
        tq_lock.lock();
        vector<tqs> ret;
        for(uint i = 0; i < tracker_queue.size(); i++) {
            if(tracker_queue.at(i).finished && tracker_queue.at(i).action == action) {
                ret.push_back(tracker_queue.at(i));
                delete_queue.push_back(i);
            }
        }
        tq_lock.unlock();
        for(uint i = 0; i < delete_queue.size(); i++) {
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
        if(id <= tracker_queue.size()-1) {
            tracker_queue.erase(tracker_queue.begin()+id);
        } else {
            log.write("Invalid id for remove_tq", 2);
        }
        tq_lock.unlock();
    }
    
    bool update_tq(string transaction_id, vector<ads> announce_queue, int seeders, int leechers) {
        tq_lock.lock();
        bool found = false;
        for(uint i = 0; i < tracker_queue.size(); i++) {
            if(tracker_queue.at(i).transaction_id == transaction_id) {
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
        for(uint i = 0; i < tracker_queue.size(); i++) {
            if(tracker_queue.at(i).transaction_id == transaction_id) {
                tracker_queue.at(i).scrape_queue = scrape_queue;
                tracker_queue.at(i).finished = true;
                found = true;
                break;
            }
        }
        tq_lock.unlock();
        return found;
    }
    
    /* Gpgme */
    
    void init_gpgme(gpgme_ctx_t *ctx, string gpgdir) {
        gpgme_check_version(NULL);
        gpgme_error_t err;

        setlocale (LC_ALL, "");
        gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
        err = gpgme_new(ctx);
        if(err) {
            log.write("Failed to get gpgme context.",2);
            string tmp1 = "Error code: ";
            tmp1 += gpg_err_code(err);
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
        if(err) {
            log.write("Failed to set gpgme engine info.", 2);
            return;
        } 
    }
    
    /* Settings */
    void read_settings(string sett, gpgme_ctx_t ctx, gpgme_key_t *the_key, string *alias, string *fingerprint) {
        gpgme_key_t key;
        gpgme_error_t err;
        vector<gpgme_key_t> temp_keystore;
        settings set(sett);
        if(set.read_settings()) {
            *alias = set.get_alias();
            string def_key = set.get_key();
            err = gpgme_op_keylist_start(ctx, NULL, 1);
            bool found = false;
            while(1) {
                err = gpgme_op_keylist_next(ctx, &key);
                if(err) {
                    break;
                }
                bool matches = true;
                for(uint i = 0; i < def_key.size(); i++) {
                    if(key->subkeys->keyid[i] != def_key.at(i)) {
                        matches = false;
                        break;
                    }
                }

                if(matches) {
                    *the_key = key;
                    *fingerprint = key->subkeys->fpr;
                    found = true;
                    break;
                }
                gpgme_key_release(key);
            }

            if(!found) {
                log.write("Couldn't find default key in keychain.",2);
                gpgme_release(ctx);
                return;
            }
        } else {
            //Ask if create new
            //Ask for alias
            *alias = DEFAULT_ALIAS;
            set.change_alias(*alias);
            err = gpgme_op_keylist_start(ctx, NULL, 1);
            int counter = 0;
            while(1) {
                err = gpgme_op_keylist_next(ctx, &key);
                if(err) {
                    break;
                }
                cout << counter << ". " << key->subkeys->keyid << " (" << key->subkeys->fpr << ")";
                if(key->uids && key->uids->name) {
                    cout << " " << key->uids->name;
                }
                if(key->uids && key->uids->email) {
                    cout << " <" << key->uids->email << ">";
                }
                cout << endl;
                counter++;
                temp_keystore.push_back(key);
                //gpgme_key_release(key);
            }

            if(counter == 0) {
                log.write("No secret keys found!",2);
                log.write("Generate key using gpg2 --gen-key and try again.", 2);
                gpgme_release(ctx);
                return;
            } else {
                log.write("Which key would you like to use ? (Finger Print in brackets will be used as a identifier)");
                cout << "Key number: ";
                string keynum;
                cin >> keynum;
                if(stoi(keynum) > temp_keystore.size()-1) {
                    while(1) {
                        log.write("Invalid number.", 2);
                        cout << "Key number: ";
                        cin >> keynum;
                        if(stoi(keynum) <= temp_keystore.size()-1) {
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


                for(uint i = 0; i < temp_keystore.size(); i++) {
                    if(i != stoi(keynum)) {
                        gpgme_key_release(temp_keystore.at(i));
                    }
                }

                temp_keystore.clear();

            }
            set.create_settings();        
        }
    }
    
    /* Tests */
    
    void test_import(string homedir, string keyserver, string key) {
        log.write("test_import start",3);
        is test = import_key_from_keyserver(homedir, keyserver, key);
        string vs = "Key vector size: ";
        vs += to_string(test.keys.size());
        log.write(vs, 4);
        for(uint i = 0; i < test.keys.size(); i++) {
            string c = to_string(i);
            c += ". ";
            c += test.keys.at(i).name;
            c += " // ";
            c += test.keys.at(i).email;
            c += " // ";
            c += test.keys.at(i).key;
            log.write(c, 4);
        }
        string proc = "Processed: ";
        proc += to_string(test.processed);
        log.write(proc, 4);
        string imp = "Imported: ";
        imp += to_string(test.imported);
        log.write(imp, 4);
        string unc = "Unchanged: ";
        unc += to_string(test.unchanged);
        log.write(unc, 4);
        log.write("test_import end",3);
    }
    
    void test_search(string homedir, string keyserver, string key) {
        log.write("test_search start", 3);
        vector<ks> test = search_keyserver(homedir, keyserver, key);
    
        if(test.size() == 0) {
            log.write("No results.", 4);
        }

        for(uint i = 0; i < test.size(); i++) {
            string c = to_string(i);
            c += ". ";
            c += test.at(i).name;
            c += " // ";
            c += test.at(i).email;
            c += " // ";
            c += test.at(i).key;
            log.write(c,4);
        }
        log.write("test_search end", 3);
    }
}
