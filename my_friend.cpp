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

#include "my_friend.h"

my_friend::my_friend(string fingerprint, string alias, string avatar, uint last_seen) {
    this->fingerprint = fingerprint;
    this->alias = alias;
    this->avatar = avatar;
    this->last_seen = last_seen;
    this->status = 0;
    this->custom_status = "";
    this->csid = "";
    
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "SELECT `id` FROM `friends` WHERE `fpr` = '";
    q += fingerprint;
    q += "' LIMIT 1";
    query.exec(q.c_str());
    while(query.next()) {
        this->db_id = query.value(0).toInt();
    }
            
    string tmp = "Last seen: ";;
    if(last_seen == 0) {
        tmp += "never.";
    } else {
        time_t tmp2 = last_seen;
        stringstream ss;
        ss << put_time(localtime(&tmp2), "%d-%m-%y %H:%M:%S");
        
        tmp += ss.str();
    }
    
    this->add_to_chatlog(tmp);
    
    this->read_from_chatlog();
}

my_friend::~my_friend() {
}

string my_friend::get_alias() {
    return this->alias;
}

string my_friend::get_avatar() {
    return this->avatar;
}

string my_friend::get_csid() {
    return this->csid;
}

string my_friend::get_custom_status() {
    return this->custom_status;
}

string my_friend::get_fingerprint() {
    return this->fingerprint;
}

int my_friend::get_status() {
    return this->status;
}

long int my_friend::get_last_seen() {
    return this->last_seen;
}

string my_friend::get_chatlog() {
    return this->chatlog;
}

void my_friend::set_alias(string alias) {
    this->alias = alias;
}

void my_friend::set_avatar(string avatar) {
    this->avatar = avatar;
}

void my_friend::set_csid(string csid) {
    this->csid = csid;
}

void my_friend::set_custom_status(string status) {
    this->custom_status = status;
}

void my_friend::set_last_seen(uint64_t last_seen) {
    this->last_seen = last_seen;
    //TODO: Update db with this number!
}

void my_friend::set_status(int status) {
    this->status = status;
}

//Returns the div id
int my_friend::add_to_chatlog(string msg, string user, bool write_to_db, time_t time_now) {
    /*random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint8_t> numb(0,61);
    auto get_symbol = [](uint8_t number) -> char {
        string symbols = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
        return symbols.at(number);
    };
    string id = "";
    for(uint i = 0; i < 10; i++) {
        id += get_symbol(numb(gen));
    }*/

    tm time = *localtime(&time_now);
    int id = this->get_chatlog_id();
    this->chatlog += "<div id=\"";
    this->chatlog += to_string(id);
    this->chatlog += "\">[";
    
    stringstream ss;
    ss << put_time(&time, "%H:%M:%S");
    
    this->chatlog += ss.str();
    this->chatlog += "] ";
    this->chatlog += user;
    this->chatlog += ": ";
    this->chatlog += msg;
    this->chatlog += "</div>";    
    if(write_to_db) {
        QSqlDatabase db = QSqlDatabase::database("data");
        QSqlQuery query(db);
        string q = "INSERT INTO `chat_log` (`friend_id`, `chat_type`, `time`, `value`) VALUES (";
        q += to_string(this->db_id);
        q += ", ";
        if(user == this->alias) {
            q += "1";
        } else {
            q += "0";
        }
        q += ", ";
        q += to_string(time_now);
        q += ", \"";
        q += msg;
        q += "\")";

        query.exec(q.c_str());
    }
    
    return id;
}

int my_friend::add_to_chatlog(string msg) {
    /*random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint8_t> numb(0,61);
    auto get_symbol = [](uint8_t number) -> char {
        string symbols = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
        return symbols.at(number);
    };
    string id = "";
    for(uint i = 0; i < 10; i++) {
        id += get_symbol(numb(gen));
    }*/
    int id = this->get_chatlog_id();
    this->chatlog += "<div id=\"";
    this->chatlog += to_string(id);
    this->chatlog += "\">";
    this->chatlog += msg;
    this->chatlog += "</div>";    
    
    return id;
}


int my_friend::add_to_chatlog_range(bool write_to_db, time_t time_from, time_t time_until) {
    /*random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint8_t> numb(0,61);
    auto get_symbol = [](uint8_t number) -> char {
        string symbols = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
        return symbols.at(number);
    };
    string id = "";
    for(uint i = 0; i < 10; i++) {
        id += get_symbol(numb(gen));
    }
    */
    tm time = *localtime(&time_from);
    tm time2 = *localtime(&time_until);
    
    int id = this->get_chatlog_id();
    this->chatlog += "<div id=\"";
    this->chatlog += to_string(id);
    this->chatlog += "\">[";
    
    stringstream ss;
    ss << put_time(&time, "%H:%M:%S");
    this->chatlog += ss.str();
    this->chatlog += " - ";
    
    ss.str(string());
    ss << put_time(&time2, "%H:%M:%S");
    this->chatlog += ss.str();
    
    this->chatlog += "] Voice Chat</div>";
    
    if(write_to_db) {
        QSqlDatabase db = QSqlDatabase::database("data");
        QSqlQuery query(db);
        string q = "INSERT INTO `chat_log` (`friend_id`, `chat_type`, `time`, `value`) VALUES (";
        q += to_string(this->db_id);
        q += ", 2, ";
        q += to_string(time_from);
        q += ", ";
        q += to_string(time_until);
        q += ")";
        query.exec(q.c_str());
    }
    return id;
}

void my_friend::read_from_chatlog() {
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "SELECT * FROM `chat_log` WHERE `friend_id` = '";
    q += to_string(this->db_id);
    q += "'";
    query.exec(q.c_str());
    while(query.next()) {
        time_t tmpt = query.value(3).toInt();
        if(query.value(2).toInt() == 0) {
            this->add_to_chatlog(query.value(4).toString().toStdString(), "Me", false, tmpt);
        } else if(query.value(2).toInt() == 1) {
            this->add_to_chatlog(query.value(4).toString().toStdString(), this->alias, false, tmpt);
        } else if(query.value(2).toInt() == 2) {
            time_t tmpt2 = query.value(4).toInt();
            this->add_to_chatlog_range(false, tmpt, tmpt2);
        } else {
            cout << "Unkown chat type !" << query.value(2).toInt() << endl;
        }
        
    }
    
}

int my_friend::get_chatlog_id() {
    int tmp = this->id;
    this->id++;
    return tmp;
}