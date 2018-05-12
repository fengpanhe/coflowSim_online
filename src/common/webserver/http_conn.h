#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "../lib/ThreadPool.h"
#include "../lib/locker.h"
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>

class RequestHandler;
class DefaultRequestHandler;

class http_conn : public ThreadClass {
public:
  static const int FILENAME_LEN = 200;       // 文件名的最大长度
  static const int READ_BUFFER_SIZE = 2048;  // 读缓存区的大小
  static const int WRITE_BUFFER_SIZE = 1024; // 写缓存区的大小
  // http的请求方法
  enum METHOD {
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
  };
  // 解析用户请求时，主机状态机所处的状态
  enum CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER,
    CHECK_STATE_CONTENT
  };
  // 服务器处理http请求的可能的中间结果
  enum HTTP_CODE {
    NO_REQUEST,
    YES_REQUEST,
    BAD_REQUEST,
    NO_RESOURCE,
    FORBIDDEN_REQUEST,
    FILE_REQUEST,
    POST_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
  };
  // 行的读取状态
  enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

public:
  http_conn() = default;
  ~http_conn();
  // 初始化新接受的连接
  void init(int sockfd, const sockaddr_in &addr);
  // 关闭连接
  void close_conn(bool real_close = true);
  // 处理客户连接
  void run() override;
  // 非阻塞读
  bool read();
  // 非阻塞写
  bool write();

  // 得到 url
  const char *getUrl();
  // 设置GET请求要访问的相对路径
  bool setFileRelativePath(const char *file_path);
  // 得到请求消息的响应主体
  const char *getContent(int &content_len_);
  // 填充响应消息的主体
  bool setResponseContent(const char *content);

  // 添加对匹配的url的处理对象
  static bool addHandler(const char *match_url,
                         RequestHandler *request_handler);
  //  bool deleteHandler(const char * match_url);
  static bool setDocRootPath(const char *root_path);

  static int m_epollfd;
  static int m_user_count;
  static RequestHandler *default_request_handler_;
  static std::vector<std::pair<char *, RequestHandler *>> handlers_;
  static char *doc_root_;

private:
  // 初始化连接
  void init();
  // 解析http请求
  HTTP_CODE process_read();
  // 填充http应答
  bool process_write(HTTP_CODE ret);
  // 以下一组函数被process_read 调用以分析 http 请求
  HTTP_CODE parse_request_line(char *text);
  HTTP_CODE parse_headers(char *text);
  HTTP_CODE parse_content(char *text);
  HTTP_CODE do_request();
  char *get_line() { return read_buf_ + start_line_; }
  LINE_STATUS parse_line();
  // 以下一组函数被process_write 调用以填充 http 应答
  void unmap();
  bool add_response(const char *format, ...);
  bool add_content(const char *content);
  bool add_status_line(int status, const char *title);
  bool add_headers(int content_length);
  bool add_content_length(int content_length);
  bool add_linger();
  bool add_blank_line();

  int sockfd_{};
  sockaddr_in address_{};
  // 读缓存区
  char read_buf_[READ_BUFFER_SIZE]{};
  // 读缓存区下一个字节的位置
  int read_idx_{};
  // 当前正在分析的字符在读缓存区的位置
  int checked_idx_{};
  // 当前正在解析的行的起始位置
  int start_line_{};
  // 写缓存区
  char write_buf_[WRITE_BUFFER_SIZE]{};
  // 写缓存区的字节数
  int write_idx_{};

  // 主机状态机当前所处的状态
  CHECK_STATE check_state_;
  // 请求方法
  METHOD method_;

  // 客户端请求的目标文件的完整路径
  char real_file_[FILENAME_LEN]{};
  // 目标文件的相对路径
  char file_relative_path_[FILENAME_LEN]{};
  // 客户请求的目标文件的文件名
  char *url_{};
  // http协议的版本号
  char *version_{};
  // 主机名
  char *host_{};
  // 请求正文的内容
  char *content_{};
  // http 请求正文的长度
  int content_length_{};
  // http请求是否要求保持连接
  bool linger_{};

  // 客户请求的目标文件被mmap到内存中的起始位置
  char *file_address_{};
  // 响应的主体内容的起始地址
  char *response_content_address_{};
  // 响应的主体内容的起始地址
  int response_content_length_;
  // 目标文件的状态，通过此可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息
  struct stat file_stat_ {};
  // 使用writev来执行写操作，iv_count表示被写内存块的数量
  struct iovec iv_[2]{};
  int iv_count_{};
};

class RequestHandler {
public:
  // 函数名为相应的request 类型，要发送的信息放到 iovec
  // 结构体中，url为解析到的url信息，root_path 为网站根目录
  virtual bool get(http_conn *http_c) = 0;
  virtual bool post(http_conn *http_c) = 0;
};

class DefaultRequestHandler : public RequestHandler {
public:
  bool get(http_conn *http_c) override {
    http_c->setFileRelativePath(http_c->getUrl());
  }
  bool post(http_conn *http_c) override { http_c->setResponseContent("{}"); }
};
#endif
