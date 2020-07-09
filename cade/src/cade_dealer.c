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
 * @file      cade_dealer.c
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


// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <malloc.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>

#include "cade_declaration.h"
#include "cade_utilities.h"
#include "cade_tlv.h"
#include "cade_msg.h"
#include "cade.h"
#include "cade_table.h"
#include "cade_socket.h"
#include "cade_common.h"
#include "cade_algorithm.h"
#include "cade_dealer.h"

//    /* Msg type                      Sending function                    Receiving function*/
//    { CADE_MSG_NOT_USED        ,     cade_do_nothing,                    cade_do_nothing                },
//    { CADE_MSG_NOTIFY          ,     cade_do_nothing,                    cade_recv_notify_msg           },
//    { CADE_MSG_IDASSIGN        ,     cade_send_idassign_msg,             cade_do_nothing                },
//    { CADE_MSG_MSEARCH         ,     cade_send_msearch_msg,              cade_do_nothing                },
//    { CADE_MSG_RESPONSE        ,     cade_do_nothing,                    cade_recv_response_msg         },
//    { CADE_MSG_PING            ,     cade_send_ping_msg,                 cade_do_nothing                },
//    { CADE_MSG_PONG            ,     cade_do_nothing,                    cade_recv_pong_msg             },
//    { CADE_MSG_CARDDEAL        ,     cade_send_carddeal_msg,             cade_do_nothing                },
//    { CADE_MSG_CARDDEAL_OK     ,     cade_do_nothing,                    cade_recv_carddeal_ok_msg      },
//    { CADE_MSG_CARDDEAL_ERR    ,     cade_do_nothing,                    cade_recv_carddeal_err_msg     },
//    { CADE_MSG_CARDCOMMIT      ,     cade_send_cardcommit_msg,           cade_do_nothing                },
//    { CADE_MSG_CARDCOMMIT_OK   ,     cade_do_nothing,                    cade_recv_cardcommit_ok_msg    },
//    { CADE_MSG_CARDCOMMIT_ERR  ,     cade_do_nothing,                    cade_recv_cardcommit_err_msg   },
//    { CADE_MSG_CARDROLLBACK    ,     cade_send_cardrollback_msg,         cade_do_nothing                },
//    { CADE_MSG_CARDROLLBACK_OK ,     cade_do_nothing,                    cade_recv_cardrollback_ok_msg  },
//    { CADE_MSG_CARDROLLBACK_ERR,     cade_do_nothing,                    cade_recv_cardrollback_err_msg }


extern char *txt_code_msg_type[];

static int cade_do_carddeal_two_phase_commit(struct cade *p_cade);
static int cade_send_msearch_msg(struct cade *p_cade);
static int cade_send_ping_msg(struct cade *p_cade);
static int cade_send_idassign_msg(struct cade *p_cade, uint8_t role);
static int cade_send_carddeal_msg(struct cade *p_cade);
static int cade_send_cardcommit_msg(struct cade *p_cade);
static int cade_send_cardrollback_msg(struct cade *p_cade);

static int cade_recv_msg_template(struct cade *p_cade, enum cade_msg_type msg_type, enum cade_state cur_state, enum cade_state next_state);
static int cade_recv_notify_msg(struct cade *p_cade);
static int cade_recv_response_msg(struct cade *p_cade);
static int cade_recv_pong_msg(struct cade *p_cade);
static int cade_recv_carddeal_ok_msg(struct cade *p_cade);
static int cade_recv_carddeal_err_msg(struct cade *p_cade);
static int cade_recv_cardcommit_ok_msg(struct cade *p_cade);
static int cade_recv_cardcommit_err_msg(struct cade *p_cade);
static int cade_recv_cardrollback_ok_msg(struct cade *p_cade);
static int cade_recv_cardrollback_err_msg(struct cade *p_cade);

static int cade_recv_msg_handle(struct cade *p_cade, uint8_t msg_type);

static void *mcast_sock_handle(void *ptr);
static void ping_sending_handle(union sigval timer_data);
static int start_ping_timer(struct cade *p_cade);

/**
 * @brief: Send MSEARCH
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_msearch_msg(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_msearch_msg p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap MSEARCH message with uuid 0
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_MSEARCH, 0, generate_sequence());
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_send_msg_on_mcast_s_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send PING
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_ping_msg(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_ping_msg p_cade NULL");
        return ERR;
    }

    uint32_t ip;
    uint16_t port, seq;
    struct cade_member_info *p_member;
    struct cade_msg_encoded *p_msg;
    int idx, encoded_len;
    bool is_need_carddeal = false;

    // Ping players
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_players; idx++)
    {
        p_member = &p_cade->cade_member_tbl.arr_cade_players[idx];
        if ( p_member->is_alive && (CADE_STATE_NORMAL == p_member->state) )
        {
            ip = p_member->ip_addr;
            port = p_member->port;
            p_msg = &p_cade->encoded_msg;

            // Encap PING message with uuid 0
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            seq = generate_sequence();
            encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_PING, 0, seq);
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            DBG_LOG("Sending PING message to player "PRIip" / %d with sequence %d...", IP_DOT(ntohl(ip)), ntohs(port), (int) seq);
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            if (p_member->ping_timeout_counter > 0)
            {
                p_member->ping_timeout_counter--;
            } else
            {
                is_need_carddeal = true;
            }
        }
    }

    // Ping watchers
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_watchers; idx++)
    {
        p_member = &p_cade->cade_member_tbl.arr_cade_watchers[idx];
        if ( p_member->is_alive && (CADE_STATE_NORMAL == p_member->state) )
        {
            ip = p_member->ip_addr;
            port = p_member->port;
            p_msg = &p_cade->encoded_msg;

            // Encap PING message with uuid 0
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            seq = generate_sequence();
            encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_PING, 0, seq);
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            DBG_LOG("Sending PING message to watcher "PRIip" / %d with sequence %d...", IP_DOT(ntohl(ip)), ntohs(port), (int) seq);
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            if (p_member->ping_timeout_counter > 0)
            {
                p_member->ping_timeout_counter--;
            } else
            {
                is_need_carddeal = true;
            }
        }
    }

    // Check if any memeber is timeout
    if (is_need_carddeal)
    {
        CADE_LOG("Some member is timeout -> Do card dealing");

        // Update alive - dead status
        check_pong_mark_if_alive_all_members(&p_cade->cade_member_tbl);

        // Do cardeal
        cade_do_carddeal_two_phase_commit(p_cade);
    }

    return OK;
}

/**
 * @brief: Send IDASSIGN
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_idassign_msg(struct cade *p_cade, uint8_t role)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_idassign_msg p_cade NULL");
        return ERR;
    }

    // Encap IDASSIGN
    struct tlv *tlv = cade_find_tlv_from_tag(&p_cade->decoded_msg.pl_tlv, CADE_PL_TAG_MACHINEID);
    if (NULL != tlv)
    {
        uint8_t *machineid = tlv->value;
        uint16_t machineid_len = tlv->length;
        int8_t uuid = -1;

        if (CADE_ROLE_PLAYER == role)
        {
            uuid = get_uuid_from_machineid(p_cade->cade_member_tbl.arr_cade_players, p_cade->cade_member_tbl.nb_players, machineid, machineid_len);
        } else if (CADE_ROLE_WATCHER == role)
        {
            uuid = get_uuid_from_machineid(p_cade->cade_member_tbl.arr_cade_watchers, p_cade->cade_member_tbl.nb_watchers, machineid, machineid_len);
        }

        if (-1 != uuid)
        {
            CADE_LOG("Send ASSIGNID with uuid %d", uuid);
            struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;
            int encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_IDASSIGN, uuid, p_cade->decoded_msg.sequence);
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            cade_send_msg_on_mcast_r_sock(p_cade, encoded_len);
        }
    }

    return OK;
}

/**
 * @brief: Send CARDDEAL
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_carddeal_msg(struct cade *p_cade)
{
    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;
    struct tlv_serialization s_serialized, *p_serialized = &s_serialized, s_grp_serialized, *p_grp_serialized = &s_grp_serialized;
    struct cade_member_info *p_player, *p_watcher;
    struct some_cards *p_cards_candidate, *p_cards_current;
    uint32_t ip;
    uint16_t port;
    int idx, jdx, kdx, encoded_len;

    // Send to players
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_players; idx++)
    {
        p_player = &p_cade->cade_member_tbl.arr_cade_players[idx];

        ip = p_player->ip_addr;
        port = p_player->port;
        p_cards_candidate = &p_player->candidate_cards;
        p_cards_current = &p_player->current_cards;

        if (!is_same_current_candidate(p_player))
        {
            CADE_LOG("Member idx %d uuid %d will have %d card", idx, p_player->uuid, p_cards_candidate->cards_num);

            // Memset 0 for buffers
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            cade_tlv_encode_init(p_serialized);

            // Encap all cards hold into tlv
            for (jdx = 0; jdx < p_cards_candidate->cards_num; jdx++)
            {
                if (!is_hold_by_this_player(p_cards_current, p_cards_candidate->cards[jdx].card_name, p_cards_candidate->cards[jdx].card_len))
                {
                    // All this card
                    cade_add_tlv_msg(p_serialized, CADE_PL_TAG_CARD_ADD, p_cards_candidate->cards[jdx].card_name, p_cards_candidate->cards[jdx].card_len);
                }
            }
            for (jdx = 0; jdx < p_cards_current->cards_num; jdx++)
            {
                if (!is_hold_by_this_player(p_cards_candidate, p_cards_current->cards[jdx].card_name, p_cards_current->cards[jdx].card_len))
                {
                    // Delelete this card
                    cade_add_tlv_msg(p_serialized, CADE_PL_TAG_CARD_DEL, p_cards_current->cards[jdx].card_name, p_cards_current->cards[jdx].card_len);
                }
            }
            // Grouping
            encoded_len = cade_encap_grp_tlv_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_CARDDEAL, 0, generate_sequence(), p_serialized);
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            CADE_LOG("Sending CARDDEAL message to "PRIip" / %d...", IP_DOT(ntohl(ip)), ntohs(port));
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            p_player->state = CADE_STATE_SENT_CARDDEAL;
            p_player->is_in_carddeal = true;
        }
    }

    /*
     * 43414445 01 01 07 0000 1900 0100 8B00
     * 0100 4600 0300 0600 636172642031 0300 0600 636172642032 0300 0600 636172642033 0300 0600 636172642034 0300 0600 636172642035 0300 0600 636172642036 0300 0600 636172642037
     * 0200 3D00 0300 0600 636172642038 0300 0600 636172642039 0300 0700 63617264203130 0300 0600 63617264204A 0300 0600 636172642051 0300 0600 63617264204B
     */
    // Send to watchers
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_watchers; idx++)
    {
        // Memset 0 for buffers
        memset(p_msg, 0, sizeof(struct cade_msg_encoded));
        cade_tlv_encode_init(p_grp_serialized);

        // Destination info
        p_watcher = &p_cade->cade_member_tbl.arr_cade_watchers[idx];
        ip = p_watcher->ip_addr;
        port = p_watcher->port;

        // Encap all cards hold into tlv
        for (jdx = 0; jdx < p_cade->cade_member_tbl.nb_players; jdx++)
        {
            p_player = &p_cade->cade_member_tbl.arr_cade_players[jdx];
            if (p_player->is_alive)
            {
                // Memset 0 for serialized buffer
                cade_tlv_encode_init(p_serialized);

                // Encap cards
                p_cards_candidate = &p_player->candidate_cards;
                for (kdx = 0; kdx < p_cards_candidate->cards_num; kdx++)
                {
                    cade_add_tlv_msg(p_serialized, CADE_PL_TAG_CARD_LST, p_cards_candidate->cards[kdx].card_name, p_cards_candidate->cards[kdx].card_len);
                }
                // Group cards by tlv with tag of player uuid
                cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, p_player->uuid);
            }
        }
        // Grouping
        encoded_len = cade_encap_grp_tlv_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_CARDDEAL, 0, generate_sequence(), p_grp_serialized);
        //dump_cade_msg((uint8_t *) p_msg, encoded_len);

        // Send message
        CADE_LOG("Sending CARDDEAL message to watcher at "PRIip" / %d...", IP_DOT(ntohl(ip)), ntohs(port));
        cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
        p_watcher->state = CADE_STATE_SENT_CARDDEAL;
        p_watcher->is_in_carddeal = true;
    }

    return OK;
}

/**
 * @brief: Send CARDCOMMIT
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_cardcommit_msg(struct cade *p_cade)
{
    uint32_t ip;
    uint16_t port;
    struct cade_member_info *p_member;
    int idx;

    // Send to players
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_players; idx++)
    {
        p_member = &p_cade->cade_member_tbl.arr_cade_players[idx];
        ip = p_member->ip_addr;
        port = p_member->port;

        struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

        if (p_member->is_alive && p_member->is_in_carddeal)
        {
            // Encap CARDCOMMIT message
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            int encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_CARDCOMMIT, p_cade->uuid, generate_sequence());
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            CADE_LOG("Sending CARDCOMMIT message to "PRIip" / %d...", IP_DOT(ntohl(ip)), ntohs(port));
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            p_member->state = CADE_STATE_SENT_COMMIT;
        }
    }

    // Send to watchers
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_watchers; idx++)
    {
        p_member = &p_cade->cade_member_tbl.arr_cade_watchers[idx];
        ip = p_member->ip_addr;
        port = p_member->port;

        struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

        if (p_member->is_alive && p_member->is_in_carddeal)
        {
            // Encap CARDCOMMIT message
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            int encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_CARDCOMMIT, p_cade->uuid, generate_sequence());
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            CADE_LOG("Sending CARDCOMMIT message to "PRIip" / %d...", IP_DOT(ntohl(ip)), ntohs(port));
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            p_member->state = CADE_STATE_SENT_COMMIT;
        }
    }

    return OK;
}

/**
 * @brief: Send CARDROLLBACK
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_cardrollback_msg(struct cade *p_cade)
{
    uint32_t ip;
    uint16_t port;
    struct cade_member_info *p_member;
    int idx;

    // Send to players
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_players; idx++)
    {
        p_member = &p_cade->cade_member_tbl.arr_cade_players[idx];
        ip = p_member->ip_addr;
        port = p_member->port;

        struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

        if ((p_member->is_alive) && p_member->is_in_carddeal)
        {
            // Encap CARDROLLBACK message
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            int encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_CARDROLLBACK, p_cade->uuid, generate_sequence());
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            CADE_LOG("Sending CARDROLLBACK message to "PRIip" / %d...", IP_DOT(ntohl(ip)), ntohs(port));
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            p_member->state = CADE_STATE_SENT_ROLLBACK;
        }
    }

    // Send to watchers
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_watchers; idx++)
    {
        p_member = &p_cade->cade_member_tbl.arr_cade_watchers[idx];
        ip = p_member->ip_addr;
        port = p_member->port;

        struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

        if ((p_member->is_alive) && p_member->is_in_carddeal)
        {
            // Encap CARDROLLBACK message
            memset(p_msg, 0, sizeof(struct cade_msg_encoded));
            int encoded_len = cade_encap_without_pl_msg(p_msg, CADE_ROLE_DEALER, CADE_MSG_CARDROLLBACK, p_cade->uuid, generate_sequence());
            //dump_cade_msg((uint8_t *) p_msg, encoded_len);

            // Send message
            CADE_LOG("Sending CARDROLLBACK message to "PRIip" / %d...", IP_DOT(ntohl(ip)), ntohs(port));
            cade_send_msg_on_unicast_sock(p_cade, encoded_len, ip, port);
            p_member->state = CADE_STATE_SENT_ROLLBACK;
        }
    }

    return OK;
}

/**
 * @brief: Receive NOTIFY message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_notify_msg(struct cade *p_cade)
{
    uint8_t role;
    struct cade_msg_decoded *p_decoded;

    p_decoded = &p_cade->decoded_msg;

    if (CADE_MSG_NOTIFY == p_decoded->msg_type)
    {
        // Get machine id info
        struct tlv *tlv = cade_find_tlv_from_tag(&p_decoded->pl_tlv, CADE_PL_TAG_MACHINEID);
        if (NULL != tlv)
        {
            uint8_t *machineid = tlv->value;
            uint16_t machineid_len = tlv->length;
            struct sockaddr_in *peer_addr = &p_cade->sock.mcast_peer_addr;

            CADE_LOG("\nReceive NOTIFY message with info:");
            CADE_LOG("   machineid: \t%.*s", machineid_len, machineid);
            CADE_LOG("   ip: \t\t"PRIip"", IP_DOT(ntohl(peer_addr->sin_addr.s_addr)));
            CADE_LOG("   port: \t%d", (int) ntohs(peer_addr->sin_port));
            CADE_LOG("   uuid: \t%d", p_decoded->uuid);
            CADE_LOG("   role: \t%d", p_decoded->role);

            role = p_decoded->role;
            // Add member to the table
            if (CADE_ROLE_PLAYER == role)
            {
                int ret = add_player_member(p_cade->cade_member_tbl.arr_cade_players, &p_cade->cade_member_tbl.nb_players, peer_addr->sin_addr.s_addr,
                        htons(UCAST_PORT), 0, machineid, machineid_len, CADE_ADDING_NOTIFY, p_cade->decoded_msg.uuid);
                if (OK == ret)
                {
                    // Send IDASSIGN
                    cade_send_idassign_msg(p_cade, role);
                }
            } else if (CADE_ROLE_WATCHER == role)
            {
                int ret = add_player_member(p_cade->cade_member_tbl.arr_cade_watchers, &p_cade->cade_member_tbl.nb_watchers, peer_addr->sin_addr.s_addr,
                        htons(UCAST_PORT), 0, machineid, machineid_len, CADE_ADDING_NOTIFY, p_cade->decoded_msg.uuid);
                if (OK == ret)
                {
                    // Send IDASSIGN
                    cade_send_idassign_msg(p_cade, role);
                }
            }
        }
    }
    return OK;
}

/**
 * @brief: Receive RESPONSE message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_response_msg(struct cade *p_cade)
{
    struct some_cards *p_cards_hold;
    struct cade_member_info *p_member;
    struct tlv_decoded *p_tlv_decoded;
    struct tlv *tlv;

    int idx;
    if (CADE_MSG_RESPONSE == p_cade->decoded_msg.msg_type)
    {
        // Write machineid to table without caring about it exists or not
        tlv = cade_find_tlv_from_tag(&p_cade->decoded_msg.pl_tlv, CADE_PL_TAG_MACHINEID);
        if (NULL != tlv)
        {
            uint8_t *machineid = tlv->value;
            uint16_t machineid_len = tlv->length;
            CADE_LOG("\nReceive RESPONSE message with uuid %d machineid %.*s", (int) p_cade->decoded_msg.uuid, machineid_len, (char *) machineid);
            struct sockaddr_in *peer_addr = &p_cade->sock.mcast_peer_addr;

            // Add member to the table without caring about it exists or not
            if (CADE_ROLE_PLAYER == p_cade->decoded_msg.role)
            {
                int ret = add_player_member(p_cade->cade_member_tbl.arr_cade_players, &p_cade->cade_member_tbl.nb_players, peer_addr->sin_addr.s_addr,
                        htons(UCAST_PORT), 0, machineid, machineid_len, CADE_ADDING_RESPONSE, p_cade->decoded_msg.uuid);
                if (OK != ret)
                {
                    ERR_LOG("cade_recv_response_msg cannot add member machineid %.*s", machineid_len, machineid);
                }

                // Get member from machineid
                p_member = get_member_from_machineid(p_cade->cade_member_tbl.arr_cade_players, p_cade->cade_member_tbl.nb_players, machineid, machineid_len);
                if (NULL != p_member)
                {
                    p_cards_hold = &p_member->current_cards;

                    // Replace cards info to card table
                    remove_cards_hold(p_cards_hold);

                    // Add cards
                    p_tlv_decoded = &p_cade->decoded_msg.pl_tlv;
                    CADE_LOG("Player with uuid %d will be added with cards:", p_member->uuid);
                    for (idx = 0; idx < p_tlv_decoded->tlv_num; idx++)
                    {
                        if (CADE_PL_TAG_CARD_LST == p_tlv_decoded->arr_tlv[idx].tag)
                        {
                            CADE_LOG("   [%.*s]", p_tlv_decoded->arr_tlv[idx].length, p_tlv_decoded->arr_tlv[idx].value);
                            cade_add_card_for_player(p_cards_hold, p_tlv_decoded->arr_tlv[idx].value, p_tlv_decoded->arr_tlv[idx].length);
                        }
                    }
                }
            } else if (CADE_ROLE_WATCHER == p_cade->decoded_msg.role)
            {
                int ret = add_player_member(p_cade->cade_member_tbl.arr_cade_watchers, &p_cade->cade_member_tbl.nb_watchers, peer_addr->sin_addr.s_addr,
                        htons(UCAST_PORT), 0, machineid, machineid_len, CADE_ADDING_RESPONSE, p_cade->decoded_msg.uuid);
                if (OK != ret)
                {
                    ERR_LOG("cade_recv_response_msg cannot add member machineid %.*s", machineid_len, machineid);
                }
                CADE_LOG("Watcher with uuid %d machineid %.*s", p_cade->decoded_msg.uuid, machineid_len, machineid);
            }
        }
    }

    return OK;
}

/**
 * @brief: Receive PONG message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_pong_msg(struct cade *p_cade)
{
    if (CADE_MSG_PONG == p_cade->decoded_msg.msg_type)
    {
        struct cade_member_info *p_member = NULL;
        if (CADE_ROLE_PLAYER == p_cade->decoded_msg.role)
        {
            p_member = get_member_from_uuid(p_cade->cade_member_tbl.arr_cade_players, p_cade->cade_member_tbl.nb_players, p_cade->decoded_msg.uuid);
        } else if (CADE_ROLE_WATCHER == p_cade->decoded_msg.role)
        {
            p_member = get_member_from_uuid(p_cade->cade_member_tbl.arr_cade_watchers, p_cade->cade_member_tbl.nb_watchers, p_cade->decoded_msg.uuid);
        }

        if ( (NULL != p_member) && p_member->is_alive && (CADE_STATE_NORMAL == p_member->state) )
        {
            p_member->ping_timeout_counter = NB_PING_TIMEOUT;
        }
    }
    return OK;
}

/**
 * @brief: Handle the message received
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_msg_template(struct cade *p_cade, enum cade_msg_type msg_type, enum cade_state cur_state, enum cade_state next_state)
{
    if (msg_type == p_cade->decoded_msg.msg_type)
    {
        struct cade_member_info *p_member = NULL;

        if (CADE_ROLE_PLAYER == p_cade->decoded_msg.role)
        {
            p_member = get_member_from_uuid(p_cade->cade_member_tbl.arr_cade_players, p_cade->cade_member_tbl.nb_players, p_cade->decoded_msg.uuid);
        } else if (CADE_ROLE_WATCHER == p_cade->decoded_msg.role)
        {
            p_member = get_member_from_uuid(p_cade->cade_member_tbl.arr_cade_watchers, p_cade->cade_member_tbl.nb_watchers, p_cade->decoded_msg.uuid);
        }

        if ((NULL != p_member) && (cur_state == p_member->state))
        {
            CADE_LOG("Receive [%s] from member uuid %d", txt_code_msg_type[msg_type], p_member->uuid);
            p_member->state = next_state;
            return OK;
        }
    }
    return ERR;
}

/**
 * @brief: Receive CARRDEAL_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_carddeal_ok_msg(struct cade *p_cade)
{
    return cade_recv_msg_template(p_cade, CADE_MSG_CARDDEAL_OK, CADE_STATE_SENT_CARDDEAL, CADE_STATE_RECV_CARDDEAL_OK);
}

/**
 * @brief: Receive CARDDEAL_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_carddeal_err_msg(struct cade *p_cade)
{
    return cade_recv_msg_template(p_cade, CADE_MSG_CARDDEAL_ERR, CADE_STATE_SENT_CARDDEAL, CADE_STATE_RECV_CARDDEAL_ERR);
}

/**
 * @brief: Receive CARDCOMMIT_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_cardcommit_ok_msg(struct cade *p_cade)
{
    return cade_recv_msg_template(p_cade, CADE_MSG_CARDCOMMIT_OK, CADE_STATE_SENT_COMMIT, CADE_STATE_RECV_COMMIT_OK);
}

/**
 * @brief: Receive CARDCOMMIT_ERR message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_cardcommit_err_msg(struct cade *p_cade)
{
    return cade_recv_msg_template(p_cade, CADE_MSG_CARDCOMMIT_ERR, CADE_STATE_SENT_COMMIT, CADE_STATE_RECV_COMMIT_ERR);
}

/**
 * @brief: Receive ROLLBACK_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_cardrollback_ok_msg(struct cade *p_cade)
{
    return cade_recv_msg_template(p_cade, CADE_MSG_CARDROLLBACK_OK, CADE_STATE_SENT_ROLLBACK, CADE_STATE_RECV_ROLLBACK_OK);
}

/**
 * @brief: Receive ROLLBACK_ERR message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_cardrollback_err_msg(struct cade *p_cade)
{
    return cade_recv_msg_template(p_cade, CADE_MSG_CARDROLLBACK_ERR, CADE_STATE_SENT_ROLLBACK, CADE_STATE_RECV_ROLLBACK_ERR);
}

/**
 * @brief: Do dealing cards with two phase commit
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_do_carddeal_two_phase_commit(struct cade *p_cade)
{
    // Calculate dealing cards
    CADE_LOG("Do dealing cards");
    time_t endsearch;
    enum cade_msg_type msg_type;

    struct cade_member_tbl *p_cade_member_tbl = &p_cade->cade_member_tbl;

    cade_do_deal_cards(p_cade);

    // Send CARDDEAL to players
    cade_send_carddeal_msg(p_cade);

    int nb_carddeal_sent = get_nb_carrdeal_sent(p_cade_member_tbl);
    CADE_LOG("Send CARDDEAL to %d members", nb_carddeal_sent);

    // Receive on unicast socket
    endsearch = time(NULL) + CADE_SEC_WAIT_CARDDEAL_REP;
    while ((get_nb_carrdeal_reply(p_cade_member_tbl) < nb_carddeal_sent) &&
            (time(NULL) < endsearch))
    {
        // Receive messages on unicast socket
        if (OK == cade_recv_msg_on_unicast_sock(p_cade))
        {
            msg_type = p_cade->decoded_msg.msg_type;
            cade_recv_msg_handle(p_cade, msg_type);
        }
    }

    // Check if go ahead with sending CARD_COMMIT or CARDROLLBACK
    if (is_all_state_carddeal_ok(p_cade_member_tbl))
    {
        CADE_LOG("\nAll members are ok with CARDDEAL");
        CADE_LOG("Send CARDCOMMIT");
        cade_send_cardcommit_msg(p_cade);
    } else
    {
        CADE_LOG("\nSome members are NOT ok with CARDDEAL");
        CADE_LOG("Send CARDROLLBACK");
        cade_send_cardrollback_msg(p_cade);
    }

    // Wait for COMMIT_OK or COMMIT_ERR or ROLLBACL_OK or ROLLBACK_ERR in 1s
    endsearch = time(NULL) + CADE_SEC_WAIT_CARDDEAL_REP;
    while ((get_nb_commit_reply(p_cade_member_tbl) < nb_carddeal_sent) &&
            (get_nb_rollback_reply(p_cade_member_tbl) < nb_carddeal_sent) &&
            (time(NULL) < endsearch))
    {
        // Receive messages on unicast socket
        if (OK == cade_recv_msg_on_unicast_sock(p_cade))
        {
            msg_type = p_cade->decoded_msg.msg_type;
            cade_recv_msg_handle(p_cade, msg_type);
        }
    }

    // Set to NORMAL state
    finalize_carddeal_transaction(p_cade_member_tbl);

    // Call callback function to info
    p_cade->cb_hdl_recv_cards_success(p_cade);

    return OK;
}

/**
 * @brief: Handle message received based on message type
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_msg_handle(struct cade *p_cade, uint8_t msg_type)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_recv_msg_handle p_cade NULL");
        return ERR;
    }

    if ((CADE_MSG_NOT_USED < msg_type) && (msg_type < CADE_MSG_END_MARK))
    {
        DBG_LOG("Receive msg type [%s] with sequence number %d", txt_code_msg_type[msg_type], (int) p_cade->decoded_msg.sequence);
        switch (msg_type)
        {
        case CADE_MSG_NOTIFY:
            cade_recv_notify_msg(p_cade);
            break;
        case CADE_MSG_RESPONSE:
            cade_recv_response_msg(p_cade);
            break;
        case CADE_MSG_PONG:
            cade_recv_pong_msg(p_cade);
            break;
        case CADE_MSG_CARDDEAL_OK:
            cade_recv_carddeal_ok_msg(p_cade);
            break;
        case CADE_MSG_CARDDEAL_ERR:
            cade_recv_carddeal_err_msg(p_cade);
            break;
        case CADE_MSG_CARDCOMMIT_OK:
            cade_recv_cardcommit_ok_msg(p_cade);
            break;
        case CADE_MSG_CARDCOMMIT_ERR:
            cade_recv_cardcommit_err_msg(p_cade);
            break;
        case CADE_MSG_CARDROLLBACK_OK:
            cade_recv_cardrollback_ok_msg(p_cade);
            break;
        case CADE_MSG_CARDROLLBACK_ERR:
            cade_recv_cardrollback_err_msg(p_cade);
            break;
        default:
            break;
        }
    } else
    {
        CADE_LOG("Receive unknown msg type [%d]", msg_type);
        return ERR;
    }
    return OK;
}

/**
 * @brief: Handle message received based on message type
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static void *mcast_sock_handle(void *ptr)
{
    struct cade *p_cade = (struct cade *) ptr;
    enum cade_msg_type msg_type = p_cade->decoded_msg.msg_type;
    struct cade_member_tbl *p_cade_member_tbl = &p_cade->cade_member_tbl;

    // Set params for polld
    struct pollfd  poll_d[1];
    memset(poll_d, 0 , sizeof(poll_d));
    poll_d[0].fd = p_cade->sock.mcast_recv_sockfd;
    poll_d[0].events = POLLIN;
    int ret;
    while (true)
    {
        // Poll the sockets for an event to occur
        ret = poll(poll_d, 1, CADE_MILI_SEC_POLL_TIMEOUT);
        if (ret == 0)
        {
            continue;
        } else if(ret < 0) {
            ERR_LOG("Mcast err in POLL from socket with cause %d", ret);
            continue;
        } else
        {
            // Receive NOTIFY messages on receiving socket (multicast)
            if (OK == cade_recv_msg_on_mcast_r_sock(p_cade))
            {
                msg_type = p_cade->decoded_msg.msg_type;
                if (CADE_MSG_NOTIFY == msg_type)
                {
                    // Lock info table
                    pthread_mutex_lock(&p_cade->tbl_lock);

                    cade_recv_msg_handle(p_cade, msg_type);

                    // Send CARDDEAL if any player does not have cards
                    if (is_one_player_not_having_card(p_cade_member_tbl))
                    {
                        CADE_LOG("At least one player not having card --> Deal cards");
                        cade_do_carddeal_two_phase_commit(p_cade);
                    }

                    // Unlock info table
                    pthread_mutex_unlock(&p_cade->tbl_lock);
                }
            }
        }
    }
    void *retrn = NULL;
    return retrn;
}

/**
 * @brief: PING timer handle
 *
 * @param [IN]
 *
 * @return: void
 */
static void ping_sending_handle(union sigval timer_data)
{
    /* Function was run each 0.5 second */
    struct cade *p_cade = (struct cade *) timer_data.sival_ptr;
    if (get_nb_alive_players(&p_cade->cade_member_tbl) > 0)
    {
        pthread_mutex_lock(&p_cade->tbl_lock);
        cade_send_ping_msg(p_cade);
        pthread_mutex_unlock(&p_cade->tbl_lock);
    }
}

/**
 * @brief: Start sending PING timer
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int start_ping_timer(struct cade *p_cade)
{
    struct sigevent     sev;
    struct itimerspec   its;
    timer_t             timerid;

    /* Establish handler and create for timer thread */
    sev.sigev_notify            = SIGEV_THREAD;
    sev.sigev_notify_function   = ping_sending_handle;
    sev.sigev_value.sival_ptr = (void *) p_cade;
    sev.sigev_notify_attributes = NULL;

    /* Setup periodic timer */
    its.it_value.tv_sec     = TIME_PING_SEND_SEC;
    its.it_value.tv_nsec    = TIME_PING_SEND_NSEC;
    its.it_interval.tv_sec  = TIME_PING_SEND_SEC;
    its.it_interval.tv_nsec = TIME_PING_SEND_NSEC;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
    {
        ERR_LOG("Can't create timer");
        return ERR;
    }

    if (timer_settime(timerid, 0, &its, NULL) == -1)
    {
        ERR_LOG("Can't set timer");
        return ERR;
    }
    return OK;
}

/**
 * @brief: Show cards hold by members
 *
 * @param [IN]
 *
 * @return : void
 */
void cade_show_members_info(struct cade *p_cade)
{
    struct cade_member_tbl *p_cade_member_tbl = &p_cade->cade_member_tbl;
    struct cade_member_info *p_cade_member_info;
    struct cards *p_cards;

    CADE_LOG("\n\n\n");
    int idx, jdx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        p_cade_member_info = &p_cade_member_tbl->arr_cade_players[idx];
        CADE_LOG("\nPlayer %d \tuuid %d \tis_alive %d \tis_in_carddeal %d \tstate %d \tmachineid %.*s\r", idx, p_cade_member_info->uuid,
                p_cade_member_info->is_alive, p_cade_member_info->is_in_carddeal, p_cade_member_info->state,
                p_cade_member_info->machineid_len, p_cade_member_info->machineid);
        CADE_N_LOG("Cards: [");
        for (jdx = 0; jdx < p_cade_member_info->current_cards.cards_num; jdx++)
        {
            p_cards = &p_cade_member_info->current_cards.cards[jdx];
            CADE_N_LOG("%.*s, ", p_cards->card_len, p_cards->card_name);
        }
        CADE_LOG("]");
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        p_cade_member_info = &p_cade_member_tbl->arr_cade_watchers[idx];
        CADE_LOG("\nWatcher %d \tuuid %d \tis_alive %d \tis_in_carddeal %d \tstate %d \tmachineid %.*s\r", idx, p_cade_member_info->uuid,
                p_cade_member_info->is_alive, p_cade_member_info->is_in_carddeal, p_cade_member_info->state,
                p_cade_member_info->machineid_len, p_cade_member_info->machineid);
    }
}

/**
 * @brief: Init resource for cade dealer instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_dealer_init(const char *if_addr)
{
    assert(NULL != if_addr);
    CADE_LOG("Initialize cade...with %lu bytes", sizeof(struct cade));

    struct cade *p_cade = (struct cade *) malloc(sizeof(struct cade));
    assert(NULL != p_cade);

    // 0. memset 0
    memset(p_cade, 0, sizeof(struct cade));

    // 1. Init role and mode
    p_cade->role = CADE_ROLE_DEALER;

    // 2. Init cade table
    init_cade_member_tbl(&p_cade->cade_member_tbl);

    // 3. Init socket
    if (OK != cade_sock_init(&p_cade->sock, if_addr))
    {
        ERR_LOG("   [ERROR] Init cade socket failed!!!");
        exit(ERR);
    }

    // 4. Init mutex lock
    if (OK != pthread_mutex_init(&p_cade->tbl_lock, NULL)) {
        ERR_LOG("Mutex init for table has failed\n");
        exit(ERR);
    }

    CADE_LOG("Initialize cade DONE");
    return p_cade;
}

/**
 * @brief: Init resource for cade dealer instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_dealer_init_uint32_ip(uint32_t if_addr)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = if_addr;
    CADE_LOG("Initialize cade with ip address [%s]", inet_ntoa(ip_addr));
    return cade_dealer_init(inet_ntoa(ip_addr));
}

/**
 * @brief: Add card for poker set
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_dealer_add_poker(struct cade *p_cade, uint8_t *card, uint16_t len)
{
    return cade_add_card_for_poker_set(p_cade, card, len);
}

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_dealer_set_cb_recv_cards_success(struct cade *p_cade, void (*cb_hdl_recv_cards)(struct cade *))
{
    return set_cb_recv_cards_success(p_cade, cb_hdl_recv_cards);
}

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_dealer_set_cb_show_cards_info(struct cade *p_cade, void (*cb_hdl_show_cards)(struct cade *))
{
    return set_cb_show_cards(p_cade, cb_hdl_show_cards);
}

/**
 * @brief: Run cade dealer
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_dealer_run(struct cade *p_cade, int magic_nb_players, int magic_nb_watchers)
{
    assert(NULL != p_cade);

    time_t endsearch;
    clock_t begin_time, cur_time;
    double duration;
    enum cade_msg_type msg_type;

    struct pollfd  poll_d[1];
    int ret;

    // 1. On starting, Sends MSEARCH, wait for RESPONSE in 1s to update all players running if any
    CADE_LOG("\n\n 1. Wait for some seconds for sending MSEARCH and receiving RESPONSE");
    begin_time = clock();
    endsearch = time(NULL) + CADE_SEC_MSEARCH_DURATION;
    while ( (time(NULL) < endsearch) && ( (get_nb_alive_players(&p_cade->cade_member_tbl) < magic_nb_players) ||
            (get_nb_alive_watchers(&p_cade->cade_member_tbl) < magic_nb_watchers) ) )
    {
        // 1.1 Send MSEARCH
        cur_time = clock();
        duration = (double)(cur_time - begin_time) / CLOCKS_PER_MILI_SEC;
        if (duration >= CADE_MILI_SEC_MSEARCH_FREQ)
        {
            CADE_LOG("\nSend MSEARCH");
            cade_send_msearch_msg(p_cade);
            begin_time = cur_time;
        }

        // 1.2 Receive messages on sending socket
        if (OK == cade_recv_msg_on_mcast_s_sock(p_cade))
        {
            msg_type = p_cade->decoded_msg.msg_type;
            cade_recv_msg_handle(p_cade, msg_type);
        }
    }

    // 2. Wait for NOTIFY messages until all players have joined or timeup
    CADE_LOG("\n\n2. Wait for NOTIFY messages until %d players have joined, current %d", magic_nb_players, get_nb_alive_players(&p_cade->cade_member_tbl));
    while (get_nb_alive_players(&p_cade->cade_member_tbl) < magic_nb_players)
    {
        // Receive NOTIFY messages on receiving socket (multicast)
        if (OK == cade_recv_msg_on_mcast_r_sock(p_cade))
        {
            msg_type = p_cade->decoded_msg.msg_type;
            cade_recv_msg_handle(p_cade, msg_type);
        }
    }

    // 3. If one player not having cards exists, do deal cards
    CADE_LOG("\n\n 3. All members for now have joined");
    struct cade_member_tbl *p_cade_member_tbl = &p_cade->cade_member_tbl;
    if (is_one_player_not_having_card(p_cade_member_tbl))
    {
        CADE_LOG("At least one player not having card --> Deal cards");
        cade_do_carddeal_two_phase_commit(p_cade);
    }

    // 4. Create new thread to listen on multicast socket
    pthread_t mcast_thread;
    ret = pthread_create(&mcast_thread, NULL, mcast_sock_handle, (void *) p_cade);
    CADE_LOG("4. Multicast receiving thread returns: %d", ret);

    // 5. Start new thread always sends PINGs
    CADE_LOG("5. Start new thread to send PING ");
    start_ping_timer(p_cade);

    // 6. Main thread keep receiving PONGs
    // Set params for polld
    memset(poll_d, 0 , sizeof(poll_d));
    poll_d[0].fd = p_cade->sock.ucast_sockfd;
    poll_d[0].events = POLLIN;

#ifdef DBG_CARDS_HOLD
    time_t show_time = time(NULL) + CADE_SEC_SHOW_CARD_DEBUG_FREQ;
#endif
    while (true)
    {
        // Poll the sockets for an event to occur
        ret = poll(poll_d, 1, CADE_MILI_SEC_POLL_TIMEOUT);
        if (ret == 0)
        {
            //continue;
        } else if(ret < 0) {
            ERR_LOG("Err in POLL from socket with cause %d", ret);
            //continue;
        } else
        {
            if (poll_d[0].revents & POLLIN)
            {
                if (OK == cade_recv_msg_on_unicast_sock(p_cade))
                {
                    msg_type = p_cade->decoded_msg.msg_type;
                    if (CADE_MSG_PONG == msg_type)
                    {
                        // Lock info table
                        pthread_mutex_lock(&p_cade->tbl_lock);

                        cade_recv_msg_handle(p_cade, msg_type);

                        // Unlock info table
                        pthread_mutex_unlock(&p_cade->tbl_lock);
                    }
                }
            }
        }

#ifdef DBG_CARDS_HOLD
        // Show cards for debug
        if (time(NULL) >= show_time)
        {
            if (NULL != p_cade->cb_hdl_show_cards_info)
            {
                p_cade->cb_hdl_show_cards_info(p_cade);
            }
            show_time = time(NULL) + CADE_SEC_SHOW_CARD_DEBUG_FREQ;
        }
#endif

    }

    return OK;
}


