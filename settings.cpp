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

#include "settings.h"
#include "log.h"

settings::settings(string dir) {
    this->log = Log::getInstance();
    this->dir = dir;
    this->gpgdir = dir;
    this->gpgdir += "gnupg/";        
    this->file = this->dir;
    this->file += DEFAULT_SETTINGS_FILE;
    this->alias = DEFAULT_ALIAS;
    this->key = "00000000";
}

settings::~settings() {
    
}

/* Settings */

void settings::create_settings() {
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    query.exec("CREATE TABLE `profiles` ("
	"`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
	"`alias`	TEXT,"
	"`pubkey`	TEXT,"
        "`avatar`       TEXT,"
        "`default`   INTEGER DEFAULT 0)");
    query.exec("CREATE TABLE `friends` ("
	"`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
	"`alias`	TEXT,"
	"`fpr`	TEXT,"
	"`avatar`	TEXT,"
	"`last_seen`	INTEGER,"
	"`pid`	INTEGER)");
    query.exec("CREATE TABLE `chat_log` ("
	"`id`	INTEGER PRIMARY KEY AUTOINCREMENT,"
	"`friend_id`	INTEGER,"
	"`chat_type`	INTEGER,"
	"`time`	INTEGER,"
	"`value`	TEXT)");

    string ins = "INSERT INTO `profiles` (`alias`, `pubkey`, `avatar`, `default`) VALUES ('";
    ins += this->alias;
    ins += "', '";
    ins += this->key;
    ins += "', '";
    ins += DEFAULT_AVATAR_FILE;
    ins += "', 1)";
    query.exec(ins.c_str());
    
    this->log.write("Creating settings db");
}

void settings::read_settings(int id) {
    this->id = id;
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "SELECT * FROM `profiles` WHERE `id` = '";
    q += to_string(id);
    q += "' LIMIT 1";
    
    query.exec(q.c_str());
    while(query.next()) {
        this->alias = query.value(1).toString().toStdString();
        this->key = query.value(2).toString().toStdString();
        this->avatar = query.value(3).toString().toStdString();
    }
}

/* Alias */

void settings::change_alias(string alias) {
    this->alias = alias;
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "UPDATE `profiles` SET `alias` = '";
    q += alias;
    q += "' WHERE `id` = '";
    q += to_string(this->id);
    q += "'";
    query.exec(q.c_str());
}

string settings::get_alias() {
    return this->alias;
}

/* Key */

string settings::get_key() {
    return this->key;
}

void settings::set_key(string key) {
    this->key = key;
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "UPDATE `profiles` SET `pubkey` = '";
    q += key;
    q += "' WHERE `id` = '";
    q += to_string(this->id);
    q += "' LIMIT 1";
    query.exec(q.c_str());
}

/* Avatar */

string settings::get_avatar() {
    return this->avatar;
}

void settings::set_avatar(string avatar) {
    this->avatar = avatar;
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    string q = "UPDATE `profiles` SET `avatar` = '";
    q += avatar;
    q += "' WHERE `id` = '";
    q += to_string(this->id);
    q += "' LIMIT 1";
    query.exec(q.c_str());
}
