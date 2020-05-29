#ifndef _TIMER_H
#define _TIMER_H

#include "log.h"
#include <netinet/in.h>
#include <fcntl.h>
#include <ctime>
#include <vector>
#include <list>

struct client_data {
    sockaddr_in addr;
    int sockfd;
};

class timer {
public:
    time_t expire;
    client_data *data;
    void (*cb_func)(client_data *);
};

class time_wheel {  //TODO: maybe add heap later
    int cur_slot;
    std::vector<std::list<timer *>> slot;

public:
    int si;

    void init(int n = 60, int interval = 5) {
        slot.resize(n);
        si = interval;
    }
    void tick();
    void add_timer(timer &);
    void del_timer(timer &);
    void adj_timer(timer &);
};

namespace utils {
    static int pipefd[2];
    static int epfd;
    time_wheel tw;

    void init(int, int);
    int set_nonblock(int);
    void addfd(int, int, bool, bool);
    void rmfd(int, int);
    void modfd(int, int, int, bool);
    void sig_handler(int);
    void addsig(int, bool restart);
    void timer_handler();
    void send_error(int connfd, const char *);
    void cb_timer(client_data *);
}

#endif
