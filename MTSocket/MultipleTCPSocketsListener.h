

#ifndef MULTIPLETCPSOCKETSLISTENER_H_
#define MULTIPLETCPSOCKETSLISTENER_H_

#include <stdio.h>
#include "TCPSocket.h"
#include "Protocol.h"

typedef struct {
    TCPSocket* sockets[MAX_PEERS];  // mảng lưu trữ các TCPSocket đc tạo ra từ accept() để lắng nghe request từ client
} MultipleTCPSocketsListener;

TCPSocket* listenToSocket(MultipleTCPSocketsListener* listener, int count, int timeout);   // Hàm lắng nghe request từ client đầu tiên trên tất cả các TCPSocket trong mảng sockets, trả về TCPSocket nếu có request, ngược lại trả về NULL


#endif /* MULTIPLETCPSOCKETSLISTENER_H_ */
