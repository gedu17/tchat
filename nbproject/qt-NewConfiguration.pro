# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/NewConfiguration/GNU-Linux-x86
TARGET = tchat
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += debug 
PKGCONFIG +=
QT = core gui widgets
SOURCES += cl_thread.cpp client.cpp client_status.cpp client_thread.cpp friend_event_filter.cpp friendslist.cpp log.cpp main.cpp mainwindow.cpp my_friend.cpp settings.cpp track_thread.cpp tracker.cpp tracker_thread.cpp utils.cpp
HEADERS += cl_thread.h client.h client_status.h client_thread.h friend_event_filter.h friendslist.h includes.h log.h mainwindow.h my_friend.h settings.h structs.h track_thread.h tracker.h tracker_thread.h utils.h
FORMS +=
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/NewConfiguration/GNU-Linux-x86
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = gcc
QMAKE_CXX = g++
DEFINES += 
INCLUDEPATH += 
LIBS += 
