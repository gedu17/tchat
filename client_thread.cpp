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

#include "client_thread.h"
#include "utils.h"

namespace client_thread {
    
    bool cycle_run = true;
    unsigned char buffer[BUFFER_LENGTH];
    
    /* Log */
    Log log = Log::getInstance();
    
    /* Client object */
    Client *obj;
    
    
    void run() {
        log.write("Starting client thread.", 1);
        while(cycle_run) {
            //check tracker queue for data
            obj->check_tq();
            
            obj->check_heartbeats();
            
            obj->check_missing_packets();
            
            obj->receive_packet();
            
            obj->send_packet();
            
            std::this_thread::sleep_for(200ms);
        }
    }
    
    void stop() {
        cycle_run = false;
    }
    
    void setClient(Client *cl) {
        obj = cl;
    }
}

