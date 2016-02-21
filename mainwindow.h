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

#ifndef MAINWINDOW_H
#define	MAINWINDOW_H
#include "includes.h"
#include "log.h"
#include "settings.h"
#include "friendslist.h"
#include "client.h"
#include "client_status.h"
class MainWindow : public QMainWindow {
    Q_OBJECT
     
public:
    MainWindow(string dir);
    
    void setupUi();
    bool init_gpgme();
    bool init_db();
    
    Client *init_client(int &port, string &fp);
    
    void retranslateUi();
    int get_profile_status();
    bool select_profile();
    void resize_me(int width, int height);
    //void set_page(uint id);
    bool read_settings(int id);
    void set_avatar();
    void set_profile();
    void read_friendlist();
    void friend_clicked(string id);
    
    
    /* Generic */
    
    QAction *actionNetwork_Settings;
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    
    /* Main page */
    
    QWidget *main_page;
    QWidget *left_side;
    QWidget *right_side;
    
    //QLabel *main_alias;
    QPushButton *main_alias;
    QPushButton *status;
    QLabel *friends_online;
    //QLabel *main_fingerprint;
    QGraphicsView *main_avatar;
    QFrame *separator;
    QVBoxLayout *left_side_layout;
    QPushButton *show_fingerprint;
    QPushButton *show_status;
    QTextBrowser *main_fingerprint;
    QLineEdit *main_alias_field;
    QLineEdit *status_field;
    
    QListWidget *friends_list;
    
    //QTextBrowser *main_text;
    QWebView *main_text;
    QPlainTextEdit *main_text_input;
    QPushButton *main_text_button;
    
    /* Settings page */
    
    QWidget *settings_page;
    QWidget *settings_page_layout;
    QVBoxLayout *settings_page_vblayout;
    QTextBrowser *settings_page_text;
    QPushButton *settings_page_button;
    QPushButton *settings_page_button2;
    QGraphicsView *settings_page_logo;
    
    /* Key selection page */
    
    QWidget *key_select_page;
    QWidget *key_select_layout;
    QVBoxLayout *key_select_vblayout;
    QTextBrowser *key_select_text;
    QRadioButton *radio_button;
    QPushButton *key_select_button;
    QGraphicsView *key_select_logo;
    QScrollArea *key_select_scroll_area;
    
    /* Menu */
    
    QMenuBar *menubar;
    QMenu *menuTchat;
    //QStatusBar *statusbar;
    

public slots:
    void exit();
    void create_settings();
    void check_key_select();
    void toggle_fingerprint_dropdown();
    void change_alias();
    void change_alias_finish();
    void change_status();
    void change_status_finish();
    void friend_context_menu(QPoint pos);
    void open_status();
    
    private:
        gpgme_ctx_t ctx;
        gpgme_key_t the_key;
        string alias;
        string fingerprint;
        string dir;
        string gpgdir;
        string avatar;
        string friend_context;
        bool fingerprint_dropdown;
        settings *set;
        Log log;
        friendslist *fl;
        QSqlDatabase db;
        uint profile_status;
        client_status status_obj;
        Client *cl;
        int current_active;
        
        void unhover_friend();
        void hover_friend();
        
        
        vector<gpgme_key_t> temp_keystore;
};

#endif	/* MAINWINDOW_H */

