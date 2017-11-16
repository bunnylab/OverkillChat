#ifndef CTRCODES_H
#define CTRCODES_H

#define LIST_AVAILABLE "#a:"
#define LIST_CONNECTED "#l:"
#define CONNECT_TO "#c:"
#define DISCONNECT_PEER "#p:"
#define DISCONNECT "#d:"
#define MESSAGE "#m:"

// List Available Peers
void listAvailable(int *sd, int *client_socket, char *buffer, int max_clients, int max_buffer);

// List all peers currently connected to
//listConnected(&client_socket, &buffer, max_clients, max_buffer);

// Connect to a peer
//connectTo(&client_socket, &buffer, max_clients, max_buffer);

// Disconnect from a peer
//disconnectPeer(&client_socket, &buffer, max_clients, max_buffer);

// Disconnect from the server
//disconnect(&client_socket, &buffer, max_clients, max_buffer);

// Send a message to your peers
//message(&client_socket, &buffer, max_clients, max_buffer);


#endif /* CTRCODES_H */
