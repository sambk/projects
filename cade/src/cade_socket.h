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
 * @file      cade_socket.h
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

#ifndef CADE_SOCKET_H_
#define CADE_SOCKET_H_

/* Struct : lssdp_ctx */
#define LSSDP_INTERFACE_NAME_LEN    16                      // IFNAMSIZ
#define LSSDP_IP_LEN                16
#define LSSDP_BUFFER_LEN            2048
#define _SIZEOF_ADDR_IFREQ          sizeof

struct lssdp_interface
{
    char        name            [LSSDP_INTERFACE_NAME_LEN]; // name[16]
    char        ip              [LSSDP_IP_LEN];             // ip[16] = "xxx.xxx.xxx.xxx"
    uint32_t    addr;                                       // address in network byte order
    uint32_t    netmask;                                    // mask in network byte order
};

// Struct socket
struct cade_sock
{
    /* Network Interface */
    struct lssdp_interface      interface;

    // Socket for multicast 239.255.255.250
    int                         mcast_recv_sockfd;
    int                         mcast_send_sockfd;
    struct sockaddr_in          mcast_dest_addr;

    struct sockaddr_in          mcast_peer_addr;
    socklen_t                   mcast_peer_len;

    // Socket for unicast
    int                         ucast_sockfd;
    struct sockaddr_in          ucast_dest_addr;
    struct sockaddr_in          ucast_peer_addr;
    socklen_t                   ucast_peer_len;
};

/**
 * @brief: show interaface info
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int show_interface_list(struct lssdp_interface *p_if);

/**
 * @brief: Init socket for cade from the idea of ssdp
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_sock_init(struct cade_sock *p_sock, const char *if_addr);

#endif /* CADE_SOCKET_H_ */
