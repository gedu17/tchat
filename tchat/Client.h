#pragma once
#include "stdafx.h"
#include "Log.h"
#include "structs.h"
#ifndef CLIENT_H
#define	CLIENT_H

class Client {
public:
    Client(string fingerprint, gpgme_ctx_t gpgctx, string homedir, string keyserver);
    virtual ~Client();

    /* Default vector for function callbacks */
    static vector<function<void(string)>> DEFAULT_FNC_VECTOR;

    /* Public methods used initialising object and client thread */
    bool create_socket();
    int get_out_port();

    /* Public methods used by client thread. */
    void check_heartbeats();
    void check_missing_packets();
    void check_tq();
    void send_packet();
    void receive_packet();

    /* Public methods used by ui */
    void connect(string ip, int port);
    void disconnect(string id);
    string decrypt(string msg);
    string encrypt(string msg, string id);
    void get_connected();
    string get_fingerprint(string id);
    string get_time();
    bool is_connection_id(string id);
    void queue_data(string to, int type, string data, vector<function<void(string)>> fnc = DEFAULT_FNC_VECTOR, int connected = 1);

    /* TESTING */

    //for testing purposes, returns first connected id
    string get_conn_id();
    //for testing missing packets
    void test_mp();
private:

    /* Log */
    Log log;

    /* Variables */
    int sock;
    int buffer_size;
    int out_port;

    bool socket_created;

    string homedir;
    string keyserver;
    string fingerprint;

    int max_missing_packets;
    int conn_hash_length;
    int backup_length;
#ifdef __linux__
    unsigned char *buffer;
#endif
#ifdef _WIN32
    char *buffer;
#endif
    gpgme_ctx_t gpgctx;

    vector<sps> send_queue;
    vector<cs> conn_queue;
    vector<cts> conn_to_queue;
    vector<mps> missing_packet_queue;

    sps last_send_struct;

    //mutex
    mutex send_lock;

    /* Struct */
    sockaddr_in get_struct(string id);
    addrinfo *get_struct(string id, bool conn);

    /* Packet */
    string create_packet(int type, string data, uint8_t packet_number, int connected);

    /* Gpgme related */
    string sign(string msg);
    bool verify(string msg, string encmsg, string fingerprint);

    /* Helpers */
    void add_to_sent_packets(string id);
    void clear_buffer();
    void inc_my_packet_num(string id);
    string generate_conn_hash();
    

    /* Getters */
    string get_conn_str_id(sockaddr_in fromaddr, bool should_be = true);
    uint8_t get_heartbeat_rate(string id);
    void get_missing_packets(string id);
    uint8_t get_my_optimal_heartbeat();
    uint8_t get_my_packet_num(string id);
    string get_random_msg();

    /* Setters */
    void set_conn_accepted(string id);
    void set_heartbeat_rate(string id, uint8_t rate);
    void set_his_packet_num(string id, uint8_t packet);
    void set_received_heartbeat(string id);
    void set_sent_heartbeat(string id);
    

    /* Booleans */
    bool is_connected(sockaddr_in str);
    bool is_fingerprint_in_keychain(string fingerprint);
    bool is_missing_packets(string id);
    bool is_still_connected(string id);

    /* Remove */
    void remove_from_missing_queue(string id, uint8_t packet_id);
    void remove_peer(string id);

    /* Response methods */
    void handle_bad_info(sockaddr_in fromaddr);
    void handle_connect(int type, sockaddr_in fromaddr);
    void handle_data(sockaddr_in fromaddr, bool skip = false);
    void handle_disconnect(sockaddr_in fromaddr);
    void handle_heartbeat(int type, sockaddr_in fromaddr);
    void handle_im(sockaddr_in fromaddr);
    void handle_missing_packets(int type, sockaddr_in fromaddr);
    
    /* Callbacks after packet is sent */
    void callback_disconnect(string id);
    void callback_heartbeat(string id);

};

#endif