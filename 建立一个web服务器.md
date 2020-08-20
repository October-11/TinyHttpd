# TinyHTTPweb服务器

## 各个函数的作用

   accept_request:  处理从套接字上监听到的一个 HTTP 请求，在这里可以很大一部分地体现服务器处理请求流程。

   bad_request: 返回给客户端这是个错误请求，HTTP 状态吗 400 BAD REQUEST.

   cat: 读取服务器上某个文件写到 socket 套接字。

   cannot_execute: 主要处理发生在执行 cgi 程序时出现的错误。

   error_die: 把错误信息写到 perror 并退出。

   execute_cgi: 运行 cgi 程序的处理，也是个主要函数。

   get_line: 读取套接字的一行，把回车换行等情况都统一为换行符结束。

   headers: 把 HTTP 响应的头部写到套接字。

   not_found: 主要处理找不到请求的文件时的情况。

   sever_file: 调用 cat 把服务器文件返回给浏览器。

   startup: 初始化 httpd 服务，包括建立套接字，绑定端口，进行监听等。

   unimplemented: 返回给浏览器表明收到的 HTTP 请求所用的 method 不被支持。

***



## 1.socklen_t

socklen_t是一种数据类型，它其实和int差不多，在32位机下，size_t和int的长度相同，都是32 bits,但在64位机下，size_t（32bits）和int（64 bits）的长度是不一样的,socket编程中的accept函数的第三个参数的长度必须和int的长度相同。于是便有了socklen_t类型。socklen_t是一种数据类型，它其实和int差不多，在32位机下，size_t和int的长度相同，都是32 bits,但在64位机下，size_t（32bits）和int（64 bits）的长度是不一样的,socket编程中的accept函数的第三个参数的长度必须和int的长度相同。于是便有了socklen_t类型。

***



## 2.sockaddr和sockaddr_in

sockaddr在头文件`#include <sys/socket.h>`中定义，sockaddr的缺陷是：sa_data把目标地址和端口信息混在一起了，如下

```c
struct sockaddr {  
     sa_family_t sin_family;//地址族
　　  char sa_data[14]; //14字节，包含套接字中的目标地址和端口信息               
　　 }; 
```



***\*sockaddr_in\****在头文件`#include<netinet/in.h>或#include <arpa/inet.h>`中定义，该结构体解决了sockaddr的缺陷，把port和addr 分开储存在两个变量中，如下

```c
struct sockaddr_in {
	sa_family_t		sin_family;		//地址族
	uint16_t		sin_port;		//16位TCP/UDP端口号
	struct in_addr	sin_addr;		//32位IP地址
	char			sin_zero[8];	//不使用
}
//该结构体中定义了另一个结构体in_addr
struct in_addr {
    In_addr_t		s_addr;			//32位IPV4地址
}

```

sin_port和sin_addr都必须是网络字节序（NBO），一般可视化的数字都是主机字节序（HBO）。



二者长度一样，都是16个字节，即占用的内存大小是一致的，因此可以互相转化。二者是并列结构，指向sockaddr_in结构的指针也可以指向sockaddr。

**sockaddr**

常用于bind、connect、recvfrom、sendto等函数的参数，指明地址信息，是一种通用的套接字地址。 
**sockaddr_in** 

是internet环境下套接字的地址形式。所以在网络编程中我们会对sockaddr_in结构体进行操作，使用sockaddr_in来建立所需的信息，最后使用类型转化就可以了。一般先把sockaddr_in变量赋值后，强制类型转换后传入用sockaddr做参数的函数：sockaddr_in用于socket定义和赋值；sockaddr用于函数参数

***



## 3.socket函數的用法

```c
int socket(int af, int type, int protocol)
    //af:表示地址族，也就是IP地址類型。AF_INET表示IPV4的地址（127.0.0.1），AF_INET6表示ipv6的地址
    //type:表示數據的傳輸方式/套接字类型，常用的类型主要有sock_stream（流式套接字/面向连接的套接字）和sock_dgram(数据报套接字/无连接套接字)
    //protocol表示传输协议，常用的用IPPROTO_TCP和IPPTOTO_UDP协议，即TCP和UDP
```



```c
int tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //IPPROTO_TCP表示TCP协议
```

这种套接字称为 TCP 套接字。



如果使用 SOCK_DGRAM 传输方式，那么满足这两个条件的协议只有 UDP，因此可以这样来调用 socket() 函数：

```c
int udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  //IPPROTO_UDP表示UDP协议
```

这种套接字称为 UDP 套接字。



上面两种情况都只有一种协议满足条件，可以将 protocol 的值设为 0，系统会自动推演出应该使用什么协议，如下所示：

```c
int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);  //创建TCP套接字
int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);  //创建UDP套接字
```

***



## 4.htons()函数的使用方法

网络字节顺序NBO(Network Byte Order): 按从高到低的顺序存储，在网络上使用统一的网络字节顺序，可以避免兼容性问题。

机字节顺序(HBO，Host Byte Order): 不同的机器HBO不相同，与CPU设计有关，数据的顺序是由cpu决定的,而与操作系统无关。

htons函数就是将整型变量从主机字节顺序转变成网络字节顺序，就是==整数在地址空间的存储方式变成高字节存放在内存的低地址处==

例如：

```c
如 Intel x86结构下, short型数0x1234表示为34 12, int型数0x12345678表示为78 56 34 12  
如 IBM power PC结构下, short型数0x1234表示为12 34, int型数0x12345678表示为12 34 56 78
```

```c++
void main {
	int a = 16, b;
	b = htons(a);
	cout << "a=" << endl;
	cout << "b=" << endl;
}

//运行结果
a = 16;
b = 4096;

/**
	* 16的16进制为0x0010,4096的为0x1000.intel机器为小端模式，因此0x0010存储时变成1000.
	* 存储4096时变成0010.因此发送网络包时为了报文中的数据为0010，需要使用htons进行字节转换
**/
```

网络字节和本地字节顺序之间的转换函数：

```c
htonl()--"将主机的无符号长整形数转换成网络字节顺序"
ntohl()--"将一个无符号长整形数从网络字节顺序转换为主机字节顺序"
htons()--"将主机的无符号短整形数转换成网络字节顺序"
ntohs()--"将一个无符号短整形数从网络字节顺序转换为主机字节顺序"
```

***



## 5.setsocketopt（）和getsocketopt（）函数

获取或者设置与某个套接字关联的选项，操作socket选项时，选项位于的层和选项的名称必须给出

```c
int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);
```

**sock**——————表示将要被设置或者选项的socket

**level**——————选项所在的协议层。

**optname**————想要访问的选项名

**optval**—————对于getsockopt（），指向返回选项值得缓冲。对于setsockopt（），指向包含新选项值得缓冲

**optlen**：————对于getsockopt（），作为入口参数时，选项得最大长度。作为出口参数时，选项值得实际长度

​								对于setsockopt（），==optval的长度==



成功执行时返回0，失败返回-1.

**errno被设为以下某个值：**

EBADF:sock不是有效的文件描述词。

EFAULT: optval指向的内存并非有效的进程空间

EINVAL：在调用setsockopt()时，optlen无效

ENOPROTOPT：指定的协议层不能识别选项

ENOTSOCK：sock描述的不是套接字。



**level指定控制套接字的层次**，可以取三种值：

1）SOL_SOCKET:通用套接字选项

2）IPPROTO_IP:IP选项

3）IPPROTO_TCP:TCP选项

optname指定控制的方式（选项的名称），我们下面详细解释：

optval获得或者是设置套接字选项，根据选项名称的数据类型类型进行转换



### 1.**SO_REUSEADDR**

一般来说，一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用

`SO_REUSEADDR`用于对TCP套接字处于TIME_WAIT状态下的socket，才可以重复绑定使用
server程序总是应该在**调用bind()之前设置SO_REUSEADDR套接字选项**
TCP，先调用close()的一方会进入TIME_WAIT状态



**SO_REUSEADDR提供如下四个功能：**

- 允许启动一个监听服务器并捆绑其众所周知端口，即使以前建立的将此端口用做他们的本地端口的连接仍存在。这通常是重启监听服务器时出现，若不设置此选项，则bind时将出错
- 允许在同一端口上启动同一服务器的多个实例，只要每个实例捆绑一个不同的本地IP地址即可。对于TCP，我们根本不可能启动捆绑相同IP地址和相同端口号的多个服务器。
- 允许单个进程捆绑同一端口到多个套接口上，只要每个捆绑指定不同的本地IP地址即可。这一般不用于TCP服务器。
- SO_REUSEADDR允许完全重复的捆绑：
  当一个IP地址和端口绑定到某个套接口上时，还允许此IP地址和端口捆绑到另一个套接口上。一般来说，这个特性仅在支持多播的系统上才有，而且只对UDP套接口而言（TCP不支持多播）。



使用这两个套接口选项的建议：

- 在所有TCP服务器中，在调用bind之前设置SO_REUSEADDR套接口选项；
- 当编写一个同一时刻在同一主机上可运行多次的多播应用程序时，设置SO_REUSEADDR选项，并将本组的多播地址作为本地IP地址捆绑

### 2.time-wait

**TIME_WAIT状态有两个存在的理由：**

- 可靠地实现TCP全双工连接的终止
- 允许老的重复分节在网络中消逝

1、如果服务器最后发送的ACK因为某种原因丢失了，那么客户一定会重新发送FIN，这样因为有TIME_WAIT的存在，服务器会重新发送ACK给客户，如果没有TIME_WAIT，那么无论客户有没有收到ACK，服务器都已经关掉连接了，此时客户重新发送FIN，服务器将不会发送ACK，而是RST，从而使客户端报错。也就是说，TIME_WAIT有助于可靠地实现TCP全双工连接的终止。

2、如果没有TIME_WAIT，我们可以在最后一个ACK还未到达客户的时候，就建立一个新的连接。那么此时，如果客户收到了这个ACK的话，就乱套了，必须保证这个ACK完全死掉之后，才能建立新的连接。也就是说，TIME_WAIT允许老的重复分节在网络中消逝。



### 3.SO_REUSEPORT

目前常见的网络编程模型就是多进程或多线程，根据accpet的位置，分为如下场景
2种场景

- 单进程或线程创建socket，并进行listen和accept，接收到连接后创建进程和线程处理连接
- 单进程或线程创建socket，并进行listen，预先创建好多个工作进程或线程accept()在同一个服务器套接字

这两种模型解充分发挥了多核CPU的优势,虽然可以做到线程和CPU核绑定，但都会存在：

- 单一listener工作进程或线程在高速的连接接入处理时会成为瓶颈
- 多个线程之间竞争获取服务套接字
- 缓存行跳跃
- 很难做到CPU之间的负载均衡
- 随着核数的扩展，性能并没有随着提升



**SO_REUSEPORT解决了什么问题**

SO_REUSEPORT支持多个进程或者线程绑定到同一端口，提高服务器程序的性能，解决的问题：

- 允许多个套接字 bind()/listen() 同一个TCP/UDP端口
- 每一个线程拥有自己的服务器套接字
- 在服务器套接字上没有了锁的竞争
- 内核层面实现负载均衡
- 安全层面，监听同一个端口的套接字只能位于同一个用户下面

***



## 6.bind()函数

```c
int bind(int socket, sockaddr* address, uint addrlen)
```



-  将一个地址和一个端口号绑定到一个socket连接上
- *socket*:之前创建的socket
- *sockaddr*:一个用来存放Ip地址和端口号的结构体
-  *addrlen*:上述结构体的长度
-  返回值：为-1表示失败，若端口被占用，会从新绑定一个随机端口（仍返回失败）
-  地址绑定为0表示绑定本机所有IP

***



## 7.listen()：仅TCP和服务器使用

将一个socket设置为监听状态，专门用来监听的socket叫master socket

```c
int listen(int socket, int maxconn)
//socket,要聆听的目标
//最大接受连接数
```

将maxconn定为10, 当有15个连接请求的时候，前面10个连接请求就被放置在请求队列中，后面5个请求被拒绝。

***



## 8.intptr_t和uintptr_t

```c
/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef long int		intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned long int	uintptr_t;
#else
# ifndef __intptr_t_defined
typedef int			intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned int		uintptr_t;
#endif
```

在64位的机器上，**intptr_t**和**uintptr_t**分别是**long int**、**unsigned long in**t的别名；在32位的机器上，**intptr_t**和**uintptr_t**分别是**int**、**unsigned int**的别名。

那么为什么要用typedef定义新的别名呢？我想主要是为了提高程序的可移植性（在32位和64位的机器上）。很明显，上述代码会根据宿主机器的位数为**intptr_t**和**uintptr_t**适配相应的数据类型。

***



## 9.HTTP的请求方法

**GET**

GET方法请求一个指定资源的表示形式. 使用GET的请求应该只被用于获取数据.

**HEAD**

HEAD方法请求一个与GET请求的响应相同的响应，但没有响应体.

**POST**

POST方法用于将实体提交到指定的资源，通常导致在服务器上的状态变化或副作用.

**PUT**

PUT方法用请求有效载荷替换目标资源的所有当前表示。

**DELETE**

DELETE方法删除指定的资源。

**CONNECT**

CONNECT方法建立一个到由目标资源标识的服务器的隧道。

**OPTIONS**

OPTIONS方法用于描述目标资源的通信选项。

**TRACE**

TRACE方法沿着到目标资源的路径执行一个消息环回测试。

**PATCH**

PATCH方法用于对资源应用部分修改。

***



## 10.URL和PATH

URL是http协议下的概念，是互联网上任一资源的指针，所以它是相对主机而言的地址，可以是本站上的资源，也可以是其他主机上的资源。http协议规定，所有的web request都以url的格式给出：
http://localhost/images/user/007/avatar.jpg

对应的文件系统路径是：
C:/xampp/htdocs/images/user/007/avatar.jpg
其实web 服务器也接受file path作为请求，只不过会转换协议：file:///C:/xampp/htdocs/images/user/007/avatar.jpg

（注意：因为file path和url都可以定位一个文件，有的文件操作函数比如fopen接受多协议参数：file path，或http url，有的函数只接受path，比如is_file,file_exists）

 http协议对url的句法做了详细规定：

所以上例url中，==path==段就是/images/user/007/avatar.jpg，web
服务器（apache等）会将这个path映射到文件系统中的file path中去，以获取C:/xampp/htdocs/images/user/007/avatar.jpg

```
<scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
```

所以上例url中，<path>段就是/images/user/007/avatar.jpg，web
服务器（apache等）会将这个path映射到文件系统中的file path中去，以获取C:/xampp/htdocs/images/user/007/avatar.jpg



## 11.HTTP请求报文

http请求报文由请求行、消息报头和请求正文组成。

```c
Request Line<CRLF> //请求行
Header-Name: header-value<CRLF>  //消息报头，一个或者多个
Header-Name: header-value<CRLF>
...
<CRLF> //空行
body//请求正文
1234567
```

​	第一行为http请求行，包含方法，URI 和http版本

​	2-7为请求头，包含浏览器，主机，接受的编码方式和压缩方式

​	第8行表示一个空行 表示请求头结束 这个空行是必须的

​	第9行是数据体，比如是需要查询的信息。

1. 请求行:

   *Method  Request_URI HTTP-Version*

   以方法符为开头，空格分开，后面跟着请求URI和协议版本。其中请求方法有GET、POST、HEAD、PUT、DELETE、TRACE、CONNECT和OPTIONS等。
   GET：请求获取Request-URI所标识资源

   *GET /form.html HTTP/1.1*

   POST: 请求服务器接受附在后面的数据。

   *POST /reg.jsp HTTP/*

2. 消息报头：
   由关键字/值对组成，每行一对
   User-Agent : 产生请求的浏览器类型
   Accept : 客户端希望接受的数据类型，比如 Accept：text/xml（application/json）表示希望接受到的是xml（json）类型
   Content-Type：发送端发送的实体数据的数据类型。
   比如，Content-Type：text/html（application/json）表示发送的是html类型。
   Host : 请求的主机名，允许多个域名同处一个IP地址，即虚拟主机

***



## 11.HTTP响应报文

http响应体由三部分组成：

http响应由三个部分组成分别是状态行，响应头，响应正文。

状态行是由：HTTP-Version+Status-Code+Reason-Phrase

比如：HTTP/1.1 200 ok

分别表示http版本 + 状态码 + 状态代码的文本描述

状态码：

 

| 1xx  | 指示信息–表示请求已接收，继续处理         |
| ---- | ----------------------------------------- |
| 2xx  | 成功–表示请求已被成功接收、理解、接受     |
| 3xx  | 重定向–要完成请求必须进行更进一步的操作。 |
| 4xx  | 客户端错误–请求有语法错误或请求无法实现。 |
| 5xx  | 服务器端错误–服务器未能实现合法的请求。   |

 

响应头：包含服务器类型，日期，长度，内容类型等

Server:Apache Tomcat/5.0.12

Date:Mon,6Oct2003 13:13:33 GMT

Content-Type:text/html

Last-Moified:Mon,6 Oct 2003 13:23:42 GMT

Content-Length:112

 响应正文响应正文就是服务器返回的HTML页面或者json数据

***



## 12.ISspace(原型：extern int isspace（int c）

  用法：#include <ctype.h>

  功能：判断字符c是否为空白符

  说明：当c为空白符时，返回非零值，否则返回零。（空白符指空格、水平制表、垂直制表、换页、回车和换行符。）函数

***



## 13.stat函数

通过文件名path获取文件信息，并保存在buf所指的结构体stat中

```
int stat(const char* path, struct stat* buf)
```

执行成功则返回0，失败返回-1，错误代码存于errno（需要include <errno.h>)

```c
struct stat
{
    dev_t     st_dev;     /* ID of device containing file */文件使用的设备号
    ino_t     st_ino;     /* inode number */    索引节点号 
    mode_t    st_mode;    /* protection */  文件对应的模式，文件，目录等
    nlink_t   st_nlink;   /* number of hard links */    文件的硬连接数  
    uid_t     st_uid;     /* user ID of owner */    所有者用户识别号
    gid_t     st_gid;     /* group ID of owner */   组识别号  
    dev_t     st_rdev;    /* device ID (if special file) */ 设备文件的设备号
    off_t     st_size;    /* total size, in bytes */ 以字节为单位的文件容量   
    blksize_t st_blksize; /* blocksize for file system I/O */ 包含该文件的磁盘块的大小   
    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */ 该文件所占的磁盘块  
    time_t    st_atime;   /* time of last access */ 最后一次访问该文件的时间   
    time_t    st_mtime;   /* time of last modification */ /最后一次修改该文件的时间   
    time_t    st_ctime;   /* time of last status change */ 最后一次改变该文件状态的时间   
}
```

***



## 14.send()函数

作用：用来发送信息到socket中。（仅连接时可用）

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);

// 下面的两个函数等效：
write（sockfd, buf, le）；
send(sockfd, buf, len, 0)；
```

send和write的区别在于最后一个参数flag，当flag为0时，send和write一样

ssize_t————————相当于long

sockfd————————接收消息的套接字的文件描述符

buf——————————要发送的消息

len——————————要发送的字节数

flags——————————f表示下列标志中的0个或多个

MSG_CONFIRM ：用来告诉链路层，

MSG_DONTROUTE：不要使用网关来发送数据，只发送到直接连接的主机上。通常只有诊断或者路由程序会使用，这只针对路由的协议族定义的，数据包的套接字没有。

MSG_DONTWAIT ：启用非阻塞操作，如果操作阻塞，就返回EAGAIN或EWOULDBLOCK

MSG_EOR ：当支持SOCK_SEQPACKET时，终止记录。

MSG_MORE ：调用方有更多的数据要发送。这个标志与TCP或者udp套接字一起使用

MSG_NOSIGNAL ：当另一端中断连接时，请求不向流定向套接字上的错误发送SIGPIPE ，EPIPE 错误仍然返回。

MSG_OOB：在支持此概念的套接字上发送带外数据（例如，SOCK_STREAM类型）；底层协议还必须支持带外数据

***



## 15.RECV()函数

用于消息的接收

```c
int recv(SOCKET s, char *buf, int len, int flags)
//参数一：指定接收端套接字描述符；
//参数二：指向一个缓冲区，该缓冲区用来存放recv函数接收到的数据；
//参数三：指明buf的长度；
//参数四：一般置为0；
//返回值：失败时，返回值小于0；超时或对端主动关闭，返回值等于0；成功时，返回值是返回接收数据的长度。
//一般flag直接置零
```

**MSG_PEEK**标志会将套接字接收队列中的可读的数据拷贝到缓冲区，但不会使套接子接收队列中的数据减少，常见的是：例如调用recv或read后，导致套接字接收队列中的数据被读取后而减少，而指定了MSG_PEEK标志，可通过返回值获得可读数据长度，并且不会减少套接字接收缓冲区中的数据，所以可以供程序的其他部分继续读取。

***



## 16.常见的HTTP返回内容

1、Http/1.1 200 OK 访问正常
 表示成功访问,为网站可正常访问时的状态。

2、Http/1.1 301 Moved Permanently 永久重定向
 对搜索引擎相对友好的跳转方式，当网站更换域名时可将原域名作301永久重定向到新域名，原域名权重可传递到新域名，也常有将不含www的域名301跳转到含www的，如xxx.com通过301跳转到www.xxx.com 

3、Http/1.1 302 Found 为临时重定向
 易被搜索引擎判为作弊,比如asp程序的response.Redirect()跳转、js跳转或静态http跳转。

4、Http/1.1 400 Bad Request 域名绑定错误
 一般是服务器上域名未绑定成功，未备案等情况。

5、Http/1.1 403 Forbidden 没有权限访问此站
 你的IP被列入黑名单，连接的用户过多，可以过后再试，网站域名解析到了空间，但空间未绑定此域名等情况。

6、Http/1.1 404 Not Found 文件或目录不存在
 表示请求文件、目录不存在或删除，设置404错误页时需确保返回值为404。常有因为404错误页设置不当导致不存在的网页返回的不是404而导致搜索引擎降权。

7、Http/1.1 500 Internal Server Error 程序或服务器错误
 表示服务器内部程序错误，出现这样的提示一般是程序页面中出现错误，如小的语法错误，数据连接故障等。
Http状态码一览表

所谓的404页就是服务器404重定向状态返回页面。数字404指的是404号状态码。一般常用到的有200号状态码和404号状态码。200号表示网页被下载成功，而404号表示不能成功下载并产生错误。下面是HTTP状态码一览表。

1xx：请求收到，继续处理
2xx：操作成功收到，分析、接受
3xx：完成此请求必须进一步处理
4xx：请求包含一个错误语法或不能完成
5xx：服务器执行一个完全有效请求失败

100——客户必须继续发出请求
101——客户要求服务器根据请求转换HTTP协议版本

200——交易成功
201——提示知道新文件的URL
202——接受和处理、但处理未完成
203——返回信息不确定或不完整
204——请求收到，但返回信息为空
205——服务器完成了请求，用户代理必须复位当前已经浏览过的文件
206——服务器已经完成了部分用户的GET请求

300——请求的资源可在多处得到
301——删除请求数据
302——在其他地址发现了请求数据
303——建议客户访问其他URL或访问方式
304——客户端已经执行了GET，但文件未变化
305——请求的资源必须从服务器指定的地址得到
306——前一版本HTTP中使用的代码，现行版本中不再使用
307——申明请求的资源临时性删除

400——错误请求，如语法错误
401——请求授权失败
402——保留有效ChargeTo头响应
403——请求不允许
404——没有发现文件、查询或URl
405——用户在Request-Line字段定义的方法不允许
406——根据用户发送的Accept拖，请求资源不可访问
407——类似401，用户必须首先在代理服务器上得到授权
408——客户端没有在用户指定的饿时间内完成请求
409——对当前资源状态，请求不能完成
410——服务器上不再有此资源且无进一步的参考地址
411——服务器拒绝用户定义的Content-Length属性请求
412——一个或多个请求头字段在当前请求中错误
413——请求的资源大于服务器允许的大小
414——请求的资源URL长于服务器允许的长度
415——请求资源不支持请求项目格式
416——请求中包含Range请求头字段，在当前请求资源范围内没有range指示值，请求也不包含If-Range请求头字段
417——服务器不满足请求Expect头字段指定的期望值，如果是代理服务器，可能是下一级服务器不能满足请求

500——服务器产生内部错误
501——服务器不支持请求的函数
502——服务器暂时不可用，有时是为了防止发生系统过载
503——服务器过载或暂停维修
504——关口过载，服务器使用另一个关口或服务来响应用户，等待时间设定值较长
505——服务器不支持或拒绝支请求头中指定的HTTP版本

***



## 17.strcasecmp()函数

忽略大小写比较字符串

```c
strcasecmp（const char* s1, const char* s2）
```

s1 和 s2 相同则返回0，s1 > s2 则返回1， s1 < s2 则返回-1

***



## 18.atoi()

字符串转整型

***



## 19.dup()和dup2()

1. dup()函数

   头文件和函数定义

```c
#include <unistd.h>
int dup(int oldfd);
```

- dup用来复制参数oldfd所指的文件描述符。当复制成功是，返回最小的尚未被使用过的文件描述符，若有错误则返回-1.错误代码存入errno中返回的新文件描述符和参数oldfd指向同一个文件，这两个描述符共享同一个数据结构，共享所有的锁定，读写指针和各项全现或标志位。

```c
int dup2(int oldfd, int newfd);
```

***



## 20.putenv()\getenv()

**putenv()**

头文件：

```
#include4<stdlib.h>
```

定义函数：

```
int` `putenv(``const` `char` `* string);
```

函数说明：putenv()用来改变或增加环境变量的内容. 参数string 的格式为name＝value, 如果该环境变量原先存在, 则变量内容会依参数string 改变, 否则此参数内容会成为新的环境变量.

返回值：执行成功则返回0, 有错误发生则返回-1.

**getenv()**

头文件：

```
#include <stdlib.h>
```

定义函数：

```
char` `* ``getenv``(``const` `char` `*name);
```

函数说明：getenv()用来取得参数name 环境变量的内容. 参数name 为环境变量的名称, 如果该变量存在则会返回指向该内容的指针. 环境变量的格式为name＝value.

返回值：执行成功则返回指向该内容的指针, 找不到符合的环境变量名称则返回NULL.

***



## 21.execl()

```c
int execl(const char * path,const char * arg,....);
```

函数说明  execl()用来执行参数path字符串所代表的文件路径，接下来的参数代表执行该文件时传递过去的argv(0)、argv[1]……，**最后一个参数必须用空指针(NULL)作结束。**

返回值   **如果执行成功则函数不会返回，执行失败则直接返回-1，失败原因存于errno中。**

***



## 22.read()和write()

**write()**

```
头文件：#include<unistd.h>
```

**原型：**

```c
ssize_t write(int fd,const void*buf,size_t count);
参数说明：
  fd:是文件描述符（write所对应的是写，即就是1）
  buf:通常是一个字符串，需要写入的字符串
  count：是每次写入的字节数
//将buf中的数据写入fd所指的文件中
```

**返回值：**

```c
 成功：返回写入的字节数
 失败：返回-1并设置errno
  ps： 写常规文件时，write的返回值通常等于请求写的字节
       数count， 而向终端设备或者网络写时则不一定
```

**read()**

```c
头文件：#include<unistd.h>
功能：用于从文件描述符对应的文件读取数据（从打开的设备或文件中读取数据）

1234
```

**原型：**

```c
ssize_t read(int fd,void*buf,size_t count)
参数说明：
fd:      是文件描述符
buf:     为读出数据的缓冲区；
count:   为每次读取的字节数（是请求读取的字节数，读上来的数据保
         存在缓冲区buf中，同时文件的当前读写位置向后移）
//将fd的文件内容读到buf中
```

**返回值：**

```c
 成功：返回读出的字节数
 失败：返回-1，并设置errno，如果在调用read
       之前到达文件末尾，则这次read返回0

12345
```

代码：

```c
#include<stdio.h>
#include<string.h>
#include<unistd.h>
int main()
{
   const char*msg="hello\n";
   int len = strlen(msg);
   write(1,msg,len);//write所对应的文件描述符为1
   char buf[1024]={0};
   read(0,buf,len);//read所对应的文件描述符为0
   return 0;
}
```

***



## 23.accept()函数

```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

头文件：

```c
<sys/types.h>
<sys/socket.h>
```

  addr用于存放客户端的地址，addrlen在调用函数时被设置为addr指向区域的长度，在函数调用结束后被设置为实际地址信息的长度。本函数会阻塞等待知道有客户端请求到达。

**返回值是一个新的套接字描述符，它代表的是和客户端的新的连接，可以把它理解成是一个客户端的socket,这个socket包含的是客户端的ip和port信息 。（当然这个new_socket会从sockfd中继承 服务器的ip和port信息，两种都有了），而参数中的**SOCKET  s**包含的是服务器的ip和port信息 。**

***



## 24.pthread_create()

**头文件**

```c
　　#include<pthread.h>
```

**函数声明**

```c
#include <pthread.h>
int pthread_create(
                 pthread_t *restrict tidp,   //新创建的线程ID指向的内存单元。
                 const pthread_attr_t *restrict attr,  //线程属性，默认为NULL
                 void *(*start_rtn)(void *), //新创建的线程从start_rtn函数的地址开始运行
                 void *restrict arg //默认为NULL。若上述函数需要参数，将参数放入结构中并将地址作为arg传入。
                  );
```

**返回值**

　　若成功则返回0，否则返回出错编号

**参数**

　　第一个参数为指向线程[标识符](http://baike.baidu.com/view/390932.htm)的指针。

　　第二个参数用来设置线程属性。

　　第三个参数是线程运行函数的地址。

　　最后一个参数是运行函数的参数。

**注意**

　　在编译时注意加上-lpthread参数，以调用静态链接库。因为pthread并非Linux系统的默认库。

​		创建成功则返回0；不成功则不为0；

***



## 25.waitpid()

 作用同于wait，但可指定pid进程清理，可以不阻塞。

```c++
pid_t waitpid(pid_t pid,int *status,int options);
```

成功：返回清理掉的子进程ID；失败：-1（无子进程）

特殊参数和返回情况：

**参数pid：**

​    \>0 回收指定ID的子进程

​    -1 回收任意子进程（相当于wait）

​    0 回收和当前调用waitpid一个组的所有子进程

​    < -1 回收指定进程组内的任意子进程

返回0：参数3为WNOHANG，且子进程正在运行。

注意：一次wait或waitpid调用只能清理一个子进程，清理多个子进程需要用到循环

**status**

用来保存被收集进程退出时的一些状态，它是一个指向int类型的指针。但如果我们对这个子进程是如何死掉的毫不在意，只想把这个僵尸进程消灭掉，（事实上绝大多数情况下，我们都会这样想），我们就可以设定这个参数为NULL 

**options** 
options提供了一些额外的选项来控制waitpid，目前在Linux中只支持WNOHANG和WUNTRACED两个选项，这是两个常数，可以用”|”运算符把它们连接起来使用 
WNOHANG： 如果没有任何已经结束的子进程则马上返回，不予以等待。 
WUNTRACED ：如果子进程进入暂停执行情况则马上返回，但结束状态不予以理会。

***



## 26.pipe()

管道是一种最基本的IPC机制，作用于有血缘关系的进程之间，完成数据传递。调用pipe系统函数即可创建一个管道。有如下特质：

1. 其本质是一个伪文件(实为内核缓冲区)

2. 由两个文件描述符引用，一个表示读端，一个表示写端。

3. 规定数据从管道的写端流入管道，从读端流出。

管道的原理: 管道实为内核使用环形队列机制，借助内核缓冲区(4k)实现。

管道的局限性：

① 数据自己读不能自己写。

② 数据一旦被读走，便不在管道中存在，不可反复读取。

③ 由于管道采用半双工通信方式。因此，数据只能在一个方向上流动。

④ 只能在有公共祖先的进程间使用管道。

常见的通信方式有，单工通信、半双工通信、全双工通信。

**创建管道**

```
  int pipe(int pipefd[2]); 成功：0；失败：-1，设置errno
```

函数调用成功返回r/w两个文件描述符。无需open，但需手动close。规定：fd[0] → r； fd[1] → w，就像0对应标准输入，1对应标准输出一样。向管道文件读写数据其实是在读写内核缓冲区。

***



## 27.read()

```c
ssize_t read(int fd, void *buf, size_t count);
```

**fd**： 这是一个已经打开的文件句柄，表示从这个文件句柄所代表的文件读取数据。

**buf**： 指缓冲区，即读取的数据会被放到这个缓冲区中去。

**count**： 表示调用一次read操作，应该读多少数量的字符。

1. 大于0：成功读取的字节数；

2. 等于0：到达文件尾；

3. -1：发生错误，通过errno确定具体错误值