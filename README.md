Example

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
