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
#include "utils.h"
#ifndef TRACKER_THREAD_H
#define	TRACKER_THREAD_H
namespace tracker_thread {
    /* Actions */
    void add_action(int id, int announce_type, string hash = "");
    void add_action(int id, vector<string> hashes);
    void remove_action(uint id);    
    
    /* Workflow */
    void run();
    void stop();
    
    /* Helpers */
    bool add_tracker(string hostname, int port);
    void create_socket();
    void create_qsocket();
    void set_announce_hash(string hash);
    void set_client_port(int port);
    void receive_packet();
    void check_reannounce();
    void do_actions();
    void check_connects();
    
}

#endif	/* TRACKER_THREAD_H */

