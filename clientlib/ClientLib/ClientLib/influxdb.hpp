#ifndef INFLUXDB_CPP_HPP
#define INFLUXDB_CPP_HPP

#ifdef WIN32
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS  // To disable deprecated warning for any overlooked cases

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#pragma comment(lib, "ws2_32.lib")

typedef struct iovec {
    void* iov_base;
    size_t iov_len;
} iovec;

inline int64_t writev(int sock, struct iovec* iov, int cnt) {
    // your implementation here
    return 0;  // for placeholder
}

// Helper for IP conversion replacing inet_addr deprecated use

inline unsigned long inet_addr_replacement(const char* cp) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, cp, &(sa.sin_addr));
    if (result == 1)
        return sa.sin_addr.s_addr;
    else
        return INADDR_NONE; // fallback
}

#else
// Linux includes and definitions
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define closesocket close
#endif

namespace influxdb_cpp {

    struct server_info {
        std::string host;
        int port;
        std::string db;
        std::string usr;
        std::string pwd;
        std::string precision;
        std::string token;

        server_info(const std::string& host_, int port_, const std::string& db_ = "", const std::string& usr_ = "",
            const std::string& pwd_ = "", const std::string& precision_ = "ns", const std::string& token_ = "")
            : host(host_), port(port_), db(db_), usr(usr_), pwd(pwd_), precision(precision_), token(token_)
        {
        }
    };

    // The ts_caller class with post_http as non-static member function
    namespace detail {

        class ts_caller {
        public:
            int post_http(const server_info& si, std::string* response, unsigned int timeout_ms);
        };

    } // namespace detail

} // namespace influxdb_cpp

#endif // INFLUXDB_CPP_HPP
