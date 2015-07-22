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

#include "settings.h"
#include "log.h"

settings::settings(string dir) {
    this->log = Log::getInstance();
    this->dir = dir;
    this->gpgdir = dir;
    this->gpgdir += "gnupg/";        
    this->file = this->dir;
    this->file += DEFAULT_SETTINGS_FILE;
    this->alias = DEFAULT_ALIAS;
    this->key = "00000000";
}

settings::~settings() {
    
}

/* Settings */

void settings::create_settings() {
    this->log.write("Creating settings file");
    
    ofstream sf;
    sf.open(this->file);
    sf << "key=" << this->key << endl;
    sf << "alias=" << this->alias << endl;
    sf.close();
}

bool settings::read_settings() {
    struct stat buffer;   
    bool status = true;
    if((stat(this->dir.c_str(), &buffer) == 0) && S_ISDIR(buffer.st_mode)){
        if((stat(this->gpgdir.c_str(), &buffer) == 0) && S_ISDIR(buffer.st_mode)){
        } else {
            mkdir(this->gpgdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
        }
    
        if(!(stat (this->file.c_str(), &buffer) == 0)) {
            this->log.write("Settings file not found !");
            status = false;
        } else {
            ifstream sf;
            sf.open(this->file);
            string temp;
            while(getline(sf, temp)) {
                if(temp.substr(0, 4) == "key=") {
                    this->key = temp.substr(4);
                } else if(temp.substr(0, 6) == "alias=") {
                    this->alias = temp.substr(6);
                } else {
                    string pr = "Unknown setting in settings file: ";
                    pr += temp;
                    log.write(pr, 2);
                    status = false;
                }
            }
        }
    } else {
        status = false;
        mkdir(this->dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
        mkdir(this->gpgdir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    }
    return status;
}

void settings::update_settings() {
    this->log.write("Updating settings file");
        
    ofstream sf;
    sf.open(this->file);
    sf << "key=" << this->key << endl;
    sf << "as=" << this->alias << endl;
    sf.close();
}

/* Alias */

void settings::change_alias(string alias) {
    this->alias = alias;
}

string settings::get_alias() {
    return this->alias;
}

/* Key */

string settings::get_key() {
    return this->key;
}

void settings::set_key(string key) {
    this->key = key;
}




