#include "stdafx.h"
#include "Client.h"
#include "Utils.h"

//static item
vector<function<void(string)>> Client::DEFAULT_FNC_VECTOR = vector<function<void(string)>>();

Client::Client(string fingerprint, gpgme_ctx_t gpgctx, string homedir, string keyserver) {
    this->log = Log::get_instance();
#ifdef __linux__
    this->buffer = new unsigned char[BUFFER_LENGTH];
#endif
#ifdef _WIN32
    this->buffer = new char[BUFFER_LENGTH];
#endif
    this->max_missing_packets = 10;
    this->conn_hash_length = 5;
    this->backup_length = 10;
    this->socket_created = false;
    this->homedir = homedir;
    this->keyserver = keyserver;
    this->fingerprint = fingerprint;
    this->gpgctx = gpgctx;
}

Client::~Client() {
    if (this->socket_created) {
#ifdef __linux__
        close(this->sock);
#endif
#ifdef _WIN32
        _close(this->sock);
#endif
    }
}

/* Public methods used initialising object and client thread */

bool Client::create_socket() {
#ifdef _WIN32
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        this->log.write("Could not create socket.", 2);
    }
#endif

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef __linux__
    fcntl(this->sock, F_SETFL, O_NONBLOCK);
#endif

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(this->sock, FIONBIO, &mode);
#endif

    if (this->sock < 0) {
        this->log.write("Could not create socket.", 2);
        return false;
    }

    struct sockaddr_in addr;

    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->out_port);

    if (::bind(this->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        this->log.write("Could not bind socket. (Port probably in use)", 2);
        return false;
    }

    socklen_t addrlen = sizeof(addr);
    int err = getsockname(this->sock, (struct sockaddr *) &addr, &addrlen);
    if (err == 0 && addr.sin_family == AF_INET && addrlen == sizeof(addr)) {
        this->out_port = ntohs(addr.sin_port);
        this->socket_created = true;
        return true;
    }
    else {
        string errmsg;
#ifdef __linux__
        errmsg = strerror(errno);

#endif
#ifdef _WIN32
        int err = WSAGetLastError();
        errmsg = "WSA code: " + to_string(err);
#endif
        this->log.write("Error getting out port. " + errmsg, 2);
        return false;
    }
}

//Returns which port was assigned by os
int Client::get_out_port() {
    return this->out_port;
}

/* Public methods used by client thread. */

void Client::check_heartbeats() {
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).heartbeat_rate != 0) {
            //check for last heartbeat received
            duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - this->conn_queue.at(i).last_heartbeat_received);
            if (diff.count() >= 5 * this->conn_queue.at(i).heartbeat_rate) {
                this->log.write("Missed hearbeat 5 times, disconnecting.", 3);
                this->remove_peer(this->conn_queue.at(i).conn_hash);
            }
            else {
                duration<double> diff2 = duration_cast<duration<double>>(steady_clock::now() - this->conn_queue.at(i).last_heartbeat);
                if (diff2.count() >= this->conn_queue.at(i).heartbeat_rate) {
                    this->log.write("Time to heartbeat!", 1);
                    vector<function<void(string)>> fnc;
                    fnc.push_back(bind(&Client::add_to_sent_packets, this, this->conn_queue.at(i).conn_hash));
                    fnc.push_back(bind(&Client::callback_heartbeat, this, this->conn_queue.at(i).conn_hash));
                    this->queue_data(this->conn_queue.at(i).conn_hash, 0x08, "", fnc);
                }
            }
        }
    }
}

void Client::check_missing_packets() {
    for (uint32_t i = 0; i < this->missing_packet_queue.size(); i++) {
        duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - this->missing_packet_queue.at(i).added);

        if (diff.count() >= 2 * this->get_heartbeat_rate(this->missing_packet_queue.at(i).id)) {
            string tmp = "Requesting to resend missing packet " + to_string(this->missing_packet_queue.at(i).packet_id)
                + " from " + this->missing_packet_queue.at(i).id;
            this->log.write(tmp, 3);
            string data = to_string(this->missing_packet_queue.at(i).packet_id);
            this->queue_data(this->missing_packet_queue.at(i).id, 0x10, data);
            this->missing_packet_queue.at(i).added = steady_clock::now();
        }
    }
}

void Client::check_tq() {
    vector<tqs> tq = utils::get_tq(1);
    if (tq.size() > 0) {
        for (uint32_t i = 0; i < tq.size(); i++) {
            string tmp = to_string(i) + ". " + tq.at(i).hash;
            this->log.write(tmp, 1);
            if (tq.at(i).action_type == 0) {
                for (uint32_t j = 0; j < tq.at(i).announce_queue.size(); j++) {
                    this->log.write(to_string(j) + ". " + tq.at(i).announce_queue.at(j).ip + ":" + to_string(tq.at(i).announce_queue.at(j).port));

                    //TODO: UNCOMMENT ME!
                    if (this->out_port != tq.at(i).announce_queue.at(j).port) {
                        //this->connect("127.0.0.1", tq.at(i).announce_queue.at(j).port);
                        this->connect(tq.at(i).announce_queue.at(j).ip, tq.at(i).announce_queue.at(j).port);
                    }
                }
            }
        }
    }
}

void Client::receive_packet() {
    struct sockaddr_in fromaddr;
    socklen_t addrlen = sizeof(fromaddr);
    int in = recvfrom(this->sock, this->buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &fromaddr, &addrlen);
    if (in > 0) {
        string tmp = "New data (" + to_string(in) + ") from ";
        #ifdef __linux__
            tmp += inet_ntoa(fromaddr.sin_addr);
        #endif
        #ifdef _WIN32
            char *out = new char[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &fromaddr.sin_addr, out, INET_ADDRSTRLEN);
            tmp += out;
        #endif
        this->log.write(tmp, 1);
        if (DEBUG) {
            for (int i = 0; i < in; i++) {
                printf("%x ", this->buffer[i]);
            }
            cout << endl;
        }
        this->buffer_size = in;
        this->handle_data(fromaddr);
        this->clear_buffer();
    }
}

void Client::send_packet() {
    if (this->send_queue.size() > 0) {
        this->send_lock.lock();
        for (uint32_t i = 0; i < this->send_queue.size(); i++) {
            if (this->send_queue.at(i).conn == 1) {
                if (this->is_still_connected(this->send_queue.at(i).id)) {
                    sockaddr_in str = this->get_struct(this->send_queue.at(i).id);
                    sendto(this->sock, this->send_queue.at(i).packet.c_str(), this->send_queue.at(i).packet.length(), 0, (struct sockaddr *) &str, sizeof(str));

                    this->last_send_struct = this->send_queue.at(i);
                    for (uint32_t j = 0; j < this->send_queue.at(i).fnc.size(); j++) {
                        this->send_queue.at(i).fnc.at(j)(this->send_queue.at(i).id);
                    }
                }
            }
            else {
                addrinfo *str = this->get_struct(this->send_queue.at(i).id, false);
                sendto(this->sock, this->send_queue.at(i).packet.c_str(), this->send_queue.at(i).packet.length(), 0, str->ai_addr, str->ai_addrlen);

                this->last_send_struct = this->send_queue.at(i);
                for (uint32_t j = 0; j < this->send_queue.at(i).fnc.size(); j++) {
                    this->send_queue.at(i).fnc.at(j)(this->send_queue.at(i).id);
                }
            }
        }
        this->send_queue.clear();
        this->send_lock.unlock();
    }
}

/* Public methods used by ui */

void Client::connect(string ip, int port) {
    sockaddr_in *server;
    addrinfo *info;
    addrinfo hints;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = PF_UNSPEC;

    err = getaddrinfo(ip.c_str(), NULL, &hints, &info);
    if (err == 0) {
        memset((char *)&server, 0, sizeof(server));
        server = (struct sockaddr_in *)info->ai_addr;
        server->sin_port = htons(port);
        cts str;
        str.str = info;
        str.id = this->generate_conn_hash();
        this->conn_to_queue.push_back(str);
        this->queue_data(str.id, 0, "", DEFAULT_FNC_VECTOR, 0);
    }
    else {
        string tmp = "Could not find the hostname " + ip + "\n";
        #ifdef __linux__
            tmp += gai_strerror(err);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            tmp += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(tmp, 2);
        return;
    }
}

void Client::disconnect(string id) {
    string tmp = "Disconnecting from peer " + id;
    this->log.write(tmp, 3);
    vector<function<void(string)>> fnc;
    fnc.push_back(bind(&Client::callback_disconnect, this, id));
    this->queue_data(id, 0x05, "", fnc);
}

string Client::decrypt(string msg) {
    if (gpgme_signers_count(this->gpgctx) > 1) {
        this->log.write("Too many signers, aborting signing.", 2);
        return "";
    }

    gpgme_error_t err;
    gpgme_data_t temp;
    gpgme_data_t enc;
    err = gpgme_data_new(&temp);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);

        return "";
    }
    gpgme_ssize_t written = gpgme_data_write(temp, (const void *)msg.c_str(), msg.size());
    if (written == -1 && msg.size() > 0) {
        string ret = "Error writing to gpg memory\n";
        ret += to_string(errno) + ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return "";
    }

    //Return the pointer to the start
    gpgme_data_seek(temp, 0, SEEK_SET);
    err = gpgme_data_new(&enc);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return "";
    }

    err = gpgme_op_decrypt(this->gpgctx, temp, enc);
    if (err) {
        string ret = "Error decrypting string (" + to_string(err) + ")\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return "";
    }
    gpgme_off_t length = gpgme_data_seek(enc, 0, SEEK_END);
    gpgme_data_seek(enc, 0, SEEK_SET);
    string result;
    char *tempres = new char[length];
    gpgme_ssize_t read = gpgme_data_read(enc, (void *)tempres, length);

    if (read == -1) {
        string ret = "Error reading from gpg memory\n";
        ret += to_string(errno) + ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return "";
    }

    for (int i = 0; i < length; i++) {
        result += tempres[i];
    }

    gpgme_data_release(temp);
    gpgme_data_release(enc);
    return result;
}

string Client::encrypt(string msg, string conn_id) {
    string fpr = this->get_fingerprint(conn_id);
    if (gpgme_signers_count(this->gpgctx) > 1) {
        this->log.write("Too many signers, aborting signing.", 2);
        return "";
    }

    gpgme_error_t err;
    gpgme_data_t temp;
    gpgme_data_t enc;
    err = gpgme_data_new(&temp);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);

        return "";
    }
    gpgme_ssize_t written = gpgme_data_write(temp, (const void *)msg.c_str(), msg.size());
    if (written == -1 && msg.size() > 0) {
        string ret = "Error writing to gpg memory\n";
        ret += to_string(errno) + ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return "";
    }

    //Return the pointer to the start
    gpgme_data_seek(temp, 0, SEEK_SET);
    err = gpgme_data_new(&enc);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return "";
    }
    gpgme_key_t key;
    gpgme_key_t recp[2];
    gpgme_encrypt_flags_t flags = GPGME_ENCRYPT_ALWAYS_TRUST;
    err = gpgme_op_keylist_start(this->gpgctx, NULL, 0);
    bool found = false;
    while (1) {
        err = gpgme_op_keylist_next(this->gpgctx, &key);
        if (err) {
            break;
        }
        bool matches = true;
        for (uint32_t i = 0; i < fpr.size(); i++) {
            if (key->subkeys->fpr[i] != fpr.at(i)) {
                matches = false;
                break;
            }
        }

        if (matches) {
            recp[0] = key;
            found = true;
            break;
        }
    }

    recp[1] = 0;

    err = gpgme_op_encrypt(this->gpgctx, recp, flags, temp, enc);
    if (err) {
        string ret = "Error encrypting string (" + to_string(err) + ")\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return "";
    }
    gpgme_off_t length = gpgme_data_seek(enc, 0, SEEK_END);
    gpgme_data_seek(enc, 0, SEEK_SET);
    string result;
    char *tempres = new char[length];
    gpgme_ssize_t read = gpgme_data_read(enc, (void *)tempres, length);

    if (read == -1) {
        string ret = "Error reading from gpg memory\n";
        ret += to_string(errno) + ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return "";
    }

    for (int i = 0; i < length; i++) {
        result += tempres[i];
    }

    gpgme_data_release(temp);
    gpgme_data_release(enc);
    gpgme_key_release(key);
    return result;
}

void Client::get_connected() {
    this->log.write("  ID  |               FINGERPRINT                | UPTIME MIN ");
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        duration<double> diff = duration_cast<duration<double>>(steady_clock::now() - this->conn_queue.at(i).uptime);
        double uptime = floor(diff.count() / 60);
        string str = this->conn_queue.at(i).conn_hash + "   " + this->conn_queue.at(i).hash + "   " + to_string((int)uptime);
        this->log.write(str);
    }
}

string Client::get_fingerprint(string id) {
    string fpr = "";
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            fpr = this->conn_queue.at(i).hash;
            break;
        }
    }
    return fpr;
}

string Client::get_time() {
    system_clock::time_point now = system_clock::now();
    time_t tt = system_clock::to_time_t(now);
    struct tm time;
    #ifdef __linux__
        time = *localtime(&tt);
    #endif
    #ifdef _WIN32
        localtime_s(&time, &tt);
    #endif
    string tmp = "[";
    tmp += (time.tm_hour < 10 ? "0" : "") + to_string(time.tm_hour) + ":";
    tmp += (time.tm_min < 10 ? "0" : "") + to_string(time.tm_min) + ":";
    tmp += (time.tm_sec < 10 ? "0" : "") + to_string(time.tm_sec) + "]";
    return tmp;
}

bool Client::is_connection_id(string id) {
    bool found = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (id == this->conn_queue.at(i).conn_hash) {
            found = true;
            break;
        }
    }
    return found;
}

void Client::queue_data(string id, int type, string data, vector<function<void(string)>> fnc, int connected) {
    if (connected == 1) {
        sps str;
        str.id = id;
        str.packet = this->create_packet(type, data, this->get_my_packet_num(id), 1);
        str.conn = connected;
        str.fnc = fnc;

        this->send_queue.push_back(str);
        this->inc_my_packet_num(id);
    }
    else {
        sps str;
        str.packet = this->create_packet(type, data, 0, 0);
        str.conn = connected;
        str.fnc = fnc;
        str.id = id;
        this->send_queue.push_back(str);
    }
}

/* TESTING */

string Client::get_conn_id() {
    if (this->conn_queue.size() > 0) {
        return this->conn_queue.at(0).conn_hash;
    }
    return "";
}

void Client::test_mp() {
    string id = this->get_conn_id();
    vector<function<void(string)>> fnc;
    fnc.push_back(bind(&Client::add_to_sent_packets, this, id));
    fnc.push_back(bind(&Client::callback_heartbeat, this, id));
    sps str;

    str.conn = 1;
    str.id = id;
    str.packet = this->create_packet(0x20, "1.vienas", this->get_my_packet_num(id), 1);
    this->conn_queue.at(0).sent_packets.push_back(str);
    this->inc_my_packet_num(id);
    str.conn = 1;
    str.id = id;
    str.packet = this->create_packet(0x20, "2.du", this->get_my_packet_num(id), 1);
    this->conn_queue.at(0).sent_packets.push_back(str);
    this->inc_my_packet_num(id);
    str.conn = 1;
    str.id = id;
    str.packet = this->create_packet(0x20, "3.trys", this->get_my_packet_num(id), 1);
    this->conn_queue.at(0).sent_packets.push_back(str);
    this->inc_my_packet_num(id);
    str.conn = 1;
    str.id = id;
    str.packet = this->create_packet(0x20, "4.keturi", this->get_my_packet_num(id), 1);
    this->conn_queue.at(0).sent_packets.push_back(str);
    this->inc_my_packet_num(id);

    //now send actual packet!
    this->queue_data(this->get_conn_id(), 0x20, "5.penki", fnc);
}

/* Struct */

sockaddr_in Client::get_struct(string id) {
    if (this->conn_queue.size() == 0) {
        this->log.write("Bad struct id", 2);
        throw "Bad struct id";
    }
    else {
        for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
            if (this->conn_queue.at(i).conn_hash == id) {
                return this->conn_queue.at(i).str;
            }
        }
        this->log.write("Bad struct id", 2);
        throw "Bad struct id";
    }
}

addrinfo *Client::get_struct(string id, bool conn) {
    if (this->conn_to_queue.size() == 0) {
        this->log.write("Bad struct id", 2);
        throw "Bad struct id";
    }
    else {
        for (uint32_t i = 0; i < this->conn_to_queue.size(); i++) {
            if (this->conn_to_queue.at(i).id == id) {
                return this->conn_to_queue.at(i).str;
            }
        }
        this->log.write("Bad struct id", 2);
        throw "Bad struct id";
    }
}

/* Packet */

string Client::create_packet(int type, string data, uint8_t packet_number, int connected) {
    string packet;
    switch (type) {
        case 0x00://conn init
        {
            if (connected == 1) {
                this->log.write("Already connected!", 2);
            }
            else {

                string msg = this->get_random_msg();
                //Placeholder for type
                //cannot append 0 (int) to string
                //packet += '~';
                packet.insert(packet.end(), 0);
                packet += packet_number;
                packet += this->fingerprint;
                packet += msg;
                packet += this->sign(msg);
                //packet.front() = 0;
            }
            break;
        }
        case 0x01://conn acc
        {
            string msg = this->get_random_msg();
            packet += (uint8_t)0x01;
            packet += packet_number;
            packet += this->fingerprint;
            packet += msg;
            packet += this->sign(msg);
            break;
        }
        case 0x02://conn ack
        {
            packet += (uint8_t)0x02;
            packet += packet_number;
            break;
        }
        case 0x03://bad info
        {
            packet += (uint8_t)0x03;
            packet += packet_number;
            packet += data;
            break;
        }
        case 0x04://already connected
        {
            packet += (uint8_t)0x04;
            packet += packet_number;
            break;
        }
        case 0x05://disconnect
        {
            packet += (uint8_t)0x05;
            packet += packet_number;
            break;
        }
        case 0x06://heartbeat proposal
        {
            packet += (uint8_t)0x06;
            packet += packet_number;
            packet += data;
            break;
        }
        case 0x07://heartbeat answer
        {
            packet += (uint8_t)0x07;
            packet += packet_number;
            packet += data;
            break;
        }
        case 0x08://heartbeat
        {
            packet += (uint8_t)0x08;
            packet += packet_number;
            break;
        }
        case 0x10://request missing packets
        {
            packet += (uint8_t)0x10;
            packet += packet_number;
            packet += data;
            break;
        }
        case 0x11://Missing packet
        {
            packet += (uint8_t)0x11;
            packet += packet_number;
            packet += data;
            break;
        }
        case 0x12://Missing packet not in queue
        {
            packet += (uint8_t)0x12;
            packet += packet_number;
            break;
        }
        case 0x20://Text message
        {
            packet += (uint8_t)0x20;
            packet += packet_number;
            packet += data;
            break;
        }
        default:
        {
            this->log.write("Unsupported type", 2);
            break;
        }
    }
    return packet;
}

/* Gpgme related */

string Client::sign(string msg) {
    if (gpgme_signers_count(this->gpgctx) > 1) {
        this->log.write("Too many signers, aborting signing.", 2);
        return "";
    }
    gpgme_error_t err;
    gpgme_data_t temp;
    gpgme_data_t sig;
    err = gpgme_data_new(&temp);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);

        return "";
    }
    gpgme_ssize_t written = gpgme_data_write(temp, (const void *)msg.c_str(), msg.size());
    if (written == -1 && msg.size() > 0) {
        string ret = "Error writing to gpg memory\n";
        ret += to_string(errno);
        ret += ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return "";
    }

    //Return the pointer to the start
    gpgme_data_seek(temp, 0, SEEK_SET);
    err = gpgme_data_new(&sig);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return "";
    }

    err = gpgme_op_sign(this->gpgctx, temp, sig, GPGME_SIG_MODE_NORMAL);
    if (err) {
        string ret = "Error signing string\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return "";
    }

    gpgme_off_t length = gpgme_data_seek(sig, 0, SEEK_END);
    gpgme_data_seek(sig, 0, SEEK_SET);
    string result;
    char *tempres = new char[length];
    gpgme_ssize_t read = gpgme_data_read(sig, (void *)tempres, length);

    if (read == -1) {
        string ret = "Error reading from gpg memory\n";
        ret += to_string(errno) + ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return "";
    }

    for (int i = 0; i < length; i++) {
        result += tempres[i];
    }

    gpgme_data_release(temp);
    gpgme_data_release(sig);

    return result;
}

bool Client::verify(string msg, string encmsg, string fingerprint) {
    string dbg = "Verifying message " + msg + " with fingerprint " + fingerprint;
    this->log.write(dbg, 1);
    gpgme_error_t err;
    gpgme_data_t temp;
    gpgme_data_t sig;
    bool ret = false;
    err = gpgme_data_new(&temp);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);

        return false;
    }

    err = gpgme_data_new(&sig);
    if (err) {
        string ret = "Error initializing gpg memory\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return false;
    }

    gpgme_ssize_t written = gpgme_data_write(sig, (const void *)encmsg.c_str(), encmsg.size());
    if (written == -1 && msg.size() > 0) {
        string ret = "Error writing to gpg memory\n";
        ret += to_string(errno) + ": ";
        #ifdef __linux__
            ret += strerror(errno);
        #endif
        #ifdef _WIN32
            char *buffer = new char[256];
            ret += strerror_s(buffer, 255, errno);
        #endif
        this->log.write(ret, 2);
        return false;
    }
    gpgme_data_seek(sig, 0, SEEK_SET);
    err = gpgme_op_verify(this->gpgctx, sig, NULL, temp);
    if (err) {
        string ret = "Failed to verify encrypted message\n";
        ret += gpgme_strsource(err);
        ret += ": ";
        ret += gpgme_strerror(err);
        this->log.write(ret, 2);
        return false;
    }
    else {
        gpgme_verify_result_t verres = gpgme_op_verify_result(this->gpgctx);
        gpgme_off_t length = gpgme_data_seek(temp, 0, SEEK_END);
        char *tempres = new char[length];
        gpgme_data_seek(temp, 0, SEEK_SET);
        gpgme_ssize_t read = gpgme_data_read(temp, tempres, length);
        if (read == -1) {
            string ret = "Error reading from gpg memory\n";
            ret += to_string(errno) + ": ";
            #ifdef __linux__
                ret += strerror(errno);
            #endif
            #ifdef _WIN32
                char *buffer = new char[256];
                ret += strerror_s(buffer, 255, errno);
            #endif
            this->log.write(ret, 2);
            return false;
        }

        bool strmatches = true;
        for (int i = 0; i < length; i++) {
            if (tempres[i] != msg.at(i)) {
                strmatches = false;
                break;
            }
        }

        if (strmatches) {
            bool fprmatches = true;
            if (verres != NULL) {
                for (uint32_t i = 0; i < fingerprint.size(); i++) {
                    if (fingerprint.at(i) != verres->signatures->fpr[i]) {
                        fprmatches = false;
                        break;
                    }
                }
            }
            if (fprmatches) {
                ret = true;
            }
            else {
                this->log.write("Fingerprints do not match", 2);
            }
        }
        else {
            this->log.write("Strings do not match", 2);
        }
    }

    gpgme_data_release(temp);
    gpgme_data_release(sig);
    return ret;
}

/* Helpers */

void Client::add_to_sent_packets(string id) {
    bool add = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            if (this->conn_queue.at(i).sent_packets.size() == this->backup_length) {
                this->conn_queue.at(i).sent_packets.erase(this->conn_queue.at(i).sent_packets.begin());
                this->conn_queue.at(i).sent_packets.push_back(this->last_send_struct);
            }
            else if ((int)this->conn_queue.at(i).sent_packets.size() > this->backup_length) {
                this->log.write("Backup queue is bigger than limit !", 2);
            }
            else {
                this->conn_queue.at(i).sent_packets.push_back(this->last_send_struct);
            }
            add = true;
            break;
        }
    }
    if (!add) {
        string tmp = "Failed to add packet to sent vector for hash " + id;
        this->log.write(tmp, 2);
    }
}

void Client::clear_buffer() {
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        this->buffer[i] = 0;
    }
}

void Client::inc_my_packet_num(string id) {
    bool increased = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            this->conn_queue.at(i).my_packet_num++;
            increased = true;
            break;
        }
    }
    if (!increased) {
        string tmp = "Failed to increase my packet on hash " + id;
        this->log.write(tmp, 2);
    }
}

string Client::generate_conn_hash() {
    random_device rd;
    mt19937_64 gen(rd());
    #ifdef __linux__
        uniform_int_distribution<uint8_t> numb(0, 15);
    #endif
    #ifdef _WIN32
        uniform_int_distribution<> numb(0, 15);
    #endif
    auto get_symbol = [](uint8_t number) -> char {
        string symbols = "0123456789ABCDEF";
        return symbols.at(number);
    };
    string ret = "";
    for (int i = 0; i < this->conn_hash_length; i++) {
        ret += get_symbol(numb(gen));
    }
    return ret;
}

/* Getters */

string Client::get_conn_str_id(sockaddr_in fromaddr, bool should_be) {
    string id = "";
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        #ifdef __linux__
            if (inet_ntoa(this->conn_queue.at(i).str.sin_addr) == inet_ntoa(fromaddr.sin_addr)) {
        #endif
        #ifdef _WIN32
            char *out = new char[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &this->conn_queue.at(i).str.sin_addr, out, INET_ADDRSTRLEN);
            char *out2 = new char[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &fromaddr.sin_addr, out2, INET_ADDRSTRLEN);
            if (strcmp(out, out2) == 0) {
        #endif
            id = this->conn_queue.at(i).conn_hash;
            break;
        }
    }
    if (id == "" && should_be) {
        string tmp = "Failed to find conn structure id for addr ";
        #ifdef __linux__
            tmp += inet_ntoa(fromaddr.sin_addr);
        #endif
        #ifdef _WIN32
            char *out = new char[23];
            InetNtopA(AF_INET, &fromaddr.sin_addr, out, INET_ADDRSTRLEN);
            tmp += out;
        #endif
        this->log.write(tmp, 2);
    }
    return id;
}

uint8_t Client::get_heartbeat_rate(string id) {
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            return this->conn_queue.at(i).heartbeat_rate;
        }
    }
    string tmp = "Failed to get heartbeat rate for " + id;
    this->log.write(tmp, 2);
    return 0;
}

void Client::get_missing_packets(string id) {
    string data;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            for (int j = this->conn_queue.at(i).his_packet_num + 1; j < this->buffer[1]; j++) {
                data += (uint8_t)j;
                mps tmp;
                tmp.added = steady_clock::now();
                tmp.id = id;
                tmp.packet_id = j;
                this->missing_packet_queue.push_back(tmp);
            }
            break;
        }
    }
    this->queue_data(id, 0x10, data);
}

uint8_t Client::get_my_optimal_heartbeat() {
    return 30;
}

uint8_t Client::get_my_packet_num(string id) {
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            return this->conn_queue.at(i).my_packet_num;
        }
    }
    return 0;
}

string Client::get_random_msg() {
    random_device rd;
    mt19937_64 gen(rd());
    #ifdef __linux__
        uniform_int_distribution<uint8_t> numb(0, 61);
    #endif
    #ifdef _WIN32
        uniform_int_distribution<> numb(0, 61);
    #endif
    auto get_symbol = [](uint8_t number) -> char {
        string symbols = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
        return symbols.at(number);
    };
    string ret = "";
    for (uint32_t i = 0; i < 20; i++) {
        ret += get_symbol(numb(gen));
    }
    return ret;
}

/* Setters */

void Client::set_conn_accepted(string id) {
    bool found = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            found = true;
            this->conn_queue.at(i).conn_acc = true;
            break;
        }
    }
    if (!found) {
        string tmp = "Failed to set connection to accepted on " + id;
        this->log.write(tmp, 2);
    }
}

void Client::set_heartbeat_rate(string id, uint8_t rate) {
    bool set = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            this->conn_queue.at(i).heartbeat_rate = rate;
            this->conn_queue.at(i).last_heartbeat = steady_clock::now();
            this->conn_queue.at(i).last_heartbeat_received = steady_clock::now();
            set = true;
            break;
        }
    }
    if (!set) {
        string tmp = "Failed to set heartbeat rate on hash " + id;
        this->log.write(tmp, 2);
    }
}

void Client::set_his_packet_num(string id, uint8_t packet) {
    bool set = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            this->conn_queue.at(i).his_packet_num = packet;
            set = true;
            break;
        }
    }
    if (!set) {
        string tmp = "Failed to set his packet on hash " + id;
        this->log.write(tmp, 2);
    }
}

void Client::set_received_heartbeat(string id) {
    bool set = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            this->conn_queue.at(i).last_heartbeat_received = steady_clock::now();
            set = true;
            break;
        }
    }
    if (!set) {
        string tmp = "Failed to set received heartbeat time on hash " + id;
        this->log.write(tmp, 2);
    }
}

void Client::set_sent_heartbeat(string id) {
    bool set = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            this->conn_queue.at(i).last_heartbeat = steady_clock::now();
            set = true;
            break;
        }
    }
    if (!set) {
        string tmp = "Failed to set sent heartbeat time on hash " + id;
        this->log.write(tmp, 2);
    }
}

/* Booleans */

bool Client::is_connected(sockaddr_in str) {
    string id = this->get_conn_str_id(str, false);
    if (id == "") {
        return false;
    }
    return true;
}

bool Client::is_fingerprint_in_keychain(string fingerprint) {
    gpgme_key_t key;
    gpgme_error_t err;

    string pattern = fingerprint.substr(fingerprint.size() - 8);
    err = gpgme_op_keylist_start(this->gpgctx, pattern.c_str(), 0);
    bool found = false;
    while (1) {
        err = gpgme_op_keylist_next(this->gpgctx, &key);
        if (err) {
            break;
        }
        bool matches = true;

        for (uint32_t i = 0; i < fingerprint.size(); i++) {
            if (fingerprint.at(i) != key->subkeys->fpr[i]) {
                matches = false;
                break;
            }
        }
        gpgme_key_release(key);
        if (matches) {
            found = true;
            break;
        }
    }
    gpgme_op_keylist_end(this->gpgctx);

    return found;
}

bool Client::is_missing_packets(string id) {
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            if ((this->conn_queue.at(i).his_packet_num + 1) == this->buffer[1]) {
                return false;
            }
            else if (this->buffer[1] >= (this->conn_queue.at(i).his_packet_num + this->max_missing_packets)) {
                string tmp = "Missing more than " + to_string(this->max_missing_packets) + 
                    " packets, disconnecting from peer " + this->conn_queue.at(i).hash;
                this->log.write(tmp, 3);
                this->disconnect(id);
                return false;
            }
            else {
                return true;
            }
        }
    }
    return false;
}

bool Client::is_still_connected(string id) {
    bool connected = false;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            connected = true;
            break;
        }
    }
    return connected;
}

/* Remove */

void Client::remove_from_missing_queue(string id, uint8_t packet_id) {
    uint32_t j = 0;
    bool found = false;
    for (uint32_t i = 0; i < this->missing_packet_queue.size(); i++) {
        if (this->missing_packet_queue.at(i).id == id) {
            found = true;
            break;
        }
        j++;
    }

    if (found) {
        this->missing_packet_queue.erase(this->missing_packet_queue.begin() + j);
    }
    else {
        this->log.write("Failed to remove from missing packet queue", 2);
    }
}

void Client::remove_peer(string id) {
    int vid = -1;
    for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
        if (this->conn_queue.at(i).conn_hash == id) {
            vid = i;
            break;
        }
    }
    if (vid != -1) {
        this->conn_queue.erase(this->conn_queue.begin() + vid);
    }
    else {
        string tmp = "Failed to remove peer with connection hash " + id;
        this->log.write(tmp, 2);
    }
}

/* Response methods */

void Client::handle_bad_info(sockaddr_in fromaddr) {
    string id = this->get_conn_str_id(fromaddr);
    string tmp = "Got bad info packet from " + id + " with packet id " + to_string(this->buffer[2]);
    this->log.write(tmp, 2);
}

        //Handles all incoming packets regarding connection
void Client::handle_connect(int type, sockaddr_in fromaddr) {
    if (type == 0) {
        if (this->is_connected(fromaddr)) {
            this->log.write("User trying to connect, but already connected.", 3);
            string id = this->get_conn_str_id(fromaddr);
            vector<function<void(string)>> fnc;
            fnc.push_back(bind(&Client::add_to_sent_packets, this, id));
            fnc.push_back(bind(&Client::callback_heartbeat, this, id));
            this->queue_data(id, 0x04, "", fnc);
        }
        else {
            this->log.write("User trying to connect.", 3);

            string fingerprint;
            for (int i = 2; i < 42; i++) {
                fingerprint += this->buffer[i];
            }

            string msg;
            for (int i = 42; i < 62; i++) {
                msg += this->buffer[i];
            }

            string encmsg;
            for (int i = 62; i < this->buffer_size; i++) {
                encmsg += this->buffer[i];
            }

            bool fingerprint_exists = true;

            if (!this->is_fingerprint_in_keychain(fingerprint)) {
                string pubkey = fingerprint.substr(24, string::npos);
                is imp = utils::import_key_from_keyserver(this->homedir, this->keyserver, pubkey);

                if (imp.imported > 0) {
                    if (imp.imported > 1) {
                        this->log.write("Imported more than one key.", 3);
                    }
                    else {
                        this->log.write("Imported one key", 1);
                    }
                }
                else {
                    this->log.write("Failed to import key", 2);
                    fingerprint_exists = false;
                }
            }


            cs str;
            str.hash = fingerprint;
            str.str = fromaddr;
            str.his_packet_num = this->buffer[1];
            str.conn_hash = this->generate_conn_hash();
            str.uptime = steady_clock::now();
            this->conn_queue.push_back(str);

            if (fingerprint_exists) {
                if (this->verify(msg, encmsg, fingerprint)) {
                    vector<function<void(string)>> fnc;
                    fnc.push_back(bind(&Client::add_to_sent_packets, this, str.conn_hash));
                    fnc.push_back(bind(&Client::callback_heartbeat, this, str.conn_hash));
                    this->queue_data(str.conn_hash, 0x01, "", fnc);
                }
                else {
                    string msg = "Couldnt verify message from ";
                    msg += fingerprint;
                    this->log.write(msg, 3);
                    this->queue_data(str.conn_hash, 0x03, "", DEFAULT_FNC_VECTOR);
                    this->disconnect(str.conn_hash);
                }
            }
            else {
                string msg = "Couldnt find fingerprint " + fingerprint + " @ " + this->keyserver;
                this->log.write(msg, 3);
                this->queue_data(str.conn_hash, 0x03, "", DEFAULT_FNC_VECTOR);
                this->disconnect(str.conn_hash);
            }
        }
    }
    else if (type == 1) {
        if (this->is_connected(fromaddr)) {
            this->log.write("User trying to connect, but already connected.", 3);
            string id = this->get_conn_str_id(fromaddr);
            vector<function<void(string)>> fnc;
            fnc.push_back(bind(&Client::add_to_sent_packets, this, id));
            fnc.push_back(bind(&Client::callback_heartbeat, this, id));
            this->queue_data(id, 0x04, "", fnc);
        }
        else {
            string fingerprint;
            for (int i = 2; i < 42; i++) {
                fingerprint += this->buffer[i];
            }

            string msg;
            for (int i = 42; i < 62; i++) {
                msg += this->buffer[i];
            }

            string encmsg;
            for (int i = 62; i < this->buffer_size; i++) {
                encmsg += this->buffer[i];
            }

            bool fingerprint_exists = true;

            if (!this->is_fingerprint_in_keychain(fingerprint)) {
                string pubkey = fingerprint.substr(24, string::npos);
                is imp = utils::import_key_from_keyserver(this->homedir, this->keyserver, pubkey);

                if (imp.imported > 0) {
                    if (imp.imported > 1) {
                        this->log.write("Imported more than one key.", 3);
                    }
                    else {
                        this->log.write("Imported one key", 1);
                    }
                }
                else {
                    this->log.write("Failed to import key", 2);
                    fingerprint_exists = false;
                }
            }

            cs str;
            str.hash = fingerprint;
            str.str = fromaddr;
            str.his_packet_num = this->buffer[1];
            str.conn_hash = this->generate_conn_hash();
            str.uptime = steady_clock::now();
            str.my_packet_num++;
            str.conn_acc = true;
            this->conn_queue.push_back(str);

            if (fingerprint_exists) {
                if (this->verify(msg, encmsg, fingerprint)) {
                    vector<function<void(string)>> fnc;
                    fnc.push_back(bind(&Client::add_to_sent_packets, this, str.conn_hash));
                    fnc.push_back(bind(&Client::callback_heartbeat, this, str.conn_hash));
                    this->queue_data(str.conn_hash, 0x02, "", fnc);
                    this->log.write("Connection accepted.", 3);

                }
                else {
                    string msg = "Couldnt verify message from " + fingerprint;
                    this->log.write(msg, 3);
                    this->queue_data(str.conn_hash, 0x03, "", DEFAULT_FNC_VECTOR);
                    this->disconnect(str.conn_hash);
                }
            }
            else {
                string msg = "Couldnt find fingerprint " + fingerprint + " @ " + this->keyserver;
                this->log.write(msg, 3);
                this->queue_data(str.conn_hash, 0x03, "", DEFAULT_FNC_VECTOR);
                this->disconnect(str.conn_hash);
            }
        }
    }
    else if (type == 2) {
        string id = this->get_conn_str_id(fromaddr);
        if (this->is_connected(fromaddr)) {
            this->log.write("Connection accepted.", 3);
            id = this->get_conn_str_id(fromaddr);
            this->set_conn_accepted(id);

            //initiate heartbeat settings
            string data = to_string(this->get_my_optimal_heartbeat());
            vector<function<void(string)>> fnc;
            fnc.push_back(bind(&Client::add_to_sent_packets, this, id));
            fnc.push_back(bind(&Client::callback_heartbeat, this, id));
            this->queue_data(id, 0x06, data, fnc);
        }
        else {
            string tmp = "Connect structure not found for peer ";
            #ifdef __linux__
                tmp += inet_ntoa(fromaddr.sin_addr);
            #endif
            #ifdef _WIN32
                char *out = new char[INET_ADDRSTRLEN];
                InetNtopA(AF_INET, &fromaddr.sin_addr, out, INET_ADDRSTRLEN);
                tmp += out;
            #endif
            this->log.write(tmp, 2);
        }
    }
    else if (type == 3) {
        string id = this->get_conn_str_id(fromaddr);
        vector<function<void(string)>> fnc;
        fnc.push_back(bind(&Client::add_to_sent_packets, this, id));
        fnc.push_back(bind(&Client::callback_heartbeat, this, id));
        this->queue_data(id, 0x04, "");
    }
    else {
        this->log.write("Unknown type for connect.", 2);
    }
}

        //Method which routes packets to other methods
void Client::handle_data(sockaddr_in fromaddr, bool skip) {
    string id = "";
    if (this->is_connected(fromaddr)) {
        //set new received timer
        id = this->get_conn_str_id(fromaddr);
        this->set_received_heartbeat(id);
        if (!skip) {
            if (this->is_missing_packets(id)) {
                this->get_missing_packets(id);
                this->set_his_packet_num(id, this->buffer[1]);
            }
            else {
                this->set_his_packet_num(id, this->buffer[1]);
            }
        }
    }
    switch (this->buffer[0]) {
        case 0x00:
        {
            this->handle_connect(0, fromaddr);
            break;
        }
        case 0x01:
        {
            this->handle_connect(1, fromaddr);
            break;
        }
        case 0x02:
        {
            this->handle_connect(2, fromaddr);
            break;
        }
        case 0x03:
        {
            this->handle_bad_info(fromaddr);
            break;
        }
        case 0x04:
        {
            this->handle_connect(3, fromaddr);
            break;
        }
        case 0x05:
        {
            this->handle_disconnect(fromaddr);
            break;
        }
        case 0x06:
        {
            this->handle_heartbeat(0, fromaddr);
            break;
        }
        case 0x07:
        {
            this->handle_heartbeat(1, fromaddr);
            break;
        }
        case 0x08:
        {
            this->handle_heartbeat(2, fromaddr);
            break;
        }
        case 0x10:
        {
            this->handle_missing_packets(0, fromaddr);
            break;
        }
        case 0x11:
        {
            this->handle_missing_packets(1, fromaddr);
            break;
        }
        case 0x12:
        {
            this->handle_missing_packets(2, fromaddr);
            break;
        }

        case 0x20://im
        {
            this->handle_im(fromaddr);
            break;
        }
        default:
        {
            this->log.write("Case " + to_string(this->buffer[0]) + "not implemented in data handler.", 2);
            break;
        }
    }
}

void Client::handle_disconnect(sockaddr_in fromaddr) {
    string id = this->get_conn_str_id(fromaddr);
    if (id == "") {
        string tmp = "Failed to disconnect from ";
        #ifdef __linux__
            tmp += inet_ntoa(fromaddr.sin_addr);
        #endif
        #ifdef _WIN32
            char *out = new char[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &fromaddr.sin_addr, out, INET_ADDRSTRLEN);
            tmp += out;
        #endif

        tmp += ". Probably already disconnected";
        this->log.write(tmp, 3);
    }
    else {
        string tmp = "Disconnected from peer ";
        #ifdef __linux__
            tmp += inet_ntoa(fromaddr.sin_addr);
        #endif
        #ifdef _WIN32
            char *out = new char[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &fromaddr.sin_addr, out, INET_ADDRSTRLEN);
            tmp += out;
        #endif
        this->log.write(tmp, 3);
        this->remove_peer(id);
    }
}

void Client::handle_heartbeat(int type, sockaddr_in fromaddr) {
    string id = this->get_conn_str_id(fromaddr);
    vector<function<void(string)>> fnc;
    fnc.push_back(bind(&Client::add_to_sent_packets, this, id));
    fnc.push_back(bind(&Client::callback_heartbeat, this, id));
    if (type == 0) {
        if (this->get_my_optimal_heartbeat() < this->buffer[2]) {
            this->set_heartbeat_rate(id, (uint8_t)this->buffer[2]);
            string data;
            data += this->buffer[2];
            this->queue_data(id, 0x07, data, fnc);
            //using his interval
        }
        else {
            this->set_heartbeat_rate(id, this->get_my_optimal_heartbeat());
            string data;
            data += this->get_my_optimal_heartbeat();
            this->queue_data(id, 0x07, data, fnc);
            //using mine interval
        }
    }
    else if (type == 1) {
        this->set_heartbeat_rate(id, (uint8_t)this->buffer[2]);
    }
    else if (type == 2) {
        this->log.write("Received heartbeat!", 1);
        this->set_received_heartbeat(id);
    }
    else {
        this->log.write("Unknown type for heartbeat", 2);
    }
}

void Client::handle_im(sockaddr_in fromaddr) {
    string id = this->get_conn_str_id(fromaddr);
    string msg;
    for (int i = 2; i < this->buffer_size; i++) {
        msg += this->buffer[i];
    }

    string tmp = this->get_time();
    tmp += id + ": " + this->decrypt(msg);
    this->log.write(tmp);
}

void Client::handle_missing_packets(int type, sockaddr_in fromaddr) {
    string id = this->get_conn_str_id(fromaddr);
    if (type == 0) {
        vector<sps> tmp;
        for (uint32_t i = 0; i < this->conn_queue.size(); i++) {
            if (this->conn_queue.at(i).conn_hash == id) {
                tmp = this->conn_queue.at(i).sent_packets;
                break;
            }
        }

        if (tmp.size() == 0) {
            this->log.write("Sent packet queue is empty.", 1);
        }
        else {
            auto is_packet = [](vector<sps> tmp, uint8_t id) -> bool {
                bool found = false;
                for (uint32_t i = 0; i < tmp.size(); i++) {
                    if (tmp.at(i).packet.at(1) == id) {
                        found = true;
                        break;
                    }
                }
                return found;
            };

            auto get_packet = [](vector<sps> tmp, uint8_t id) -> string {
                for (uint32_t i = 0; i < tmp.size(); i++) {
                    if (tmp.at(i).packet.at(1) == id) {
                        return tmp.at(i).packet;
                    }
                }
                throw "Couldn't get packet";
            };

            for (int i = 2; i < this->buffer_size; i++) {
                if (is_packet(tmp, this->buffer[i])) {
                    string tmpstr = "Resending packet " + get_packet(tmp, (uint8_t)this->buffer[i]) + " to " + id;
                    this->log.write(tmpstr, 1);
                    vector<function<void(string)>> fnc;
                    fnc.push_back(bind(&Client::callback_heartbeat, this, id));
                    this->queue_data(id, 0x11, get_packet(tmp, (uint8_t)this->buffer[i]), fnc);
                }
                else {
                    string tmp = "Dismissing packet " + to_string(this->buffer[i]) + " to " + id;
                    this->log.write(tmp, 1);
                    vector<function<void(string)>> fnc;
                    fnc.push_back(bind(&Client::callback_heartbeat, this, id));
                    this->queue_data(id, 0x12, "", fnc);
                }

            }
        }
    }
    else if (type == 1) {
        this->remove_from_missing_queue(id, (uint8_t)this->buffer[3]);
        for (int i = 0; i < this->buffer_size - 2; i++) {
            this->buffer[i] = this->buffer[i + 2];
        }
        this->buffer_size = this->buffer_size - 2;
        this->handle_data(fromaddr, true);
    }
    else if (type == 2) {
        this->remove_from_missing_queue(id, (uint8_t)this->buffer[3]);
    }
    else {
        this->log.write("Unknown type for missing packet", 2);
    }
}

/* Callbacks after packet is sent */

void Client::callback_disconnect(string id) {
    this->log.write("Found disconnect packet, removing from connected vector.", 1);
    this->remove_peer(id);
}

void Client::callback_heartbeat(string id) {
    this->log.write("Updating sent heartbeat.", 1);
    this->set_sent_heartbeat(id);
}