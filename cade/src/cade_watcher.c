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
 * @file      cade_watcher.c
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
#include "cade_watcher.h"
#include "cade_member.h"

/**
 * @brief: Get info about player and cards hold for watcher
 *
 * @param [IN]
 *
 * @return : void
 */
int cade_get_cards_hold_by_players(struct cade *p_cade, struct player_info *current_player_info)
{
    if (NULL == p_cade)
    {
        CADE_LOG("get_cards_hold p_cade NULL");
        return ERR;
    }
    if (NULL == current_player_info)
    {
        CADE_LOG("get_cards_hold current_player_info NULL");
        return ERR;
    }

    memcpy(current_player_info, &p_cade->current_player_info, sizeof(struct player_info));
    return OK;
}

/**
 * @brief: Show cards hold
 *
 * @param [IN]
 *
 * @return : void
 */
void cade_show_cards_hold_by_players(struct cade *p_cade)
{
    struct player_card_info *p_player_info;
    struct cards *p_cards;
    CADE_N_LOG("Watcher: ");

    int idx, jdx;
    for (idx = 0; idx < p_cade->current_player_info.nb_players; idx++)
    {
        p_player_info = &p_cade->current_player_info.arr_players[idx];
        CADE_N_LOG(" Player uuid %d: [", p_player_info->uuid);

        for (jdx = 0; jdx < p_player_info->cards_hold.cards_num; jdx++)
        {
            p_cards = &p_player_info->cards_hold.cards[jdx];
            CADE_N_LOG("%.*s, ", p_cards->card_len, p_cards->card_name);
        }
        CADE_N_LOG("]");
    }
    CADE_LOG(" ");
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
struct cade *cade_watcher_init(const char *if_addr)
{
    return cade_member_init(CADE_ROLE_WATCHER, if_addr);
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
struct cade *cade_watcher_init_uint32_ip(uint32_t if_addr)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = if_addr;
    CADE_LOG("Initialize cade with ip address [%s]", inet_ntoa(ip_addr));
    return cade_watcher_init(inet_ntoa(ip_addr));
}

/**
 * @brief: Wait for uuid. This function will block until getting uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
uint16_t cade_watcher_request_for_uuid(struct cade *p_cade)
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
void cade_watcher_set_cb_recv_cards_success(struct cade *p_cade, void (*cb_hdl_recv_cards)(struct cade *))
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
void cade_watcher_set_cb_keep_alived_mark(struct cade *p_cade, void (*cb_keep_alived_mark)(void))
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
void cade_watcher_set_cb_show_cards_info(struct cade *p_cade, void (*cb_hdl_show_cards)(struct cade *))
{
    return set_cb_show_cards(p_cade, cb_hdl_show_cards);
}

/**
 * @brief: Run cade watcher business
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_watcher_run(struct cade *p_cade)
{
    return cade_member_run(p_cade);
}





