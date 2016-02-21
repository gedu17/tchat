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

#include "track_thread.h"
    /* Actions */

    void track_thread::add_action(int id, int announce_type, string hash) {
        this->action_lock.lock();
        tas temp;
        temp.act = 1;
        temp.announce_type = announce_type;
        temp.id = id;
        if (hash == "") {
            temp.hash = this->announce_hash;
        } else {
            temp.hash = hash;
        }
        this->action_queue.push_back(temp);
        this->action_lock.unlock();
    }

    void track_thread::add_action(int id, vector<string> hashes) {
        this->action_lock.lock();
        tas temp;
        temp.act = 2;
        temp.id = id;
        temp.hashes = hashes;
        this->action_queue.push_back(temp);
        this->action_lock.unlock();
    }

    void track_thread::remove_action(uint id) {
        this->action_lock.lock();
        if (this->action_queue.size() >= id) {
            this->action_queue.erase(this->action_queue.begin() + id);
        } else {
            string tmp = "Cannot delete id ";
            tmp += id;
            this->log.write(tmp, 1);
        }
        this->action_lock.unlock();
    }

    /* Workflow */

    void track_thread::run() {
        this->log = Log::getInstance();
        this->log.write("Starting tracker thread.", 1);
        
        this->socket_created = false;
        this->out_port = 0;
        this->cl_port = false;
        this->cl_hash = false;
        this->connected_trackers = 0;
        this->cycle_run = true;
        
        while (this->cycle_run) {
            if(this->cl_port && this->cl_hash) {
                if (!this->socket_created) {
                    //create_socket();
                    this->create_qsocket();
                }

                this->check_connects();

                this->receive_packet();

                this->do_actions();

                this->check_reannounce();

                
            }
            std::this_thread::sleep_for(200ms);
        }
    }

    void track_thread::stop() {
        this->cycle_run = false;
    }

    /* Helpers */

    bool track_thread::add_tracker(string hostname, int port) {
        bool is_in_list = false;
        for (uint i = 0; i < this->tracker_list.size(); i++) {
            if (this->tracker_list.at(i).get_hostname() == hostname) {
                is_in_list = true;
                break;
            }
        }
        if (!is_in_list) {
            Tracker *temp = new Tracker(hostname, port);
            this->tracker_list.push_back(*temp);
            return true;
        }
        return false;
    }

    void track_thread::check_reannounce() {
        //Checking if i need to reannounce myself
        for (uint i = 0; i < this->tracker_list.size(); i++) {
            if (this->tracker_list.at(i).get_announcing()) {
                duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - this->tracker_list.at(i).get_announce_time());
                if (diff.count() >= this->tracker_list.at(i).get_reannounce()) {
                    string tmp = "I am reannouncing myself to ";
                    tmp += this->tracker_list.at(i).get_hostname();
                    this->log.write(tmp, 1);
                    add_action(i, this->tracker_list.at(i).get_announce_type());
                    this->tracker_list.at(i).set_announce_time(steady_clock::now());
                }
            }
        }
    }

    void track_thread::create_socket() {
        this->sock = socket(AF_INET, SOCK_DGRAM, 0);
        fcntl(this->sock, F_SETFL, O_NONBLOCK);

        if (this->sock < 0) {
            this->log.write("Could not create socket.", 2);
        }

        struct sockaddr_in addr;

        memset((char *) &addr, 0, sizeof (addr));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(out_port);

        if (bind(this->sock, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
            this->log.write("Could not bind socket. (Port probably in use)", 2);
        } else {
            this->socket_created = true;
        }
    }
    
    void track_thread::create_qsocket() {
        this->qsock = new QUdpSocket();
        this->qsock->bind();
        this->out_port = this->qsock->localPort();
        this->socket_created = true;
    }

    void track_thread::do_actions() {
        this->action_lock.lock();
        for (uint i = 0; i < this->action_queue.size(); i++) {
            if (this->tracker_list.at(this->action_queue.at(i).id).get_waiting_response() == false && this->tracker_list.at(this->action_queue.at(i).id).is_connected()) {
                if (this->action_queue.at(i).act == 1) {
                    this->tracker_list.at(this->action_queue.at(i).id).announce(this->action_queue.at(i).hash, this->action_queue.at(i).announce_type, this->client_port);
                    this->tracker_list.at(this->action_queue.at(i).id).set_action_id(i);
                    this->tracker_list.at(this->action_queue.at(i).id).set_action_time(steady_clock::now());
                } else if (this->action_queue.at(i).act == 2) {
                    this->tracker_list.at(this->action_queue.at(i).id).scrape(this->action_queue.at(i).hashes);
                    this->tracker_list.at(this->action_queue.at(i).id).set_action_id(i);
                    this->tracker_list.at(this->action_queue.at(i).id).set_action_time(steady_clock::now());
                } else {
                    this->log.write("Unknown action.");
                    break;
                }
            } else {
                duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - this->tracker_list.at(this->action_queue.at(i).id).get_action_time());
                if (diff.count() >= 15) {
                    this->log.write("No response for 15 seconds, resending request.", 1);
                    this->tracker_list.at(action_queue.at(i).id).set_waiting_response(false);
                    this->tracker_list.at(action_queue.at(i).id).set_action_time(steady_clock::now());
                }
            }
        }
        this->action_lock.unlock();
    }

    void track_thread::check_connects() {
        if (this->connected_trackers < this->tracker_list.size()) {
            this->log.write("Connecting to trackers.", 1);
            for (uint i = 0; i < this->tracker_list.size(); i++) {
                if (!this->tracker_list.at(i).get_connected()) {
                    
                    //tracker_list.at(i).set_socket(sock);
                    this->tracker_list.at(i).set_socket(qsock);
                    
                    this->tracker_list.at(i).connect();
                    this->connected_trackers++;
                }
            }
        }
    }

    void track_thread::set_announce_hash(string hash) {
        this->announce_hash = hash;
        this->cl_hash = true;
    }

    void track_thread::set_client_port(int port) {
        this->client_port = port;
        this->cl_port = true;
    }

    void track_thread::receive_packet() {
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
        
        while(this->qsock->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(qsock->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            qint64 len = this->qsock->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            string tmp = "New data ( ";
            tmp += to_string(datagram.size());
            tmp += " ) from ";
            tmp += sender.toString().toStdString();
            this->log.write(tmp, 1);

            int id = -1;
            for (uint i = 0; i < this->tracker_list.size(); i++) {
                //if (inet_ntoa(tracker_list.at(i).get_hostname_in()) == inet_ntoa(fromaddr.sin_addr) &&
                //        tracker_list.at(i).get_port_in() == fromaddr.sin_port) {
                if(sender.toIPv4Address() == this->tracker_list.at(i).get_hostname_in() && this->tracker_list.at(i).get_port_in() == senderPort){
                    id = i;
                    break;
                }
            }
            
            if(id == -1) {
                throw "Unknown error!";
            }
            
            //rewrite to our buffer
            for(int i = 0; i < datagram.size(); i++) {
                this->buffer[i] = datagram.at(i);
            }
            
            //buffer_size = datagram.size();            
            /*us tmpus;
            tmpus.adr = sender;
            tmpus.port = senderPort;
            this->handle_data(tmpus);
            this->clear_buffer();*/
            
            if (this->buffer[0] == 3) {
                string err = "Tracker responded with error message. Tid: ";
                for (uint i = 4; i < 8; i++) {
                    err += buffer[i];
                }
                err += ". Message: ";
                for (uint i = 8; i < len; i++) {
                    err += to_string(this->buffer[i]);
                }
                this->log.write(err, 2);
            } else if (this->buffer[0] == 0 && !this->tracker_list.at(id).get_connected()) {
                this->tracker_list.at(id).set_buffer(this->buffer, len);
                this->tracker_list.at(id).finish_connect();
                string tmp = "Succesfully connected to ";
                tmp += this->tracker_list.at(id).get_hostname();
                log.write(tmp, 1);
            } else if (buffer[3] == 1) {
                tracker_list.at(id).set_buffer(this->buffer, len);
                tracker_list.at(id).finish_announce();
                remove_action(this->tracker_list.at(id).get_action_id());
            } else if (this->buffer[3] == 2) {
                this->tracker_list.at(id).set_buffer(this->buffer, len);
                this->tracker_list.at(id).finish_scrape();
                this->remove_action(this->tracker_list.at(id).get_action_id());
            }
        }        
    }
