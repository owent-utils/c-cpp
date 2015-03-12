/**
 * @file CompatSocket.h
 * @brief 兼容型Socket封装
 *        兼容Windows Socket和BSD Socket
 *        仅提供基本socket操作(测试用)
 * Licensed under the MIT licenses.
 *
 * @version 1.0
 * @author OWenT
 * @date 2013.04.26
 *
 * @history
 *
 */

#ifndef _UTIL_SOCKET_COMPAT_SOCKET_H__
#define _UTIL_SOCKET_COMPAT_SOCKET_H__

#include <string>
#include <list>

#ifdef WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int                socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/tcp.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <unistd.h>
    typedef int                SOCKET;

    //#pragma region define win32 const variable in linux
    #define INVALID_SOCKET    -1
    #define SOCKET_ERROR    -1
    //#pragma endregion
#endif

namespace util
{

    namespace socket
    {

        struct DnsInfo {
            enum struct ADDR_TYPE {
                UNIX = AF_UNIX,
                IPV4 = AF_INET,
                IPV6 = AF_INET6,
            };

            ADDR_TYPE type;
            std::string address;
        };

        class CompatSocket 
        {
        public:
            typedef std::list<DnsInfo> dns_result_t;

        public:
            CompatSocket(SOCKET sock = INVALID_SOCKET);
            ~CompatSocket();

            // set option
            int SetOption(int optmame, const void* optval, int optlen, int level = SOL_SOCKET);

            // Create socket object for snd/recv data
            bool Create(int af = PF_INET, int type = SOCK_STREAM, int protocol = 0);

            // Connect socket
            bool Connect(const char* ip, uint16_t port, DnsInfo::ADDR_TYPE type = DnsInfo::ADDR_TYPE::IPV4);
            // #region server
            // Bind socket
            bool Bind(uint16_t port);

            // Listen socket
            bool Listen(int backlog = 5); 

            // Accept socket
            bool Accept(CompatSocket& s, DnsInfo* from);
            // #endregion
            
            // Send socket
            int Send(const char* buf, int len);

            // Recv socket
            int Recv(char* buf, int len);
            
            // Select
            int Select(bool read, bool write, int iSecond = 0, int iMicroSeconds = 0);

            // Close socket
            int Close();

            // Get errno
            int GetError();
            
            //#pragma region just for win32
            // Init winsock DLL 
            static int Init();    
            // Clean winsock DLL
            static int Clean();
            //#pragma endregion

            // Domain parse
            static bool DnsParse(const char* domain, std::list<DnsInfo>& dns_res);

            CompatSocket& operator = (SOCKET s);

            operator SOCKET ();

            bool IsValid() const;

            void SetNoBlock(bool no_block = false);

            void SetNoDelay(bool no_delay = false);

            void SetKeepAlive(bool keep_alive = false);

            int SetTimeout(int recvtimeout, int sendtimeout, int lingertimeout);
        protected:
            SOCKET m_uSock;

        private:
            template<typename TSrc, typename TD>
            void _assign(TSrc& s, const TD& td) {
                s = static_cast<TSrc>(td);
            }
        };

    }
}

#endif
