#include "stdafx.h"
#include "Settings.h"
#include "Log.h"

settings::settings(string dir) {
    this->log = Log::get_instance();
    this->dir = dir;
    this->file = this->dir;
    this->file += DEFAULT_SETTINGS_FILE;
    this->alias = DEFAULT_ALIAS;
    this->key = "00000000";
}

settings::~settings() {

}

/* Settings */

void settings::create_settings() {
    this->log.write("Creating settings file ", 1);
    this->write_data();
}

bool settings::read_settings() {
    struct stat buffer;
    bool status = true;
    #ifdef __linux__
        if ((stat(this->dir.c_str(), &buffer) == 0) && S_ISDIR(buffer.st_mode)) {
    #endif
    #ifdef _WIN32
        if ((stat(this->dir.c_str(), &buffer) == 0) && (buffer.st_mode & _S_IFDIR)) {
    #endif
        if (!(stat(this->file.c_str(), &buffer) == 0)) {
            this->log.write("Settings file not found !", 2);
            status = false;
        }
        else {
            ifstream sf;
            sf.open(this->file);
            string temp;
            while (getline(sf, temp)) {
                if (temp.substr(0, 4) == "key=") {
                    this->key = temp.substr(4);
                }
                else if (temp.substr(0, 6) == "alias=") {
                    this->alias = temp.substr(6);
                }
                else if (temp.substr(0, 11) == "keyservers=") {
                    string ks = temp.substr(11, string::npos);
                    int offset = 0;
                    do  {

                        int diff = ks.find(",", offset) - offset;;
                        string item = ks.substr(offset, diff);
                        if(!item.empty()) {
                            this->key_servers.push_back(item);
                            offset = ks.find(",", offset)+1;
                        }
                    } while (ks.find(",", offset) != string::npos);
                    int diff = ks.find(",", offset) - offset;
                    string item = ks.substr(offset, diff);
                    if (!item.empty() && item != this->key_servers.at(this->key_servers.size()-1)) {
                        this->key_servers.push_back(item);
                    }
                }
                else if (temp.substr(0, 9) == "trackers=") {
                    string ts = temp.substr(9, string::npos);
                    int offset = 0;
                    do {
                        int diff = ts.find(",", offset) - offset;
                        string item = ts.substr(offset, diff);
                        if(!item.empty()) {
                            this->trackers.push_back(item);
                            offset = ts.find(",", offset) + 1;
                        }
                    } while (ts.find(",", offset) != string::npos);

                    int diff = ts.find(",", offset) - offset;
                    string item = ts.substr(offset, diff);
                    if (!item.empty() && item != this->trackers.at(this->trackers.size() - 1)) {
                        this->trackers.push_back(item);
                    }
                }
                else {
                    string pr = "Unknown setting in settings file: " + temp;
                    log.write(pr, 2);
                    status = false;
                }
            }
        }
    }
    else {
        status = false;
        #ifdef __linux__
            mkdir(this->dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
        #endif
        #ifdef _WIN32
            _mkdir(this->dir.c_str());
        #endif
    }
    return status;
}

void settings::update_settings() {
    this->log.write("Updating settings file", 1);
    this->write_data();
}

/* Alias */

void settings::set_alias(string alias) {
    this->alias = alias;
}

string settings::get_alias() {
    return this->alias;
}

/* Key */

string settings::get_key() {
    return this->key;
}

void settings::set_key(string key) {
    this->key = key;
}

/* Key servers */

vector<string> settings::get_key_servers() {
    return this->key_servers;
}

void settings::add_key_server(string server) {
    this->key_servers.push_back(server);
}

/* Trackers */

vector<string> settings::get_trackers() {
    return this->trackers;
}

void settings::add_tracker(string tracker) {
    this->trackers.push_back(tracker);
}

/* Private methods */

void settings::write_data() {
    ofstream sf;
    sf.open(this->file);
    sf << "key=" << this->key << endl;
    sf << "alias=" << this->alias << endl;
    sf << "keyservers=";
    for (uint i = 0; i < this->key_servers.size(); i++) {
        sf << this->key_servers.at(i);
        if (i != this->key_servers.size() - 1) {
            sf << ",";
        }
    }
    sf << endl;
    sf << "trackers=";
    for (uint i = 0; i < this->trackers.size(); i++) {
        sf << this->trackers.at(i);
        if (i != this->trackers.size() - 1) {
            sf << ",";
        }
    }
    sf << endl;
    sf.close();
}