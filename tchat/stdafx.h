#pragma once
#ifndef STDAFX_H
#define	STDAFX_H


#ifdef _WIN32
#include "targetver.h"
#define uint unsigned int
#endif

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
//Sockets
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifdef __APPLE__
#include <sys/socket.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#endif
#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <WinDef.h>
#include <direct.h>
#include <atlstr.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <map>

/* Gpgme */
#include <gpgme.h>

/* Defines */
#define BUFFER_LENGTH 65535
#define DEBUG 0
#define DEFAULT_ALIAS "Unnamed"
#define DEFAULT_KEY_SERVER "pgp.mit.edu"
#define DEFAULT_SETTINGS_FILE "settings.txt"

/* Namespaces */
using namespace std;

using namespace std::chrono;

#endif