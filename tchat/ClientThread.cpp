#include "stdafx.h"
#include "ClientThread.h"
#include "Utils.h"

namespace clientThread {

    bool cycle_run = true;

    /* Log */
    Log log = Log::get_instance();

    /* Client object */
    Client *obj;


    void run() {
        log.write("Starting client thread.", 1);
        while (cycle_run) {
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

    void setClient(Client *object) {
        obj = object;
    }
}