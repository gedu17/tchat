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


#include "friend_event_filter.h"
friend_event_filter::friend_event_filter(MainWindow *window) {
    this->window = window;
}

friend_event_filter::~friend_event_filter() {
}

bool friend_event_filter::eventFilter(QObject* obj, QEvent* ev) {
    if(ev->type() == QEvent::ContextMenu) {
            this->last_event = ev->type();
            this->obj_name = obj->objectName().toStdString();
            return QObject::eventFilter(obj, ev);
    } else {
        if(this->last_event == QEvent::MouseButtonPress) {
            this->window->friend_clicked(this->obj_name);
        }
        
        this->last_event = ev->type();
        this->obj_name = obj->objectName().toStdString();
        
        if(ev->type() == QEvent::MouseButtonPress) {
            return true;
        } else {
            return QObject::eventFilter(obj, ev);
        }
    }
}