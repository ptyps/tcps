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

    public:
      Socket() : tcp::Socket() {

      }

      status connect(uint16_t port, pstd::vstring host, net::family family = net::family::Unspecified) {
        auto s = tcp::Socket::connect(port, host, family);

        if (s == tcp::status::FAIL)
          return status::FAIL;

        sid = ssl::wrapOutgoing(id);

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

  class Server {

  };
}
