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

#ifndef FRIENDSLIST_H
#define	FRIENDSLIST_H
#include "includes.h"
#include "structs.h"
#include "log.h"
#include "my_friend.h"
class friendslist {
public:
    friendslist(string file, int id);

    virtual ~friendslist();
    uint get_friend_list_count();
    int get_online_count();
    bool set_custom_status(string fingerprint, string status);
    bool set_status(string fingerprint, int status);
    bool add_friend(string csid, string alias, string fingerprint, string avatar);
    bool remove_friend(string fingerprint);
    void read_friend_list();
    my_friend *get_friend(uint id);
    bool set_avatar(string fingerprint, string avatar);
private:    
    //void create_friend_list();
    long int get_current_time();
    vector<my_friend *> friend_list;
    int online_counter;
    string file;
    Log log;
    int id;
};

#endif	/* FRIENDSLIST_H */

