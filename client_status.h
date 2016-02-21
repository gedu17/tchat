/* 
 * File:   status.h
 * Author: Gediminas Dulskas <dulskasg@gmail.com>
 *
 * Created on Antradienis, 2015, Rugpjūčio 25, 13.03
 */

#ifndef STATUS_H
#define	STATUS_H
#include "includes.h"
class client_status {
public:
    client_status();
    virtual ~client_status();
    void add_to_status(string msg);
    string get_status();
private:
    vector<string> status_vector;
};

#endif	/* STATUS_H */

