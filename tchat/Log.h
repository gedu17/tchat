#pragma once
#ifndef LOG_H
#define	LOG_H
class Log {
public:
    Log() {
        this->using_file = false;
        this->outfile = "";
        this->log_enum[1] = "Debug";
        this->log_enum[2] = "Error";
        this->log_enum[3] = "Notification";
        this->log_enum[4] = "Test";
    };


    /* GetInstance */
    static Log& get_instance() {
        static Log instance;
        return instance;
    };


    void unset_file();
    void set_file(string filename);
    void write(string msg, short type = 0);
    
    
    
private:
    bool using_file;
    string outfile;
    map<short, string> log_enum;
};

#endif
