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
        }
        return ret;
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
    
    void remove_tq(uint id) {
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
