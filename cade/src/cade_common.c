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
 * @file      cade_common.c
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
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <errno.h>

#include "cade_declaration.h"
#include "cade_utilities.h"
#include "cade_tlv.h"
#include "cade_msg.h"
#include "cade_socket.h"
#include "cade.h"
#include "cade_table.h"
#include "cade_common.h"

/**
 * @brief: Usually machine-id is 32-byte-string like "ebb4f0a9c3474b7c99ac8f5a6ad85b86"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_machine_id(uint8_t *machine_id, uint16_t max_len)
{
    int len, ret_len = 0;
    FILE *fp = fopen("/sys/class/dmi/id/product_uuid", "r");
    if(fp != NULL)
    {
        char txt[128];
        fgets(txt, sizeof(txt), fp);

        len = strlen(txt);
        if (len <= max_len)
        {
            memcpy(machine_id, txt, len);
            CADE_LOG("Machine id: [%.*s]", len, machine_id);
            ret_len = len;
        } else
        {
            memcpy(machine_id, txt, max_len);
            CADE_LOG("Machine id: [%.*s]", max_len, machine_id);
            ret_len = max_len;
        }
        fclose(fp);
        return ret_len;
    }
    return ERR;
}

/**
 * @brief: Free cade memory
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_destroy(struct cade *p_cade)
{
    if (NULL != p_cade)
    {
        free(p_cade);
    }
}

/**
 * @brief: Send message on multicast "sending socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_send_msg_on_mcast_s_sock(struct cade *p_cade, int len)
{
    if (sendto(p_cade->sock.mcast_send_sockfd, (const char *) &p_cade->encoded_msg, len, 0, (struct sockaddr *) &p_cade->sock.mcast_dest_addr,
            sizeof(p_cade->sock.mcast_dest_addr)) == -1)
    {
        ERR_LOG("   mcast sendto %s (%s) failed, errno = %s (%d)\n", p_cade->sock.interface.name, p_cade->sock.interface.ip, strerror(errno), errno);
        return ERR;
    }
    return OK;
}

/**
 * @brief: Send message on multicast "receiving socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_send_msg_on_mcast_r_sock(struct cade *p_cade, int len)
{
    if (sendto(p_cade->sock.mcast_recv_sockfd, (const char *) &p_cade->encoded_msg, len, 0, (struct sockaddr *) &p_cade->sock.mcast_peer_addr,
            p_cade->sock.mcast_peer_len) == -1)
    {
        ERR_LOG("   sendto %s (%s) failed, errno = %s (%d)\n", p_cade->sock.interface.name, p_cade->sock.interface.ip, strerror(errno), errno);
        return ERR;
    }
    return OK;
}

/**
 * @brief: Send message on unicast socket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_send_msg_on_unicast_sock(struct cade *p_cade, int len, uint32_t ip, uint16_t port)
{
    p_cade->sock.ucast_dest_addr.sin_family = AF_INET;
    p_cade->sock.ucast_dest_addr.sin_addr.s_addr = ip;
    p_cade->sock.ucast_dest_addr.sin_port = port;
    if (sendto(p_cade->sock.ucast_sockfd, (const char *) &p_cade->encoded_msg, len, 0, (struct sockaddr *) &p_cade->sock.ucast_dest_addr,
            sizeof(p_cade->sock.ucast_dest_addr)) == -1)
    {
        ERR_LOG("   ucast sendto %s (%s) failed, errno = %s (%d)\n", p_cade->sock.interface.name, p_cade->sock.interface.ip, strerror(errno), errno);
        return ERR;
    }
    return OK;
}

/**
 * @brief: Reply message on unicast socket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_rep_msg_on_unicast_sock(struct cade *p_cade, int len)
{
    if (sendto(p_cade->sock.ucast_sockfd, (const char *) &p_cade->encoded_msg, len, 0, (struct sockaddr *) &p_cade->sock.ucast_peer_addr,
            p_cade->sock.ucast_peer_len) == -1)
    {
        ERR_LOG("   ucast Reply on %s (%s) failed, errno = %s (%d)\n", p_cade->sock.interface.name, p_cade->sock.interface.ip, strerror(errno), errno);
        return ERR;
    }
    return OK;
}

/**
 * @brief: Receive message on multicast "sending socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_recv_msg_on_mcast_s_sock(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        return ERR;
    }

    struct sockaddr_in *peer_addr;
    int recv_len;
    struct cade_msg_decoded *p_decoded_msg;
    uint8_t buffer[MAXLINE] = {0};
    peer_addr = &p_cade->sock.mcast_peer_addr;
    p_decoded_msg = &p_cade->decoded_msg;

    // Receive message
    recv_len = recvfrom(p_cade->sock.mcast_send_sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) peer_addr, &p_cade->sock.mcast_peer_len);
    if (recv_len > 0)
    {
        //DUMP_MSG("Receive on s socket: ", buffer, recv_len);
        memset(p_decoded_msg, 0, sizeof(struct cade_msg_decoded));
        return cade_decap_msg(p_decoded_msg, buffer, recv_len);
    }
    return ERR;
}

/**
 * @brief: Receive message on multicast "receiving socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_recv_msg_on_mcast_r_sock(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        return ERR;
    }

    struct sockaddr_in *peer_addr;
    int recv_len;
    struct cade_msg_decoded *p_decoded_msg;
    uint8_t buffer[MAXLINE] = {0};
    peer_addr = &p_cade->sock.mcast_peer_addr;
    p_decoded_msg = &p_cade->decoded_msg;

    // Receive message
    memset(buffer, 0, MAXLINE);
    recv_len = recvfrom(p_cade->sock.mcast_recv_sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) peer_addr, &p_cade->sock.mcast_peer_len);
    if (recv_len > 0)
    {
        //DUMP_MSG("Receive on r socket: ", buffer, recv_len);
        memset(p_decoded_msg, 0, sizeof(struct cade_msg_decoded));
        return cade_decap_msg(p_decoded_msg, buffer, recv_len);
    }
    return ERR;
}

/**
 * @brief: Receive message on "unicast socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_recv_msg_on_unicast_sock(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        return ERR;
    }

    int recv_len;
    struct cade_msg_decoded *p_decoded_msg;
    uint8_t buffer[MAXLINE] = {0};
    p_decoded_msg = &p_cade->decoded_msg;

    // Receive message
    memset(buffer, 0, MAXLINE);
    recv_len = recvfrom(p_cade->sock.ucast_sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &p_cade->sock.ucast_peer_addr, &p_cade->sock.ucast_peer_len);
    if (recv_len > 0)
    {
        //DUMP_MSG("Receive on unicast socket: ", buffer, recv_len);
        memset(p_decoded_msg, 0, sizeof(struct cade_msg_decoded));
        return cade_decap_msg(p_decoded_msg, buffer, recv_len);
    }
    return ERR;
}

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void set_cb_recv_cards_success(struct cade *p_cade, void (*cb_hdl_recv_cards)(struct cade *))
{
    p_cade->cb_hdl_recv_cards_success = cb_hdl_recv_cards;
}

/**
 * @brief: Set callback handle to monitor core health
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void set_cb_keep_alived_mark(struct cade *p_cade, void (*cb_keep_alived_mark)(void))
{
    p_cade->cb_hdl_keep_alived_mark = cb_keep_alived_mark;
}

/**
 * @brief: Set callback handle to be called to show cards hold
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void set_cb_show_cards(struct cade *p_cade, void (*cb_hdl_show_cards_info)(struct cade *))
{
    p_cade->cb_hdl_show_cards_info = cb_hdl_show_cards_info;
}









