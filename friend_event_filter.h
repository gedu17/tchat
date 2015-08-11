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

#ifndef FRIEND_EVENT_FILTER_H
#define	FRIEND_EVENT_FILTER_H
#include "includes.h"
#include "mainwindow.h"
class friend_event_filter : public QObject {
    Q_OBJECT
    
    public:
    friend_event_filter(MainWindow *window);
    virtual ~friend_event_filter();
    protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    MainWindow *window;
    QEvent::Type last_event;
    string obj_name;
};

#endif	/* FRIEND_EVENT_FILTER_H */

