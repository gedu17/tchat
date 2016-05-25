#pragma once
#include "Log.h"
#include "Utils.h"
#ifndef TRACKER_THREAD_H
#define	TRACKER_THREAD_H
namespace trackerThread {
    /* Actions */
    void add_action(int id, int announce_type, string hash = "");
    void add_action(int id, vector<string> hashes);
    void remove_action(uint32_t id);
    void remove_tracker_actions(int id);

    /* Workflow */
    void run();
    void stop();

    /* Helpers */
    bool add_tracker(string hostname, int port);
    void create_socket();
    void check_connects();
    void check_reannounce();
    void do_actions();
    int get_id(string hostname);
    void receive_packet();
    bool remove_tracker(uint id);
    void set_announce_hash(string hash);
    void set_client_port(int port);
}

#endif

