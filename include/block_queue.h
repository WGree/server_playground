#ifndef _BLOCK_QUEUE_H
#define _BLOCK_QUEUE_H

#include "sync.h"
#include <queue>
#include <exception>
#include <sys/time.h>

template<class T>
class block_queue {
    locker m_mutex;
    cond m_cond;
    std::queue<T> que;
    int m_mx_size;

public:
    explicit block_queue(int mx_size) {
        if (mx_size <= 0)
            throw std::exception();
        m_mx_size = mx_size;
    }

    ~block_queue() = default;

    void clear() {
        m_mutex.lock();
        std::queue<T> tmp;
        que.swap(tmp);
        m_mutex.unlock();
    }

    bool full() {
        m_mutex.lock();
        bool ret = que.size() >= m_mx_size;
        m_mutex.unlock();
        return ret;
    }

    bool empty() {
        m_mutex.lock();
        bool ret = que.empty();
        m_mutex.unlock();
        return ret;
    }

    int size() {
        m_mutex.lock();
        int ret = que.size;
        m_mutex.unlock();
        return ret;
    }

    int max_size() {
        m_mutex.lock();
        int ret = m_mx_size;
        m_mutex.unlock();
        return ret;
    }

    bool push(const T &msg) {
        m_mutex.lock();
        int ret = false;
        if (!full()) {
            ret = true;
            que.push(msg);
        }
        m_mutex.unlock();
        return ret;
    }

    bool pop(T &msg) {
        m_mutex.lock();
        while (empty()) {
            if (!m_cond.wait(m_mutex)) {
                m_mutex.unlock();
                return false;
            }
        }
        msg = que.front();
        que.pop();
        m_mutex.unlock();
        return true;
    }

    bool pop(T &msg, int mtime) {
        m_mutex.lock();
        timeval now;
        gettimeofday(&now, nullptr);
        if (empty()) {
            timespec t = {now.tv_sec + mtime / 1000, mtime % 1000 * 1000};
            if (!m_cond.timewait(m_mutex, t)) {
                m_mutex.unlock();
                return false;
            }
        }
        if (empty()) {
            m_mutex.unlock();
            return false;
        }
        msg = que.front();
        que.pop();
        m_mutex.unlock();
        return true;
    }
};

#endif
