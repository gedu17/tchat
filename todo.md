## Log class
 - [ ] Add ability to write to file
 - [ ] Add more diagnostic info
 - [ ] Rewrite write method to take x amount of arguments

## Tracker thread
 - [x] Check if its time to reannounce yourself
 - [x] Add timer for responses, if no response in 15sec => resend
 - [x] Split code from run function to few separate functions

## App
 - [x] Add client thread
 - [ ] Recover from network loss
 - [x] Create and use separate keychain for gpg keys
 
## Client thread
 - [x] Wait for messages from others
 - [ ] Ability to check wether sender is real
 - [ ] Ability to communicate with more than one real sender
 - [ ] Check if connection sender has announced himself in my hash, if didnt do not respond
 - [x] Check for lost packets after every received packet
 - [ ] Check for infinite loops with bad data, ignore sender after some time
 - [x] Create packet vector where last n packets should be saved (separate vector for each connection, exclude stream packets (voice, audio)
 - [x] Generate unique id for every connection
 - [x] Implement a way to do callbacks in send_data after the data is sent
 - [x] Add missing packet handler somewhere!
 - [x] Update conn structure with current time point after each received packet
 - [x] Implement callbacks in all queue_data invocations
 - [ ] Add pgp keyserver vector
 - [ ] Remove from conn_to_struct when unused (do freeaddrinfo on str)