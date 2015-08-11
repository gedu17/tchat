# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Debug/CLang-Linux-x86
TARGET = tchat
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += debug 
PKGCONFIG +=
QT = core gui widgets sql webkit
SOURCES += client.cpp client_thread.cpp friend_event_filter.cpp friendslist.cpp log.cpp main.cpp mainwindow.cpp my_friend.cpp settings.cpp tracker.cpp tracker_thread.cpp utils.cpp
HEADERS += client.h client_thread.h friend_event_filter.h friendslist.h includes.h log.h mainwindow.h my_friend.h settings.h structs.h tracker.h tracker_thread.h utils.h
FORMS +=
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Debug/CLang-Linux-x86
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = clang
QMAKE_CXX = clang++
DEFINES += 
INCLUDEPATH += 
LIBS += -lgpgme  
QMAKE_CXXFLAGS += -pthread -lgpgme -std=c++14
QT += webkitwidgets
