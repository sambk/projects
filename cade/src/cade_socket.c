/******************************************************************************
 *
 * Viettel Group.
 * (c) Copyright 2015-2017 VTTEK, VIETTEL GROUP
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY VTTEK "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL VTTEK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************//*!
 *
 * @file      cade_socket.c
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      Apr 28, 2020
 *
 * @brief
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if.h>

#include "cade_declaration.h"
#include "cade_utilities.h"
#include "cade_tlv.h"
#include "cade_msg.h"
#include "cade.h"
#include "cade_table.h"
#include "cade_socket.h"

/**
 * @brief: lssdp_network_interface_update
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int get_interface_from_ip(struct lssdp_interface *p_if, const char *if_addr) {
    if (p_if == NULL) {
        ERR_LOG("lssdp should not be NULL\n");
        return ERR;
    }

    int ret = ERR;

    /* Reference to this article:
     * http://stackoverflow.com/a/8007079
     */

    // 3. create UDP socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        ERR_LOG("   create socket failed, errno = %s (%d)\n", strerror(errno), errno);
        goto end;
    }

    // 4. get ifconfig
    char buffer[LSSDP_BUFFER_LEN] = {};
    struct ifconf ifc = {
        .ifc_len = sizeof(buffer),
        .ifc_buf = (caddr_t) buffer
    };

    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
        ERR_LOG("   ioctl SIOCGIFCONF failed, errno = %s (%d)\n", strerror(errno), errno);
        goto end;
    }

    // 5. setup p_sock->interface
    size_t i;
    struct ifreq * ifr;
    for (i = 0; i < ifc.ifc_len; i += _SIZEOF_ADDR_IFREQ(*ifr)) {
        ifr = (struct ifreq *)(buffer + i);
        if (ifr->ifr_addr.sa_family != AF_INET) {
            // only support IPv4
            continue;
        }

        // 5-1. get interface ip string
        char ip[LSSDP_IP_LEN] = {};
        struct sockaddr_in * addr = (struct sockaddr_in *) &ifr->ifr_addr;
        if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)) == NULL) {
            ERR_LOG("   inet_ntop failed, errno = %s (%d)\n", strerror(errno), errno);
            continue;
        }

        if (0 == strcmp(ip, if_addr))
        {
            // 5-2. get network mask
            struct ifreq netmask = {};
            strcpy(netmask.ifr_name, ifr->ifr_name);
            if (ioctl(fd, SIOCGIFNETMASK, &netmask) != 0) {
                ERR_LOG("   ioctl SIOCGIFNETMASK failed, errno = %s (%d)\n", strerror(errno), errno);
                continue;
            }

            // 5-4. set interface
            snprintf(p_if->name, LSSDP_INTERFACE_NAME_LEN, "%s", ifr->ifr_name); // name
            snprintf(p_if->ip,   LSSDP_IP_LEN,             "%s", ip);            // ip string
            p_if->addr = addr->sin_addr.s_addr;                                  // address in network byte order

            // set network mask
            addr = (struct sockaddr_in *) &netmask.ifr_addr;
            p_if->netmask = addr->sin_addr.s_addr;                               // mask in network byte order

            ret = OK;
            goto end;
        }
    }

end:
    // close socket
    if (fd >= 0 && close(fd) != 0) {
        ERR_LOG("   close fd failed, errno = %s (%d)\n", strerror(errno), errno);
    }

    return ret;
}

/**
 * @brief: show interaface info
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int show_interface_list(struct lssdp_interface *p_if) {
    printf("\nNetwork Interface:\n");
    printf("   %-6s: %-15s (%d.%d.%d.%d)\n",
        p_if->name,
        p_if->ip,
        (p_if->netmask >> 0)  & 0xff,
        (p_if->netmask >> 8)  & 0xff,
        (p_if->netmask >> 16) & 0xff,
        (p_if->netmask >> 24) & 0xff
    );
    return 0;
}

/**
 * @brief: Init socket for cade from the idea of ssdp
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_sock_init(struct cade_sock *p_sock, const char *if_addr)
{
    assert(NULL != p_sock);
    assert(NULL != if_addr);
    CADE_LOG("Initialize socket cade...");

    // Get interface info
    int ret = get_interface_from_ip(&p_sock->interface, if_addr);
    if (OK != ret)
    {
        ERR_LOG("   Cannot find interface with ip %s\n", if_addr);
        return ERR;
    } else
    {
        show_interface_list(&p_sock->interface);
    }

    /************************************************
     * create multicast UDP socket for receiving
     ************************************************/

    // 1. create UDP socket
    p_sock->mcast_recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (p_sock->mcast_recv_sockfd < 0)
    {
        ERR_LOG("   create socket failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 2. set non-blocking
    int opt = 1;
    if (ioctl(p_sock->mcast_recv_sockfd, FIONBIO, &opt) != 0)
    {
        ERR_LOG("   ioctl FIONBIO failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 3. set reuse address
    if (setsockopt(p_sock->mcast_recv_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
    {
        ERR_LOG("setsockopt SO_REUSEADDR failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 4. set FD_CLOEXEC (http://kaivy2001.pixnet.net/blog/post/32726732)
    int sock_opt = fcntl(p_sock->mcast_recv_sockfd, F_GETFD);
    if (sock_opt == -1)
    {
        ERR_LOG("   fcntl F_GETFD failed, errno = %s (%d)\n", strerror(errno), errno);
    } else
    {
        // F_SETFD
        if (fcntl(p_sock->mcast_recv_sockfd, F_SETFD, sock_opt | FD_CLOEXEC) == -1)
        {
            ERR_LOG("   fcntl F_SETFD FD_CLOEXEC failed, errno = %s (%d)\n", strerror(errno), errno);
        }
    }

    // 5. bind socket
    struct sockaddr_in rc_addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(SSDP_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    if (bind(p_sock->mcast_recv_sockfd, (struct sockaddr *)&rc_addr, sizeof(rc_addr)) != 0) {
        ERR_LOG("   bind failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 6. set IP_ADD_MEMBERSHIP
    struct ip_mreq imr =
    {
        .imr_multiaddr.s_addr = inet_addr(SSDP_ADDR),
        .imr_interface.s_addr = p_sock->interface.addr,
    };
    if (setsockopt(p_sock->mcast_recv_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(struct ip_mreq)) != 0)
    {
        ERR_LOG("   setsockopt IP_ADD_MEMBERSHIP failed: %s (%d)\n", strerror(errno), errno);
        return ERR;
    }


    /************************************************
     * create multicast UDP socket for sending
     * **********************************************/

    // 1. create UDP socket
    p_sock->mcast_send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (p_sock->mcast_send_sockfd < 0) {
        ERR_LOG("   create socket failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 2. set non-blocking
    opt = 1;
    if (ioctl(p_sock->mcast_send_sockfd, FIONBIO, &opt) != 0)
    {
        ERR_LOG("   ioctl FIONBIO failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 3. bind socket
    struct sockaddr_in sd_addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = p_sock->interface.addr
    };
    if (bind(p_sock->mcast_send_sockfd, (struct sockaddr *)&sd_addr, sizeof(sd_addr)) < 0) {
        ERR_LOG("   bind failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 4. disable IP_MULTICAST_LOOP
    char m_opt = 0;
    if (setsockopt(p_sock->mcast_send_sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &m_opt, sizeof(m_opt)) < 0) {
        ERR_LOG("   setsockopt IP_MULTICAST_LOOP failed, errno = %s (%d)\n", strerror(errno), errno);
        return ERR;
    }

    // 5. Set multicast destination address
    p_sock->mcast_dest_addr.sin_family = AF_INET;
    p_sock->mcast_dest_addr.sin_port = htons(SSDP_PORT);
    if (inet_aton(SSDP_ADDR, &p_sock->mcast_dest_addr.sin_addr) == 0) {
        ERR_LOG("   inet_aton failed, errno = %s (%d)\n", strerror(errno), errno);
        exit(ERR);
    }

    // Setting peer address
    p_sock->mcast_peer_len = sizeof(p_sock->mcast_peer_addr);


    /************************************************
     * create unicast UDP socket
     * **********************************************/
    p_sock->ucast_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 > p_sock->ucast_sockfd)
    {
        ERR_LOG("os_udpsocket_open: create socket fail, errno [%s]", strerror(errno));
        return ERR;
    }

    // Set non-blocking mode
    int flags = fcntl(p_sock->ucast_sockfd, F_GETFL, 0);
    fcntl(p_sock->ucast_sockfd, F_SETFL, flags | O_NONBLOCK);

    /* setsockopt to device interface only - this is a private VLAN */
    struct ifreq    ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy((char *)ifr.ifr_name, (const char *) p_sock->interface.name, IFNAMSIZ);
    ret = setsockopt(p_sock->ucast_sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr));
    if (ret < 0)
    {
        ERR_LOG("os_udpsocket_open: setsockopt failed, error [%s]", strerror(errno));
        close(p_sock->ucast_sockfd);
        return ERR;
    }

    // Bind socket with address
    struct sockaddr_in local_addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(UCAST_PORT),
        .sin_addr.s_addr = p_sock->interface.addr
    };
    memset(&(local_addr.sin_zero), 0, 8);

    ret = bind(p_sock->ucast_sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr));
    if(ret < 0)
    {
        ERR_LOG("os_udpsocket_open: bind socket to [%s / "PRIip" / %u] failed, errno [%s]", (const char *) p_sock->interface.name,
                IP_DOT(ntohl(p_sock->interface.addr)), ntohs(UCAST_PORT), (const char *) strerror(errno));
        close(p_sock->ucast_sockfd);
        return ERR;
    }

    // Setting peer address
    p_sock->ucast_peer_len = sizeof(p_sock->ucast_peer_addr);

    return OK;
}



