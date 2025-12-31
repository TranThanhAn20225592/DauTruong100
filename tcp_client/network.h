#ifndef NETWORK_H
#define NETWORK_H

void flushSocket(int sockfd);
int recvProcessor(int sockfd, char *buffer, int bufferSize);

#endif

