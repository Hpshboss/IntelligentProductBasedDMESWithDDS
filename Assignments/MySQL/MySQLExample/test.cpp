/* test.cpp */
#include <mysql.h>
#include <iostream>

using namespace std;

int main() {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
 
    const char *server = "localhost";
    const char *user = "root";
    const char *password = "";         /* 此处改成你的密码 */
    const char *database = "mysql";    /* 数据库名字，比如mysql*/
 
    conn = mysql_init(NULL);
 
    /* 连接数据库 */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        cout << mysql_error(conn);
        return -1;
    }
 
    /* 查询语句 */
    if (mysql_query(conn, "insert into sys.teat_table (id, name) values (338, \"DannyO\");")) {
        cout << mysql_error(conn);
        return -2;
    }
 
    res = mysql_use_result(conn);
 
    /* 打印查询结果 */
    while ((row = mysql_fetch_row(res)) != NULL) printf("%s \n", row[0]);
 
    /* 断开连接 */
    mysql_free_result(res);
    mysql_close(conn);
}