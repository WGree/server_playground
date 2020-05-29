#include "timer.h"
#include "http.h"
#include <cstdio>
#include <exception>
#include <csignal>
#include <unistd.h>
#include <sys/epoll.h>

void time_wheel::tick() {
    std::list<timer *> &now = slot[cur_slot];
    time_t cur = time(nullptr);
    int cnt = 0;
    while (!now.empty()) {
        timer &tmp = *now.front();
        if (cur < tmp.expire) {
            break;
        }
        tmp.cb_func(tmp.data);
        now.pop_front();
        ++cnt;
    }
    cur_slot = ++cur_slot % slot.size();
}

void time_wheel::add_timer(timer &tm) {
    int dst_slot = tm.expire / si % slot.size();
    std::list<timer *> &now = slot[dst_slot];
    for (auto it = now.begin(); it != now.end(); ++it) {
        if ((*it)->expire >= tm.expire) {
            now.emplace(it, &tm);
            return;
        }
    }
    now.emplace_back(&tm);
}

void time_wheel::del_timer(timer &tm) {
    int dst_slot = tm.expire / si % slot.size();
    std::list<timer *> &now = slot[dst_slot];
    for (auto it = now.begin(); it != now.end(); ++it) {
        if (*it == &tm) {
            now.erase(it);
        }
    }
}

void time_wheel::adj_timer(timer &tm) {
    int dst_slot = tm.expire / si % slot.size();
    std::list<timer *> &now = slot[dst_slot];
    for (auto it = now.begin(); it != now.end(); ++it) {
        if (*it == &tm) {
            now.emplace(it, &tm);
        }
    }
}

void utils::init(int timeslot, int interval) {
    tw.init(timeslot, interval);
}

int utils::set_nonblock(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, new_option);
    return old_option;
}

void utils::addfd(int epoll_fd, int fd, bool oneshot, bool ETMode){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if (ETMode)
        event.events |= EPOLLET;
    if (oneshot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_nonblock(fd);
}

void utils::rmfd(int epoll_fd, int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void utils::modfd(int epoll_fd, int fd, int ev, bool ETMode) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLIN | EPOLLRDHUP;
    if (ETMode)
        event.events |= EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void utils::sig_handler(int sig) {
    int old_errno = errno;
    send(pipefd[1], (char *)&sig, 1, 0);
    errno = old_errno;
}

void utils::addsig(int sig, bool restart) {
    struct sigaction sa = {};
    sa.sa_handler = sig_handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(sig, &sa, nullptr) == -1) {
        throw std::exception();
    }
}

void utils::timer_handler() {
    tw.tick();
    alarm(tw.si);
}

void send_error(int connfd, const char *info) {
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

void utils::cb_timer(client_data *data) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, data->sockfd, nullptr);
    close(data->sockfd);
    //--http::m_user_cnt;
}