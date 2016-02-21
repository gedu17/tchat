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

#include "mainwindow.h"
#include "includes.h"
#include "settings.h"
#include "friendslist.h"
#include "friend_event_filter.h"
#include "tracker_thread.h"
#include "client_thread.h"
#include "tracker.h"
#include "client.h"
#include "client_thread.h"
#include "utils.h"
#include "client_status.h"
MainWindow::MainWindow(string dir) {
    this->setupUi();
    this->dir = dir;
    this->gpgdir = dir;
    this->gpgdir += "gnupg/";
    this->log = Log::getInstance();
    this->set = new settings(dir);
    this->avatar = "images/avatar.png";
    this->fingerprint_dropdown = false;
    this->retranslateUi();
    this->profile_status = 0;
    this->setMouseTracking(true);
    this->status_obj = client_status();
    this->current_active = -1;
}

void MainWindow::exit() {
    QApplication::exit(0);
}

void MainWindow::resize_me(int width, int height) {
    this->resize(width, height);
}

void MainWindow::setupUi() {  
        if (this->objectName().isEmpty())
            this->setObjectName(QStringLiteral("MainWindow"));
        this->setWindowModality(Qt::NonModal);
        this->resize(650, 550);
        
        /* Icon */
        QIcon icon;
        icon.addFile(QStringLiteral("images/gooby_logo.png"), QSize(), QIcon::Normal, QIcon::Off);
        this->setWindowIcon(icon);
        
        
        
        actionNetwork_Settings = new QAction(this);
        actionNetwork_Settings->setObjectName(QStringLiteral("actionNetwork_Settings"));
        
        this->setStyleSheet("QMainWindow {background: #2581B3; color: #fff;}");
        
        
        centralwidget = new QWidget(this);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        stackedWidget->setGeometry(QRect(0, 0, 650, 550));
        
        /* Logo */
        
        QImage logo = QImage("images/logo.png");
        QGraphicsPixmapItem *logo_pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(logo));
        QGraphicsScene *logo_scene = new QGraphicsScene;
        logo_scene->addItem(logo_pixmap);
        
        
        /* Main Page */
        main_page = new QWidget();
        main_page->setObjectName(QStringLiteral("main_page"));
        
        /* Left Side */
        
        left_side = new QWidget(main_page);
        left_side->setObjectName(QStringLiteral("left_side"));
        left_side->setGeometry(0,0,260, 550);
        left_side_layout = new QVBoxLayout(left_side);
        left_side_layout->setObjectName(QStringLiteral("left_side_layout"));
        left_side_layout->setContentsMargins(0, 0, 0, 0);
        
        
        this->main_avatar = new QGraphicsView(left_side);
        this->main_avatar->setObjectName(QStringLiteral("main_avatar"));
        this->main_avatar->setGeometry(QRect(5, 10, 69, 69));
        
        this->main_alias = new QPushButton(left_side);
        this->main_alias->setObjectName(QStringLiteral("main_alias"));
        this->main_alias->setGeometry(QRect(78, 12, 182, 21));
        
        this->main_alias_field = new QLineEdit(left_side);
        this->main_alias_field->setObjectName(QStringLiteral("main_alias_field"));
        this->main_alias_field->setGeometry(QRect(78, 12, 182, 21));
        this->main_alias_field->setVisible(false);
        
        this->status = new QPushButton(left_side);
        this->status->setObjectName(QStringLiteral("status"));
        this->status->setGeometry(QRect(78, 35, 182, 21));
        
        this->status_field = new QLineEdit(left_side);
        this->status_field->setObjectName(QStringLiteral("status_field"));
        this->status_field->setGeometry(QRect(78, 35, 182, 21));
        this->status_field->setVisible(false);
        
        /*this->status = new QLabel(left_side);
        this->status->setObjectName(QStringLiteral("status"));
        this->status->setGeometry(QRect(78, 35, 182, 15));*/
        
        friends_online = new QLabel(left_side);
        friends_online->setObjectName(QStringLiteral("friends_online"));
        friends_online->setGeometry(QRect(78, 54, 182, 16));
        friends_online->setFrameShape(QFrame::NoFrame);
        friends_online->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        
        separator = new QFrame(left_side);
        separator->setObjectName(QStringLiteral("separator"));
        separator->setGeometry(QRect(-2, 94, 260, 3));
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        
        show_fingerprint = new QPushButton(left_side);
        show_fingerprint->setObjectName(QStringLiteral("show_fingerprint"));
        show_fingerprint->setGeometry(QRect(200, 55, 15, 15));
        
        show_status = new QPushButton(left_side);
        show_status->setObjectName(QStringLiteral("show_status"));
        show_status->setGeometry(QRect(230, 55, 15, 15));
        
        this->main_fingerprint = new QTextBrowser(main_page);
        this->main_fingerprint->setObjectName(QStringLiteral("main_fingerprint"));
        this->main_fingerprint->setGeometry(QRect(-2, 75, 260, 22));
        this->main_fingerprint->setEnabled(true);
        this->main_fingerprint->setVisible(false);
        
        this->friends_list = new QListWidget(left_side);
        this->friends_list->setGeometry(QRect(0, 110, 260, 281));
        this->friends_list->setSortingEnabled(false);
        this->friends_list->setPalette(QPalette(QColor("#2581B3")));
        
        this->friends_list->setStyleSheet("QListWidget::item {background-color: #2581B3;}"
            "QListWidget {background-color: #2581B3;}");
        
        this->friends_list->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this->friends_list, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(friend_context_menu(QPoint)));
        
        /* Right side */
        
        right_side = new QWidget(main_page);
        right_side->setObjectName(QStringLiteral("right_side"));
        right_side->setGeometry(260, 0, 390, 550);
        
        /*main_text = new QTextBrowser(right_side);
        main_text->setObjectName(QStringLiteral("main_text"));
        main_text->setEnabled(true);
        main_text->setAcceptDrops(false);
        main_text->setGeometry(0, 0, 390, 450);
        main_text->setAlignment(Qt::AlignVCenter);
        main_text->setHtml("<div style=\"vertical-align: bottom;  bottom: 0px;\">Kieciaks yra kiec nieka cia vaikeli nepadarisi xD</div>");*/
        main_text = new QWebView(right_side);
        main_text->setObjectName(QStringLiteral("main_text"));
        main_text->setGeometry(1, 0, 388, 449);
        //main_text->setHtml("<div style=\"position:absolute; bottom: 0px;\">Hello .</div>");
        
        
        
        separator = new QFrame(right_side);
        separator->setObjectName(QStringLiteral("separator"));
        separator->setGeometry(QRect(2, 450, 387, 3));
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        
        main_text_input = new QPlainTextEdit(right_side);
        main_text_input->setObjectName("main_text_input");
        main_text_input->setGeometry(QRect(0, 453, 300, 71));
        
        main_text_button = new QPushButton(right_side);
        main_text_button->setObjectName("main_text_input");
        main_text_button->setGeometry(QRect(300, 455, 89, 68));
        main_text_button->setText("Send!");
        
        stackedWidget->addWidget(main_page);
        
        
        
        /* Settings page used in startup */
        
        settings_page = new QWidget();
        settings_page->setObjectName(QStringLiteral("settings_page"));
        
        settings_page_layout = new QWidget(settings_page);
        settings_page_layout->setObjectName(QStringLiteral("settings_page_layout"));
        settings_page_layout->setGeometry(QRect(0, 210, 650, 140));
        
        settings_page_vblayout = new QVBoxLayout(settings_page_layout);
        settings_page_vblayout->setObjectName(QStringLiteral("settings_page_vblayout"));
        settings_page_vblayout->setContentsMargins(0, 0, 0, 0);
        
        settings_page_text = new QTextBrowser(settings_page_layout);
        settings_page_text->setObjectName(QStringLiteral("settings_page_text"));
        settings_page_text->setEnabled(true);
        settings_page_text->setAcceptDrops(false);

        settings_page_vblayout->addWidget(settings_page_text);

        settings_page_button = new QPushButton(settings_page);
        settings_page_button->setObjectName(QStringLiteral("settings_page_button"));

        settings_page_button2 = new QPushButton(settings_page);
        settings_page_button2->setObjectName(QStringLiteral("settings_page_button2"));
        settings_page_button2->setVisible(false);

        settings_page_logo = new QGraphicsView(settings_page);
        settings_page_logo->setObjectName(QStringLiteral("settings_page_logo"));
        settings_page_logo->setGeometry(QRect(195, 0, 260, 209));
        settings_page_logo->setScene(logo_scene);
        settings_page_logo->setBackgroundBrush(QBrush(QColor("#2581B3"), Qt::SolidPattern));
        
        stackedWidget->addWidget(settings_page);
        
        /* Key selection page */
        
        key_select_page = new QWidget();
        key_select_page->setObjectName(QStringLiteral("key_select_page"));
        
        key_select_logo = new QGraphicsView(key_select_page);
        key_select_logo->setObjectName(QStringLiteral("key_select_logo"));
        key_select_logo->setGeometry(QRect(195, 0, 260, 209));
        key_select_logo->setScene(logo_scene);
        key_select_logo->setBackgroundBrush(QBrush(QColor("#2581B3"), Qt::SolidPattern));
        
        
        key_select_layout = new QWidget(key_select_page);
        key_select_layout->setObjectName(QStringLiteral("key_select_layout"));
        key_select_layout->setGeometry(QRect(0, 240, 610, 240));
        key_select_layout->setPalette(QPalette(QColor("#2581B3")));
        
        
        key_select_vblayout = new QVBoxLayout(key_select_layout);
        key_select_vblayout->setObjectName(QStringLiteral("key_select_vblayout"));
        key_select_vblayout->setContentsMargins(0, 0, 0, 0);
        
        key_select_scroll_area = new QScrollArea(key_select_page);
        key_select_scroll_area->setObjectName(QStringLiteral("key_select_scroll_area"));
        key_select_scroll_area->setWidgetResizable(true);
        key_select_scroll_area->setGeometry(QRect(0, 240, 650, 240));
        key_select_scroll_area->setWidget(key_select_layout);
        key_select_scroll_area->setAlignment(Qt::AlignTop);
        
        key_select_text = new QTextBrowser(key_select_page);
        key_select_text->setObjectName(QStringLiteral("key_select_text"));
        key_select_text->setEnabled(true);
        key_select_text->setAcceptDrops(false);
        key_select_text->setGeometry(0, 210, 650, 30);

        key_select_button = new QPushButton(key_select_page);
        key_select_button->setObjectName(QStringLiteral("key_select_button"));
        key_select_button->setGeometry(0, 480, 650, 27);
        
        stackedWidget->addWidget(key_select_page);
        
        
        /* generic stuff */
        this->setCentralWidget(centralwidget);
        
        menubar = new QMenuBar(this);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 650, 25));
        
        menuTchat = new QMenu(menubar);
        menuTchat->setObjectName(QStringLiteral("menuTchat"));
        this->setMenuBar(menubar);
        
        
        /*statusbar = new QStatusBar(this);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        this->setStatusBar(statusbar);*/
        
        this->setFixedSize(650, 550);

        menubar->addAction(menuTchat->menuAction());
        menuTchat->addAction(actionNetwork_Settings);
        
        retranslateUi();
        
        stackedWidget->setCurrentIndex(0);
        QMetaObject::connectSlotsByName(this);
}

void MainWindow::retranslateUi()
    {
        this->setWindowTitle(QApplication::translate("MainWindow", "Tchat", 0));
#ifndef QT_NO_TOOLTIP
        this->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        this->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        this->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
#ifndef QT_NO_ACCESSIBILITY
        this->setAccessibleName(QApplication::translate("MainWindow", "Tchat", 0));
#endif // QT_NO_ACCESSIBILITY
#ifndef QT_NO_ACCESSIBILITY
        this->setAccessibleDescription(QApplication::translate("MainWindow", "Tchat client", 0));
#endif // QT_NO_ACCESSIBILITY
        actionNetwork_Settings->setText(QApplication::translate("MainWindow", "Network Settings", 0));
        
        settings_page_button->setText(QApplication::translate("MainWindow", "Refresh", 0));
        
        key_select_button->setText(QApplication::translate("MainWindow", "Select", 0));
        menuTchat->setTitle(QApplication::translate("MainWindow", "Settings", 0));
        
        show_fingerprint->setText(QApplication::translate("MainWindow", "?", 0));
        show_status->setText(QApplication::translate("MainWindow", "S", 0));
        
        
    } 

bool MainWindow::read_settings(int id) {
    gpgme_key_t key;
    gpgme_error_t err;
    this->set->read_settings(id);
    this->alias = this->set->get_alias();
    string def_key = this->set->get_key();
    err = gpgme_op_keylist_start(this->ctx, NULL, 1);
    bool found = false;
    while(1) {
        err = gpgme_op_keylist_next(this->ctx, &key);
        if(err) {
            cout << "K L A I D A !" << endl;
            string tmp1 = "Error code: ";
            tmp1 += to_string(gpg_err_code(err));
            tmp1 += "\n";
            tmp1 += "Error string: ";
            tmp1 += gpgme_strerror(err);
            cout << tmp1 << endl;
            break;
        }
        bool matches = true;
        for(uint i = 0; i < def_key.size(); i++) {
            if(key->subkeys->keyid[i] != def_key.at(i)) {
                matches = false;
                break;
            }
        }
        cout << key->subkeys->fpr << endl;
        cout << key->subkeys->keyid << endl;
        cout << "****" << endl;
        if(matches) {
            this->the_key = key;
            this->fingerprint = key->subkeys->fpr;
            found = true;
            break;
        }
        gpgme_key_release(key);
    }

    if(!found) {
        QMessageBox msg;
        msg.setWindowTitle("Secret key not found.");
        string errmsg = def_key;
        errmsg += " not found in keychain.\n";
        errmsg += "Try manually importing neccessary key through cli using --homedir ";
        errmsg += this->dir;
        errmsg += "gnupg/ and restart program.";
        msg.setText(errmsg.c_str());
        msg.setIcon(QMessageBox::Critical);
        msg.setStandardButtons(QMessageBox::Close);
        msg.setDefaultButton(QMessageBox::Close);
        msg.exec();
        return false;
    }
    
    gpgme_signers_add(this->ctx, this->the_key);
    this->profile_status = 1;
    this->fl = new friendslist(dir, id);
    return true;
}

void MainWindow::create_settings() {
    
    gpgme_key_t key;
    gpgme_error_t err;
    this->alias = DEFAULT_ALIAS;
    err = gpgme_op_keylist_start(this->ctx, NULL, 1);
    int counter = 0;
    while(1) {
        err = gpgme_op_keylist_next(this->ctx, &key);
        if(err) {
            break;
        } 
        counter++;
        this->temp_keystore.push_back(key);
    }

    if(counter == 0) {
        this->profile_status = -1;
        QMessageBox msg;
        msg.setWindowTitle("No secret keys found.");
        string errmsg = "No secret keys found in keychain.\n";
        errmsg += "Generate secret key gpg2 --gen-key --homedir ";
        errmsg += this->dir;
        errmsg += "gnupg/ and restart program.";
        msg.setText(errmsg.c_str());
        msg.setIcon(QMessageBox::Critical);
        msg.setStandardButtons(QMessageBox::Close);
        msg.setDefaultButton(QMessageBox::Close);
        msg.exec();
        gpgme_release(this->ctx);
        return;
    } else {
        
        for(uint i = 0; i < this->temp_keystore.size(); i++) {
            radio_button = new QRadioButton(key_select_layout);
            radio_button->setPalette(QPalette(QColor("#2581B3")));
            string tmp = "RadioButton_";
            tmp += to_string(i);
            radio_button->setStyleSheet("QRadioButton {color: #fff;}");
            radio_button->setObjectName(QApplication::translate("MainWindow", tmp.c_str(), 0));
            string tmp2 = this->temp_keystore.at(i)->subkeys->keyid;
            tmp2 += " ";
            if(this->temp_keystore.at(i)->uids && this->temp_keystore.at(i)->uids->name) {
                tmp2 += this->temp_keystore.at(i)->uids->name;
            }
            if(this->temp_keystore.at(i)->uids && this->temp_keystore.at(i)->uids->email) {
                tmp2 += " <";
                tmp2 += this->temp_keystore.at(i)->uids->email;
                tmp2 += ">\n";
            }
            tmp2 += "(";
            tmp2 += this->temp_keystore.at(i)->subkeys->fpr;
            tmp2 += ")";
            radio_button->setText(QString(tmp2.c_str()));
            key_select_vblayout->addWidget(radio_button);
        }

        QObject::connect(key_select_button, SIGNAL(pressed()), this, SLOT(check_key_select()));
        
        string tmp = "<body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal; background-color: #2581B3; color: #fff;"
                "margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\n";
        tmp += "Which key would you like to use ? (Finger Print in brackets will be used as a identifier)</body>";
        key_select_text->setHtml(QApplication::translate("MainWindow", tmp.c_str(), 0));
        
        stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::check_key_select() {
    QObjectList tmp = key_select_layout->children();
    uint size = tmp.size() - 1;
    bool checked = false;
    uint id;
    for(uint i = 0; i < size; i++) {
        string tmp = "RadioButton_";
        tmp += to_string(i);
        QRadioButton *r = key_select_layout->findChild<QRadioButton *>(QString(tmp.c_str()));
        if(r->isChecked()) {
            checked = true;
            id = i;
        }
    }
    
    if(checked) {
        this->the_key = this->temp_keystore.at(id);
        
        this->alias = this->temp_keystore.at(id)->uids->name;
        this->fingerprint = this->temp_keystore.at(id)->subkeys->fpr;
        for(uint i = 0; i < this->temp_keystore.size(); i++) {
            if(i != id) {
                gpgme_key_release(this->temp_keystore.at(i));
            }
        }
        
        this->temp_keystore.clear();
        this->set->set_key(this->fingerprint.substr(24, string::npos));
        this->set->change_alias(this->the_key->uids->name);
        this->set->create_settings();
        this->set_profile();
        
        stackedWidget->setCurrentIndex(0);
    }   
}

void MainWindow::set_avatar() {
    QImage logo = QImage(this->avatar.c_str());
    QGraphicsPixmapItem *avatar_pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(logo));
    QGraphicsScene *avatar_scene = new QGraphicsScene;
    avatar_scene->addItem(avatar_pixmap);
    this->main_avatar->setScene(avatar_scene);
    this->main_avatar->setBackgroundBrush(QBrush(QColor("#2581B3"), Qt::SolidPattern));
    this->main_avatar->setStyleSheet("border: 0px");
}

void MainWindow::set_profile() {
    
    this->friends_online->setText(QApplication::translate("MainWindow", "0 Friends Online", 0));

    QObject::connect(show_status, SIGNAL(pressed()), this, SLOT(open_status()));
    
    /* Fingerprint */
    string tmp = "<body style=\" font-family:'Sans'; font-size:7pt; font-weight:400; font-style:normal; background-color: #129BE6; color: #fff;"
                "margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\n";
        tmp += this->fingerprint;
        tmp += "</body>";
    this->main_fingerprint->setHtml(QApplication::translate("MainWindow", tmp.c_str(), 0));
    QObject::connect(show_fingerprint, SIGNAL(pressed()), this, SLOT(toggle_fingerprint_dropdown()));
    
    /* WebView */
    string mtmp = "<div style=\"position:absolute; bottom: 0px;\">Hello ";
    mtmp += this->alias;
    mtmp += "</div>";
    this->main_text->setHtml(mtmp.c_str());
    
    /* Alias */
    this->main_alias->setText(QApplication::translate("MainWindow", this->alias.c_str(), 0));
    string mastyle = "width: 80px; background: none; font-size: 14pt; color: #fff; border: none; margin: 0px; padding: 0px; text-align: left;";
    this->main_alias->setStyleSheet(mastyle.c_str());
    QObject::connect(this->main_alias, SIGNAL(pressed()), this, SLOT(change_alias()));
    this->main_alias_field->setText(QApplication::translate("MainWindow", this->alias.c_str(), 0));
    QObject::connect(this->main_alias_field, SIGNAL(editingFinished()), this, SLOT(change_alias_finish()));
    
    /* Status */
    this->status->setText(QApplication::translate("MainWindow", "Online", 0));
    this->status->setStyleSheet(mastyle.c_str());
    QObject::connect(this->status, SIGNAL(pressed()), this, SLOT(change_status()));
    this->status_field->setText(QApplication::translate("MainWindow", "Online", 0));
    QObject::connect(this->status_field, SIGNAL(editingFinished()), this, SLOT(change_status_finish()));
    
    /* Avatar */
    this->set_avatar();
    //
    //main_text->setHtml("<div style=\"position:absolute; bottom: 0px;\">imagas iks dey ! <br /> <img src=\"images/test.gif\" /></div>");
    //
    if(this->fl->get_friend_list_count() == 0) {
        //TODO: test if gifs are shown !
        
    } else {
        my_friend *frnd;



        for(uint i = 0; i < this->fl->get_friend_list_count(); i++) {
            try {
                frnd = this->fl->get_friend(i);
                QWidget *wid = new QWidget();
                friend_event_filter *fef = new friend_event_filter(this);
                wid->installEventFilter(fef);
                wid->setPalette(QPalette(QColor("#2581B3")));
                //setobjname
                string widname = "friend_";
                widname += to_string(i);
                wid->setGeometry(QRect(QPoint(0,0), QPoint(260,64)));
                wid->setObjectName(widname.c_str());
                QGraphicsView *avatar = new QGraphicsView(wid);
                //setobjname
                avatar->setGeometry(0, 0, 69, 69);
                QImage img = QImage(frnd->get_avatar().c_str());
                QGraphicsPixmapItem *avatar_pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(img));
                QGraphicsScene *avatar_scene = new QGraphicsScene;
                avatar_scene->addItem(avatar_pixmap);
                avatar->setScene(avatar_scene);
                avatar->setBackgroundBrush(QBrush(QColor("#2581B3"), Qt::SolidPattern));
                avatar->setStyleSheet("border: 0px");
                widname += "_avatar";
                avatar->setObjectName(widname.c_str());

                //add avatar here!
                QLabel *alias = new QLabel(wid);
                alias->setGeometry(QRect(70, 0, 190, 21));
                alias->setText(frnd->get_alias().c_str());

                QLabel *status = new QLabel(wid);
                status->setGeometry(QRect(70, 40, 190, 21));
                if(frnd->get_custom_status() == "") {
                    if(frnd->get_status() == 0) {
                        status->setText("Offline");
                    } else {
                        status->setText("Online");
                    }
                } else {
                    status->setText(frnd->get_custom_status().c_str());
                }            

                QListWidgetItem *item = new QListWidgetItem();
                item->setSizeHint(QSize(235, 64));
                this->friends_list->addItem(item);
                this->friends_list->setItemWidget(item, wid);
            } catch(...) {
                cout << "MainWindow::set_profile() - " << endl;
            }
        }
    }
    
}

void MainWindow::toggle_fingerprint_dropdown() {
    this->main_fingerprint->setVisible(!this->fingerprint_dropdown);
    this->fingerprint_dropdown = !this->fingerprint_dropdown;
}

void MainWindow::change_alias() {
    this->main_alias->setVisible(false);
    this->main_alias_field->setVisible(true);
    
}

void MainWindow::change_alias_finish() {
    //Workaround for bug emitting 2 signals after enter key press
    this->main_alias_field->blockSignals(true);
    //
    this->alias = this->main_alias_field->text().toStdString();
    this->main_alias->setText(QApplication::translate("MainWindow", this->alias.c_str(), 0));
    this->set->change_alias(this->alias);

    //this->set->update_settings();
    this->main_alias_field->setVisible(false);
    this->main_alias->setVisible(true);
    //
    this->main_alias_field->blockSignals(false);
    //
}

void MainWindow::change_status() {
    this->status->setVisible(false);
    this->status_field->setVisible(true);
    
}

void MainWindow::change_status_finish() {
    //Workaround for bug emitting 2 signals after enter key press
    this->status_field->blockSignals(true);
    //
    this->status->setText(QApplication::translate("MainWindow", this->status_field->text().toStdString().c_str(), 0));
    this->status_field->setVisible(false);
    this->status->setVisible(true);
    //
    this->status_field->blockSignals(false);
    
    //

    //TODO: send new status to every online friend!
}

void MainWindow::read_friendlist() {
    this->fl->read_friend_list();
}

void MainWindow::friend_clicked(string id) {
    int item_id = stoi(id.substr(7));
    my_friend *tmp = this->fl->get_friend(item_id);
    
    this->main_text->setHtml(tmp->get_chatlog().c_str());    
    
    if(item_id != this->current_active) {
        if(this->current_active != -1) {
            this->unhover_friend();
        } 
        this->current_active = item_id;
        this->hover_friend();
    }
   /* cout << "** Chatlog Test Started **" << endl;
    tmp->add_to_chatlog("Labas as krabas iksdey!", tmp->get_alias());
    this->main_text->setHtml(tmp->get_chatlog().c_str());
    this_thread::sleep_for(1s);
    tmp->add_to_chatlog("Labukaaaa ! :)))~~~~", "Me");
    this->main_text->setHtml(tmp->get_chatlog().c_str());
    time_t tmptime = time(nullptr);
    this_thread::sleep_for(1s);
    tmp->add_to_chatlog("kalbam!", tmp->get_alias());
    this->main_text->setHtml(tmp->get_chatlog().c_str());
    this_thread::sleep_for(1s);
    tmp->add_to_chatlog_range(tmptime, time(nullptr));
    cout << "** Chatlog Test Finished **" << endl;
    this->main_text->setHtml(tmp->get_chatlog().c_str());*/
    //Testing chatlog!
    //this->main_text->setHtml(tmp->get_chatlog().c_str());
}

void MainWindow::friend_context_menu(QPoint pos) {
    int id;
    
    QModelIndex tmp = this->friends_list->indexAt(pos);
    
    if(tmp.row() == -1) {
        id = this->friends_list->count();
    } else {
        id = tmp.row();
    }
    
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Action 1", this));
    menu->addAction(new QAction("Action 2", this));
    menu->addAction(new QAction("Action 3", this));
    menu->popup(this->friends_list->viewport()->mapToGlobal(pos));
}

bool MainWindow::init_db() {
    struct stat buffer;
    string dbname = this->dir;
    dbname += "data.sqlite";
    if(!(stat (dbname.c_str(), &buffer) == 0)) {
        QMessageBox msg;
        msg.setWindowTitle("Cannot open database");
        msg.setText("Unable to find database file.\n"
            "Click OK to create new database \n "
            "Click Cancel to exit.");
        msg.setIcon(QMessageBox::Question);
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Ok);
        int val = msg.exec();
        if(val == QMessageBox::Ok) {
            this->db = QSqlDatabase::addDatabase("QSQLITE", "data");
            this->db.setDatabaseName(dbname.c_str());
            this->create_settings();
        } else if(val == QMessageBox::Cancel) {
            return false;
        }
    } else {
        this->db = QSqlDatabase::addDatabase("QSQLITE", "data");
        this->db.setDatabaseName(dbname.c_str());

        if(!this->db.open()) {
            QMessageBox msg;
            msg.setWindowTitle("Cannot open database");
            msg.setText("Unable to open database file.\n"
                "Click Close to exit.");
            msg.setIcon(QMessageBox::Critical);
            msg.setStandardButtons(QMessageBox::Close);
            msg.setDefaultButton(QMessageBox::Close);
            int val = msg.exec();
            if(val == QMessageBox::Close) {
                return false;
            }
        }        
        
        this->profile_status = 1;
    }    
    return true;
}

bool MainWindow::init_gpgme() {
    gpgme_check_version(NULL);
    gpgme_error_t err;

    setlocale (LC_ALL, "");
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
    err = gpgme_new(&this->ctx);
    if(err) {
        QMessageBox msg;
        msg.setWindowTitle("Failed to get gpgme context");
        string tmp1 = "Error code: ";
        tmp1 += gpg_err_code(err);
        tmp1 += "\n";
        tmp1 += "Error string: ";
        tmp1 += gpgme_strerror(err);
        msg.setText(tmp1.c_str());
        msg.setIcon(QMessageBox::Critical);
        msg.setStandardButtons(QMessageBox::Close);
        msg.setDefaultButton(QMessageBox::Close);
        msg.exec();
        return false;
    }
    gpgme_engine_info_t eng = gpgme_ctx_get_engine_info(this->ctx);
    gpgme_protocol_t proto = eng->protocol;
    const char *filename = eng->file_name;
    err = gpgme_ctx_set_engine_info(this->ctx, proto, filename, this->gpgdir.c_str());
    if(err) {
        QMessageBox msg;
        msg.setWindowTitle("Failed to set gpgme engine info");
        string tmp1 = "Error code: ";
        tmp1 += gpg_err_code(err);
        tmp1 += "\n";
        tmp1 += "Error string: ";
        tmp1 += gpgme_strerror(err);
        msg.setText(tmp1.c_str());
        msg.setIcon(QMessageBox::Critical);
        msg.setStandardButtons(QMessageBox::Close);
        msg.setDefaultButton(QMessageBox::Close);
        msg.exec();
        return false;
    } 
    return true;
}

int MainWindow::get_profile_status() {
    return this->profile_status;
}
        
bool MainWindow::select_profile() {
    QSqlDatabase db = QSqlDatabase::database("data");
    QSqlQuery query(db);
    query.exec("SELECT * FROM `profiles`");
    int def = 0;
    while(query.next()) {
        if(query.value(4).toInt() == 1) {
            def = query.value(0).toInt();
            break;
            
        }
    }
    if(def != 0) {
        return this->read_settings(def);
    } else {
        cout << "TODO: Profile selector." << endl;
        return false;
    }    
}        

Client *MainWindow::init_client(int &port, string &fp) {
    this->cl = new Client(this->fingerprint, this->ctx, this->dir, DEFAULT_KEY_SERVER, this->fl);
    this->cl->create_qsocket();
    port = this->cl->get_out_port();
    
    //tracker_thread::set_client_port(this->cl->get_out_port());
    //client_thread::setClient(this->cl);
    //tracker_thread::set_announce_hash(this->fingerprint);
    fp = this->fingerprint;
    return this->cl;
}

void MainWindow::open_status() {
    this->status_obj.add_to_status("<div>Testavone 123123123123123</div>");
    this->main_text->setHtml(this->status_obj.get_status().c_str());
    //TODO: change color of current active friend to inactive
    if(this->current_active != -1) {
        this->unhover_friend();
        this->current_active = -1;
    }
}

void MainWindow::unhover_friend() {
    string name = "friend_";
    name += to_string(this->current_active);
    QWidget *tmp = this->left_side->findChild<QWidget *>(name.c_str());
    tmp->setStyleSheet("background: #2581B3;"); 
    name += "_avatar";
    QGraphicsView *avatar = tmp->findChild<QGraphicsView *>(name.c_str());
    avatar->setBackgroundBrush(QBrush(QColor("#2581B3"), Qt::SolidPattern));
}

void MainWindow::hover_friend() {
    string name = "friend_";
    name += to_string(this->current_active);
    
    QWidget *tmp = this->left_side->findChild<QWidget *>(name.c_str());
    tmp->setStyleSheet("background: #ffffff;"); 
    name += "_avatar";
    QGraphicsView *avatar = tmp->findChild<QGraphicsView *>(name.c_str());
    avatar->setBackgroundBrush(QBrush(QColor("#ffffff"), Qt::SolidPattern));
}