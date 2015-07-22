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
#include "log.h"
#include "tracker.h"
#include "tracker_thread.h"
#include "client.h"
#include "client_thread.h"
#include "settings.h"
#include "utils.h"
/*
 * 
 */
int main(int argc, char** argv) {

    string gpgdir = getenv("HOME");
    gpgdir += "/.tchat/gnupg";
    string homedir = "";
    if(homedir == "") {
        homedir = getenv("HOME");
        homedir += "/.tchat/";
    }
    string alias;
    string fingerprint;
    gpgme_key_t the_key;
    gpgme_ctx_t ctx;
 
    utils::init_gpgme(&ctx, gpgdir);
    utils::read_settings(homedir, ctx, &the_key, &alias, &fingerprint);
    gpgme_signers_add(ctx, the_key);
    cout << "Alias = " << alias << endl;
    cout << "Fingerprint = " << fingerprint << endl;
    Client *cl = new Client(fingerprint, ctx, homedir, DEFAULT_KEY_SERVER);
    tracker_thread::set_client_port(cl->get_out_port());
    client_thread::setClient(cl);
    tracker_thread::set_announce_hash(fingerprint);
    thread trackers(tracker_thread::run);
    thread client(client_thread::run);
    
    tracker_thread::add_tracker("open.demonii.com", 1337);
    
    tracker_thread::add_action(0, 1);
    
    trackers.join();
    client.join();

    return 0;
}

