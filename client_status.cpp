/* 
 * File:   status.cpp
 * Author: Gediminas Dulskas <dulskasg@gmail.com>
 * 
 * Created on Antradienis, 2015, Rugpjūčio 25, 13.03
 */

#include "client_status.h"

client_status::client_status() {
    //this->status_vector;
}

client_status::~client_status() {
}


void client_status::add_to_status(string msg) {
    this->status_vector.push_back(msg);
}

string client_status::get_status() {   
    return accumulate( this->status_vector.begin(), this->status_vector.end(), string("\n") );
}