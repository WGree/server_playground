#ifndef _LOG_H
#define _LOG_H

#include "sync.h"
#include "block_queue.h"
#include <string>
#include <cstring>
#include <cstdarg>

class log_t {
    using string = std::string;

    char dir_name[128];
    char log_name[128];
    int m_split_lines;
    int m_log_buf_size;
    long long m_cnt;
    int m_today;
    FILE *m_fptr;
    char *m_buf;
    block_queue<string> *m_que;
    locker m_mutex;
    bool m_isasync;
    bool m_close_log;

    log_t() = default;
    ~log_t();

    void *async_write_log(); //异步写入磁盘

public:
    static log_t &get_instance();  //单例模式
    static void *reader_thread(void *);  //读者线程
    bool init(char *, bool, int, int, int);
    void write(int, char *, ...);
    void flush();
};

#define LOG_DEBUG(format, ...) if(m_close_log == false) {log_t &it = log_t::get_instance(); it.write(0, format, ##__VA_ARGS__); it.flush();}
#define LOG_INFO(format, ...) if(m_close_log == false) {log_t &it = log_t::get_instance(); it.write(1, format, ##__VA_ARGS__); it.flush();}
#define LOG_WARNNING(format, ...) if(m_close_log == false) {log_t &it = log_t::get_instance(); it.write(2, format, ##__VA_ARGS__); it.flush();}
#define LOG_ERROR(format, ...) if(m_close_log == false) {log_t &it = log_t::get_instance(); it.write(3, format, ##__VA_ARGS__); it.flush();}

#endif
