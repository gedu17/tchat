# Tchat
Current version is 0.15
## About
Program that enables users to communicate with each other directly, using UDP torrent trackers to find each other and PGP key servers to fetch each other’s public keys.
Data encryption is done using GNU Privacy Guard. Program uses a protocol defined in client protocol.txt to communicate between users. 
UDP is used for communication between peers, missing packet recovery system is implemented. 

## Installation

###WINDOWS
1. Download GPG from https://www.gpg4win.org/download.html
2. Add to environment variable PATH:
    For x86 - C:\Program Files\GNU\GnuPG;
    For x64 - C:\Program Files (x86)\GNU\GnuPG;
3. Open tchat.sln
4. If its x86 system change VC++ Directories in Properties:
    Executable Directories: C:\Program Files (x86)\GNU\GnuPG to C:\Program Files\GNU\GnuPG
    Include Directories: C:\Program Files (x86)\GNU\GnuPG\include to C:\Program Files\GNU\GnuPG\include
    Library Directories: C:\Program Files (x86)\GNU\GnuPG\lib to C:\Program Files\GNU\GnuPG\lib
5. Build solution for x86 regardless of operating systems instruction set

###LINUX
1. Install GNU Privacy Guard if not installed.
2. chmod +X tchat/build
3. ./tchat/build

## Usage
Type help in Tchat for all available commands

### First run
1. Generate asymmetrical key in GPG if you do not have one
2. Start Tchat
3. Select which key you want to use
4. Use command "add tracker <hostname>:<port>" to add tracker
5. Use command "add keyserver <hostname>" to add keyserver

### Finding others and communicating
To search for other users use "add connection <hash>". If connection is successful type "list connections" to get user ID. Then you can send messages with "msg <id> <message>".

### Disconnecting
Type "list connections" to get user ID. Then "rem connection <id>".