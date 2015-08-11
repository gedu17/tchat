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

#include "friendslist.h"
#include "log.h"
#include "structs.h"

friendslist::friendslist(string file, int id) {
    this->log = Log::getInstance();
    this->file = file;
    this->file += DEFAULT_FRIENDSLIST_FILE;
    this->id = id;
}

friendslist::~friendslist() {
}

uint friendslist::get_friend_list_count() {
    return this->friend_list.size();
}

int friendslist::get_online_count() {
    return this->online_counter;
}

bool friendslist::set_custom_status(string fingerprint, string status) {
    bool ret = false;
    for(uint i = 0; i < this->friend_list.size(); i++) {
        if(fingerprint == this->friend_list.at(i)->get_fingerprint()) {
            this->friend_list.at(i)->set_custom_status(status);
            ret = true;
            break;
        }
    }
    return ret;
}

bool friendslist::set_status(string fingerprint, int status) {
    bool ret = false;
    for(uint i = 0; i < this->friend_list.size(); i++) {
        if(fingerprint == this->friend_list.at(i)->get_fingerprint()) {
            this->friend_list.at(i)->set_status(status);
            ret = true;
            break;
        }
    }
    return ret;
}

bool friendslist::add_friend(string csid, string alias, string fingerprint, string avatar) {
    bool ret = true;
    for(uint i = 0; i < this->friend_list.size(); i++) {
        if(this->friend_list.at(i)->get_fingerprint() == fingerprint) {
            ret = false;
            break;
        }
    }
    
    if(ret) {
        my_friend *tmp = new my_friend(fingerprint, alias, avatar, 0);
        tmp->set_csid(csid);
        this->friend_list.push_back(tmp);
        QSqlDatabase db = QSqlDatabase::database("data");
        QSqlQuery query(db);
        string q = "INSERT INTO `friends` (`alias`, `fpr`, `avatar`, `last_seen`, `pid`) VALUES ('";
        q += alias;
        q += "', '";
        q += fingerprint;
        q += "', '";
        q += avatar;
        q += "', '";
        q += to_string(this->get_current_time());
        q += "', '";
        q += to_string(this->id);
        q += "')";
        query.exec(q.c_str());
    }
    return ret;
}

bool friendslist::remove_friend(string fingerprint) {
    bool ret = false;
    uint id = 0;
    for(uint i = 0; i < this->friend_list.size(); i++) {
        if(fingerprint == this->friend_list.at(i)->get_fingerprint()) {
            id = i;
            ret = true;
            break;
        }
        
    }
    
    if(ret) {
        this->friend_list.erase(this->friend_list.begin()+id);
        QSqlDatabase db = QSqlDatabase::database("data");
        QSqlQuery query(db);
        string q = "DELETE FROM `friends` WHERE `pid` = '";
        q += to_string(this->id);
        q += "' && `fpr` = '";
        q += fingerprint;
        q += "' LIMIT 1";
        query.exec(q.c_str());
    }
    return ret;
}

void friendslist::read_friend_list() {
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "SELECT * FROM `friends` WHERE `pid` = '";
    q += to_string(this->id);
    q += "'";
    query.exec(q.c_str());
    while(query.next()) {
        my_friend *tmp = new my_friend(query.value(2).toString().toStdString(), query.value(1).toString().toStdString(), 
                query.value(3).toString().toStdString(), query.value(4).toInt());
        this->friend_list.push_back(tmp);
    }
}

my_friend *friendslist::get_friend(uint id) {
    if(this->friend_list.size() < id) {
        throw "friendslist::get_friend: Friend id too big";
    } else {
        return this->friend_list.at(id);
    }
}

long int friendslist::get_current_time() {
    return system_clock::now().time_since_epoch().count();
}