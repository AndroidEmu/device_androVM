/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "TcpStream.h"
#include <cutils/sockets.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#else
#include <ws2tcpip.h>
#endif

TcpStream::TcpStream(size_t bufSize) :
    SocketStream(bufSize)
{
}

TcpStream::TcpStream(int sock, size_t bufSize) :
    SocketStream(sock, bufSize)
{
    // disable Nagle algorithm to improve bandwidth of small
    // packets which are quite common in our implementation.
#ifdef _WIN32
    DWORD  flag;
#else
    int    flag;
#endif
    flag = 1;
    setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag) );
}

int TcpStream::listen(unsigned short port)
{
    m_sock = socket_loopback_server(port, SOCK_STREAM);
    if (!valid()) return int(ERR_INVALID_SOCKET);

    return 0;
}

SocketStream * TcpStream::accept()
{
    int clientSock = -1;

    while (true) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        clientSock = ::accept(m_sock, (sockaddr *)&addr, &len);

        if (clientSock < 0 && errno == EINTR) {
            continue;
        }
        break;
    }

    TcpStream *clientStream = NULL;

    if (clientSock >= 0) {
        clientStream =  new TcpStream(clientSock, m_bufsize);
    }
    return clientStream;
}

int TcpStream::connect(unsigned short port)
 {
    m_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!valid()) {
        ALOGE("TcpStream::connect() - socket() returns %d errno=%d\n", m_sock, errno);
        return -1;
    }
    struct sockaddr_in addr, addr_peer;
    socklen_t alen;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (::connect(m_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ALOGE("TcpStream::connect() - connect() errno=%d\n", errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    memset(&addr_peer, 0, sizeof(addr_peer));
    alen = sizeof(addr);
    getsockname(m_sock, (struct sockaddr *)&addr, &alen);
    alen = sizeof(addr_peer);
    getpeername(m_sock, (struct sockaddr *)&addr_peer, &alen);
    ALOGE("%s: Process:%d Thread:%u connected through local ports %d to %d", __FUNCTION__,
          getpid(), pthread_self(), ntohs(addr.sin_port), ntohs(addr_peer.sin_port));

#ifdef _WIN32
    DWORD  flag;
#else
    int    flag;
#endif
    flag = 1;
    setsockopt( m_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag) );

    return 0;
}

int TcpStream::connect(const char* hostname, unsigned short port)
{
    ALOGE("TcpStream::connect %s %d", hostname, port);
    m_sock = socket_network_client(hostname, port, SOCK_STREAM);
    ALOGE("TcpStream::connect returns %d", m_sock);
    if (!valid()) return -1;
#ifdef _WIN32
    DWORD  flag;
#else
    int    flag;
#endif
    flag = 1;
    setsockopt( m_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag) );

    return 0;
}
