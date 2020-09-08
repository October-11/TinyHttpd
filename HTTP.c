#include <stdio.h>
#include <winsock.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

#define ISspace(x) isspace(int x)
#define STDIN  0
#define STDOUT 1
#define STDERR 2

void accept_request(* arg);
void startup(u_short port);
void error_die(const char*);
void execute_cgi(int, const char*, const char*, const char*);
int getline(int, const char*, int);
void cat(int, FILE*);
void unimplemented(int);

/*********************************************************************/
/*  先计算post方式下的content-length的长度，通过管道实现进程间的通信，
    子进程负责设置环境变量并执行cgi程序，父进程负责读取和返回内容。      */
/*********************************************************************/

void execute_cgi(int client, char* path, char* method, char* query_string) {
    char* buf[1024];
    int cgi_output[2];                                                      //设置管道
    int cgi_input[2];
    pid_t pid;
    int status;
    int i;
    char c;
    int numchars = 1;
    int content_len = -1;

    buf[0] = 'A';
    buf[1] = '\0';
    if (strcasecmp("GET", method) == 0) {                                   //如果是get方式，则循环放弃
        while ((numchars > 0) && strcmp("\n", buf)) {
            numchars = getline(client, buf, sizeof(buf));
        }
    }
    else if (strcasecmp("POST", method) == 0) {                             //如果是POST方式，需要找到content字段
        numchars = getline(client, buf, sizeof(buf));
        while (numchars > 0 && strcmp("\n", buf)) {
            buf[15] = '\0';
            if (strcasecmp("content-length:", buf) == 0) {
                content_len = atoi(&(buf[16]));
            }
            numchars = getline(client, buf, sizeof(buf));
        }
        if (content_len == -1) {
            bad_request(client);
            return;
        }
    }
    else {

    }

    if (pipe(cgi_output) == -1) {
        cannot_execute(client);
        return;
    }
    if (pipe(cgi_input) == -1) {
        cannot_execute(client);
        return;
    }
    if ((pid = fork()) < 0) {
        cannot_execute(client);
        return;
    }
    sprintf(buf, "HTTP/1.0 200 ok\r\n");
    send(client, buf, strlen(buf), 0);

    if (pid == 0) {                                         //处理子进程
        char meth_env[255];
        char query_env[255];
        char length_env[255];
        
        dup2(cgi_input[0], STDIN);
        dup2(cgi_output[1], STDOUT);
        close(cgi_input[1]);
        close(cgi_output[0]);

        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);
        if (strcasecmp("GET", method) == 0) {
            sprintf(query_env. "QUERT_STRING=%s", query_string);
            putenv(query_env);
        }
        else {
            sprintf(length_env, "CONTENT_LENGTH=%d", content_len);
            putenv(length_env);
        }
        execl(path, NULL);
        exit(0);
    }
    else {                                              //父进程执行
        close(cgi_input[0]);
        close(cgi_output[1]);
        if (strcasecmp("POST", method) == 0) {
            for (i = 0; i < content_len; i++) {
                recv(client, &c, 1, 0);
                write(cgi_input[1], &c);
            }
        }
        while (read(cgi_output[0], &c, 1) > 0) {        //如果成功读取了信息，则发送到客户端
            send(client, &c, 1, 0);
        }
        close(cgi_input[1]);
        close(cgi_output[0]);
        waitpid(pid, &status, 0);
    }
}

/************************************
 * 处理HTTP报文，判断请求方法，拆解URL，
 * 运行CGI程序
************************************/

void accept_request(* arg) {
    int client = (intptr_t)arg;
    char* buf[1024];                                                        //设置缓冲区
    size_t numchars;                                                        //存放http请求行的大小
    char* method[255];
    char* url[255];
    char* path[512];
    size_t i, j;                                                            //用来提取method和url
    struct stat st;                                                         //存放path表示的文件
    int cgi = 0;                                                            
    char* query_string = NULL;                                              //存放get方法中？后面的索引
    
    numbchars = getline(client, buf, sizeof(buf));
    i = 0;
    j = 0;
    while (!ISspace(buf[i]) && (i < sizeof(method) - 1)) {
        method[i] = buf[i];
        i++;
    }
    j = i;
    method[i] = '\0';

    if (strcasecmp("POST", method) && strcasecmp("GET", method)) {
        unimplemented(client);
        return;
    }

    if (strcasecmp("POST", method) == 0) {
        cgi = 1;                                                            //如果是post方法则执行cgi程序
    }
    i = 0;
    while (ISspace(buf[j] && j < numchars)) {
        j++;                                                                //去除buf首部的空格，因为mehod和url之间由空格隔开
    }
    while (!ISspace(buf[j]) && (j < sizeof(url) - 1)) {                     //提取url
        url[i] = buf[j];
        i++;
        j++;
    }
    url[i] = '\0';

    if (strcasecmp("GET", method) == 0) {                                  //如果是get方式，将索引取出
        query_string = url;
        while ((* query_string != '?') && (* query_string) != '\0') {
            query_string++;
        }
        if ((* query_string) == '?') {
            * query_string = '\0';
            query_string++;
            cgi = 1;
        }
    }
    sprintf(path, "htdoc%s", url);
    if (path[strlen[path - 1] == '/']) {
        strcat(path, "index.html")
    }
    if (stat(path, &st) < 0) {
        while ((numchars > 0) && strcmp("\n", buf)) {
            numchars = getline(client, buf, sizeof(buf));
        }
        not_found(client);
    }
    else {
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            strcat(path, "/index.html");
        }
        if ((st.st_mode & S_IRUSR) || (st.st_mode & S_IRGRP) || (st.st_mode & S_IROTH)) {       //如果文件具有可执行权限
            cgi = 1;
        }
        
        if (cgi == 0) {
            serve_file(client, path);                                       //如果不需要cgi程序，则直接将客户端文件返回
        }
        else {
            execute_cgi(client, path, method, query_string);
        }

        close(client);
    }
}

/*********************************
 创建socket，主要有以下几步：
 * 创建socket文件描述符
 * 设置端口复用
 * 绑定端口号（如果没有合适的端口号则自动获取一个端口号）
 * 设置监听
**********************************/

void startup(u_short port) {
    int httpd = 0;
    int on = 1;
    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1) {
        error_die("socket");
    }
    memset(&name, 0, sizeof(name));
    name.sinfamily = AF_INET;                                               //IPV4地址
    name.sin_port = htons(*port);                                           //端口号
    name.sin_addr.s_addr = htonl(INADDR_ANY);                               //ip地址
    if (setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        error_die("setsockopt");
    }
    if (bind(httpd, (struct sockaddr_in*)&name, sizeof(name)) < 0) {
        error_die("bind");
    }
    if (* port == 0) {                                                      //如果端口已被占用，则需要动态分配一个端口
        socklen_t namelen = sizeof(name);
        if (getsockname(httpd, (struct sockaddr_in*)&name, namelen) < 0) {
            error_die("getsockname");
        }
        * port = ntohs(name.sin_port);
    }
    if (listen(httpd, 5) < 0) {
        error_die("listen");
    }
    return httpd;
}

/*****************************
 * 主函数负责建立服务器socket
 * 通过accept函数去响应客户端
 * 的连接，对于每个连接建立一个
 * 新的线程进行连接。
*****************************/

int main() {
    int sock_server = -1;
    u_short port = 4000;
    int sock_client = -1;
    struct sockaddr_in client_name;
    socketlen_t client_name_len = sizeof(client_name);
    pthread_t newthread;

    sock_server = startup(& port);                  //建立监听socket
    printf("httpd are running on port %d\n", port);

    while (1) {
        sock_client = accept(sock_server, (struct sockaddr_in*)&client_name, client_name_len);      //建立一个socket进行通信
        if (sock_client == -1) {
            error_die("accept");
        }
        if (pthread_create(&newthread, NULL, (void*)accept_request, (void*)(intptr_t)sock_client) != 0) {   //建立一个新线程进行解析
            perror("pthread_create");
        }
    }
    close(sock_server);
    return 0;
}


/*****************************************************************/
/*  getline()函数用来抓取HTTP报文中一行字符，返回本行中字符的数量   */
/*****************************************************************/


int getline(int sock, char* buf, int size) {
    int i = 0;
    char c = '\0';
    int n;
    while (i < size - 1 && c != '\n') {
        n = recv(sock, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                n = recv(sock, &c, 1, MSG_PEEK);
                if (n > 0 && c == '\n') {
                    recv(sock, &c, 1, 0);
                }
                else {
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        }
        else {
            c = '/n';
        }
    }
    buf[i] = '\0';
    return i;
}


void bad_request(int client)                //HTTP无法执行
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);(
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
}


/*****************************************************/
/*           将服务器上的文件写入client中             */
/*****************************************************/


void cat(int client, FILE* resource) {
    char buf[1024];

    fgets(buf, sizeof(buf), resource);
    while (!feof(resource)) {
        send(cilent, buf, sizeof(buf)， 0)；
        fgets(buf, sizeof(buf), resource);
    }
}


/*****************************************************/
/*                  输出错误原因                      */
/*****************************************************/


void error_die(const char *sc)
{
    perror(sc);                 //输出错误原因
    exit(1);
}


void headers(int client, const char *filename)
{
    char buf[1024];
    (void)filename;  /* could use filename to determine file type */

    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}


void not_found(int client)                  //客户端未找到
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}


void serve_file(int client, const char *filename)
{
    FILE *resource = NULL;
    int numchars = 1;
    char buf[1024];

    buf[0] = 'A'; buf[1] = '\0';                  //读取后丢弃header
    while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
        numchars = get_line(client, buf, sizeof(buf));

    resource = fopen(filename, "r");            //打开文件
    if (resource == NULL)
        not_found(client);
    else
    {
        headers(client, filename);              //将文件发送到客户端
        cat(client, resource);
    }
    fclose(resource);
}


void unimplemented(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}