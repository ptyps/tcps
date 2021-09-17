Server Example

```cpp
#include "tcps.hpp"

#include <thread>

int main() {
  tcps::enableDebug();
  tcp::enableDebug();
  ssl::enableDebug();
  net::enableDebug();

  auto key = std::string("/path/to/privkey.pem");
  auto cert = std::string("/path/to/cert.pem");
  
  auto srv = tcps::Server(cert, key);

  srv.bind(6697, "localhost");

  while (!0) {
    auto inc = srv.accept();

    auto thread = std::thread([&]() {
      while (!0) {
        auto vari = inc->recv();

        if (pstd::has<tcps::event>(vari)) {
          break;
        }

        else {
          auto recvd = std::get<std::string>(vari);
          pstd::log(recvd);
        }
      }

      delete inc;
    });

    thread.detach();
  }
}
```

Client Example

```cpp
#include "tcps.hpp"

int main() {
  tcps::enableDebug();
  tcp::enableDebug();
  ssl::enableDebug();
  net::enableDebug();
  
  auto soq = tcps::Socket();

  auto i = soq->connect(6697, "irc.libera.chat");

  if (i == tcps::status::FAIL)
    throw pstd::exception("unable to connect");

  while (!0) {
    auto vari = soq->recv();

    if (pstd::has<tcps::event>(vari)) {
      auto evnt = std::get<tcps::event>(vari);
      pstd::log("event recieved: %i %i", evnt, errno);
      break;
    }

    else {
      auto recvd = std::get<std::string>(vari);
      pstd::log(recvd);
    }
  }
  
  delete soq;
}
```
