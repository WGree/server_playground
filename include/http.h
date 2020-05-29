#ifndef _HTTP_H
#define _HTTP_H

#include "sql_conn_pool.h"
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace http
{
    const int FILENAME_MAXLEN = 200;
    const int READ_BUF_SIZE = 2048;
    const int WRITE_BUF_SIZE = 1024;

    const char *ok_200_title = "OK";
    const char *error_400_title = "Bad Request";
    const char *error_400_info = "Your request has bad syntax or is inherently impossible to staisfy.\n";
    const char *error_403_title = "Forbidden";
    const char *error_403_info = "You do not have permission to get file form this server.\n";
    const char *error_404_title = "Not Found";
    const char *error_404_info = "The requested file was not found on this server.\n";
    const char *error_500_title = "Internal Error";
    const char *error_500_info = "There was an unusual problem serving the request file.\n";

    enum METHOD
    {
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE
    {
        CHECL_STATE_REQLINE,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum LINE_STATE
    {
        LINE_OK,
        LINE_BAD,
        LINE_OPEN
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUSET,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    class http_con
    {
        sockaddr_in m_address;
        char *root_dict;
        char m_read_buf[READ_BUF_SIZE];
        char m_write_buf[WRITE_BUF_SIZE];
        char sql_user[100];
        char sql_passwd[100];
        char sql_name[100];
        int m_start_line;
        int m_SQLVerify;
        int m_ETMode;
        int m_close_log;

        int m_sockfd;

        void init();
        HTTP_CODE do_read();
        LINE_STATE prase_line();
        HTTP_CODE prase_req_line(char *);
        HTTP_CODE prase_headers(char *);
        HTTP_CODE prase_content(char *);
        HTTP_CODE do_response();
        bool process_write(HTTP_CODE);
        char *get_line() { return m_read_buf + m_start_line; };
        void unmap();
        bool add_response(const char *, ...);
        bool add_status_line(const char *);
        bool add_headers(const int);
        bool add_content_type();
        bool add_content_length();
        bool add_linger();
        bool add_blank_line();

    public:
        static int m_epollfd;
        static int m_usr_cnt;
        //MYSQL *mysql;
        //bool m_sql_state;
        http_con() {};
        ~http_con() {};
        void init(int, const sockaddr_in&, char*, int, bool, int, char*, char*, char*);
        void close_con(bool real_close);
        void process();
        bool read_once();
        bool write();
        sockaddr_in *get_address()
        {
            return &m_address;
        };
        //TODO SQL
    };

} // namespace http

#endif
