#include "http.h"
#include "timer.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

using namespace http;
using namespace utils;

void http_con::init(int sockfd, const sockaddr_in &addr, char *root, int SQLVerify, bool ETMode, int close_log, char *user, char *passwd, char *sqlname) {
    m_sockfd = sockfd;
    m_address = addr;
    addfd(m_epollfd, sockfd, true, ETMode);
    ++m_usr_cnt;
    
    root_dict = root;
    m_SQLVerify = SQLVerify;
    m_ETMode = ETMode;
    m_close_log = close_log;


}

void http_con::close_con(bool real_close = 0) {
    if (real_close && (m_sockfd != -1)) {
        std::cout << "close " << m_sockfd << std::endl;
        rmfd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        --m_usr_cnt;
    }
}
