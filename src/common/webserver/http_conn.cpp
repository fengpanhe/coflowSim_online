#include "http_conn.h"
#include "../lib/epollFunctions.h"
#include "../lib/locker.h"
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form =
    "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form =
    "You do not have permission to get file from this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form =
    "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form =
    "There was an unusual problem serving the requested file.\n";
// const char *doc_root_ = "../html";

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;
RequestHandler *http_conn::default_request_handler_ =
    new DefaultRequestHandler();
std::vector<std::pair<char *, RequestHandler *>> http_conn::handlers_;
char *http_conn::doc_root_ = nullptr;

void http_conn::close_conn(bool real_close) {
  if (real_close && (sockfd_ != -1)) {
    // modfd( m_epollfd, sockfd_, EPOLLIN );
    removefd(m_epollfd, sockfd_);
    sockfd_ = -1;
    m_user_count--;
  }
}

void http_conn::init(int sockfd, const sockaddr_in &addr) {
  sockfd_ = sockfd;
  address_ = addr;
  int error = 0;
  socklen_t len = sizeof(error);
  getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &error, &len);
  int reuse = 1;
  setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  addfd(m_epollfd, sockfd, true);
  m_user_count++;

  init();
}

void http_conn::init() {
  check_state_ = CHECK_STATE_REQUESTLINE;
  linger_ = false;

  method_ = GET;
  url_ = nullptr;
  version_ = nullptr;
  content_length_ = 0;
  host_ = nullptr;
  start_line_ = 0;
  checked_idx_ = 0;
  read_idx_ = 0;
  write_idx_ = 0;
  memset(read_buf_, '\0', READ_BUFFER_SIZE);
  memset(write_buf_, '\0', WRITE_BUFFER_SIZE);
  memset(real_file_, '\0', FILENAME_LEN);
}

http_conn::LINE_STATUS http_conn::parse_line() {
  char temp;
  for (; checked_idx_ < read_idx_; ++checked_idx_) {
    temp = read_buf_[checked_idx_];
    if (temp == '\r') {
      if ((checked_idx_ + 1) == read_idx_) {
        return LINE_OPEN;
      } else if (read_buf_[checked_idx_ + 1] == '\n') {
        read_buf_[checked_idx_++] = '\0';
        read_buf_[checked_idx_++] = '\0';
        return LINE_OK;
      }

      return LINE_BAD;
    } else if (temp == '\n') {
      if ((checked_idx_ > 1) && (read_buf_[checked_idx_ - 1] == '\r')) {
        read_buf_[checked_idx_ - 1] = '\0';
        read_buf_[checked_idx_++] = '\0';
        return LINE_OK;
      }
      return LINE_BAD;
    }
  }

  return LINE_OPEN;
}

bool http_conn::read() {
  if (read_idx_ >= READ_BUFFER_SIZE) {
    return false;
  }

  int bytes_read = 0;
  while (true) {
    bytes_read = static_cast<int>(
        recv(sockfd_, read_buf_ + read_idx_,
             static_cast<size_t>(READ_BUFFER_SIZE - read_idx_), 0));
    if (bytes_read == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      return false;
    } else if (bytes_read == 0) {
      return false;
    }

    read_idx_ += bytes_read;
  }
  return true;
}

http_conn::HTTP_CODE http_conn::parse_request_line(char *text) {
  url_ = strpbrk(text, " \t");
  if (!url_) {
    return BAD_REQUEST;
  }
  *url_++ = '\0';

  char *method = text;
  if (strcasecmp(method, "GET") == 0) {
    method_ = GET;
  } else if (strcasecmp(method, "POST") == 0) {
    method_ = POST;
  } else if (strcasecmp(method, "HEAD") == 0) {
    method_ = HEAD;
  } else if (strcasecmp(method, "PUT") == 0) {
    method_ = PUT;
  } else {
    return BAD_REQUEST;
  }

  url_ += strspn(url_, " \t");
  version_ = strpbrk(url_, " \t");
  if (!version_) {
    return BAD_REQUEST;
  }
  *version_++ = '\0';
  version_ += strspn(version_, " \t");
  if (strcasecmp(version_, "HTTP/1.1") != 0) {
    return BAD_REQUEST;
  }

  if (strncasecmp(url_, "http://", 7) == 0) {
    url_ += 7;
    url_ = strchr(url_, '/');
  }

  if (!url_ || url_[0] != '/') {
    return BAD_REQUEST;
  }

  check_state_ = CHECK_STATE_HEADER;
  return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_headers(char *text) {
  if (text[0] == '\0') {
    //  若content_length_不为0，则转为到CHECK_STATE_CONTENT状态
    if (content_length_ != 0) {
      check_state_ = CHECK_STATE_CONTENT;
      return NO_REQUEST;
    }
    return YES_REQUEST;
  } else if (strncasecmp(text, "Connection:", 11) == 0) {
    text += 11;
    text += strspn(text, " \t");
    if (strcasecmp(text, "keep-alive") == 0) {
      linger_ = true;
    }
  } else if (strncasecmp(text, "Content-Length:", 15) == 0) {
    text += 15;
    text += strspn(text, " \t");
    content_length_ = static_cast<int>(atol(text));
  } else if (strncasecmp(text, "Host:", 5) == 0) {
    text += 5;
    text += strspn(text, " \t");
    host_ = text;
  } else {
    printf("oop! unknow header %s\n", text);
  }

  return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_content(char *text) {
  //  printf("content:%s\n", text);
  if (read_idx_ >= (content_length_ + checked_idx_)) {
    text[content_length_] = '\0';
    return YES_REQUEST;
  }
  return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::process_read() {
  LINE_STATUS line_status = LINE_OK;
  HTTP_CODE ret = NO_REQUEST;
  char *text = nullptr;

  while (((check_state_ == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) ||
         ((line_status = parse_line()) == LINE_OK)) {
    text = get_line();
    start_line_ = checked_idx_;
    printf("got 1 http line: %s\n", text);

    switch (check_state_) {
    case CHECK_STATE_REQUESTLINE: {
      ret = parse_request_line(text);
      if (ret == BAD_REQUEST) {
        return BAD_REQUEST;
      }
      break;
    }
    case CHECK_STATE_HEADER: {
      ret = parse_headers(text);
      if (ret == BAD_REQUEST) {
        return BAD_REQUEST;
      } else if (ret == YES_REQUEST) {
        return do_request();
      }
      this->content_ = text + 2;
      break;
    }
    case CHECK_STATE_CONTENT: {
      ret = parse_content(text);
      if (ret == YES_REQUEST) {
        printf("parse_content\n");
        return do_request();
      }
      line_status = LINE_OPEN;
      break;
    }
    default: { return INTERNAL_ERROR; }
    }
  }

  return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::do_request() {
  switch (method_) {
  case GET: {

    default_request_handler_->get(this);
    for (auto &it : handlers_) {
      if (strcasecmp(it.first, this->url_) == 0) {
        it.second->get(this);
        break;
      }
    }

    strcpy(real_file_, doc_root_);
    auto len = static_cast<int>(strlen(doc_root_));
    strncpy(real_file_ + len, this->file_relative_path_,
            static_cast<size_t>(FILENAME_LEN - len - 1));
    if (stat(real_file_, &file_stat_) < 0) {
      return NO_RESOURCE;
    }

    if (!(file_stat_.st_mode & S_IROTH)) {
      return FORBIDDEN_REQUEST;
    }

    if (S_ISDIR(file_stat_.st_mode)) {
      return BAD_REQUEST;
    }

    int fd = open(real_file_, O_RDONLY);
    file_address_ =
        (char *)mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
    break;
  }
  case POST: {
    default_request_handler_->post(this);
    for (auto &it : handlers_) {
      if (strcasecmp(it.first, this->url_) == 0) {
        it.second->post(this);
        break;
      }
    }
    return POST_REQUEST;
    break;
  }
  default: { return BAD_REQUEST; }
  }
}

void http_conn::unmap() {
  if (file_address_) {
    munmap(file_address_, static_cast<size_t>(file_stat_.st_size));
    file_address_ = nullptr;
  }
  if (this->response_content_address_) {
    delete response_content_address_;
    response_content_address_ = nullptr;
    response_content_length_ = 0;
  }
}

bool http_conn::write() {
  int temp = 0;
  int bytes_have_send = 0;
  int bytes_to_send = write_idx_;
  if (bytes_to_send == 0) {
    modfd(m_epollfd, sockfd_, EPOLLIN);
    init();
    return true;
  }

  while (true) {
    temp = static_cast<int>(writev(sockfd_, iv_, iv_count_));
    if (temp <= -1) {
      if (errno == EAGAIN) {
        modfd(m_epollfd, sockfd_, EPOLLOUT);
        return true;
      }
      unmap();
      return false;
    }

    bytes_to_send -= temp;
    bytes_have_send += temp;
    if (bytes_to_send <= bytes_have_send) {
      unmap();
      if (linger_) {
        init();
        modfd(m_epollfd, sockfd_, EPOLLIN);
        return true;
      } else {
        modfd(m_epollfd, sockfd_, EPOLLIN);
        return false;
      }
    }
  }
}

bool http_conn::add_response(const char *format, ...) {
  if (write_idx_ >= WRITE_BUFFER_SIZE) {
    return false;
  }
  va_list arg_list;
  va_start(arg_list, format);
  int len = vsnprintf(write_buf_ + write_idx_,
                      static_cast<size_t>(WRITE_BUFFER_SIZE - 1 - write_idx_),
                      format, arg_list);
  if (len >= (WRITE_BUFFER_SIZE - 1 - write_idx_)) {
    return false;
  }
  write_idx_ += len;
  va_end(arg_list);
  return true;
}

bool http_conn::add_status_line(int status, const char *title) {
  return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool http_conn::add_headers(int content_len) {
  add_content_length(content_len);
  add_linger();
  add_blank_line();
}

bool http_conn::add_content_length(int content_len) {
  return add_response("Content-Length: %d\r\n", content_len);
}

bool http_conn::add_linger() {
  return add_response("Connection: %s\r\n", linger_ ? "keep-alive" : "close");
}

bool http_conn::add_blank_line() { return add_response("%s", "\r\n"); }

bool http_conn::add_content(const char *content) {
  return add_response("%s", content);
}

bool http_conn::process_write(HTTP_CODE ret) {
  switch (ret) {
  case INTERNAL_ERROR: {
    add_status_line(500, error_500_title);
    add_headers(static_cast<int>(strlen(error_500_form)));
    if (!add_content(error_500_form)) {
      return false;
    }
    break;
  }
  case BAD_REQUEST: {
    add_status_line(400, error_400_title);
    add_headers(static_cast<int>(strlen(error_400_form)));
    if (!add_content(error_400_form)) {
      return false;
    }
    break;
  }
  case NO_RESOURCE: {
    add_status_line(404, error_404_title);
    add_headers(static_cast<int>(strlen(error_404_form)));
    if (!add_content(error_404_form)) {
      return false;
    }
    break;
  }
  case FORBIDDEN_REQUEST: {
    add_status_line(403, error_403_title);
    add_headers(static_cast<int>(strlen(error_403_form)));
    if (!add_content(error_403_form)) {
      return false;
    }
    break;
  }
  case FILE_REQUEST: {
    add_status_line(200, ok_200_title);
    if (file_stat_.st_size != 0) {
      add_headers(static_cast<int>(file_stat_.st_size));
      iv_[0].iov_base = write_buf_;
      iv_[0].iov_len = static_cast<size_t>(write_idx_);
      iv_[1].iov_base = file_address_;
      iv_[1].iov_len = static_cast<size_t>(file_stat_.st_size);
      iv_count_ = 2;
      return true;
    } else {
      const char *ok_string = "<html><body></body></html>";
      add_headers(static_cast<int>(strlen(ok_string)));
      if (!add_content(ok_string)) {
        return false;
      }
    }
  }
  case POST_REQUEST: {
    add_status_line(200, ok_200_title);
    if (response_content_length_ != 0) {
      add_headers(response_content_length_);
      iv_[0].iov_base = write_buf_;
      iv_[0].iov_len = static_cast<size_t>(write_idx_);
      iv_[1].iov_base = response_content_address_;
      iv_[1].iov_len = static_cast<size_t>(response_content_length_);
      iv_count_ = 2;
      return true;
    } else {
      const char *ok_string = "<html><body></body></html>";
      add_headers(static_cast<int>(strlen(ok_string)));
      if (!add_content(ok_string)) {
        return false;
      }
    }
  }
  default: { return false; }
  }

  iv_[0].iov_base = write_buf_;
  iv_[0].iov_len = static_cast<size_t>(write_idx_);
  iv_count_ = 1;
  return true;
}

void http_conn::run() {
  HTTP_CODE read_ret = process_read();
  if (read_ret == NO_REQUEST) {
    modfd(m_epollfd, sockfd_, EPOLLIN);
    return;
  }

  bool write_ret = process_write(read_ret);
  if (!write_ret) {
    close_conn();
  }

  modfd(m_epollfd, sockfd_, EPOLLOUT);
}
const char *http_conn::getUrl() { return this->url_; }
bool http_conn::setFileRelativePath(const char *file_path) {
  if (FILENAME_LEN >= strlen(file_path)) {
    strcpy(file_relative_path_, file_path);
    return true;
  }
  return false;
}
bool http_conn::addHandler(const char *match_url,
                           RequestHandler *request_handler) {
  auto *url = new char[strlen(match_url)];
  strcpy(url, match_url);
  handlers_.emplace_back(url, request_handler);
  return true;
}
http_conn::~http_conn() {
  for (auto &it : handlers_) {
    delete it.first;
    delete it.second;
  }
  if (default_request_handler_ != nullptr) {
    delete default_request_handler_;
  }

  if (doc_root_ != nullptr) {
    delete doc_root_;
    doc_root_ = nullptr;
  }
}
const char *http_conn::getContent(int &content_len_) {
  content_len_ = this->content_length_;
  return this->content_;
}
bool http_conn::setResponseContent(const char *content) {
  response_content_length_ = static_cast<int>(strlen(content));
  response_content_address_ = new char[response_content_length_];
  strcpy(this->response_content_address_, content);
  return true;
}
bool http_conn::setDocRootPath(const char *root_path) {
  if (doc_root_ != nullptr) {
    delete doc_root_;
  }
  doc_root_ = new char[strlen(root_path)];
  strcpy(doc_root_, root_path);
  return true;
}
