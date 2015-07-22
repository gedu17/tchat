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

#include "log.h"

void Log::set_file(string filename) {
    if(this->check_if_writeable(filename)){
        this->outfile = filename;
        this->using_file = true;
    } else {
        this->usable = false;
        cout << "File " << filename << "is not writable" << endl;
    }
}

void Log::unset_file() {
    this->outfile = "";
    this->using_file = false;
    this->usable = true;
}

void Log::write(string msg, short type) {
    if(this->usable) {
        if(this->using_file) {
            //TODO: implement this part!
            cout << "NOT IMPLEMENTED." << endl;
        } else {
            switch(type) {
                case 1:
                    if(DEBUG) {
                        cout << "DBG: " << msg << endl;
                    }
                    break;
                case 2:
                    cout << "ERR: " << msg << endl;
                    break;
                case 3:
                    cout << "NOT: " << msg << endl;
                    break;
                case 4:
                    cout << "TEST: " << msg << endl;
                    break;
                default:
                    cout << msg << endl;
                    break;
            }
        }
    }
}

bool Log::check_if_writeable(string filename) {
    return true;
}

