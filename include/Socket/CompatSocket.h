/**
 * @file CompatSocket.h
 * @brief 兼容型Socket封装
 *        兼容Windows Socket和BSD Socket
 *        仅提供基本socket操作(测试用)
 * Licensed under the MIT licenses.
 *
 * @note
 * 　　客户端调用int connect(int sockfd, const struct sockaddr *addr, socklen_t len);发起对服务器的socket的连接请求，如果客户端socket描述符为阻塞模式则会一直阻塞到连接建立或者连接失败(注意阻塞模式的超时时间可能为75秒到几分钟之间)，而如果为非阻塞模式，则调用connect之后如果连接不能马上建立则返回-1(errno设置为EINPROGRESS，注意连接也可能马上建立成功比如连接本机的服务器进程),如果没有马上建立返回，此时TCP的三路握手动作在背后继续，而程序可以做其他的东西，然后调用select检测非阻塞connect是否完成(此时可以指定select的超时时间，这个超时时间可以设置为比connect的超时时间短)，如果select超时则关闭socket，然后可以尝试创建新的socket重新连接，如果select返回非阻塞socket描述符可写则表明连接建立成功，如果select返回非阻塞socket描述符既可读又可写则表明连接出错(注意：这儿必须跟另外一种连接正常的情况区分开来，就是连接建立好了之后，服务器端发送了数据给客户端，此时select同样会返回非阻塞socket描述符既可读又可写，这时可以通过以下方法区分:
 *　1.调用getpeername获取对端的socket地址.如果getpeername返回ENOTCONN,表示连接建立失败,然后用SO_ERROR调用getsockopt得到套接口描述符上的待处理错误;
 *　2.调用read,读取长度为0字节的数据.如果read调用失败,则表示连接建立失败,而且read返回的errno指明了连接失败的原因.如果连接建立成功,read应该返回0;
 *　3.再调用一次connect.它应该失败,如果错误errno是EISCONN,就表示套接口已经建立,而且第一次连接是成功的;否则,连接就是失败的;
 *    对于无连接的socket类型(SOCK_DGRAM)，客户端也可以调用connect进行连接,此连接实际上并不建立类似SOCK_STREAM的连接，而仅仅是在本地保存了对端的地址，这样后续的读写操作可以默认以连接的对端为操作对象。
 *
 * @version 1.0
 * @author OWenT
 * @date 2013.04.26
 *
 * @history
 *
 */

#pragma once

#include <string>
#include <list>

#ifdef WIN32
    #include <WinSock2.h>
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
            struct ADDR_TYPE {
                enum type {
                    UNIX = AF_UNIX,
                    IPV4 = AF_INET,
                    IPV6 = AF_INET6,
                };
            };
            
            ADDR_TYPE::type type;
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
            bool Connect(const char* ip, uint16_t port, DnsInfo::ADDR_TYPE::type type = DnsInfo::ADDR_TYPE::IPV4);
            // #region server
            // Bind socket
            bool Bind(uint16_t port);

            // Listen socket
            bool Listen(int backlog = 5); 

            // Accept socket
            bool Accept(CompatSocket& s, DnsInfo* from);
            // #endregion
            
            bool GetPeerName(DnsInfo& peer);

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
