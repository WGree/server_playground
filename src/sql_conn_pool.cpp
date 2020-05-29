#include "sql_conn_pool.h"
#include "log.h"
#include <cstring>

sql_conn_pool::~sql_conn_pool() {
    destroy_pool();
}

sql_conn_pool *sql_conn_pool::get_instance() {
    static sql_conn_pool instance;
    return &instance;
}

void sql_conn_pool::init(const char *url, const char *user, const char *passwd, const char *dbname,
        int port, int maxconn, bool close_log) {
    m_num_max = maxconn;
    m_close_log = close_log;

    for (int i = 0; i < maxconn; ++i) {
        MYSQL *con = mysql_init(nullptr);
        if (con == nullptr) {
            LOG_ERROR("MySQL_connection_pool initializion Error");
            exit(111);
        }
        con = mysql_real_connect(con, url, user, passwd, dbname, port, nullptr, 0);
        if (con == nullptr) {
            LOG_ERROR("MySQL_connection_pool initializion Error");
            exit(112);
        }
        pool.push_back(con);
    }
    reserve = sem(maxconn);
}

MYSQL *sql_conn_pool::get_conn() {
    mutex.lock();
    if (pool.empty()) {
        mutex.unlock();
        return nullptr;
    }
    reserve.wait();
    MYSQL *con = pool.front();
    pool.pop_front();
    ++m_num_used;
    mutex.unlock();
    return con;
}

bool sql_conn_pool::release_conn(MYSQL *con) {
    if (con == nullptr)
        return true;
    mutex.lock();
    pool.push_back(con);
    --m_num_used;
    reserve.post();
    mutex.unlock();
    return true;
}

//int sql_conn_pool::get_freeconn() {
//    return pool.size();
//}

void sql_conn_pool::destroy_pool() {
    mutex.lock();
    while (!pool.empty()) {
        mysql_close(pool.front());
        pool.pop_front();
    }
    mutex.unlock();
}

connection::connection(MYSQL **sql, sql_conn_pool *conpool) {
    *sql = conpool->get_conn();

    con = *sql;
    pool = conpool;
}

connection::~connection() {
    pool->release_conn(con);
}
