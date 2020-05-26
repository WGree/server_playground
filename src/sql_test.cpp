#include <map>
#include <string>
#include <cstring>
#include <cstdio>
#include <mysql/mysql.h>
#include "sync.h"

using string = std::string;

int main(int argc, char *argv[]) {
    std::map<string, string> users;
    locker mutex;
    if (argc != 7) {
        printf("usage: sqltest uesrname passwd [login|logon] sql_username sql_passwd sql_dbname\n");
        return 3;
    }
    string option(argv[3]);
    int op = 0;
    if (option == "logon") {
        op = 1;
    } else if (option == "login") {
        op = 2;
    }
    if (!op) {
        printf("usage: sqltest uesrname passwd [signin|signon] sql_usernqme sql_passwd sql_dbname\n");
        return 3;
    }
    char *sql_user = argv[4];
    char *sql_passwd = argv[5];
    char *sql_name = argv[6];

    MYSQL *con = mysql_init(con);
    if (con == nullptr) {
        printf("Error: %s\n", mysql_error(con));
        exit(1);
    }
    con = mysql_real_connect(con, "localhost", sql_user, sql_passwd, sql_name, 3306,
                             nullptr, 0);
    if (con == nullptr) {
        printf("Error: %s\n", mysql_error(con));
        exit(11);
    }
    if (mysql_query(con, "SELECT username, passwd FROM userinfo")) {
        printf("INSERT error: %s\n", mysql_error(con));
        exit(2);
    }
    MYSQL_RES *res = mysql_store_result(con);
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        string usr(row[0]);
        string pawd(row[1]);
        users[usr] = pawd;
    }
    mysql_free_result(res);

    string name(argv[1]);
    string passwd(argv[2]);
    if (op == 1) {
        if (users.count(name)) {
            printf("username has been used!\n");
            return 0;
        }

        char sql_insert[200];
        strcpy(sql_insert, "INSERT INTO userinfo(username, passwd) VALUES('");
        strcat(sql_insert, name.c_str());
        strcat(sql_insert, "','");
        strcat(sql_insert, passwd.c_str());
        strcat(sql_insert, "')");
        mutex.lock();
        int res = mysql_query(con, sql_insert);
        mutex.unlock();

        if (res == 0) {
            printf("logon successed!\n");
        } else {
            printf("databae failed!\n");
        }
    } else {
        if (users.count(name) == 0) {
            printf("username doesn't exist!\n");
        } else if (users[name] == passwd) {
            printf("login success!\n");
        } else {
            printf("wrong username or password!\n");
        }
    }
}