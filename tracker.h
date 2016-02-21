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

#ifndef TRACKER_H
#define	TRACKER_H
#include "includes.h"
#include "log.h"
#include "utils.h"
class Tracker {
public:
    Tracker(string hostname, int port);
    virtual ~Tracker();
    
    /* Setters */
    void set_action(char action);
    void set_action_id(uint id);
    void set_action_time(steady_clock::time_point time);
    void set_announce_time(steady_clock::time_point time);
    void set_announce_port(uint16_t port);
    void set_buffer(unsigned char buffer[BUFFER_LENGTH], int size);
    void set_socket(int sock);
    void set_socket(QUdpSocket *sock);
    void set_socket_port(quint16 port);
    void set_socket_port(int port);
    void set_waiting_response(bool value);    
    
    /* Getters */
    uint get_action_id();
    steady_clock::time_point get_action_time();
    steady_clock::time_point get_announce_time();
    int get_announce_type();
    bool get_announcing();
    bool get_connected();
    string get_hostname();
    quint32 get_hostname_in();
    quint16 get_port_in();
    int get_reannounce();
    bool get_waiting_response();  
    
    /* Announce */
    void announce(string hash, int type, uint16_t port);
    void finish_announce();
    
    /* Connect */
    void connect();
    void finish_connect();
    
    /* Scrape */
    void scrape(vector<string> hashes);
    void finish_scrape();
    
    /* Helpers */
    bool is_connected();
    void dump();

private:
    /* variables */
    char action;
    //Used in tracker thread action queue vector
    uint action_id;
    steady_clock::time_point action_time;
    uint16_t announce_port;
    steady_clock::time_point announce_time;
    int announce_type;
    //True when announced my fingerprint to server
    bool announcing;
    //Ascii representation of hash (fingerprint)
    string ascii_hash;
    
    unsigned char *buffer;
    int buffer_size;
    
    bool connected;
    vector<unsigned char> connection_id;
    
    string hash;
    vector<string> hashes;
    string hostname;
    
    Log log;  
    
    string packet;
    vector<unsigned char> peer_id;
    int port;
    
    vector<unsigned char> random_key;
    int reannounce;
    
    //struct sockaddr_in *server_sock;
    QHostAddress server_sock;
    int sock;
    QUdpSocket *qsock;
    quint16 socket_port;
    
    vector<unsigned char> transaction_id;
    
    bool waiting_response;
    
    
    /* Setters */
    void set_conn_id();
    void set_peer_id();
    void set_std_conn_id();   
    
    /* Packets */
    void create_packet(int type);
    int send_packet();
    
    /* Helpers */
    bool check_transaction_id();
    void clear_buffer();
    void generate_random_key();
    void generate_transaction_id();
    int string_to_hex(char f, char s);
    
};

#endif	/* TRACKER_H */

