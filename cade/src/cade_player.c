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
 * @file      cade_player.c
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


// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <poll.h>

#include "cade_declaration.h"
#include "cade_utilities.h"
#include "cade_tlv.h"
#include "cade_msg.h"
#include "cade.h"
#include "cade_table.h"
#include "cade_socket.h"
#include "cade_common.h"
#include "cade_player.h"
#include "cade_member.h"

/**
 * @brief: Get current card info
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_get_cards_hold(struct cade *p_cade, struct some_cards *current_cards)
{
    if (NULL == p_cade)
    {
        CADE_LOG("get_cards_hold p_cade NULL");
        return ERR;
    }
    if (NULL == current_cards)
    {
        CADE_LOG("get_cards_hold current_cards NULL");
        return ERR;
    }

    memcpy(current_cards, &p_cade->current_cards, sizeof(struct some_cards));
    return OK;
}

/**
 * @brief: Show cards hold
 *
 * @param [IN]
 *
 * @return : void
 */
void cade_show_cards_hold(struct cade *p_cade)
{
    CADE_N_LOG("Cards hold: [");

    int idx;
    for (idx = 0; idx < p_cade->current_cards.cards_num; idx++)
    {
        CADE_N_LOG("%.*s, ", p_cade->current_cards.cards[idx].card_len, p_cade->current_cards.cards[idx].card_name);
    }
    CADE_LOG("]");
}

/**
 * @brief: Init resource for cade player instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_player_init(const char *if_addr)
{
    return cade_member_init(CADE_ROLE_PLAYER, if_addr);
}

/**
 * @brief: Init resource for cade player instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_player_init_uint32_ip(uint32_t if_addr)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = if_addr;
    CADE_LOG("Initialize cade with ip address [%s]", inet_ntoa(ip_addr));
    return cade_player_init(inet_ntoa(ip_addr));
}


/**
 * @brief: Wait for uuid. This function will block until getting uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
uint16_t cade_player_request_for_uuid(struct cade *p_cade)
{
    return cade_member_request_for_uuid(p_cade);
}

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_player_set_cb_recv_cards_success(struct cade *p_cade, void (*cb_hdl_recv_cards)(struct cade *))
{
    return set_cb_recv_cards_success(p_cade, cb_hdl_recv_cards);
}

/**
 * @brief: Set callback to monitor core health
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_player_set_cb_keep_alived_mark(struct cade *p_cade, void (*cb_keep_alived_mark)(void))
{
    return set_cb_keep_alived_mark(p_cade, cb_keep_alived_mark);
}

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_player_set_cb_show_cards_info(struct cade *p_cade, void (*cb_hdl_show_cards)(struct cade *))
{
    return set_cb_show_cards(p_cade, cb_hdl_show_cards);
}

/**
 * @brief: Run cade player business
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_player_run(struct cade *p_cade)
{
    return cade_member_run(p_cade);
}





