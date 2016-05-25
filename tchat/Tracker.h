#pragma once
#ifndef TRACKER_H
#define	TRACKER_H
#include "stdafx.h"
#include "Log.h"
#include "Utils.h"
class Tracker {
public:
    Tracker(string hostname, int port);
    virtual ~Tracker();

    /* Setters */
    void set_action(char action);
    void set_action_id(uint32_t id);
    void set_action_time(steady_clock::time_point time);
    void set_announce_time(steady_clock::time_point time);
    void set_announce_port(uint16_t port);
#ifdef __linux__
    void set_buffer(unsigned char buffer[BUFFER_LENGTH], int size);
#endif
#ifdef _WIN32
    void set_buffer(char buffer[BUFFER_LENGTH], int size);
#endif
    void set_socket(int sock);
    void set_socket_port(int port);
    void set_waiting_response(bool value);

    /* Getters */
    uint32_t get_action_id();
    steady_clock::time_point get_action_time();
    steady_clock::time_point get_announce_time();
    int get_announce_type();
    bool get_announcing();
    bool get_connected();
    string get_hostname();
    in_addr get_hostname_in();
#ifdef __linux__
    in_port_t get_port_in();
#endif
#ifdef _WIN32
    unsigned short get_port_in();
#endif
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

    void dump();

private:
    /* variables */
    char action;
    //Used in tracker thread action queue vector
    uint32_t action_id;
    steady_clock::time_point action_time;
    uint16_t announce_port;
    steady_clock::time_point announce_time;
    int announce_type;
    //True when announced my fingerprint to server
    bool announcing;
    //Ascii representation of hash (fingerprint)
    string ascii_hash;

#ifdef __linux__
    unsigned char *buffer;
#endif
#ifdef _WIN32
    char *buffer;
#endif

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

    addrinfo *server_sock;
    int sock;
    int socket_port;

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
    void clear_buffer();
    void generate_random_key();
    void generate_transaction_id();
    int string_to_hex(char f, char s);

};

#endif
