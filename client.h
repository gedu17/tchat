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
#include "structs.h"
#ifndef CLIENT_H
#define	CLIENT_H
#include "friendslist.h"
class Client {
public:
    Client(string fingerprint, gpgme_ctx_t gpgctx, string homedir, string keyserver, friendslist *fl);
    virtual ~Client();
    
    /* Default vector for function callbacks */
    static vector<function<void(string)>> DEFAULT_FNC_VECTOR;
    static bool obj_status;
    /* Public methods used initialising object and client thread */
    bool create_socket();
    bool create_qsocket();
    int get_out_port();
    
    /* Public methods used by client thread. */
    void check_heartbeats();
    void check_missing_packets();
    void check_tq();
    void send_packet();
    void receive_packet();
    
    void send_qpacket();
    void receive_qpacket();
    
    /* Public methods used by ui */
    void queue_data(string to, int type, string data, vector<function<void(string)>> fnc = DEFAULT_FNC_VECTOR, int connected = 1);
    void get_connected();
    void connect(string ip, int port);
    void disconnect(string id);
    
    
    /*TESTING*/
    
    //for testing purposes, returns first connected id
    string get_conn_id();    
    //for testing missing packets
    void test_mp();
private:
    
    /* Log */
    Log log;
    
    /* Variables */
    int sock;
    QUdpSocket *qsock;
    uint buffer_size;
    int out_port;
    
    bool socket_created;
    
    string homedir;
    string keyserver;
    string fingerprint;
  
    uint max_missing_packets;
    uint conn_hash_length;
    uint backup_length;
    unsigned char *buffer;
    
    gpgme_ctx_t gpgctx;
    
    vector<sps> send_queue;
    vector<cs> conn_queue;
    vector<cts> conn_to_queue;
    vector<mps> missing_packet_queue;

    sps last_send_struct;
    
    friendslist *fl;
    
    //mutex
    mutex send_lock;
    
    /* Struct */
    us get_struct(string id);
    //addrinfo *get_structv2(string id);
    
    /* Packet */
    string create_packet(int type, string data, uint8_t packet_number, int connected);
    
    /* Gpgme related */
    string encrypt(string msg);    
    string decrypt(string msg);
    string sign(string msg);
    bool verify(string msg, string encmsg, string fingerprint);
    
    /* Helpers */
    void clear_buffer();
    void inc_my_packet_num(string id);
    string generate_conn_hash();
    void add_to_sent_packets(string id);
    
    /* Getters */
    void get_missing_packets(string id);
    uint8_t get_heartbeat_rate(string id);
    uint8_t get_my_optimal_heartbeat();
    uint8_t get_my_packet_num(string id);
    string get_conn_str_id(us fromaddr, bool should_be = true);
    string get_random_msg();
    
    /* Setters */
    void set_conn_accepted(string id);
    void set_heartbeat_rate(string id, uint8_t rate);
    void set_received_heartbeat(string id);
    void set_sent_heartbeat(string id);
    void set_his_packet_num(string id, uint8_t packet);
    
    /* Booleans */
    bool is_connected(us str);
    bool is_missing_packets(string id);
    bool is_still_connected(string id);
    bool is_fingerprint_in_keychain(string fingerprint);
    
    /* Remove */
    void remove_from_missing_queue(string id, uint8_t packet_id);
    void remove_peer(string id);
    
    /* Response methods */
    void handle_connect(int type, us fromaddr);
    void handle_data(us fromaddr, bool skip = false);
    void handle_bad_info(us fromaddr);
    void handle_disconnect(us fromaddr);
    void handle_heartbeat(int type, us fromaddr);
    void handle_missing_packets(int type, us fromaddr);
    
    /* Callbacks after packet is sent */
    void callback_disconnect(string id);
    void callback_heartbeat(string id);

};

#endif	/* CLIENT_H */

