#include "stdafx.h"
#include "Log.h"

void Log::unset_file()
{
    this->outfile = "";
    this->using_file = false;
}

void Log::set_file(string filename)
{
    this->outfile = filename;
    this->using_file = true;
}

void Log::write(string msg, short type)
{
    if (this->using_file)
    {
        ofstream out;
        out.open(this->outfile, fstream::app | fstream::out);
        
        if(type != 1) {
            out << "[" << this->log_enum[type] << "]: " <<  msg << endl;
        }
        else if(DEBUG) {
            out << "[" << this->log_enum[type] << "]: " <<  msg << endl;
        }
        out.close();
    }
    else
    {
        if(type > 0) {
            if(type != 1) {
                cout << this->log_enum[type] << ": " << msg << endl;
            }
            else if(DEBUG) {
                cout << this->log_enum[type] << ": " << msg << endl;
            }
        } 
        else {
            cout << msg << endl;
        }
    }
}