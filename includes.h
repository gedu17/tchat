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

#ifndef INCLUDES_H
#define	INCLUDES_H

/* C++ */
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <algorithm>

/* C */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
//#include <cstring>
#include <string.h>
/* Gpgme */
#include <gpgme.h>

/* Qt */
#include <QApplication>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>

#include <QGraphicsPixmapItem>
#include <QRadioButton>
#include <QDebug>
#include <QList>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QtWebKitWidgets/QWebView>
#include <QtSql>
#include <QMessageBox>
#include <QUdpSocket>
#include <QHostInfo>
#include <QThread>


/* Defines */
#define BUFFER_LENGTH 512
#define DEBUG 1
#define DEFAULT_ALIAS "I am unnamed"
#define DEFAULT_KEY_SERVER "pgp.mit.edu"
#define DEFAULT_SETTINGS_FILE "settings.txt"
#define DEFAULT_FRIENDSLIST_FILE "friends.txt"
#define DEFAULT_AVATAR_FILE "images/avatar.png"
#define CHATLOG_SELF_CONST "Me"
/* Namespaces */
using namespace std;

using namespace std::chrono;
#endif	/* INCLUDES_H */

