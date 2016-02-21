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


#include "includes.h"
#include "mainwindow.h"
#include "tracker_thread.h"
#include "client_thread.h"
#include "track_thread.h"
#include "cl_thread.h"
int main(int argc, char** argv) {
    string homedir = "";
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--settings") == 0) {
            homedir = argv[i+1];
        }
    }
    
    if(homedir == "") {
        homedir = getenv("HOME");
        homedir += "/.tchat/";
    }
    
    cout << "homedir is " << homedir << endl;
    //thread trackers(tracker_thread::run);
    
    //gpgme_signers_add(ctx, the_key);
    QApplication app(argc, argv);
    /* Qt */
    track_thread tt;
    QObject::connect(&tt, SIGNAL(finished()), &app, SLOT(quit()));
    tt.start();
    //track_thread::start();
    MainWindow *win = new MainWindow(homedir);
    
    if(win->init_gpgme()) {
        if(win->init_db()) {
            

            if(!win->select_profile()) {
                return 0;
            }
            string fp;
            int port;
            Client *cl = win->init_client(port, fp);
            cout << "port is " << to_string(port) << endl;
            cout << "fp is " << fp << endl;
            tt.set_client_port(port);
            tt.set_announce_hash(fp);
            //FIXME: move into mainwindow class !
            //thread trackers(tracker_thread::run);
            //thread client(client_thread::run);
            cl_thread ct;
            ct.set_client(cl);
            ct.start();
            
            
            tt.add_tracker("open.demonii.com", 1337);
            tt.add_action(0, 1);
            //tracker_thread::add_tracker("open.demonii.com", 1337);

            //tracker_thread::add_action(0, 1);
    
            
            
            
            if(win->get_profile_status() == -1) {
                return 0;
            } else if(win->get_profile_status() == 1) {
                win->read_friendlist();
                win->set_profile();
            } else if(win->get_profile_status() == 0) {
                cout << "Something bad happened :(" << endl;
            }
            
            win->show();   
            
            
            
            //trackers.join();
            //client.join();
        } else {
            return 0;
        }
    }else {
        return 0;
    }
    
    
    //trackers.join();
    return app.exec();
    /*Client *cl = new Client(fingerprint, ctx, homedir, DEFAULT_KEY_SERVER);
    tracker_thread::set_client_port(cl->get_out_port());
    client_thread::setClient(cl);
    tracker_thread::set_announce_hash(fingerprint);
    thread trackers(tracker_thread::run);
    thread client(client_thread::run);
    
    tracker_thread::add_tracker("open.demonii.com", 1337);
    
    tracker_thread::add_action(0, 1);
    
    trackers.join();
    client.join();*/
}

