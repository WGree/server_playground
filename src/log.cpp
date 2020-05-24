#include "log.h"
#include <ctime>

log_t::~log_t() {
    delete m_que;
    if (m_fptr != nullptr)
        fclose(m_fptr);
}

log_t &log_t::get_instance() {
    static log_t instance;
    return instance;
}

void *log_t::reader_thread(void *arg) {
    log_t::get_instance().async_write_log();
}

bool log_t::init(char *file_name, bool close_log, int log_buf_size = 8192, int split_lines = 10000000,
                 int max_queue_size = 0) {
    m_close_log = close_log;
    m_split_lines = split_lines;
    m_log_buf_size = log_buf_size;
    m_buf = new char[m_log_buf_size];

    if (max_queue_size) {
        m_isasync = true;
        m_que = new block_queue<string>(max_queue_size);
        pthread_t thid;  //一个写线程
        pthread_create(&thid, nullptr, reader_thread, nullptr);
    }

    time_t t = time(nullptr);
    tm *sys_time = localtime(&t);
    tm &my_time = *sys_time;

    const char *p = strrchr(file_name, '/');
    char log_full_name[256];
    if (p == nullptr)
        snprintf(log_full_name, 255, "%d-%02d-%02d-%s", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday,
                 file_name);
    else {
        strcpy(log_name, p + 1);
        strncpy(dir_name, file_name, p - file_name + 1);
        snprintf(log_name, 127, "%d-%02d-%02d-%s", my_time.tm_year + 1900, my_time.tm_mon + 1,
                 my_time.tm_mday, log_name);
        snprintf(log_full_name, 255, "%s%s", dir_name, log_name);
    }
    m_today = my_time.tm_mday;
    m_fptr = fopen(log_full_name, "a");
    return m_fptr != nullptr;
}

void log_t::write(int level, char *format, ...) {
    timeval now;
    gettimeofday(&now, nullptr);
    time_t t = now.tv_sec;
    tm *sys_time = localtime(&t);
    tm &my_time = *sys_time;
    char s[16];
    if (level == 3)
        strcpy(s, "[ERROR]: ");
    else if (level == 2)
        strcpy(s, "[WARNN]: ");
    else if (level == 0)
        strcpy(s, "[debug]: ");
    else
        strcpy(s, "[INFOM]: ");
    m_mutex.lock();
    ++m_cnt;
    if (m_today != my_time.tm_mday || m_cnt % m_split_lines == 0) {
        char new_log[256];
        fflush(m_fptr);
        fclose(m_fptr);
        char tail[16];
        snprintf(tail, 16, "%d-%02d-%02d-", my_time.tm_year + 1900, my_time.tm_mon + 1, my_time.tm_mday);
        if (m_today != my_time.tm_mday) {
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today = my_time.tm_mday;
            m_cnt = 0;
        } else
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_cnt / m_split_lines);
        m_fptr = fopen(new_log, "a");
    }
    m_mutex.unlock();

    va_list valist;
    va_start(valist, format);
    m_mutex.lock();
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d: %s ", my_time.tm_year + 1900, my_time.tm_mon + 1,
                     my_time.tm_mday, my_time.tm_hour, my_time.tm_min, my_time.tm_sec, s);
    int m = vsnprintf(m_buf + n, m_log_buf_size - n - 1, format, valist);
    m_buf[n + m] = '\n';
    m_buf[m + m + 1] = 0;
    string log_str(m_buf);
    m_mutex.unlock();

    if (m_isasync && !m_que->full())
        m_que->push(log_str);
    else {
        m_mutex.lock();
        fputs(log_str.c_str(), m_fptr);
        m_mutex.unlock();
    }
    va_end(valist);
}

void log_t::flush() {
    m_mutex.lock();
    fflush(m_fptr);
    m_mutex.unlock();
}

void *log_t::async_write_log() {
    string a_log;
    while (m_que->pop(a_log)) {
        m_mutex.lock();
        fputs(a_log.c_str(), m_fptr);
        m_mutex.unlock();
    }
}