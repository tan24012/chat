

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdatomic.h>

#include "User.h"
#include "MThread.h"
#include "Protocol.h"
#include "MultipleTCPSocketsListener.h"

typedef struct {
    pthread_mutex_t login_dispatcher_mutex;
	User* login_users[MAX_PEERS];    // mảng user đã login
	int login_users_count;
    TCPSocket* login_users_sockets[MAX_PEERS];    // mảng socket của user đã login
	int login_users_sockets_count;
	char* all_users[MAX_PEERS];     // mảng user name đã đăng ký
    int all_users_count;
    MThread* mthread;
    atomic_bool status;
} Dispatcher;

void initDispatcher(Dispatcher* dispatcher);
void getAllUsers(Dispatcher* dispatcher);    // Lấy tất cả username đã đăng ký trong hệ thống 
bool checkLogined(Dispatcher* dispatcher, char* userName);    // Kiểm tra xem user đã login trc đó chưa
void add_user(Dispatcher* dispatcher, TCPSocket* sockfd, const char* username);  // Thêm user đã login vào mảng login_users, mở 1 thread để chạy dispatcher nếu chưa chạy
void add_all_users(Dispatcher* dispatcher, const char* username);   // Thêm user mới đăng ký vào mảng all_users
void open_session(Dispatcher* dispatcher, User* user);  // Xử lý yêu cầu mở session giữa 2 user
void close_session(Dispatcher* dispatcher, User* user);  // Xử lý yêu cầu đóng session giữa 2 user
void runDispatcher(void* arg);   // Hàm xử lý các request sau khi login/sign-up thành công
void closeDispatcher(Dispatcher* dispatcher);
void user_exit(current_user);

#endif /* DISPATCHER_H_ */
