#pragma once
#include "Client.h"
#include "Log.h"
#ifndef CLIENT_THREAD_H
#define	CLIENT_THREAD_H
namespace clientThread {
    void run();
    void stop();
    /* Sets client for thread */
    void setClient(Client *object);
}

#endif

