## Log class
 - [ ] Add ability to write to file
 - [ ] Add more diagnostic info
 - [ ] Rewrite write method to take x amount of arguments

## Tracker thread

## App
 - [ ] Recover from network loss
 - [ ] Ability to change the id which is sent to tracker
 - [ ] Add gif send/receive mechanism
 - [ ] Add gif enable/disable globally & for every friend separately

## Client thread
 - [ ] Ability to check wether sender is real
 - [ ] Ability to communicate with more than one real sender
 - [ ] Check if connection sender has announced himself in my hash, if didnt do not respond
 - [ ] Check for infinite loops with bad data, ignore sender after some time
 - [ ] Add pgp keyserver vector
 - [ ] Remove from conn_to_struct when unused (do freeaddrinfo on str)
 - [ ] Rewrite methods to have overloads instead of default values

## My Friend Class
 - [ ] Change div ids from random generated to sequential