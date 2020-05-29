#ifndef _SQL_CONN_POOL_H
#define _SQL_CONN_POOL_H

#include "sync.h"
#include <mysql/mysql.h>
#include <queue>

class sql_conn_pool {
    int m_num_max;
    int m_num_used;
    locker mutex;
    std::deque<MYSQL *> pool;
    sem reserve;

    sql_conn_pool() = default;
    ~sql_conn_pool();

public:
    bool m_close_log;

    static sql_conn_pool *get_instance();
    void init(const char *, const char *, const char *, const char *, int, int, bool);
    MYSQL *get_conn();
    bool release_conn(MYSQL *);
//    int get_freeconn();
    void destroy_pool();
};

class connection { //RAII
    MYSQL *con;
    sql_conn_pool *pool;
public:
    connection(MYSQL**, sql_conn_pool*);
    ~connection();
};

#endif