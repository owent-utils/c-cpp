// Licensed under the MIT licenses.
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <cstring>
#include "Socket/CompatSocket.h"

#if defined(_MSC_VER) && defined(WIN32)
    #pragma comment(lib, "wsock32")
#endif


namespace util
{

    namespace socket
    {
    
        CompatSocket::CompatSocket(SOCKET sock)
        {
            m_uSock = sock;
        }

        CompatSocket::~CompatSocket()
        {
            Close();
        }

        int CompatSocket::Init()
        {
        #ifdef WIN32
            static int ret = -1;
            if (0 == ret)
            {
                return ret;
            }
            /*
            http://msdn.microsoft.com/zh-cn/vstudio/ms741563(en-us,VS.85).aspx

            typedef struct WSAData { 
                WORD wVersion;                                //winsock version
                WORD wHighVersion;                            //The highest version of the Windows Sockets specification that the Ws2_32.dll can support
                char szDescription[WSADESCRIPTION_LEN+1]; 
                char szSystemStatus[WSASYSSTATUS_LEN+1]; 
                unsigned short iMaxSockets; 
                unsigned short iMaxUdpDg; 
                char FAR * lpVendorInfo; 
            }WSADATA, *LPWSADATA; 
            */
            WSADATA wsaData;
            //#define MAKEWORD(a,b) ((WORD) (((BYTE) (a)) | ((WORD) ((BYTE) (b))) << 8)) 
            WORD version = MAKEWORD(2, 0);
            ret = WSAStartup(version, &wsaData);//win sock start up
            if ( ret ) {
                std::cerr << "Initilize winsock error !" << std::endl;
            }

            return ret;
        #else
            return 0;
        #endif
        }
        //this is just for windows
        int CompatSocket::Clean()
        {
        #ifdef WIN32
            return (WSACleanup());
        #endif
            return 0;
        }

        CompatSocket& CompatSocket::operator = (SOCKET s)
        {
            m_uSock = s;
            return (*this);
        }

        CompatSocket::operator SOCKET ()
        {
            return m_uSock;
        }

        int CompatSocket::SetOption(int optmame, const void* optval, int optlen, int level)
        {
            #ifdef WIN32
                return setsockopt(m_uSock, level, optmame, static_cast<const char*>(optval), optlen);
            #else
                return setsockopt(m_uSock, level, optmame, optval, optlen);
            #endif
        }

        //create a socket object win/lin is the same
        // af:
        bool CompatSocket::Create(int af, int type, int protocol)
        {
            m_uSock = ::socket(af, type, protocol);
            return INVALID_SOCKET != m_uSock;
        }

        bool CompatSocket::Connect(const char* ip, uint16_t port, DnsInfo::ADDR_TYPE::type type)
        {
            struct sockaddr_in svraddr;
            _assign(svraddr.sin_family, type);
            inet_pton((int)type, ip, &svraddr.sin_addr);
            svraddr.sin_port = htons(port);
            int ret = connect(m_uSock, (struct sockaddr*)&svraddr, sizeof(svraddr));
            if ( ret == SOCKET_ERROR ) {
                return false;
            }
            return true;
        }

        bool CompatSocket::Bind(uint16_t port)
        {
            struct sockaddr_in svraddr;
            svraddr.sin_family = AF_INET;
            svraddr.sin_addr.s_addr = INADDR_ANY;
            svraddr.sin_port = htons(port);

            int opt =  1;
            if ( setsockopt(m_uSock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0 ) 
                return false;

            int ret = bind(m_uSock, (struct sockaddr*)&svraddr, sizeof(svraddr));
            if ( ret == SOCKET_ERROR ) {
                return false;
            }
            return true;
        }
        //for server
        bool CompatSocket::Listen(int backlog)
        {
            int ret = listen(m_uSock, backlog);
            if ( ret == SOCKET_ERROR ) {
                return false;
            }
            return true;
        }

        bool CompatSocket::Accept(CompatSocket& s, DnsInfo* from)
        {
            struct sockaddr_in cliaddr;
            socklen_t addrlen = sizeof(cliaddr);
            SOCKET sock = accept(m_uSock, (struct sockaddr*)&cliaddr, &addrlen);
            if ( static_cast<int>(sock) == static_cast<int>(SOCKET_ERROR) ) {
                return false;
            }

            s = sock;
            if (from != NULL) {
                char ip_buf[64] = { 0 };
                inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ip_buf, sizeof(ip_buf));
                from->address = ip_buf;
                _assign(from->type, cliaddr.sin_family);
            }

            return true;
        }

        bool CompatSocket::GetPeerName(DnsInfo& peer) {
            struct sockaddr_in peer_addr;
            socklen_t addrlen = sizeof(peer_addr);
            int res = getpeername(m_uSock, (struct sockaddr*)&peer_addr, &addrlen);
            if (0 != res) {
                return false;
            }

            char ip_buf[64] = { 0 };
            inet_ntop(peer_addr.sin_family, &peer_addr.sin_addr, ip_buf, sizeof(ip_buf));
            peer.address = ip_buf;
            _assign(peer.type, peer_addr.sin_family);

            return true;
        }

        int CompatSocket::Send(const char* buf, int len)
        {
            //������send 
            int iRet = 0;

            #ifdef WIN32
            iRet = send(m_uSock, buf, len, 0);
            #else
            iRet = send(m_uSock, buf, static_cast<size_t>(len), 0);
            #endif
            
            if (iRet < 0) {
                int err = GetError();
                // ��һ�ַ�����(Ч������д���֤)
                // #ifdef WIN32
                // if (WSAEWOULDBLOCK != err)
                // #else
                // if (EINPROGRESS != err && EAGAIN != err)
                // #endif
                
                if (EBADF == err || ENOTSOCK == err) {
                    m_uSock = INVALID_SOCKET;
                } else {
                    switch (err) {
                    case ENOTCONN:
                    case ECONNRESET:
                        Close();
                        break;

                    default:
                        break;
                    };
                }
            }

            return iRet;
        }

        int CompatSocket::Recv(char* buf, int len)
        {
            //������ recv 
            int iRet = 0;
            #ifdef WIN32
            iRet = recv(m_uSock, buf, len, 0);
            #else
            iRet = recv(m_uSock, buf, static_cast<size_t>(len), 0);
            #endif

            // http://linux.die.net/man/2/recv
            // recv ����0������socket�Զ˶Ͽ�
            if (0 == iRet) {
                Close();
            } else if (iRet < 0) {
                int err = GetError();
                // ��һ�ַ�����(Ч������д���֤)
                // #ifdef WIN32
                // if (WSAEWOULDBLOCK != err)
                // #else
                // if (EINPROGRESS != err && EAGAIN != err)
                // #endif

                if (EBADF == err || ENOTSOCK == err) {
                    m_uSock = INVALID_SOCKET;
                } else {
                    switch (err) {
                    case ECONNREFUSED:
                    case ENOTCONN:
                        Close();
                        break;

                    default:
                        break;
                    };
                }
            }

            return iRet;
        }

        int CompatSocket::Select(bool read, bool write, int iSecond, int iMicroSeconds)
        {
            // Select����
            fd_set wset;
            if (write) {
                FD_ZERO(&wset);
                FD_SET(m_uSock, &wset);
            }

            fd_set rset;
            if (read) {
                FD_ZERO(&rset);
                FD_SET(m_uSock, &rset);
            }

            struct timeval tm;
            tm.tv_sec = iSecond;
            tm.tv_usec = iMicroSeconds;

            int iRet = select(m_uSock + 1, read ? &rset : NULL, write? &wset: NULL, NULL, &tm);
            if (iRet == 0)
            {
                // Time out
                return -1;
            }

            iRet = -2;
            if (read && FD_ISSET(m_uSock, &rset))
            {
                iRet = 0;
            }

            if (write && FD_ISSET(m_uSock, &wset))
            {
                iRet = 0;
            }

            return iRet;
        }

        int CompatSocket::Close()
        {
            SOCKET uSock = m_uSock;
            m_uSock = INVALID_SOCKET;

            if (INVALID_SOCKET == uSock) {
                return 0;
            }

        #ifdef WIN32
            return (closesocket(uSock));
        #else
            return (close(uSock));
        #endif
        }

        int CompatSocket::GetError()
        {
        #ifdef WIN32
            return (WSAGetLastError());
        #else
            return (errno);
        #endif
        }


        bool CompatSocket::IsValid() const {
            return INVALID_SOCKET != m_uSock;
        }

        void CompatSocket::SetNoBlock(bool no_block) {
            if (no_block) {
                int val = 0;
                SetOption(SO_RCVTIMEO, &val, sizeof(val));
                SetOption(SO_SNDTIMEO, &val, sizeof(val));
            }

#ifdef WIN32
            u_long mode = no_block? 1: 0;
            ioctlsocket(m_uSock, FIONBIO, &mode);
#else
            int flags = fcntl(m_uSock, F_GETFL, 0);
            if (no_block) {
                flags = flags | O_NONBLOCK;
            } else {
                flags = flags & (~O_NONBLOCK);
            }
            fcntl(m_uSock, F_SETFL, flags);
#endif
        }

        void CompatSocket::SetNoDelay(bool no_delay) {
            int val = no_delay ? 1 : 0;
#ifdef TCP_NODELAY
            SetOption(TCP_NODELAY, &val, sizeof(val));
#else
            // SetOption(TCP_NODELAY, &val, sizeof(val));
#endif
        }

        void CompatSocket::SetKeepAlive(bool keep_alive) {
            int val = keep_alive? 1: 0;
            SetOption(SO_KEEPALIVE, &val, sizeof(val));
        }

        int CompatSocket::SetTimeout(int recvtimeout, int sendtimeout, int lingertimeout) {
            int rt = -1;
            if (IsValid())
            {
                rt = 0;
#if defined(WIN32)  
                if (lingertimeout > -1)
                {
                    struct linger  lin;
                    lin.l_onoff = lingertimeout;
                    lin.l_linger = lingertimeout;
                    rt = setsockopt(m_uSock, SOL_SOCKET, SO_DONTLINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
                }
                if (recvtimeout > 0 && rt == 0)
                {
                    rt = rt | (setsockopt(m_uSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvtimeout, sizeof(int)) == 0 ? 0 : 0x2);
                }
                if (sendtimeout > 0 && rt == 0)
                {
                    rt = rt | (setsockopt(m_uSock, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendtimeout, sizeof(int)) == 0 ? 0 : 0x4);
                }
#else
                struct timeval timeout;
                if (lingertimeout>-1)
                {
                    struct linger  lin;
                    lin.l_onoff = lingertimeout>0 ? 1 : 0;
                    lin.l_linger = lingertimeout / 1000;
                    rt = setsockopt(m_uSock, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(linger)) == 0 ? 0 : 0x1;
                }
                if (recvtimeout > 0 && rt == 0)
                {
                    timeout.tv_sec = recvtimeout / 1000;
                    timeout.tv_usec = (recvtimeout % 1000) * 1000;
                    rt = rt | (setsockopt(m_uSock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0 ? 0 : 0x2);
                }
                if (sendtimeout > 0 && rt == 0)
                {
                    timeout.tv_sec = sendtimeout / 1000;
                    timeout.tv_usec = (sendtimeout % 1000) * 1000;
                    rt = rt | (setsockopt(m_uSock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == 0 ? 0 : 0x4);
                }
#endif  
            }

            return rt;
        }

        bool CompatSocket::DnsParse(const char* domain, std::list<DnsInfo>& dns_res)
        {
            struct addrinfo *result = NULL;
            struct addrinfo *ptr = NULL;
            struct addrinfo hints;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            int res = getaddrinfo(domain, NULL, &hints, &result);
            if (res != 0) {
                return false;
            }

            for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
                switch (ptr->ai_family) {
                case AF_UNIX:
                case AF_INET:
                case AF_INET6:
                {
                    struct sockaddr_in * sockaddr_ip = (struct sockaddr_in *) ptr->ai_addr;
                    dns_res.push_back(DnsInfo());
                    DnsInfo& rec = dns_res.back();

                    switch (ptr->ai_family) {
                    case AF_UNIX:
                    {
                        rec.type = DnsInfo::ADDR_TYPE::UNIX;
                        break;
                    }

                    case AF_INET:
                    {
                        rec.type = DnsInfo::ADDR_TYPE::IPV4;
                        break;
                    }
                    case AF_INET6:
                    {
                        rec.type = DnsInfo::ADDR_TYPE::IPV6;
                        break;
                    }
                    default:
                        break;
                    }

                    char addr_buff[64] = { 0 };
                    inet_ntop(ptr->ai_family, &sockaddr_ip->sin_addr, addr_buff, sizeof(addr_buff));
                    rec.address = addr_buff;
                    break;
                }
                default: {
                    break;
                }
                }
            }
            return true;
        }
    }
}
