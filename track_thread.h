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

#ifndef TRACK_THREAD_H
#define	TRACK_THREAD_H
#include "includes.h"
#include "log.h"
#include "utils.h"
#include "tracker.h"
class track_thread : public QThread {
public:
    void set_announce_hash(string hash);
    void set_client_port(int port);
    
    /* Actions */
    void add_action(int id, int announce_type, string hash = "");
    void add_action(int id, vector<string> hashes);
    void remove_action(uint id);    
    
    /* Workflow */
    //void run();
    void stop();
    
    /* Helpers */
    bool add_tracker(string hostname, int port);
    void create_socket();
    void create_qsocket();
    
    void receive_packet();
    void check_reannounce();
    void do_actions();
    void check_connects();
    Q_OBJECT
protected:
    void run();
    
private:
    mutex action_lock;
    

    /* Vectors */

    vector<tas> action_queue;
    vector<Tracker> tracker_list;

    /* Log */
    Log log;

    /* Sockets */
    bool socket_created;
    int out_port;
    int sock;
    QUdpSocket *qsock;
    int client_port;

    bool cl_port;
    bool cl_hash;

    /* Various */
    uint connected_trackers;

    string announce_hash;

    unsigned char buffer[BUFFER_LENGTH];

    bool cycle_run;

};

#endif	/* TRACK_THREAD_H */

