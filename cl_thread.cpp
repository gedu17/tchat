/* 
 * File:   cl_thread.cpp
 * Author: Gediminas Dulskas <dulskasg@gmail.com>
 * 
 * Created on Trečiadienis, 2015, Rugsėjo 30, 15.29
 */

#include "cl_thread.h"

void cl_thread::run() {
    this->cycle_run = true;
    this->log = Log::getInstance();
    this->log.write("Starting client thread.", 1);
        while(this->cycle_run) {
            if(this->obj->obj_status) {
                //check tracker queue for data
                this->obj->check_tq();

                this->obj->check_heartbeats();
                
                this->obj->check_missing_packets();
                
                //obj->receive_packet();
                
                this->obj->receive_qpacket();
                
                this->obj->send_packet();
                
            }
            std::this_thread::sleep_for(200ms);
            
        }
    
}
    
void cl_thread::stop() {
    this->cycle_run = false;
}

void cl_thread::set_client(Client *cl) {
    this->obj = cl;
}