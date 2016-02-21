/* 
 * File:   cl_thread.h
 * Author: Gediminas Dulskas <dulskasg@gmail.com>
 *
 * Created on Trečiadienis, 2015, Rugsėjo 30, 15.29
 */
#include "includes.h"
#include "client.h"
#include "log.h"
#ifndef CL_THREAD_H
#define	CL_THREAD_H

class cl_thread : public QThread {
public:
    /* Sets client for thread */
    void set_client(Client *cl);
    void stop();
    Q_OBJECT
    
    
    
    
protected:
    void run();
private:
    bool cycle_run;
    unsigned char buffer[BUFFER_LENGTH];
    
    /* Log */
    Log log;
    
    /* Client object */
    Client *obj;
};

#endif	/* CL_THREAD_H */

