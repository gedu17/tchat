#pragma once
#include "stdafx.h"
#include "Log.h"
#ifndef SETTINGS_H
#define	SETTINGS_H

class settings {
public:
    settings(string filename);
    virtual ~settings();


    /* Settings */
    void create_settings();
    bool read_settings();
    void update_settings();

    /* Alias */
    void set_alias(string alias);
    string get_alias();

    /* Key */
    string get_key();
    void set_key(string key);

    /* Key servers */
    vector<string> get_key_servers();
    void add_key_server(string server);

    /* Trackers */
    vector<string> get_trackers();
    void add_tracker(string tracker);


private:
    /* Variables */
    string file;
    string key;
    string dir;
    string alias;
    vector<string> key_servers;
    vector<string> trackers;

    void write_data();

    /* Log */
    Log log;
};

#endif

