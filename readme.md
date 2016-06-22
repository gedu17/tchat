# Tchat
Work in progress for 0.2 version (needs feature merge from 0.15)
## About
Program that enables users to communicate with each other directly, using UDP torrent trackers to find each other and PGP key servers to fetch each other’s public keys.
Data encryption is done using GNU Privacy Guard. Program uses a protocol defined in client protocol.txt to communicate between users. 
UDP is used for communication between peers, missing packet recovery system is implemented. 