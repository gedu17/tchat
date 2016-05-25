#include "stdafx.h"
#include "Utils.h"
#include "Log.h"
#include "Tracker.h"
#include "TrackerThread.h"
#include "Client.h"
#include "ClientThread.h"
#include "Settings.h"

int main(int argc, char** argv) {
    #ifdef __linux__
        string homedir = "";
        string gpgdir = "";
        if(argc == 2) {
            homedir = argv[1];
            homedir += "/.tchat/";
            gpgdir = homedir;
            gpgdir += ".gnupg";
        } 
        else {
            homedir = getenv("HOME");
            homedir += "/.tchat/";
            gpgdir = getenv("HOME");
            gpgdir += "/.gnupg";
        }
    #endif

    #ifdef _WIN32
        char *buf;
        size_t len;
        _dupenv_s(&buf, &len, "APPDATA");
        string gpgdir = buf;
        gpgdir += "\\gnupg";
        string homedir = "";
        if (homedir == "") {
            char *buf;
            size_t len;
            _dupenv_s(&buf, &len, "APPDATA");
            homedir = buf;
            homedir += "\\tchat\\";
        }
    #endif

    string alias;
    string fingerprint;
    vector<string> key_servers;
    vector<string> track;
    gpgme_key_t the_key;
    gpgme_ctx_t ctx;

    utils::init_gpgme(&ctx, gpgdir);
    utils::read_settings(homedir, ctx, &the_key, &alias, &fingerprint, &key_servers, &track);
    gpgme_signers_add(ctx, the_key);
    cout << "*** Settings data ***" << endl;
    cout << "Alias = " << alias << endl;
    cout << "Fingerprint = " << fingerprint << endl;
    cout << "Key servers:" << endl;
    for (uint i = 0; i < key_servers.size(); i++) {
        cout << "  " << i << ". " << key_servers.at(i) << endl;
        is data = utils::import_key_from_keyserver(gpgdir, key_servers.at(i), fingerprint.substr(32, string::npos));
        if(data.processed == 0) {
            utils::export_key_to_keyserver(gpgdir, key_servers.at(i), fingerprint.substr(32, string::npos));
        }
    }
    cout << "Trackers:" << endl;
    for (uint i = 0; i < track.size(); i++) {
        cout << "  " << i << ". " << track.at(i) << endl;
    }
    cout << "*********************" << endl << endl;
    //TODO: Make client work with multiple key servers
    
    Client *cl;
    if (key_servers.size() == 0) {
        cl = new Client(fingerprint, ctx, gpgdir, DEFAULT_KEY_SERVER);
    }
    else {
        cl = new Client(fingerprint, ctx, gpgdir, key_servers.at(0));
    }
    cl->create_socket();
    trackerThread::set_client_port(cl->get_out_port());
    clientThread::setClient(cl);
    trackerThread::set_announce_hash(fingerprint);
    thread trackers(trackerThread::run);
    thread client(clientThread::run);

    for (uint i = 0; i < track.size(); i++) {
        int pos = track.at(i).find(":");
        string host = track.at(i).substr(0, pos);;
        short port = stoi(track.at(i).substr(pos + 1, string::npos));
        bool add = trackerThread::add_tracker(host, port);
        if (add) {
            trackerThread::add_action(i, 1);
        }
    }

    string cmd;
    bool run = true;
    cout << "Write help for available commands" << endl;

    while (run) {
        getline(cin, cmd);

        if (cmd == "help") {
            cout << "** Available commands:" << endl;
            cout << "   list trackers" << endl;
            cout << "   list keyservers" << endl;
            cout << "   list connections" << endl;
            cout << "   add tracker <host>:<port>" << endl;
            cout << "   add keyserver <host>" << endl;
            cout << "   add connection <hash>" << endl;
            cout << "   rem tracker <id>" << endl;
            cout << "   rem keyserver <id>" << endl;
            cout << "   rem connection <id>" << endl;
            cout << "   msg <id> <message>" << endl;
        }
        else if (cmd.substr(0, 4) == "list") {
            if (cmd.substr(5, string::npos) == "trackers") {
                cout << "Trackers:" << endl;
                for (uint i = 0; i < track.size(); i++) {
                    cout << "  " << i << ". " << track.at(i) << endl;
                }
            }
            else if (cmd.substr(5, string::npos) == "keyservers") {
                cout << "Key servers:" << endl;
                for (uint i = 0; i < key_servers.size(); i++) {
                    cout << "  " << i << ". " << key_servers.at(i) << endl;
                }
            }
            else if (cmd.substr(5, string::npos) == "connections") {
                cl->get_connected();
            }
            else {
                cout << "Unkown command." << endl;
            }
        }
        else if (cmd.substr(0, 3) == "add") {
            if (cmd.substr(4, 7) == "tracker") {
                string tmp = cmd.substr(12, string::npos);
                if (tmp.find(":") == string::npos) {
                    cout << "Symbol : not found." << endl;
                }
                else {
                    string host = tmp.substr(0, tmp.find(":"));
                    short port = stoi(tmp.substr(tmp.find(":") + 1, string::npos));
                    bool add = trackerThread::add_tracker(host, port);
                    if (add) {
                        int id = trackerThread::get_id(host);
                        track.push_back(tmp);
                        if (id != -1) {
                            trackerThread::add_action(id, 1);
                        }
                    }
                }
            }
            else if (cmd.substr(4, 9) == "keyserver") {
                bool found = false;
                string ks = cmd.substr(14, string::npos);
                for (uint i = 0; i < key_servers.size(); i++) {
                    if (key_servers.at(i) == ks) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    key_servers.push_back(ks);

                    is data = utils::import_key_from_keyserver(gpgdir, ks, fingerprint.substr(32, string::npos));
                    if (data.processed == 0) {
                        utils::export_key_to_keyserver(gpgdir, ks, fingerprint.substr(32, string::npos));
                    }

                    cout << "Keyserver added." << endl;
                }
                else {
                    cout << "Keyserver already exists." << endl;
                }

            }
            else if (cmd.substr(4, 10) == "connection") {
                string hash = cmd.substr(15, string::npos);
                for (uint i = 0; i < track.size(); i++) {
                    trackerThread::add_action(i, 0, hash);
                }
            }
            else {
                cout << "Unkown command." << endl;
            }
        }

        else if (cmd.substr(0, 3) == "rem") {
            if (cmd.substr(4, 7) == "tracker") {
                int id = stoi(cmd.substr(12, string::npos));
                bool rem = trackerThread::remove_tracker(id);
                if (rem) {
                    track.erase(track.begin() + id);
                    cout << "Tracker removed." << endl;
                }
                else {
                    cout << "Bad id." << endl;
                }
            }
            else if (cmd.substr(4, 9) == "keyserver") {
                bool found = false;
                uint id = stoi(cmd.substr(14, string::npos));
                if (id > key_servers.size() - 1) {
                    cout << "Id too big." << endl;
                }
                else {
                    key_servers.erase(key_servers.begin() + id);
                    cout << "Keyserver removed." << endl;
                }
            }
            else if (cmd.substr(4, 10) == "connection") {
                string id = cmd.substr(15, string::npos);
                bool rem = cl->is_connection_id(id);
                if (rem) {;
                    cl->disconnect(id);
                }
                else {
                    cout << "Bad id." << endl;
                }
            }
            else {
                cout << "Unkown command." << endl;
            }
        }

        else if (cmd.substr(0, 3) == "msg") {                
            string id = cmd.substr(4, 5);
            bool rem = cl->is_connection_id(id);
            if (rem) {
                string msg = cmd.substr(10, string::npos);
                    
                string encmsg = cl->encrypt(msg, id);
                cout << "Message length: " << msg.size() << " / " << encmsg.size() << endl;
                cl->queue_data(id, 0x20, encmsg);
                string tmp = cl->get_time();
                tmp += alias + ": " + msg;
                cout << tmp << endl;
            }
            else {
                cout << "Bad id." << endl;
            }
                
        }
        else if (cmd == "exit" || cmd == "quit") {
            cout << "Exiting... " << endl;
            run = false;
        }
        else {
            cout << "Unkown command." << endl;
        }
    }

    trackerThread::stop();
    clientThread::stop();
    trackers.join();
    client.join();
    utils::update_settings(homedir, key_servers, track, alias, fingerprint.substr(24, string::npos));
    return 0;
}

