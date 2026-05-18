

#ifndef LOGINANDSIGNUP_H_
#define LOGINANDSIGNUP_H_

#include <stdbool.h>

#include "Dispatcher.h"
#include "MThread.h"
// #include "Common.h"
#include "Protocol.h"
#include "MultipleTCPSocketsListener.h"

typedef struct {
    pthread_mutex_t peers_mutex;
    TCPSocket* peers[MAX_PEERS];    // mảng lưu trữ các TCPSocket đc tạo ra từ accept() chưa login
    MultipleTCPSocketsListener* sockets; // chứa mảng lưu trữ các TCPSocket để lắng nghe request từ client
    Dispatcher* dispatcher;  // dispatcher để xử lý các request sau khi login/sign-up thành công
    int peers_count;    // số lượng TCPSocket trong mảng peers
    MThread* mthread;   // thread để xử lý login/sign-up
    atomic_bool  status;    // dừng/tiếp tục thread
} LoginAndSignUp;

void initLoginAndSignUp(LoginAndSignUp* loginAndSign);   // Khởi tạo LoginAndSignUp
void addPeer(LoginAndSignUp* loginAndSign, TCPSocket* peer);    // Thêm các TCPSocket đc tạo ra từ accept() vào mảng peers và tạo thread để xử lý các TCPSocket đó nếu chưa tạo
bool login(Dispatcher* dispatcher, char* username, char* password);   // check đúng user password ko
void remove_peer(LoginAndSignUp* loginAndSign, TCPSocket* peer);    // Xóa TCPSocket khỏi mảng peers
void runLoginAndSignUp(void* arg);   // Hàm xử lý login/sign-up cho các TCPSocket trong mảng peers
void closeLoginAndSignUp(LoginAndSignUp* loginAndSign);

#endif /* LOGINANDSIGNUP_H_ */
