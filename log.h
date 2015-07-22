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

#ifndef LOG_H
#define	LOG_H
#include "includes.h"
class Log {
public:
    Log(){ 
        this->using_file = false;
        this->usable = true;
        this->outfile = "";
    };    
    
    
    /* GetInstance */
    static Log& getInstance() {
        static Log instance;
        return instance;
    }; 
    
    
    void write(string msg, short type = 0);
    void set_file(string filename);
    void unset_file();
    bool check_if_writeable(string filename);
private:
    bool using_file;
    bool usable;
    string outfile;

};

#endif	/* LOG_H */

