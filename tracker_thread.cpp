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

#include "tracker_thread.h"
#include "tracker.h"
#include "utils.h"
#include "structs.h"

namespace tracker_thread {

    /* Mutexes */

    mutex action_lock;

    /* Vectors */

    vector<tas> action_queue;
    vector<Tracker> tracker_list;

    /* Log */
    Log log = Log::getInstance();

    /* Sockets */
    bool socket_created = false;
    int out_port = 0;
    int sock;
    QUdpSocket *qsock;
    int client_port;

    bool cl_port = false;
    bool cl_hash = false;

    /* Various */
    uint connected_trackers = 0;

    string announce_hash;

    unsigned char buffer[BUFFER_LENGTH];

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
        } else {
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

    void remove_action(uint id) {
        action_lock.lock();
        if (action_queue.size() >= id) {
            action_queue.erase(action_queue.begin() + id);
        } else {
            string tmp = "Cannot delete id ";
            tmp += id;
            log.write(tmp, 1);
        }
        action_lock.unlock();
    }

    /* Workflow */

    void run() {
        log.write("Starting tracker thread.", 1);
        while (cycle_run) {
            if(cl_port && cl_hash) {
                if (!socket_created) {
                    //create_socket();
                    create_qsocket();
                }

                check_connects();

                receive_packet();

                do_actions();

                check_reannounce();

                
            }
            std::this_thread::sleep_for(200ms);
        }
    }

    void stop() {
        cycle_run = false;
    }

    /* Helpers */

    bool add_tracker(string hostname, int port) {
        bool is_in_list = false;
        for (uint i = 0; i < tracker_list.size(); i++) {
            if (tracker_list.at(i).get_hostname() == hostname) {
                is_in_list = true;
                break;
            }
        }
        if (!is_in_list) {
            Tracker *temp = new Tracker(hostname, port);
            tracker_list.push_back(*temp);
            return true;
        }
        return false;
    }

    void check_reannounce() {
        //Checking if i need to reannounce myself
        for (uint i = 0; i < tracker_list.size(); i++) {
            if (tracker_list.at(i).get_announcing()) {
                duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - tracker_list.at(i).get_announce_time());
                if (diff.count() >= tracker_list.at(i).get_reannounce()) {
                    string tmp = "I am reannouncing myself to ";
                    tmp += tracker_list.at(i).get_hostname();
                    log.write(tmp, 1);
                    add_action(i, tracker_list.at(i).get_announce_type());
                    tracker_list.at(i).set_announce_time(steady_clock::now());
                }
            }
        }
    }

    void create_socket() {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        fcntl(sock, F_SETFL, O_NONBLOCK);

        if (sock < 0) {
            log.write("Could not create socket.", 2);
        }

        struct sockaddr_in addr;

        memset((char *) &addr, 0, sizeof (addr));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(out_port);

        if (bind(sock, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
            log.write("Could not bind socket. (Port probably in use)", 2);
        } else {
            socket_created = true;
        }
    }
    
    void create_qsocket() {
        qsock = new QUdpSocket();
        qsock->bind();
        out_port = qsock->localPort();
        socket_created = true;
    }

    void do_actions() {
        action_lock.lock();
        for (uint i = 0; i < action_queue.size(); i++) {
            if (tracker_list.at(action_queue.at(i).id).get_waiting_response() == false && tracker_list.at(action_queue.at(i).id).is_connected()) {
                if (action_queue.at(i).act == 1) {
                    tracker_list.at(action_queue.at(i).id).announce(action_queue.at(i).hash, action_queue.at(i).announce_type, client_port);
                    tracker_list.at(action_queue.at(i).id).set_action_id(i);
                    tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                } else if (action_queue.at(i).act == 2) {
                    tracker_list.at(action_queue.at(i).id).scrape(action_queue.at(i).hashes);
                    tracker_list.at(action_queue.at(i).id).set_action_id(i);
                    tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                } else {
                    log.write("Unknown action.");
                    break;
                }
            } else {
                duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - tracker_list.at(action_queue.at(i).id).get_action_time());
                if (diff.count() >= 15) {
                    log.write("No response for 15 seconds, resending request.", 1);
                    tracker_list.at(action_queue.at(i).id).set_waiting_response(false);
                    tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                }
            }
        }
        action_lock.unlock();
    }

    void check_connects() {
        if (connected_trackers < tracker_list.size()) {
            log.write("Connecting to trackers.", 1);
            for (uint i = 0; i < tracker_list.size(); i++) {
                if (!tracker_list.at(i).get_connected()) {
                    
                    //tracker_list.at(i).set_socket(sock);
                    tracker_list.at(i).set_socket(qsock);
                    
                    tracker_list.at(i).connect();
                    connected_trackers++;
                }
            }
        }
    }

    void set_announce_hash(string hash) {
        announce_hash = hash;
        cl_hash = true;
    }

    void set_client_port(int port) {
        client_port = port;
        cl_port = true;
    }

    void receive_packet() {
        //check for incoming data
        /*struct sockaddr_in fromaddr;
        socklen_t addrlen = sizeof (fromaddr);

        uint len = recvfrom(sock, buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &fromaddr, &addrlen);
        if (len > 0) {
            log.write("Received response from server.", 1);
            //find neccesary tracker object
            int id;
            for (uint i = 0; i < tracker_list.size(); i++) {
                if (inet_ntoa(tracker_list.at(i).get_hostname_in()) == inet_ntoa(fromaddr.sin_addr) &&
                        tracker_list.at(i).get_port_in() == fromaddr.sin_port) {
                    id = i;
                    break;
                }
            }

            if (buffer[0] == 3) {
                string err = "Tracker responded with error message. Tid: ";
                for (uint i = 4; i < 8; i++) {
                    err += buffer[i];
                }
                err += ". Message: ";
                for (uint i = 8; i < len; i++) {
                    err += to_string(buffer[i]);
                }
                log.write(err, 2);
            } else if (buffer[0] == 0 && !tracker_list.at(id).get_connected()) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_connect();
                string tmp = "Succesfully connected to ";
                tmp += tracker_list.at(id).get_hostname();
                log.write(tmp, 1);
            } else if (buffer[3] == 1) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_announce();
                remove_action(tracker_list.at(id).get_action_id());
            } else if (buffer[3] == 2) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_scrape();
                remove_action(tracker_list.at(id).get_action_id());
            }
        }*/
        
        while(qsock->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(qsock->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            qint64 len = qsock->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            string tmp = "New data ( ";
            tmp += to_string(datagram.size());
            tmp += " ) from ";
            tmp += sender.toString().toStdString();
            log.write(tmp, 1);

            int id;
            for (uint i = 0; i < tracker_list.size(); i++) {
                //if (inet_ntoa(tracker_list.at(i).get_hostname_in()) == inet_ntoa(fromaddr.sin_addr) &&
                //        tracker_list.at(i).get_port_in() == fromaddr.sin_port) {
                if(sender.toIPv4Address() == tracker_list.at(i).get_hostname_in() && tracker_list.at(i).get_port_in() == senderPort){
                    id = i;
                    break;
                }
            }
            
            //rewrite to our buffer
            for(int i = 0; i < datagram.size(); i++) {
                buffer[i] = datagram.at(i);
            }
            //buffer_size = datagram.size();            
            /*us tmpus;
            tmpus.adr = sender;
            tmpus.port = senderPort;
            this->handle_data(tmpus);
            this->clear_buffer();*/
            
            if (buffer[0] == 3) {
                string err = "Tracker responded with error message. Tid: ";
                for (uint i = 4; i < 8; i++) {
                    err += buffer[i];
                }
                err += ". Message: ";
                for (uint i = 8; i < len; i++) {
                    err += to_string(buffer[i]);
                }
                log.write(err, 2);
            } else if (buffer[0] == 0 && !tracker_list.at(id).get_connected()) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_connect();
                string tmp = "Succesfully connected to ";
                tmp += tracker_list.at(id).get_hostname();
                log.write(tmp, 1);
            } else if (buffer[3] == 1) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_announce();
                remove_action(tracker_list.at(id).get_action_id());
            } else if (buffer[3] == 2) {
                tracker_list.at(id).set_buffer(buffer, len);
                tracker_list.at(id).finish_scrape();
                remove_action(tracker_list.at(id).get_action_id());
            }
            
        }        
    }
}