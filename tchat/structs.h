#pragma once
#include "stdafx.h"
#ifndef STRUCTS_H
#define	STRUCTS_H

//tracker action struct
struct tas {
    int id;
    int act;
    int announce_type = 0;
    string hash;
    vector<string> hashes;
};

//send packet struct
struct sps {
    string id;
    string packet;
    int conn;
    vector<function<void(string)>> fnc;
};

//connected struct
struct cs {
    string hash;
    sockaddr_in str;
    uint8_t his_packet_num = 0;
    uint8_t my_packet_num = 0;
    string conn_hash;
    bool conn_acc = false;
    uint8_t heartbeat_rate = 0;
    steady_clock::time_point last_heartbeat;
    steady_clock::time_point last_heartbeat_received;
    steady_clock::time_point uptime;
    vector<sps> sent_packets;
};

//connect to struct
struct cts {
    addrinfo *str;
    string id;
};

//missing packet struct
struct mps {
    string id;
    uint8_t packet_id;
    steady_clock::time_point added;
};
//key struct
struct ks {
    string name;
    string email;
    string key;
};
//import struct
struct is {
    vector<ks> keys;
    uint8_t processed = 0;
    uint8_t imported = 0;
    uint8_t unchanged = 0;
};
//address struct
struct ads {
    string ip;
    uint16_t port;
};
//scrape struct
struct ss {
    string fingerprint;
    int complete;
    int downloaded;
    int incomplete;
};
//tracker queue struct   
struct tqs {
    string hash;
    int action;
    int action_type;
    string transaction_id;
    vector<ss> scrape_queue;
    vector<ads> announce_queue;
    bool finished = false;
    int seeders = 0;
    int leechers = 0;
};

#endif
