//
// Created by he on 5/9/18.
//

#ifndef WEB_SERVER_CPP_INDEXHANDLER_H
#define WEB_SERVER_CPP_INDEXHANDLER_H
#include "webserver/http_conn.h"
class IndexRequestHandler : public RequestHandler {
public:
  bool get(http_conn *http_c) override {
    http_c->setFileRelativePath("/index.html");
  }
  bool post(http_conn *http_c) override {}
};
#endif // WEB_SERVER_CPP_INDEXHANDLER_H
