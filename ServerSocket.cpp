/*
 * Copyright ©2024 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2024 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <arpa/inet.h>   // for inet_ntop()
#include <errno.h>       // for errno, used by strerror()
#include <netdb.h>       // for getaddrinfo()
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <unistd.h>      // for close(), fcntl()
#include <cstdio>        // for snprintf()
#include <cstring>       // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.hpp"

namespace searchserver {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::bind_and_listen(int* listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // NOTE: You only have to support IPv6, you do not have to support IPv4

  // TODO: implement
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;
  hints.ai_flags = AI_PASSIVE;
  // hints.ai_flags |= AI_V4MAPPED;
  hints.ai_socktype = 0;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  // Should return 0

  char port_str[10];
  snprintf(port_str, 10, "%u", port_);
  int res = getaddrinfo(nullptr, port_str, &hints, &result);
  // return false if getaddrinfo() failed
  if (res != 0) {
    // std::cerr << "getaddrinfo() failed: ";
    std::cerr << gai_strerror(res) << std::endl;
    return false;
  }

  for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
    listen_sock_fd_ = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    // std::cout << "listen_sock_fd_: " << listen_sock_fd_ << std::endl;

    if (listen_sock_fd_ == -1) {
      // Creating this socket failed.  So, loop to the next returned
      // result and try again.
      std::cerr << "socket() failed: " << strerror(errno) << std::endl;
      listen_sock_fd_ = 0;
      continue;
    }

    // Configure the socket; we're setting a socket "option."  In
    // particular, we set "SO_REUSEADDR", which tells the TCP stack
    // so make the port we bind to available again as soon as we
    // exit, rather than waiting for a few tens of seconds to recycle it.
    int optval = 1;
    setsockopt(listen_sock_fd_, SOL_SOCKET, SO_REUSEADDR, &optval,
               sizeof(optval));

    // Try binding the socket to the address and port number returned
    // by getaddrinfo().
    if (bind(listen_sock_fd_, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Bind worked
      break;
    }

    // The bind failed.  Close the socket, then loop back around and
    // try the next address/port returned by getaddrinfo().
    close(listen_sock_fd_);
    listen_sock_fd_ = -1;
  }

  // Free the structure returned by getaddrinfo().
  // std::cout << "freeaddinfo(result)" << std::endl;
  freeaddrinfo(result);

  // Did we succeed in binding to any addresses?
  if (listen_sock_fd_ == -1) {
    // No.  Quit with failure.
    std::cerr << "Couldn't bind to any addresses." << std::endl;
    return false;
  }

  // Success. Tell the OS that we want this to be a listening socket.
  if (listen(listen_sock_fd_, SOMAXCONN) != 0) {
    std::cerr << "Failed to mark socket as listening: ";
    std::cerr << strerror(errno) << std::endl;
    close(listen_sock_fd_);
    return false;
  }

  *listen_fd = listen_sock_fd_;

  return true;
}

bool ServerSocket::accept_client(int* accepted_fd,
                                 std::string* client_addr,
                                 uint16_t* client_port,
                                 std::string* client_dns_name,
                                 std::string* server_addr,
                                 std::string* server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // TODO: implement

  // Loop forever, accepting a connection from a client and doing
  // an echo trick to it.
  while (1) {
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    int client_fd =
        accept(listen_sock_fd_, reinterpret_cast<struct sockaddr*>(&caddr),
               &caddr_len);
    // std::cout << "client_fd: " << client_fd << std::endl;
    // std::cout << "listen_sock_fd_: " << listen_sock_fd_ << std::endl;
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
        continue;
      std::cerr << "Failure on accept: " << strerror(errno) << std::endl;
      return false;
    }

    // save outputs

    // get client fd
    // std::cout << "accepted fd" << std::endl;
    *accepted_fd = client_fd;

    // get client IP address
    struct sockaddr_in6* in6 = reinterpret_cast<struct sockaddr_in6*>(&caddr);
    char astring[INET_ADDRSTRLEN];
    inet_ntop(caddr.ss_family, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    *client_addr = astring;

    // get client port number
    *client_port = ntohs(in6->sin6_port);

    // get client dns name
    char hostname[1024];  // ought to be big enough.
    if (getnameinfo(reinterpret_cast<struct sockaddr*>(&caddr), caddr_len,
                    hostname, 1024, nullptr, 0, 0) != 0) {
      sprintf(hostname, "[reverse DNS failed]");
      return false;
    }
    *client_dns_name = hostname;
    // std::cout << "client_dns_name: " << *client_dns_name << std::endl;

    // get server IP address
    struct sockaddr_in6 srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET6_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr*)&srvr, &srvrlen);
    inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    *server_addr = addrbuf;

    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    char hname[1024];
    hname[0] = '\0';
    int res_get_name = getnameinfo((const struct sockaddr*)&srvr, srvrlen,
                                   hname, 1024, nullptr, 0, 0);
    if (res_get_name != 0) {
      // std::cout << "getnameinfo failed" << std::endl;
      *server_dns_name = addrbuf;
    } else {
      *server_dns_name = hname;
      // std::cout << "hname: " << hname << std::endl;
      // std::cout << "server_dns_name: " << *server_dns_name << std::endl;
    }

    break;
  }

  return true;
}

}  // namespace searchserver
