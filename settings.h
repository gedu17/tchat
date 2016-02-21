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

#include "includes.h"
#include "log.h"
#ifndef SETTINGS_H
#define	SETTINGS_H

class settings {
public:
    settings(string filename);
    settings();
    virtual ~settings();
    
    
    /* Settings */
    void create_settings();
    void read_settings(int id);
    //void update_settings();
    
    /* Alias */
    void change_alias(string alias);
    string get_alias();
    
    /* Key */
    string get_key();
    void set_key(string key);
    
    /* Avatar */
    string get_avatar();
    void set_avatar(string avatar);
private:
    /* Variables */
    string file;
    string key;
    string dir;
    string gpgdir;
    string alias;
    string avatar;
    int id;
    
    /* Log */
    Log log;
    
    /* QtSql */
    QSqlDatabase *db;
};

#endif	/* SETTINGS_H */

