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
 * @file      cade_member.c
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      May 29, 2020
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
#include "cade_member.h"

//    /* Msg type                      Sending function                    Receiving function*/
//    { CADE_MSG_NOT_USED        ,     cade_do_nothing,                    cade_do_nothing            },
//    { CADE_MSG_NOTIFY          ,     cade_send_notify_msg,               cade_do_nothing            },
//    { CADE_MSG_IDASSIGN        ,     cade_do_nothing,                    cade_recv_idassign_msg     },
//    { CADE_MSG_MSEARCH         ,     cade_do_nothing,                    cade_recv_msearch_msg      },
//    { CADE_MSG_RESPONSE        ,     cade_send_response_msg,             cade_do_nothing            },
//    { CADE_MSG_PING            ,     cade_do_nothing,                    cade_recv_ping_msg         },
//    { CADE_MSG_PONG            ,     cade_send_pong_msg,                 cade_do_nothing            },
//    { CADE_MSG_CARDDEAL        ,     cade_do_nothing,                    cade_recv_carddeal_msg     },
//    { CADE_MSG_CARDDEAL_OK     ,     cade_send_carddeal_ok,              cade_do_nothing            },
//    { CADE_MSG_CARDDEAL_ERR    ,     cade_send_carddeal_err,             cade_do_nothing            },
//    { CADE_MSG_CARDCOMMIT      ,     cade_do_nothing,                    cade_recv_cardcommit_msg   },
//    { CADE_MSG_CARDCOMMIT_OK   ,     cade_send_cardcommit_ok,            cade_do_nothing            },
//    { CADE_MSG_CARDCOMMIT_ERR  ,     cade_send_cardcommit_err,           cade_do_nothing            },
//    { CADE_MSG_CARDROLLBACK    ,     cade_do_nothing,                    cade_recv_cardrollback_msg },
//    { CADE_MSG_CARDROLLBACK_OK ,     cade_send_cardrollback_ok,          cade_do_nothing            },
//    { CADE_MSG_CARDROLLBACK_ERR,     cade_send_cardrollback_err,         cade_do_nothing            }

extern char *txt_code_msg_type[];

static int cade_send_notify_msg(struct cade *p_cade);
static int cade_send_response_msg(struct cade *p_cade);
static int cade_send_pong_msg(struct cade *p_cade);
static int cade_send_carddeal_ok(struct cade *p_cade);
static int cade_send_carddeal_err(struct cade *p_cade);
static int cade_send_cardcommit_ok(struct cade *p_cade);
static int cade_send_cardcommit_err(struct cade *p_cade);
static int cade_send_cardrollback_ok(struct cade *p_cade);
static int cade_send_cardrollback_err(struct cade *p_cade);

static int cade_recv_idassign_msg(struct cade *p_cade);
static int cade_recv_msearch_msg(struct cade *p_cade);
static int cade_recv_ping_msg(struct cade *p_cade);
static int cade_recv_carddeal_msg(struct cade *p_cade);
static int cade_recv_cardcommit_msg(struct cade *p_cade);
static int cade_recv_cardrollback_msg(struct cade *p_cade);

static int cade_recv_msg_handle(struct cade *p_cade, uint8_t msg_type);

/**
 * @brief: Send NOTIFY message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_notify_msg(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_notify_msg p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;
    struct tlv_serialization s_serialized, *p_serialized = &s_serialized;

    // Encap NOTIFY message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    cade_tlv_encode_init(p_serialized);
    cade_add_tlv_msg(p_serialized, CADE_PL_TAG_MACHINEID, p_cade->machineid, p_cade->machineid_len);
    int encoded_len = cade_encap_grp_tlv_msg(p_msg, p_cade->role, CADE_MSG_NOTIFY, p_cade->uuid, generate_sequence(), p_serialized);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_send_msg_on_mcast_s_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send RESPONSE message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_response_msg(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_response_msg p_cade NULL");
        return ERR;
    }

    int idx, encoded_len;

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;
    struct tlv_serialization s_serialized, *p_serialized = &s_serialized;

    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    cade_tlv_encode_init(p_serialized);

    // Encap machineid
    cade_add_tlv_msg(p_serialized, CADE_PL_TAG_MACHINEID, p_cade->machineid, p_cade->machineid_len);

    if (CADE_ROLE_PLAYER == p_cade->role)
    {
        // Encap all cards hold into tlv
        for (idx = 0; idx < p_cade->current_cards.cards_num; idx++)
        {
            cade_add_tlv_msg(p_serialized, CADE_PL_TAG_CARD_LST, p_cade->current_cards.cards[idx].card_name, p_cade->current_cards.cards[idx].card_len);
        }
    }
    encoded_len = cade_encap_grp_tlv_msg(p_msg, p_cade->role, CADE_MSG_RESPONSE, p_cade->uuid, generate_sequence(), p_serialized);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_send_msg_on_mcast_r_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send PONG message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_pong_msg(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_pong_msg p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap PONG message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_PONG, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send CARDDEAL_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_carddeal_ok(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_carddeal_ok p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap CARDDEAL_OK message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_CARDDEAL_OK, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send CARDDEAL_ERR message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_carddeal_err(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_carddeal_err p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap CARDDEAL_OK message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_CARDDEAL_ERR, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send CARDCOMMIT_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_cardcommit_ok(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_cardcommit_ok p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap CARDCOMMIT_OK message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_CARDCOMMIT_OK, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send CARDCOMMIT_ERR message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_cardcommit_err(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_cardcommit_err p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap CARDCOMMIT_ERR message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_CARDCOMMIT_ERR, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send CARDROLLBACK_OK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_cardrollback_ok(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_cardrollback_ok p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap ROLLBACK message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_CARDROLLBACK_OK, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Send CARDROLLBACK_ERR message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_send_cardrollback_err(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_send_cardrollback_err p_cade NULL");
        return ERR;
    }

    struct cade_msg_encoded *p_msg = &p_cade->encoded_msg;

    // Encap ROLLBACK message
    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    int encoded_len = cade_encap_without_pl_msg(p_msg, p_cade->role, CADE_MSG_CARDROLLBACK_ERR, p_cade->uuid, p_cade->decoded_msg.sequence);
    //dump_cade_msg((uint8_t *) p_msg, encoded_len);

    // Send message
    cade_rep_msg_on_unicast_sock(p_cade, encoded_len);

    return OK;
}

/**
 * @brief: Receive IDASSIGN
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_idassign_msg(struct cade *p_cade)
{
    if (CADE_MSG_IDASSIGN == p_cade->decoded_msg.msg_type)
    {
        uint16_t uuid = p_cade->decoded_msg.uuid;
        if (uuid > 0)
        {
            CADE_LOG("   Uuid assigned is %d", uuid);
            p_cade->uuid = uuid;
            p_cade->state = CADE_STATE_NORMAL;
        } else
        {
            ERR_LOG("   Not valid uuid %d", uuid);
            return ERR;
        }
    }
    return OK;
}

/**
 * @brief: Receive MSEARCH message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_msearch_msg(struct cade *p_cade)
{
    if (CADE_MSG_MSEARCH == p_cade->decoded_msg.msg_type)
    {
        CADE_LOG("   Send RESPONSE");
        cade_send_response_msg(p_cade);
    }
    return OK;
}

/**
 * @brief: Receive MSEARCH message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_ping_msg(struct cade *p_cade)
{
    if (CADE_MSG_PING == p_cade->decoded_msg.msg_type)
    {
        DBG_LOG("Receive ping with sequence number %d -> Reply PONG", (int) p_cade->decoded_msg.sequence);
        cade_send_pong_msg(p_cade);
    }
    return OK;
}

/**
 * @brief: Receive CARDDEAL message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_carddeal_msg(struct cade *p_cade)
{
    bool is_valid = true;
    if (CADE_MSG_CARDDEAL == p_cade->decoded_msg.msg_type)
    {
        if (CADE_ROLE_PLAYER == p_cade->role)
        {
            struct tlv *p_tlv;
            struct tlv_decoded *p_tlv_decoded = &p_cade->decoded_msg.pl_tlv;

            // Firstly copy current card to candidate cards
            copy_player_cards_hold(&p_cade->candidate_cards, &p_cade->current_cards);

            // Check if the CARDDEAL is valid. If valid, apply it to candidate zone
            int idx;
            for (idx = 0; idx < p_tlv_decoded->tlv_num; idx++)
            {
                p_tlv = &p_tlv_decoded->arr_tlv[idx];
                if (CADE_PL_TAG_CARD_ADD == p_tlv->tag)
                {
                    CADE_LOG("   ADD: [%.*s]", p_tlv->length,  p_tlv->value);
                    if (is_hold_by_this_player(&p_cade->candidate_cards,  p_tlv->value, p_tlv->length))
                    {
                        is_valid = false;
                    } else
                    {
                        cade_add_card_for_player(&p_cade->candidate_cards, p_tlv->value, p_tlv->length);
                    }
                } else if (CADE_PL_TAG_CARD_DEL ==  p_tlv->tag)
                {
                    CADE_LOG("   DELETE: [%.*s]",  p_tlv->length,  p_tlv->value);
                    if (!is_hold_by_this_player(&p_cade->candidate_cards,  p_tlv->value, p_tlv->length))
                    {
                        is_valid = false;
                    } else
                    {
                        cade_delete_card_from_player(&p_cade->candidate_cards, p_tlv->value, p_tlv->length);
                    }
                }
            }
        } else if (CADE_ROLE_WATCHER == p_cade->role)
        {
            struct player_info *p_player_info;
            struct tlv_decoded s_sub_tlv, *p_sub_tlv = &s_sub_tlv;
            uint16_t uuid;

            struct tlv_decoded *p_tlv_decoded = &p_cade->decoded_msg.pl_tlv;

            // Firstly delete candidate zone
            p_player_info = &p_cade->candidate_player_info;
            remove_players_info(p_player_info);

            int idx, jdx;
            for (idx = 0; idx < p_tlv_decoded->tlv_num; idx++)
            {
                uuid = p_tlv_decoded->arr_tlv[idx].tag;
                CADE_LOG("CARDDEAL player uuid %d", uuid);
                cade_add_player_for_watcher(p_player_info, uuid);

                // Decode deeper
                cade_tlv_decode_init(p_sub_tlv);
                cade_tlv_decode(p_sub_tlv, (uint8_t *) p_tlv_decoded->arr_tlv[idx].value, p_tlv_decoded->arr_tlv[idx].length);
                for (jdx = 0; jdx < p_sub_tlv->tlv_num; jdx++)
                {
                    CADE_LOG("   Cards: %.*s", p_sub_tlv->arr_tlv[jdx].length, p_sub_tlv->arr_tlv[jdx].value);
                    cade_add_card_for_player(&p_player_info->arr_players[idx].cards_hold, p_sub_tlv->arr_tlv[jdx].value, p_sub_tlv->arr_tlv[jdx].length);
                }
            }
            is_valid = true;
        }

        if (is_valid)
        {
            // Send CADE_CARDDEAL_OK
            CADE_LOG("CARDDEAL is validated -> Send CARDEAL_OK");
            cade_send_carddeal_ok(p_cade);
        } else
        {
            // Send CADE_CARDDEAL_ERR
            CADE_LOG("CARDDEAL is not valid -> Send CARDEAL_ERR");
            cade_send_carddeal_err(p_cade);
        }

        // Set state
        p_cade->state = CADE_STATE_SENT_REPLY_CARDDEAL;
    }

    return OK;
}

/**
 * @brief: Receive CARDCOMMIT message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_cardcommit_msg(struct cade *p_cade)
{
    int ret = ERR;
    if (CADE_MSG_CARDCOMMIT == p_cade->decoded_msg.msg_type)
    {
        if (CADE_ROLE_PLAYER == p_cade->role)
        {
            ret = copy_player_cards_hold(&p_cade->current_cards, &p_cade->candidate_cards);
        } else if (CADE_ROLE_WATCHER == p_cade->role)
        {
            ret = copy_watcher_player_info(&p_cade->current_player_info, &p_cade->candidate_player_info);
        }

        if (OK == ret)
        {
            CADE_LOG("Commit sucess -> Send COMMIT_OK");
            cade_send_cardcommit_ok(p_cade);

            // Call callback function to info
            p_cade->cb_hdl_recv_cards_success(p_cade);
        } else
        {
            CADE_LOG("Commit failed -> Send COMMIT_ERR");
            cade_send_cardcommit_err(p_cade);
        }

        // Set state to normal
        p_cade->state = CADE_STATE_NORMAL;
    }
    return OK;
}

/**
 * @brief: Receive CARDROLLBACK message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_recv_cardrollback_msg(struct cade *p_cade)
{
    int ret = ERR;
    if (CADE_MSG_CARDROLLBACK == p_cade->decoded_msg.msg_type)
    {
        if (CADE_ROLE_PLAYER == p_cade->role)
        {
            ret = copy_player_cards_hold(&p_cade->candidate_cards, &p_cade->current_cards);
        } else if (CADE_ROLE_WATCHER == p_cade->role)
        {
            ret = OK;
        }

        if (OK == ret)
        {
            CADE_LOG("Rollback sucess -> Send ROLLBACK_OK");
            cade_send_cardrollback_ok(p_cade);
        } else
        {
            CADE_LOG("Rollback failed -> Send ROLLBACK_ERR");
            cade_send_cardrollback_err(p_cade);
        }

        // Set state to normal
        p_cade->state = CADE_STATE_NORMAL;
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
static int cade_recv_msg_handle(struct cade *p_cade, uint8_t msg_type)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_recv_msg_handle p_cade NULL");
        return ERR;
    }

    if ((CADE_MSG_NOT_USED < msg_type) && (msg_type < CADE_MSG_END_MARK))
    {
        DBG_LOG("Receive msg type [%s]", txt_code_msg_type[msg_type]);
        switch (msg_type)
        {
        case CADE_MSG_IDASSIGN:
            if (CADE_STATE_SENT_NOTIFY == p_cade->state)
            {
                cade_recv_idassign_msg(p_cade);
            }
            break;
        case CADE_MSG_MSEARCH:
            if (CADE_STATE_NORMAL == p_cade->state)
            {
                cade_recv_msearch_msg(p_cade);
            }
            break;
        case CADE_MSG_PING:
            if (CADE_STATE_NORMAL == p_cade->state)
            {
                cade_recv_ping_msg(p_cade);
            }
            break;
        case CADE_MSG_CARDDEAL:
            if (CADE_STATE_NORMAL == p_cade->state)
            {
                cade_recv_carddeal_msg(p_cade);
            }
            break;
        case CADE_MSG_CARDCOMMIT:
            if (CADE_STATE_SENT_REPLY_CARDDEAL == p_cade->state)
            {
                cade_recv_cardcommit_msg(p_cade);
            }
            break;
        case CADE_MSG_CARDROLLBACK:
            if (CADE_STATE_SENT_REPLY_CARDDEAL == p_cade->state)
            {
                cade_recv_cardrollback_msg(p_cade);
            }
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
 * @brief: Init resource for cade member instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_member_init(enum cade_role role, const char *if_addr)
{
    assert(NULL != if_addr);

    int ret;

    CADE_LOG("Initialize cade...with %lu bytes", sizeof(struct cade));

    struct cade *p_cade = (struct cade *) malloc(sizeof(struct cade));
    assert(NULL != p_cade);

    // 0. memset 0
    memset(p_cade, 0, sizeof(struct cade));

    // 1. Init role and id
    p_cade->role = role;
    p_cade->uuid = 0;

    ret = get_machine_id(p_cade->machineid, MACHINEID_LEN);
    if (ERR == ret)
    {
        ERR_LOG("   [ERROR] Cannot get machine id!!!");
        exit(ERR);
    } else
    {
        p_cade->machineid_len = ret;
    }

    // Init cards hold
    memset(&p_cade->current_cards, 0, sizeof(struct some_cards));

    // 2. Init socket
    if (OK != cade_sock_init(&p_cade->sock, if_addr))
    {
        ERR_LOG("   [ERROR] Init cade socket failed!!!");
        exit(ERR);
    }

    CADE_LOG("Initialize cade DONE");
    return p_cade;
}

/**
 * @brief: Wait for uuid. This function will block until getting uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
uint16_t cade_member_request_for_uuid(struct cade *p_cade)
{
    assert(NULL != p_cade);

    uint8_t msg_type = 0;
    clock_t begin_time = clock();
    clock_t end_time;
    double duration;

    // 1. Wait for until getting uuid. uuid is get by p_cade->uuid
    CADE_LOG("\n\n 1. Wait for until getting uuid");
    while (0 == p_cade->uuid)
    {
        // Send NOTIFY message
        end_time = clock();
        duration = (double)(end_time - begin_time) / CLOCKS_PER_MILI_SEC;
        if (duration >= CADE_MILI_SEC_NOTIFY_FREQ)
        {
            CADE_LOG("\nSend CADE_MSG_NOTIFY");
            cade_send_notify_msg(p_cade);
            p_cade->state = CADE_STATE_SENT_NOTIFY;
            begin_time = end_time;
        }

        // Receive messages on sending socket
        if (OK == cade_recv_msg_on_mcast_s_sock(p_cade))
        {
            msg_type = p_cade->decoded_msg.msg_type;
            cade_recv_msg_handle(p_cade, msg_type);
        }
    }

    return p_cade->uuid;
}

/**
 * @brief: Run cade member business
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_member_run(struct cade *p_cade)
{
    assert(NULL != p_cade);

    uint8_t msg_type = 0;
    struct pollfd  poll_d[NB_R_SOCK];
    int ret;

    // Set params for polld
    memset(poll_d, 0 , sizeof(poll_d));
    poll_d[0].fd = p_cade->sock.ucast_sockfd;
    poll_d[0].events = POLLIN;
    poll_d[1].fd = p_cade->sock.mcast_recv_sockfd;
    poll_d[1].events = POLLIN;

#ifdef DBG_CARDS_HOLD
    time_t show_time = time(NULL) + CADE_SEC_SHOW_CARD_DEBUG_FREQ;
#endif

    // 2. Wait for MSEARCH, PING, CARDDEAL and handle it
    CADE_LOG("\n\n 2 .Wait for MSEARCH, PING, CARDDEAL and handle it");
    while (true)
    {
        // Mark itself alived
        if (NULL != p_cade->cb_hdl_keep_alived_mark)
        {
            p_cade->cb_hdl_keep_alived_mark();
        }

        // Poll the sockets for an event to occur
        ret = poll(poll_d, NB_R_SOCK, CADE_MILI_SEC_POLL_TIMEOUT);
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
                // Receive messages on unicast socket
                if (OK == cade_recv_msg_on_unicast_sock(p_cade))
                {
                    msg_type = p_cade->decoded_msg.msg_type;
                    cade_recv_msg_handle(p_cade, msg_type);
                }
            } else if (poll_d[1].revents & POLLIN)
            {
                // Receive messages on receiving socket
                if (OK == cade_recv_msg_on_mcast_r_sock(p_cade))
                {
                    msg_type = p_cade->decoded_msg.msg_type;
                    cade_recv_msg_handle(p_cade, msg_type);
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
}





