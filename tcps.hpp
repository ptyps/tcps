#pragma once

#include "tcp.hpp"
#include "ssl.hpp"

namespace tcps {
  static debugging::Debug* debug = new debugging::Debug("tcps");

  void enableDebug() {
    debug->enable();
  }

  void disableDebug() {
    debug->disable();
  }

  template <typename ...A>
    void log(pstd::vstring str, A ...args) {
      debug->log(str, args ...);
    }

  // ----

  struct exception : public pstd::exception {
    public:
      template <typename ...A>
        exception(pstd::vstring str, A ...args) : pstd::exception(str, args ...) {

        }
  };

  enum class event {
    ERROR = EOF,
    DISCONNECTED
  };

  enum class status {
    FAIL = EOF,
    OK
  };
  
  // ----

  class Socket : public tcp::Socket {
    friend class Server;

    private:
      SSL* sid;

      // used by Server
      Socket(SSL* sid, std::pair<uint, addrinfo*> pair) : sid(sid), tcp::Socket(pair) {
        log("socket created by server");
      }

    public:
      void operator delete(void* ptr) {
        log("delete operator called");

        auto us = static_cast<Socket*>(ptr);

        std::free(us->sid);
        std::free(us->ai);
      }

      Socket() : tcp::Socket() {
        log("creating socket");
      }

      status connect(uint16_t port, pstd::vstring host, net::family family = net::family::Unspecified) {
        auto s = tcp::Socket::connect(port, host, family);

        if (s == tcp::status::FAIL)
          return status::FAIL;

        sid = ssl::wrap::out(id);

        auto c = ssl::connect(sid);

        if (c == ssl::status::FAIL)
          return status::FAIL;

        log("connection successful");

        return status::OK;
      }

      std::variant<event, std::string> recv() {
        auto out = std::variant<event, std::string>();

        auto vari = ssl::recv(sid);

        if (pstd::has<std::string>(vari))
          out = std::get<std::string>(vari);

        else {
          auto evnt = std::get<ssl::event>(vari);

          if (evnt == ssl::event::DISCONNECTED)
            out = event::DISCONNECTED;

          if (evnt == ssl::event::ERROR)
            out = event::ERROR;
        }

        return out;
      }
  };

  class Server : public tcp::Server {
    private:
      pstd::vstring cert;
      pstd::vstring key;
      SSL_CTX* ctx;
      SSL* sid;

    public:
      static void *operator new      (size_t) = delete;
      static void *operator new[]    (size_t) = delete;

      Server(pstd::vstring cert, pstd::vstring key) : cert(cert), key(key), tcp::Server() {
        log("creating server");
      }

      status bind(uint16_t port, pstd::vstring host, net::family family = net::family::Unspecified) {
        auto b = tcp::Server::bind(port, host, family);

        if (b == tcp::status::FAIL)
          return status::FAIL;

        auto pair = ssl::wrap::server(id);

        sid = std::move(pair.second);
        ctx = std::move(pair.first);

        ssl::loadPEMCert(ctx, cert, key);

        return status::OK;
      }

      auto accept() {
        auto i = net::listen(id);

        if (i == net::status::FAIL)
          throw exception("unable to listen");

        auto pair = net::accept(id, ai);

        auto csid = ssl::wrap::in(pair.first, ctx);

        auto a = ssl::accept(csid);

        if (a == ssl::status::FAIL)
          throw exception("unable to accept ssl client");

        return new Socket(csid, pair);
      }
  };
}
