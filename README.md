# TinyHttpd
一个开源的web服务器开发小项目，学习了其中的基本代码并自己实现了一遍，并将其中的基本函数整理了出来

**来源：**

官网:[http://tinyhttpd.sourceforge.net](http://tinyhttpd.sourceforge.net/)

**学习目的：**

- 基本的socket编程原理、步骤和实现方法
- 网络编程的函数使用
- HTTP协议内容，请求报文和响应报文的生成
- 了解多线程并发
- 了解父子进程的分工协作以及基本的通信方式
- 管道的实现和工作原理
- CGI解析协议

**基本函数的作用**

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

**编程中所遇到的操作函数及解析包含在文件夹中**