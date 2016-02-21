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

#include "tracker.h"
#include "utils.h"
#include "includes.h"
#include "structs.h"
Tracker::Tracker(string hostname, int port) {
    this->hostname = hostname;
    this->port = port;
    this->connected = false;
    this->sock = 0;
    this->socket_port = port;
    this->action = 0;
    this->hash = "";
    this->waiting_response = false;
    this->action_time = steady_clock::now();
    this->log = Log::getInstance();
    this->announcing = false;
}

/* Setters */
void Tracker::set_action(char action) {
    this->action = action;
}

void Tracker::set_action_id(uint id) {
    this->action_id = id; 
}

void Tracker::set_action_time(steady_clock::time_point time) {
    this->action_time = time;
}

void Tracker::set_announce_port(uint16_t port) {
    this->announce_port = port;
}

void Tracker::set_announce_time(steady_clock::time_point time) {
    this->announce_time = time;
}

void Tracker::set_buffer(unsigned char *buffer, int size) {
    this->buffer = buffer;
    this->buffer_size = size;
}

void Tracker::set_conn_id() {
    this->connection_id.clear();
    this->connection_id.push_back(this->buffer[8]);
    this->connection_id.push_back(this->buffer[9]);
    this->connection_id.push_back(this->buffer[10]);
    this->connection_id.push_back(this->buffer[11]);
    this->connection_id.push_back(this->buffer[12]);
    this->connection_id.push_back(this->buffer[13]);
    this->connection_id.push_back(this->buffer[14]);
    this->connection_id.push_back(this->buffer[15]);
}

void Tracker::set_peer_id() {
    this->peer_id.push_back(0x54);//T
    this->peer_id.push_back(0x50);//O
    this->peer_id.push_back(0x52);//R
    this->peer_id.push_back(0x43);//C
    this->peer_id.push_back(0x48);//H
    this->peer_id.push_back(0x41);//A
    this->peer_id.push_back(0x54);//T
    this->peer_id.push_back(0x2d);//-
    this->peer_id.push_back(0x31);//1
    this->peer_id.push_back(0x2d);//-

    linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647> eng;
    eng();
    
    int base;
    int number;
    for(int j = 0; j < 2; j++){
        number = eng();
        base = 10000000;
        for(int i = 0; i < 5; i++){
            div_t res;
            res = div(number, base);
            this->peer_id.push_back(res.quot);
            number = number - (res.quot * base);
            base = base / 10;
        }
    }
}

void Tracker::set_socket(int sock) {
    this->sock = sock;
}

void Tracker::set_socket(QUdpSocket *sock) {
    this->qsock = sock;
}

void Tracker::set_socket_port(int port) {
    this->socket_port = port;
}

//Sets connection id to 0x41727101980
void Tracker::set_std_conn_id() {
    this->connection_id.push_back(0x00);
    this->connection_id.push_back(0x00);
    this->connection_id.push_back(0x04);
    this->connection_id.push_back(0x17);
    this->connection_id.push_back(0x27);
    this->connection_id.push_back(0x10);
    this->connection_id.push_back(0x19);
    this->connection_id.push_back(0x80);
}

void Tracker::set_waiting_response(bool value) {
    this->waiting_response = value;
}


/* Getters */

uint Tracker::get_action_id() {
    return this->action_id;
}

steady_clock::time_point Tracker::get_action_time() {
    return this->action_time;
}

steady_clock::time_point Tracker::get_announce_time() {
    return this->announce_time;
}

int Tracker::get_announce_type() {
    return this->announce_type;
}

bool Tracker::get_announcing() {
    return this->announcing;
}

bool Tracker::get_connected() {
    return this->connected;
}

string Tracker::get_hostname() {
    return this->hostname;
}

quint32 Tracker::get_hostname_in() {
    //sockaddr_in *tmp = (struct sockaddr_in *)this->server_sock->ai_addr;
    //return tmp->sin_addr;
    //return 
    //return this->hostname;
    return this->server_sock.toIPv4Address();
}

quint16 Tracker::get_port_in() {
    //sockaddr_in *tmp = (struct sockaddr_in *)this->server_sock->ai_addr;
    //return tmp->sin_port;
    return this->socket_port;
}

int Tracker::get_reannounce() {
    return this->reannounce;
}

bool Tracker::get_waiting_response() {
    return this->waiting_response;
}

/* Methods */

/* Announce */

//Types = 0 - None (searching for others); 1 - Completed (source); 2 - Started (not used); 3 -  Stopped (not used)
void Tracker::announce(string hash, int type, uint16_t port) {
    if(this->connected) {
        string tmp = "Announcing myself to ";
        tmp += this->hostname;
        tmp += " with hash ";
        tmp += hash;
        this->log.write(tmp, 1);
        this->set_announce_port(port);
        this->hash.clear();
        int j = 0;
        cout << "hash = " << hash << " ; " << hash.size() << endl;
        for(int i = 0; i< 20; i++) {
            this->hash += this->string_to_hex(hash.at(j), hash.at(j+1));
            j += 2;
        }
        cout << "veikiam !" << endl;
        this->ascii_hash = hash;
        this->action = 1;
        this->announce_type = type;
        
        this->generate_random_key();
        cout << "veikiam 2" << endl;
        this->create_packet(1);
        cout << "veikiam 3" << endl;
        tqs str;
        str.action = 1;
        str.action_type = type;
        str.hash = hash;
        string tid;
        cout << "veikiam 4" << endl;
        for(uint i = 0; i < this->transaction_id.size(); i++) {
            tid += this->transaction_id.at(i);
        }
        str.transaction_id = tid;
        utils::insert_tq(str);    
        cout << "veikiam 5" << endl;
        this->dump();
        this->send_packet();
        this->waiting_response = true;
    } else {
        this->log.write("Need to connect to server before announcing", 2);
    }
}

void Tracker::finish_announce() {
    if(this->check_transaction_id()) {
        int leechers = (this->buffer[12] << 24) + (this->buffer[13] << 16) + (this->buffer[14] <<8) + this->buffer[15];
        int seeders = (this->buffer[16] << 24) + (this->buffer[17] << 16) + (this->buffer[18] <<8) + this->buffer[19];
        vector<ads> as;
        int c = 20;
        
        for(int w = 0; w < (this->buffer_size-19)/6; w++) {
            ads tmp;
            tmp.ip = to_string(this->buffer[w+c]);
            tmp.ip += ".";
            tmp.ip += to_string(this->buffer[w+c+1]);
            tmp.ip += ".";
            tmp.ip += to_string(this->buffer[w+c+2]);
            tmp.ip += ".";
            tmp.ip += to_string(this->buffer[w+c+3]);
            tmp.port = (this->buffer[w+c+4] << 8) + this->buffer[w+c+5];
            
            as.push_back(tmp);
            c += 5;
        }

        string tid;
        for(uint i = 0; i < this->transaction_id.size(); i++) {
            tid += this->transaction_id.at(i);
        }

        if(!utils::update_tq(tid, as, seeders, leechers)) {
            string tmp = "Failed to update tracker queue for tid: ";
            tmp += tid;
            this->log.write(tmp, 2);
        }
        
        if(this->announce_type == 1) {
            this->reannounce = (this->buffer[8] << 24) + (this->buffer[9] << 16) + (this->buffer[10] <<8) + this->buffer[11];
            string rean = "Reannouncing myself in ";
            rean += to_string(this->reannounce);
            rean += " seconds to ";
            rean += this->hostname;
            this->log.write(rean, 1);
            this->set_announce_time(steady_clock::now());
            this->announcing = true;
        } 
    } else {
        this->log.write("Transaction ids do not match.", 2);
    }
    this->waiting_response = false;
}

/* Connect */

void Tracker::connect() {
    this->set_std_conn_id();
    if (!this->connected) {
        string tmpmsg = "Connecting to tracker ";
        tmpmsg += this->hostname;
        this->log.write(tmpmsg, 1);
        this->create_packet(0);
        
        /*struct sockaddr_in *server;
        struct addrinfo *info;
        addrinfo hints;
        int err;

        memset (&hints, 0, sizeof (hints));
        hints.ai_socktype = SOCK_DGRAM;

        hints.ai_flags = AI_CANONNAME;
        hints.ai_family = PF_UNSPEC;
        
        err = getaddrinfo(this->hostname.c_str(), NULL, &hints, &info);
        if(err == 0) {
            memset((char *) &server, 0, sizeof (server));
            server = (struct sockaddr_in *)info->ai_addr;
            server->sin_port = htons(this->port);
            this->server_sock = info;
        } else {
            string tmp = "Could not find the hostname ";
            tmp += this->hostname;
            tmp += "\n";
            tmp += gai_strerror(err);
            this->log.write(tmp, 2);
            return;
        }*/
        QHostInfo info = QHostInfo::fromName(QString(this->hostname.c_str()));
        QList<QHostAddress> list = info.addresses();
        if(info.addresses().isEmpty()) {
            string tmp = "Could not find the hostname ";
            tmp += this->hostname;
            tmp += "\n";
            this->log.write(tmp, 2);
            return;
        } else {            
            this->server_sock = info.addresses().at(0);
            this->send_packet();
            this->waiting_response = true;        
        }
        
    } else {
        this->log.write("Already connected to tracker.");
    }
}

void Tracker::finish_connect() {
    if(this->check_transaction_id()) {
        this->set_conn_id();
        this->log.write("New connection id set.", 1);
        this->connected = true;
        this->set_peer_id();
    } else {
        this->log.write("Transaction ids do not match.", 2);
    } 
    this->waiting_response = false;
}

/* Scrape */

void Tracker::scrape(vector<string> hashes) {
    if(this->connected) {
        string tmp = "Scraping hashes ";
        for(uint i = 0; i < hashes.size(); i++) {
            tmp += hashes.at(i);
            if(i != hashes.size()-1) {
                tmp += ", ";
            }
        }
        tmp += " on server ";
        tmp += this->hostname;
        this->log.write(tmp, 1);
        this->hash.clear();
        
        for(uint i = 0; i < hashes.size(); i++) {
            transform(hashes.at(i).begin(), hashes.at(i).end(), hashes.at(i).begin(), ::toupper);
        }
        
        this->hashes = hashes;
        this->action = 2;
        this->create_packet(2);
        
        tqs str;
        str.action = 2;
        string tid;
        for(uint i = 0; i < this->transaction_id.size(); i++) {
            tid += this->transaction_id.at(i);
        }
        str.transaction_id = tid;  
        utils::insert_tq(str);
        this->send_packet();
        this->waiting_response = true;
    } else {
        this->log.write("Need to connect to server before scraping", 2);
    }
}

void Tracker::finish_scrape() {
    if(this->check_transaction_id()) {
        int bufpos = 8;
        vector<ss> sss;
        ss tmp;
        for(uint q = 0; q < this->hashes.size(); q++) {
            tmp.complete = (this->buffer[bufpos] << 24) + (this->buffer[bufpos+1] << 16) + (this->buffer[bufpos+2] << 8) + this->buffer[bufpos+3];
            bufpos += 4;

            tmp.downloaded = (this->buffer[bufpos] << 24) + (this->buffer[bufpos+1] << 16) + (this->buffer[bufpos+2] << 8) + this->buffer[bufpos+3];
            bufpos += 4;

            tmp.incomplete = (this->buffer[bufpos] << 24) + (this->buffer[bufpos+1] << 16) + (this->buffer[bufpos+2] << 8) + this->buffer[bufpos+3];
            bufpos += 4;
            tmp.fingerprint = this->hashes.at(q);                

            sss.push_back(tmp);                
        }

        string tid;
        for(uint i = 0; i < this->transaction_id.size(); i++) {
            tid += this->transaction_id.at(i);
        }

        if(!utils::update_tq(tid, sss)) {
           string tmp = "Failed to update scrape queue for tid: ";
           tmp += tid;
           this->log.write(tmp, 2);
        }
    } else {
        this->log.write("Transaction ids do not match.", 2);
    }
    this->waiting_response = false;    
}

/* Packet handling */
//Based on <http://www.rasterbar.com/products/libtorrent/udp_tracker_protocol.html>
void Tracker::create_packet(int type) {
    this->packet.clear();
    
    //Connection id
    for (uint i = 0; i< this->connection_id.size(); i++) {
        this->packet += this->connection_id.at(i);
    }
    
    //Action
    //Format: 000 ActionId = (0 - connect; 1 - announce; 2 - scrape)
    this->packet.insert(this->packet.end(), 0);
    this->packet.insert(this->packet.end(), 0);
    this->packet.insert(this->packet.end(), 0);
    this->packet += this->action;

    //Transaction id
    this->generate_transaction_id();
    for (uint i = 0; i< this->transaction_id.size(); i++) {
        this->packet += this->transaction_id.at(i);
    }
    
    if (type == 1) {
        if(this->hash.size() == 0) {
            this->log.write("Announcing hash is empty.", 2);
        }
        //info hash
        for(uint j = 0; j < this->hash.size(); j++) {
            this->packet += this->hash.at(j);
        }
        //peer id
        for(uint j = 0; j < this->peer_id.size(); j++) {
            this->packet += this->peer_id.at(j);
        }
        //Downloaded, Left, Uploaded (24 bytes) + Event (3bytes) 

        //I am searching for announcer (= leecher)
        if(this->announce_type == 0) {
            for(uint j = 0; j <= 14; j++) {
                this->packet.insert(this->packet.end(), 0);
            }
            
            this->packet += 64;
            
            for(uint j = 0; j <= 10; j++) {
                this->packet.insert(this->packet.end(), 0);
            }
        } else {
        //I am the announcer (= seeder)
            for(uint j = 0; j <= 26; j++) {
                this->packet.insert(this->packet.end(), 0);
            }
        }
        
        //event significant byte
        this->packet += this->announce_type;
        
        //my ip address
        for(uint j = 0; j <= 3; j++) {
            this->packet.insert(this->packet.end(), 0);
        }
        
        //key
        for(uint j = 0; j < this->random_key.size(); j++) {
            this->packet += this->random_key.at(j);
        }
        
        this->packet += 128;
        this->packet.insert(this->packet.end(), 0);
        this->packet.insert(this->packet.end(), 0);
        this->packet += 1;
        
        //port
        this->packet += (this->announce_port >> 8);
        this->packet += (this->announce_port & 0x00FF);
        
        //extensions
        this->packet.insert(this->packet.end(), 0);
        this->packet.insert(this->packet.end(), 0);
        
    } else if (type == 2) {
        if(this->hashes.size() == 0) {
            this->log.write("Scraping hash is empty.", 2);
        }
        
        for(uint j = 0; j < this->hashes.size(); j++) {
            int nr = 0;
            for(uint k = 0; k < (this->hashes.at(j).size()/2); k++) {
                this->packet += this->string_to_hex(this->hashes.at(j).at(nr), this->hashes.at(j).at(nr+1));
                nr += 2;
            }
        }
    } else if(type != 0) {
        this->log.write("Unknown packet type", 2);
    }
}

int Tracker::send_packet() {
   /* int len = sendto(this->sock, this->packet.c_str(), this->packet.size(), 0, this->server_sock->ai_addr, this->server_sock->ai_addrlen);
    if (len < 0) {
        this->log.write("Failed to send UDP packet to server", 2);
        string tmp = to_string(errno);
        tmp += " \n";
        tmp += strerror(errno);
        this->log.write(tmp, 2);
    }
    return len;*/
    qint64 len = this->qsock->writeDatagram(this->packet.c_str(), this->packet.size(), this->server_sock, this->socket_port);
    if (len < 0) {
        this->log.write("Failed to send UDP packet to server", 2);
        string tmp = to_string(errno);
        tmp += " \n";
        tmp += strerror(errno);
        this->log.write(tmp, 2);
    }
    return (int)len;
    
}

/* Helper methods */

void Tracker::dump() {
    if(DEBUG) {
        string temp = "      Hostname: ";
        temp += this->hostname;
        temp += "\n";
        temp += "          Port: ";
        temp += to_string(this->port);
        temp += "\n";
        temp += "     Connected: ";
        temp += this->connected ? "True" : "False";
        temp += "\n";
        temp += " Connection Id: 0x";

        stringstream temp2;
        for (uint i = 0; i< this->connection_id.size(); i++) {
            if ((int) this->connection_id.at(i) < 10) {
                temp2 << std::dec << 0;
            }
            temp2 << std::hex << (int) this->connection_id.at(i);
        }

        temp += temp2.str();
        temp += "\n";
        temp += "Transaction Id: 0x";

        temp2.str("");

        for (uint i = 0; i< this->transaction_id.size(); i++) {
            if ((int) this->transaction_id.at(i) < 10) {
                temp2 << std::dec << 0;
            }
            temp2 << std::hex << (int) this->transaction_id.at(i);
        }

        temp += temp2.str();
        temp += "\n";

        temp += "        Packet: 0x";
        temp2.str("");
        for (uint i = 0; i < this->packet.size(); i++) {
            if (this->packet[i] < 10) {
                temp2 << std::dec << 0;
            }
            temp2 << std::hex << (int) this->packet[i];
        }
        temp += temp2.str();
        temp += "\n";

        temp += "       Peer id: 0x";
        temp2.str("");
        for(uint i = 0; i < this->peer_id.size(); i++) {
            if(this->peer_id.at(i) < 10) {
                temp2 << std::dec << 0;
            }
            temp2 << std::hex << (int) this->peer_id.at(i);
        }
        temp += temp2.str();
        temp += "\n";


        this->log.write(temp);
    }
}

bool Tracker::check_transaction_id(){
    if (this->buffer[4] == this->transaction_id.at(0) && this->buffer[5] == this->transaction_id.at(1)
        && this->buffer[6] == this->transaction_id.at(2) && this->buffer[7] == this->transaction_id.at(3)) {
        return true;
    }
    return false;
}

void Tracker::clear_buffer() {
    for(int i = 0; i < BUFFER_LENGTH; i++) {
        this->buffer[i] = 0;
    }
}

void Tracker::generate_random_key() {
    this->random_key.clear();
    linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647> eng;
    eng();
    
    int base;
    int number;
    number = eng();
    base = 10000000;
    for(int i = 0; i < 4; i++){
        div_t res;
        res = div(number, base);
        this->random_key.push_back(res.quot);
        number = number - (res.quot * base);
        base = base / 10;
    }
}

void Tracker::generate_transaction_id() {
    default_random_engine e((unsigned int) time(0));
    int i = e();
    this->transaction_id.clear();
    div_t res;
    int base = 10000000;
    for (int j = 0; j < 4; j++) {
        res = div(i, base);
        this->transaction_id.push_back(res.quot);
        i = i - (res.quot * base);
        base = base / 10;
    }
}

int Tracker::string_to_hex(char first, char second){
    int firstval;
    int secondval;
    first = first - 48;
    second = second - 48;
    switch(first){
        case 17:
            firstval = 10;
            break;
        case 18:
            firstval = 11;
            break;
        case 19:
            firstval = 12;
            break;
        case 20:
            firstval = 13;
            break;
        case 21:
            firstval = 14;
            break;
        case 22:
            firstval = 15;
            break;
        default:
            firstval = first;
            break;
    }
        
    switch(second){
        case 17:
            secondval = 10;
            break;
        case 18:
            secondval = 11;
            break;
        case 19:
            secondval = 12;
            break;
        case 20:
            secondval = 13;
            break;
        case 21:
            secondval = 14;
            break;
        case 22:
            secondval = 15;
            break;
        default:
            secondval = second;
            break;
    }
    
    return (16*firstval + secondval);
}

bool Tracker::is_connected() {
    return this->connected;
}

Tracker::~Tracker() {
    //freeaddrinfo(this->server_sock);
    close(this->sock);
    
}

