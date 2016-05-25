#include "stdafx.h"
#include "TrackerThread.h"
#include "Tracker.h"
#include "Utils.h"
#include "structs.h"

namespace trackerThread {

    /* Mutexes */

    mutex action_lock;
    mutex list_lock;

    /* Vectors */

    vector<tas> action_queue;
    vector<Tracker> tracker_list;

    /* Log */
    Log log = Log::get_instance();

    /* Sockets */
    bool socket_created = false;
    int out_port = 0;
    int sock;
    int client_port;


    /* Various */
    uint32_t connected_trackers = 0;

    string announce_hash;

    #ifdef __linux__
        unsigned char buffer[BUFFER_LENGTH];
    #endif
    #ifdef _WIN32
        char buffer[BUFFER_LENGTH];
    #endif

    bool cycle_run = true;

    /* Actions */

    void add_action(int id, int announce_type, string hash) {
        action_lock.lock();
        tas temp;
        temp.act = 1;
        temp.announce_type = announce_type;
        temp.id = id;
        if (hash == "") {
            temp.hash = announce_hash;
        }
        else {
            temp.hash = hash;
        }
        action_queue.push_back(temp);
        action_lock.unlock();
    }

    void add_action(int id, vector<string> hashes) {
        action_lock.lock();
        tas temp;
        temp.act = 2;
        temp.id = id;
        temp.hashes = hashes;
        action_queue.push_back(temp);
        action_lock.unlock();
    }

    void remove_action(uint32_t id) {
        action_lock.lock();
        if (action_queue.size() >= id) {
            action_queue.erase(action_queue.begin() + id);
        }
        else {
            string tmp = "Cannot delete id ";
            tmp += id;
            log.write(tmp, 1);
        }
        action_lock.unlock();
    }

    void remove_tracker_actions(int id) {
        action_lock.lock();
        vector<tas> tmp;
        for (uint i = 0; i < action_queue.size(); i++) {
            if (action_queue.at(i).id != id) {
                tmp.push_back(action_queue.at(i));
            }
        }
        action_queue = tmp;
        action_lock.unlock();
    }

    /* Workflow */

    void run() {
        log.write("Starting tracker thread.", 1);
        while (cycle_run) {

            if (!socket_created) {
                create_socket();
            }

            check_connects();

            receive_packet();

            do_actions();

            check_reannounce();

            std::this_thread::sleep_for(200ms);

        }
    }

    void stop() {
        cycle_run = false;
        #ifdef __linux__
            close(sock);
        #endif
        #ifdef _WIN32
            closesocket(sock);
        #endif
    }

    /* Helpers */

    bool add_tracker(string hostname, int port) {
        bool is_in_list = false;
        list_lock.lock();
        for (uint32_t i = 0; i < tracker_list.size(); i++) {
            if (tracker_list.at(i).get_hostname() == hostname && tracker_list.at(i).get_port_in() == port) {
                is_in_list = true;
                break;
            }
        }
        if (!is_in_list) {
            Tracker *temp = new Tracker(hostname, port);
            tracker_list.push_back(*temp);
            list_lock.unlock();
            return true;
        }
        list_lock.unlock();
        return false;
    }

    void create_socket() {
        #ifdef _WIN32
            WSADATA wsaData;
            int iResult;
            iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (iResult != NO_ERROR) {
                log.write("Could not create socket.", 2);
            }
        #endif

        sock = socket(AF_INET, SOCK_DGRAM, 0);

        #ifdef __linux__
            fcntl(sock, F_SETFL, O_NONBLOCK);
        #endif

        #ifdef _WIN32
            u_long mode = 1;
            ioctlsocket(sock, FIONBIO, &mode);
        #endif

        if (sock < 0) {
            log.write("Could not create socket.", 2);
        }

        struct sockaddr_in addr;

        memset((char *)&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(out_port);
        if (::bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            log.write("Could not bind socket. (Port probably in use)", 2);
        }
        socket_created = true;
    }

    void check_connects() {
        list_lock.lock();
        if (connected_trackers < tracker_list.size()) {
            for (uint32_t i = 0; i < tracker_list.size(); i++) {
                if (!tracker_list.at(i).get_connected()) {
                    tracker_list.at(i).set_socket(sock);
                    tracker_list.at(i).connect();
                }
            }
        }
        list_lock.unlock();
    }

    void check_reannounce() {
        //Checking if i need to reannounce myself
        list_lock.lock();
        for (uint32_t i = 0; i < tracker_list.size(); i++) {
            if (tracker_list.at(i).get_announcing()) {
                duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - tracker_list.at(i).get_announce_time());
                if (diff.count() >= tracker_list.at(i).get_reannounce()) {
                    string tmp = "I am reannouncing myself to " + tracker_list.at(i).get_hostname();
                    log.write(tmp, 1);
                    add_action(i, tracker_list.at(i).get_announce_type());
                    tracker_list.at(i).set_announce_time(steady_clock::now());
                }
            }
        }
        list_lock.unlock();
    }

    void do_actions() {
        action_lock.lock();
        list_lock.lock();
        for (uint32_t i = 0; i < action_queue.size(); i++) {
            if (tracker_list.at(action_queue.at(i).id).get_waiting_response() == false) {
                if (action_queue.at(i).act == 1) {
                    tracker_list.at(action_queue.at(i).id).announce(action_queue.at(i).hash, action_queue.at(i).announce_type, client_port);
                    tracker_list.at(action_queue.at(i).id).set_action_id(i);
                    tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                }
                else if (action_queue.at(i).act == 2) {
                    tracker_list.at(action_queue.at(i).id).scrape(action_queue.at(i).hashes);
                    tracker_list.at(action_queue.at(i).id).set_action_id(i);
                    tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                }
                else {
                    log.write("Unknown action.", 2);
                    break;
                }
            }
            else {
                duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - tracker_list.at(action_queue.at(i).id).get_action_time());
                if (diff.count() >= 15) {
                    log.write("No response for 15 seconds, resending request.", 1);
                    tracker_list.at(action_queue.at(i).id).set_waiting_response(false);
                    tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                }
            }
        }
        list_lock.unlock();
        action_lock.unlock();
    }

    int get_id(string hostname) {
        list_lock.lock();
        int res = -1;
        for (uint i = 0; i < tracker_list.size(); i++) {
            if (tracker_list.at(i).get_hostname() == hostname) {
                res = i;
                break;
            }
        }
        list_lock.unlock();
        return res;
    }

    void receive_packet() {
        //check for incoming data
        struct sockaddr_in fromaddr;
        socklen_t addrlen = sizeof(fromaddr);

        int len = recvfrom(sock, buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &fromaddr, &addrlen);
        if (len > 0) {
            log.write("Received response from server.", 1);
            //find neccesary tracker object
            int id;
            list_lock.lock();
            for (uint32_t i = 0; i < tracker_list.size(); i++) {
                #ifdef __linux__
                    if (inet_ntoa(tracker_list.at(i).get_hostname_in()) == inet_ntoa(fromaddr.sin_addr) &&
                        tracker_list.at(i).get_port_in() == fromaddr.sin_port) {
                #endif
                #ifdef _WIN32
                    char *out = new char[INET_ADDRSTRLEN];
                    InetNtopA(AF_INET, &tracker_list.at(i).get_hostname_in(), out, INET_ADDRSTRLEN);
                    char *out2 = new char[INET_ADDRSTRLEN];
                    InetNtopA(AF_INET, &fromaddr.sin_addr, out2, INET_ADDRSTRLEN);
                    if (strcmp(out, out2) == 0 &&
                        tracker_list.at(i).get_port_in() == fromaddr.sin_port) {
                #endif
                    id = i;
                    break;
                }
            }
            

            if (buffer[0] == 3) {
                string err = "Tracker responded with error message. Tid: ";
                for (int i = 4; i < 8; i++) {
                    err += buffer[i];
                }
                err += ". Message: ";
                for (int i = 8; i < len; i++) {
                    err += to_string(buffer[i]);
                }
                log.write(err, 2);
            }
            else if (buffer[0] == 0 && !tracker_list.at(id).get_connected()) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_connect();
                string tmp = "Succesfully connected to ";
                tmp += tracker_list.at(id).get_hostname();
                log.write(tmp, 3);
                connected_trackers++;
            }
            else if (buffer[3] == 1) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_announce();
                remove_action(tracker_list.at(id).get_action_id());
            }
            else if (buffer[3] == 2) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_scrape();
                remove_action(tracker_list.at(id).get_action_id());
            }
            list_lock.unlock();
        }
    }

    bool remove_tracker(uint id) {
        if (id > tracker_list.size() - 1) {
            return false;
        }
        else {
            list_lock.lock();
            tracker_list.erase(tracker_list.begin() + id);
            list_lock.unlock();
            remove_tracker_actions(id);
            return true;
        }
    }

    void set_announce_hash(string hash) {
        announce_hash = hash;
    }

    void set_client_port(int port) {
        client_port = port;
    }
}