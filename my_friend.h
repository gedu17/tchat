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

#ifndef MY_FRIEND_H
#define	MY_FRIEND_H
#include "includes.h"
class my_friend {
public:
    my_friend(string fingerprint, string alias, string avatar, uint last_seen);
    //my_friend(const my_friend& orig);
    void set_csid(string csid);
    void set_custom_status(string status);
    void set_avatar(string avatar);
    void set_alias(string alias);
    void set_last_seen(uint64_t last_seen);
    void set_status(int status);
    
    string get_fingerprint();
    string get_custom_status();
    int get_status();
    string get_alias();
    string get_avatar();
    long int get_last_seen();
    string get_csid();
    
    string get_chatlog();
    
    
    int add_to_chatlog(string msg, string user, bool write_to_db, time_t time_now = time(nullptr));
    int add_to_chatlog(string msg);
    int add_to_chatlog_range(bool write_to_db, time_t time_from, time_t time_until);
    
    virtual ~my_friend();
private:
    string fingerprint;
    int status;
    string csid;
    string custom_status;
    string alias;
    string avatar;
    uint64_t last_seen;
    string chatlog;
    int id;
    int db_id;
    void refresh_main();
    int get_chatlog_id();
    void read_from_chatlog();
};

#endif	/* MY_FRIEND_H */

