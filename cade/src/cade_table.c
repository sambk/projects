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
 * @file      cade_table.c
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      Apr 29, 2020
 *
 * @brief
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

#include "cade_utilities.h"
#include "cade_declaration.h"
#include "cade.h"
#include "cade_table.h"

static uint16_t request_uuid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member);
static int get_idx_from_machineid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint8_t *machineid, uint16_t machineid_len);
static int get_idx_from_uuid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint16_t uuid);
static bool is_uuid_in_using(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint16_t uuid);

// ======================== For member info ========================== //

/**
 * @brief: Get index from machine id
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
static int get_idx_from_machineid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint8_t *machineid, uint16_t machineid_len)
{
    if (NULL == lst_cade_member_info)
    {
        ERR_LOG("   get_uuid_from_machineid lst_cade_member_info NULL");
        return -1;
    }
    if (NULL == machineid)
    {
        ERR_LOG("   get_uuid_from_machineid machineid NULL");
        return -1;
    }
    if (0 == machineid_len)
    {
        ERR_LOG("   get_uuid_from_machineid machineid_len 0");
        return -1;
    }

    if (machineid_len > MACHINEID_LEN)
    {
        ERR_LOG("   add_new_member machineid too long %d", machineid_len);
        return -1;
    }

    uint8_t *tbl_machineid;
    uint16_t tbl_machineid_len;
    int idx;

    // Find the position of machine_id
    for (idx = 0; idx < nb_member; idx++)
    {
        tbl_machineid = lst_cade_member_info[idx].machineid;
        tbl_machineid_len = lst_cade_member_info[idx].machineid_len;
        if ((lst_cade_member_info[idx].is_alive) && (tbl_machineid_len == machineid_len) && (0 == memcmp(tbl_machineid, machineid, machineid_len)))
        {
            return idx;
        }
    }

    return -1;
}

/**
 * @brief: Get index from uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
static int get_idx_from_uuid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint16_t uuid)
{
    if (NULL == lst_cade_member_info)
    {
        ERR_LOG("   get_uuid_from_machineid lst_cade_member_info NULL");
        return -1;
    }

    int idx;

    // Find the position of machine_id
    for (idx = 0; idx < nb_member; idx++)
    {
        if ((lst_cade_member_info[idx].is_alive) && (uuid == lst_cade_member_info[idx].uuid))
        {
            return idx;
        }
    }

    return -1;
}

/**
 * @brief: Check if the uuid is used by any player
 *
 * @param [IN]
 *
 * @return
 *          true  : uuid is in using
 *          false : uuis is not in using
 */
static bool is_uuid_in_using(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint16_t uuid)
{
    int idx;
    for (idx = 0; idx < nb_member; idx++)
    {
        if ((lst_cade_member_info[idx].is_alive) && (uuid == lst_cade_member_info[idx].uuid))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief: Init cade table to store information of members
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void init_cade_member_tbl(struct cade_member_tbl *p_cade_member_tbl)
{
    memset(p_cade_member_tbl, 0, sizeof(struct cade_member_tbl));
    p_cade_member_tbl->nb_players = 0;
}

/**
 * @brief: Add a new member
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int add_player_member(struct cade_member_info *lst_cade_member_info, uint16_t *nb_member, uint32_t ip_addr, uint16_t port, uint8_t role,
        uint8_t *machineid, uint16_t machineid_len, enum cade_adding_member_type adding_type, uint16_t uuid)
{
    if (NULL == lst_cade_member_info)
    {
        ERR_LOG("   add_new_member p_cade_member_tbl NULL");
        return -1;
    }

    if (NULL == machineid)
    {
        ERR_LOG("   add_new_member machineid NULL");
        return ERR;
    }
    if (0 == machineid_len)
    {
        ERR_LOG("   add_new_member machineid_len 0");
        return ERR;
    }

    if (machineid_len > MACHINEID_LEN)
    {
        ERR_LOG("   add_new_member machineid too long %d", machineid_len);
        return ERR;
    }

    if (*nb_member >= MAX_CADE_MEMBERS)
    {
        ERR_LOG("   add_new_member reach to maximum players %d", *nb_member);
        return ERR;
    }

    uint8_t *tbl_machineid;
    uint16_t tbl_machineid_len;
    struct cade_member_info *p_cade_member;
    int idx;

    // Find the position of machine_id
    for (idx = 0; idx < *nb_member; idx++)
    {
        p_cade_member = &lst_cade_member_info[idx];
        tbl_machineid = lst_cade_member_info[idx].machineid;
        tbl_machineid_len = lst_cade_member_info[idx].machineid_len;
        if ((tbl_machineid_len == machineid_len) && (0 == memcmp(tbl_machineid, machineid, machineid_len)))
        {
            CADE_LOG("Player exist %.*s", machineid_len, machineid);
            // Replace value if the machineid exists
            p_cade_member->ip_addr = ip_addr;
            p_cade_member->port = port;
            p_cade_member->role = role;
            p_cade_member->is_alive = true;
            p_cade_member->is_in_carddeal = false;
            p_cade_member->state = CADE_STATE_NORMAL;
            p_cade_member->ping_timeout_counter = NB_PING_TIMEOUT;

            CADE_LOG("Player exist uuid %d machineid %.*s", p_cade_member->uuid, machineid_len, machineid);
            return OK;
        }
    }

    // If the machine is new, assgin uuid and increase nb_member
    if (idx == *nb_member)
    {
        p_cade_member = &lst_cade_member_info[idx];

        memcpy(p_cade_member->machineid, machineid, machineid_len);
        p_cade_member->machineid_len = machineid_len;
        p_cade_member->ip_addr = ip_addr;
        p_cade_member->port = port;
        p_cade_member->role = role;
        p_cade_member->is_alive = true;
        p_cade_member->is_in_carddeal = false;
        p_cade_member->state = CADE_STATE_NORMAL;
        p_cade_member->ping_timeout_counter = NB_PING_TIMEOUT;

        // If this member has not been assigned uuid, assign for it
        if (CADE_ADDING_NOTIFY == adding_type)
        {
            p_cade_member->uuid = request_uuid(lst_cade_member_info, *nb_member);
        } else if (CADE_ADDING_RESPONSE == adding_type)
        {
            p_cade_member->uuid = uuid;
        }

        CADE_LOG("New member uuid %d machineid %.*s", p_cade_member->uuid, machineid_len, machineid);
        (*nb_member)++;
    }
    return OK;
}

/**
 * @brief: Count the number of alive players
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_alive_players(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx, count = 0;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if (p_cade_member_tbl->arr_cade_players[idx].is_alive)
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief: Count the number of alive watchers
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_alive_watchers(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx, count = 0;
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if (p_cade_member_tbl->arr_cade_watchers[idx].is_alive)
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief: Get uuid from machine id
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
int get_uuid_from_machineid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint8_t *machineid, uint16_t machineid_len)
{
    int idx = get_idx_from_machineid(lst_cade_member_info, nb_member, machineid, machineid_len);

    if (-1 != idx)
    {
        return lst_cade_member_info[idx].uuid;
    }

    return -1;
}

/**
 * @brief: Get member from machine id
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
struct cade_member_info *get_member_from_machineid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint8_t *machineid, uint16_t machineid_len)
{
    int idx = get_idx_from_machineid(lst_cade_member_info, nb_member, machineid, machineid_len);

    if (-1 != idx)
    {
        return &lst_cade_member_info[idx];
    }

    return NULL;
}

/**
 * @brief: Get member from uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
struct cade_member_info *get_member_from_uuid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint16_t uuid)
{
    int idx = get_idx_from_uuid(lst_cade_member_info, nb_member, uuid);

    if (-1 != idx)
    {
        return &lst_cade_member_info[idx];
    }

    return NULL;
}

/**
 * @brief: Request the uuid
 *
 * @param [IN]
 *
 * @return
 *          uuid : Success
 */
uint16_t request_uuid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member)
{
    uint16_t uuid;
    for (uuid = 1; uuid < UINT16_MAX; uuid++)
    {
        if (!is_uuid_in_using(lst_cade_member_info, nb_member, uuid))
        {
            return uuid;
        }
    }
    return 0;
}

/**
 * @brief: Count the number of CARDDEAL sent
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_carrdeal_sent(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx, count = 0;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if (p_cade_member_tbl->arr_cade_players[idx].is_alive && p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal)
        {
            count++;
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if (p_cade_member_tbl->arr_cade_watchers[idx].is_alive && p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal)
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief: Count the number of CARDDEAL reply
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_carrdeal_reply(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx, count = 0;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_players[idx].is_alive) && (p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal) &&
                ( (CADE_STATE_RECV_CARDDEAL_OK == p_cade_member_tbl->arr_cade_players[idx].state) ||
                        (CADE_STATE_RECV_CARDDEAL_ERR == p_cade_member_tbl->arr_cade_players[idx].state) ) )
        {
            count++;
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_watchers[idx].is_alive) && (p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal) &&
                ( (CADE_STATE_RECV_CARDDEAL_OK == p_cade_member_tbl->arr_cade_watchers[idx].state) ||
                        (CADE_STATE_RECV_CARDDEAL_ERR == p_cade_member_tbl->arr_cade_watchers[idx].state) ) )
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief: Count the number of COMMIT reply
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_commit_reply(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx, count = 0;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_players[idx].is_alive) && (p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal) &&
                ( (CADE_STATE_RECV_COMMIT_OK == p_cade_member_tbl->arr_cade_players[idx].state) ||
                        (CADE_STATE_RECV_COMMIT_ERR == p_cade_member_tbl->arr_cade_players[idx].state) ) )
        {

            count++;
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_watchers[idx].is_alive) && (p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal) &&
                ( (CADE_STATE_RECV_COMMIT_OK == p_cade_member_tbl->arr_cade_watchers[idx].state) ||
                        (CADE_STATE_RECV_COMMIT_ERR == p_cade_member_tbl->arr_cade_watchers[idx].state) ) )
        {

            count++;
        }
    }
    return count;
}

/**
 * @brief: Count the number of ROLLBACK reply
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_rollback_reply(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx, count = 0;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_players[idx].is_alive) && (p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal) &&
                ( (CADE_STATE_RECV_ROLLBACK_OK == p_cade_member_tbl->arr_cade_players[idx].state) ||
                        (CADE_STATE_RECV_ROLLBACK_ERR == p_cade_member_tbl->arr_cade_players[idx].state) ) )
        {

            count++;
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_watchers[idx].is_alive) && (p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal) &&
                ( (CADE_STATE_RECV_ROLLBACK_OK == p_cade_member_tbl->arr_cade_watchers[idx].state) ||
                        (CADE_STATE_RECV_ROLLBACK_ERR == p_cade_member_tbl->arr_cade_watchers[idx].state) ) )
        {

            count++;
        }
    }
    return count;
}

/**
 * @brief: Remove all cards
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int remove_cards_hold(struct some_cards *p_cards_hold)
{
    memset(p_cards_hold, 0, sizeof(struct some_cards));
    return OK;
}

/**
 * @brief: Remove all players info
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int remove_players_info(struct player_info *p_player_info)
{
    memset(p_player_info, 0, sizeof(struct player_info));
    return OK;
}

/**
 * @brief: Count the number of PING reply (PONG)
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int check_pong_mark_if_alive_all_members(struct cade_member_tbl *p_cade_member_tbl)
{
    struct cade_member_info *p_member;
    int idx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        p_member = &p_cade_member_tbl->arr_cade_players[idx];
        if ( p_member->is_alive && (0 == p_member->ping_timeout_counter) )
        {
            // Mark this not alive
            p_member->is_alive = false;

            // Remove all cards hold
            remove_cards_hold(&p_member->current_cards);
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        p_member = &p_cade_member_tbl->arr_cade_watchers[idx];
        if ( p_member->is_alive && (0 == p_member->ping_timeout_counter) )
        {
            // Mark this not alive
            p_member->is_alive = false;
        }
    }
    return OK;
}

/**
 * @brief: Finalize if the carddeal business is success or failed or out of synchronous
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int finalize_carddeal_transaction(struct cade_member_tbl *p_cade_member_tbl)
{
    // Check if lost of synchronization
    CADE_LOG("\nFinalize carddeal");
    bool is_success = true;
    int idx;

    for (idx = 0; idx < p_cade_member_tbl->nb_players - 1; idx++)
    {
        if ( p_cade_member_tbl->arr_cade_players[idx].is_alive && p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal &&
                p_cade_member_tbl->arr_cade_players[idx + 1].is_alive && p_cade_member_tbl->arr_cade_players[idx + 1].is_in_carddeal &&
                (p_cade_member_tbl->arr_cade_players[idx].state != p_cade_member_tbl->arr_cade_players[idx + 1].state) )
        {
            ERR_LOG("############# CRITICAL: CARDDEAL lost synchronization #############");
        }
    }

    // Check if carddeal transaction is success or failed
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if ( p_cade_member_tbl->arr_cade_players[idx].is_alive && p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal &&
                (CADE_STATE_RECV_COMMIT_OK != p_cade_member_tbl->arr_cade_players[idx].state))
        {
            is_success = false;
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if ( p_cade_member_tbl->arr_cade_watchers[idx].is_alive && p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal &&
                (CADE_STATE_RECV_COMMIT_OK != p_cade_member_tbl->arr_cade_watchers[idx].state))
        {
            is_success = false;
        }
    }

    // Set state to NORMAL if ok
    if (is_success)
    {
        CADE_LOG("############# CARDDEAL is successful #############");

        // Apply candidate to current
        copy_dealer_can_to_cur_cards(p_cade_member_tbl);

        // Set state to NORMAL
        for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
        {
            if ( p_cade_member_tbl->arr_cade_players[idx].is_alive && p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal )
            {
                p_cade_member_tbl->arr_cade_players[idx].state = CADE_STATE_NORMAL;
                p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal = false;
            }
        }
        for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
        {
            if ( p_cade_member_tbl->arr_cade_watchers[idx].is_alive && p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal )
            {
                p_cade_member_tbl->arr_cade_watchers[idx].state = CADE_STATE_NORMAL;
                p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal = false;
            }
        }
    } else
    {
        ERR_LOG("############# CARDDEAL is failed #############");
        for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
        {
            if ( p_cade_member_tbl->arr_cade_players[idx].is_alive && p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal )
            {
                ERR_LOG("   Player uuid %d with state %d", p_cade_member_tbl->arr_cade_players[idx].uuid, p_cade_member_tbl->arr_cade_players[idx].state);
                // Set to NORMAL also
                p_cade_member_tbl->arr_cade_players[idx].state = CADE_STATE_NORMAL;
                p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal = false;
            }
        }
        for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
        {
            if ( p_cade_member_tbl->arr_cade_watchers[idx].is_alive && p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal )
            {
                ERR_LOG("   Watcher uuid %d with state %d", p_cade_member_tbl->arr_cade_watchers[idx].uuid, p_cade_member_tbl->arr_cade_watchers[idx].state);
                // Set to NORMAL also
                p_cade_member_tbl->arr_cade_watchers[idx].state = CADE_STATE_NORMAL;
                p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal = false;
            }
        }
    }

    return OK;
}

/**
 * @brief: Check if there is all CARDDEAL_OK reply
 *
 * @param [IN]
 *
 * @return
 *          true  : all CARDDEAL_OK reply
 *          false : Failed
 */
bool is_all_state_carddeal_ok(struct cade_member_tbl *p_cade_member_tbl)
{
    int idx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_players[idx].is_alive) && (p_cade_member_tbl->arr_cade_players[idx].is_in_carddeal) &&
                (CADE_STATE_RECV_CARDDEAL_OK != p_cade_member_tbl->arr_cade_players[idx].state) )
        {
            ERR_LOG("Player %d uuid %d not ok with CARDDEAL state %d", idx, p_cade_member_tbl->arr_cade_players[idx].uuid,
                    p_cade_member_tbl->arr_cade_players[idx].state);
            return false;
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if ( (p_cade_member_tbl->arr_cade_watchers[idx].is_alive) && (p_cade_member_tbl->arr_cade_watchers[idx].is_in_carddeal) &&
                (CADE_STATE_RECV_CARDDEAL_OK != p_cade_member_tbl->arr_cade_watchers[idx].state) )
        {
            ERR_LOG("Watcher %d uuid %d not ok with CARDDEAL state %d", idx, p_cade_member_tbl->arr_cade_watchers[idx].uuid,
                    p_cade_member_tbl->arr_cade_watchers[idx].state);
            return false;
        }
    }
    return true;
}

// Printf all member of cade member table
void dump_cade_member_tbl(struct cade_member_tbl *p_cade_member_tbl)
{
    if (NULL == p_cade_member_tbl)
    {
        ERR_LOG("   Cade member table not init");
        return;
    }

    struct cade_member_info *cade_member;

    int idx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        CADE_LOG("Member %d: ", idx);
        cade_member = &p_cade_member_tbl->arr_cade_players[idx];
        CADE_LOG("   role:              %d\t", cade_member->role);
        CADE_LOG("   ip_addr:           "PRIip" \t", IP_DOT(ntohl(cade_member->ip_addr)));
        CADE_LOG("   port:              %d\t", cade_member->port);
        CADE_LOG("   machineid_len:     %d\t", cade_member->machineid_len);
        CADE_LOG("   machineid:         %s\t", cade_member->machineid);
        CADE_LOG("   uuid:              %d\t", cade_member->uuid);
    }
}

// Test cade member table
void test_cade_member_tbl()
{
    struct cade_member_tbl s_cade_member_tbl, *p_cade_member_tbl = &s_cade_member_tbl;
    struct in_addr ip_addr;

    init_cade_member_tbl(&s_cade_member_tbl);
    inet_pton(AF_INET, "192.168.0.0", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 80, 0, (uint8_t *) "123456789012345678901234567890120", 33, 0, 0);
    inet_pton(AF_INET, "192.168.0.1", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 81, 1, (uint8_t *) "123456789012345678901234567890121", 33, 0, 1);
    inet_pton(AF_INET, "192.168.0.2", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 82, 2, (uint8_t *) "123456789012345678901234567890122", 33, 0, 2);
    inet_pton(AF_INET, "192.168.0.3", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 83, 3, (uint8_t *) "123456789012345678901234567890123", 33, 0, 3);
    inet_pton(AF_INET, "192.168.0.4", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 84, 4, (uint8_t *) "123456789012345678901234567890124", 33, 0, 4);
    dump_cade_member_tbl(p_cade_member_tbl);

    CADE_LOG("Add a more duplicated machine id");
    inet_pton(AF_INET, "192.168.0.30", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 830, 30, (uint8_t *) "123456789012345678901234567890123", 33, 0, 3);
    dump_cade_member_tbl(p_cade_member_tbl);

    CADE_LOG("Add a more normal machine id");
    inet_pton(AF_INET, "192.168.0.5", &(ip_addr.s_addr)); add_player_member(p_cade_member_tbl->arr_cade_players, &p_cade_member_tbl->nb_players, ip_addr.s_addr, 85, 5, (uint8_t *) "123456789012345678901234567890125", 33, 0, 5);
    dump_cade_member_tbl(p_cade_member_tbl);
}

// ======================== For cards info ========================== //

/**
 * @brief: Add a new card for player
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_add_card_for_player(struct some_cards *p_cards_hold, uint8_t *card, uint16_t len)
{
    if (NULL == p_cards_hold)
    {
        ERR_LOG("   add_new_card_hold p_cards_hold not init");
        return;
    }
    if ((0 == len) || (NULL == card))
    {
        ERR_LOG("   add_new_card_hold card not valid");
        return;
    }
    if (len > MAX_CARD_LEN)
    {
        ERR_LOG("   cade_add_card_for_player card too long %d", len);
        return;
    }
    if (p_cards_hold->cards_num >= MAX_CARD_NUM)
    {
        ERR_LOG("   cade_add_card_for_player reach to maximum card number %d", p_cards_hold->cards_num);
        return;
    }

    // Check if the card already exists
    int idx;
    for (idx = 0; idx < p_cards_hold->cards_num; idx++)
    {
        if ((len == p_cards_hold->cards[idx].card_len) && (0 == memcmp(p_cards_hold->cards[idx].card_name, card, len)))
        {
            CADE_LOG("   Already had this card: %.*s", len, card);
            return;
        }
    }

    // Add new card
    if (idx == p_cards_hold->cards_num)
    {
        memcpy(p_cards_hold->cards[idx].card_name, card, len);
        p_cards_hold->cards[idx].card_len = len;
        p_cards_hold->cards_num++;
    }
}

/**
 * @brief: Delete a card from player
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_delete_card_from_player(struct some_cards *p_cards_hold, uint8_t *card, uint16_t len)
{
    if (NULL == p_cards_hold)
    {
        ERR_LOG("   delete_card_hold p_cards_hold not init");
        return;
    }
    if ((0 == len) || (NULL == card))
    {
        ERR_LOG("   delete_card_hold card not valid");
        return;
    }

    // Check if the card exists
    int idx;
    for (idx = 0; idx < p_cards_hold->cards_num; idx++)
    {
        if ((len == p_cards_hold->cards[idx].card_len) && (0 == memcmp(p_cards_hold->cards[idx].card_name, card, len)))
        {
            CADE_LOG("   Detected card: %.*s", len, card);

            // Delete this card
            memset(p_cards_hold->cards[idx].card_name, 0, len);
            p_cards_hold->cards[idx].card_len = 0;

            // Shift elements backward 1 element
            int new_idx;
            for (new_idx = idx; new_idx < p_cards_hold->cards_num; new_idx++)
            {
                memcpy(p_cards_hold->cards[new_idx].card_name, p_cards_hold->cards[new_idx + 1].card_name, p_cards_hold->cards[new_idx + 1].card_len);
                p_cards_hold->cards[new_idx].card_len = p_cards_hold->cards[new_idx + 1].card_len;
            }

            // Memset the last element for sure
            memset(p_cards_hold->cards[new_idx].card_name, 0, p_cards_hold->cards[new_idx].card_len);
            p_cards_hold->cards[new_idx].card_len = 0;

            // Decrease the number of cards
            p_cards_hold->cards_num--;

            return;
        }
    }

    ERR_LOG("   delete_card_hold not exist %.*s", len, card);
}

/**
 * @brief: Check if one player or watcher not having card or not
 *
 * @param [IN]
 *
 * @return
 *          true  : one player have not had cards
 *          false :
 */
bool is_one_player_not_having_card(struct cade_member_tbl *p_cade_member_tbl)
{
    if (NULL == p_cade_member_tbl)
    {
        ERR_LOG("   Cade member table not init");
        return false;
    }

    int idx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        if (p_cade_member_tbl->arr_cade_players[idx].is_alive)
        {
            if (0 == p_cade_member_tbl->arr_cade_players[idx].current_cards.cards_num)
            {
                return true;
            }
        }
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        if (p_cade_member_tbl->arr_cade_watchers[idx].is_alive)
        {
            if (0 == p_cade_member_tbl->arr_cade_watchers[idx].current_player_info.nb_players)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief: Check if the card is hold by any player
 *
 * @param [IN]
 *
 * @return
 *          true  : one player have this card
 *          false :
 */
bool is_hold_by_any_player_on_candicate(struct cade_member_tbl *p_cade_member_tbl, uint8_t *card, uint16_t len)
{
    if (NULL == p_cade_member_tbl)
    {
        ERR_LOG("   Cade member table not init");
        return false;
    }

    struct some_cards *p_cards_hold;

    // Check if the card already exists
    int m_idx, c_idx;
    for (m_idx = 0; m_idx < p_cade_member_tbl->nb_players; m_idx++)
    {
        if (p_cade_member_tbl->arr_cade_players[m_idx].is_alive)
        {
            p_cards_hold = &p_cade_member_tbl->arr_cade_players[m_idx].candidate_cards;

            for (c_idx = 0; c_idx < p_cards_hold->cards_num; c_idx++)
            {
                if ((len == p_cards_hold->cards[c_idx].card_len) && (0 == memcmp(p_cards_hold->cards[c_idx].card_name, card, len)))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/**
 * @brief: Check if the card is hold by this player
 *
 * @param [IN]
 *
 * @return
 *          true  : this player have this card
 *          false :
 */
bool is_hold_by_this_player(struct some_cards *p_cards_hold, uint8_t *card, uint16_t len)
{
    // Check if the card is hold by this player
    int idx;
    for (idx = 0; idx < p_cards_hold->cards_num; idx++)
    {
        if ((len == p_cards_hold->cards[idx].card_len) && (0 == memcmp(p_cards_hold->cards[idx].card_name, card, len)))
        {
            return true;
        }
    }


    return false;
}

/**
 * @brief: Copy dealer current to candidate cards
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_dealer_cur_to_can_cards(struct cade_member_tbl *p_cade_member_tbl)
{
    if (NULL == p_cade_member_tbl)
    {
        ERR_LOG("   Cade p_cade_member_tbl NULL");
        return -1;
    }

    struct cade_member_info *p_member;

    int idx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        p_member = &p_cade_member_tbl->arr_cade_players[idx];
        memcpy(&p_member->candidate_cards, &p_member->current_cards, sizeof(struct some_cards));
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        p_member = &p_cade_member_tbl->arr_cade_watchers[idx];
        memcpy(&p_member->candidate_player_info, &p_member->current_player_info, sizeof(struct player_info));
    }

    return OK;
}

/**
 * @brief: Copy dealer candidate to current cards
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_dealer_can_to_cur_cards(struct cade_member_tbl *p_cade_member_tbl)
{
    if (NULL == p_cade_member_tbl)
    {
        ERR_LOG("   Cade p_cade_member_tbl NULL");
        return -1;
    }

    struct cade_member_info *p_member;

    int idx;
    for (idx = 0; idx < p_cade_member_tbl->nb_players; idx++)
    {
        p_member = &p_cade_member_tbl->arr_cade_players[idx];
        memcpy(&p_member->current_cards, &p_member->candidate_cards, sizeof(struct some_cards));
    }
    for (idx = 0; idx < p_cade_member_tbl->nb_watchers; idx++)
    {
        p_member = &p_cade_member_tbl->arr_cade_watchers[idx];
        memcpy(&p_member->current_player_info, &p_member->candidate_player_info, sizeof(struct player_info));
    }

    return OK;
}

/**
 * @brief: Copy player cards hold
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_player_cards_hold(struct some_cards *p_cards_dst, struct some_cards *p_candidate_src)
{
    if (NULL == p_cards_dst)
    {
        ERR_LOG("   Cade p_cade NULL");
        return -1;
    }
    if (NULL == p_candidate_src)
    {
        ERR_LOG("   Cade p_cade NULL");
        return -1;
    }
    memcpy(p_cards_dst, p_candidate_src, sizeof(struct some_cards));
    return OK;
}

/**
 * @brief: Check if there is the same between current cards and candidate cards
 *
 * @param [IN]
 *
 * @return
 *          true  : if there is difference
 *          false :
 */
bool is_same_current_candidate(struct cade_member_info *p_cade_member_info)
{
    if (0 == memcmp(&p_cade_member_info->current_cards, &p_cade_member_info->candidate_cards, sizeof(struct some_cards)))
    {
        return true;
    } else
    {
        return false;
    }
}

// Test adding cards
void test_cade_add_cards()
{
    struct some_cards s_cards_hold, *p_cards_hold = &s_cards_hold;
    memset(p_cards_hold, 0, sizeof(struct some_cards));
    cade_add_card_for_player(p_cards_hold, (uint8_t *) "card 1",   (uint16_t) strlen("card 1"));
    cade_add_card_for_player(p_cards_hold, (uint8_t *) "card 12",  (uint16_t) strlen("card 12"));
    cade_add_card_for_player(p_cards_hold, (uint8_t *) "card 123", (uint16_t) strlen("card 123"));
    cade_add_card_for_player(p_cards_hold, (uint8_t *) "card 123", (uint16_t) strlen("card 123"));
    cade_add_card_for_player(p_cards_hold, (uint8_t *) "card J",   (uint16_t) strlen("card J"));
    cade_add_card_for_player(p_cards_hold, (uint8_t *) "card Q",   (uint16_t) strlen("card Q"));

    int idx;
    for (idx = 0; idx < p_cards_hold->cards_num; idx++)
    {
        CADE_LOG ("   Cards %d: %.*s", idx, p_cards_hold->cards[idx].card_len, p_cards_hold->cards[idx].card_name);
    }

    cade_delete_card_from_player(p_cards_hold, (uint8_t *) "card 12", (uint16_t) strlen("card 12"));
    cade_delete_card_from_player(p_cards_hold, (uint8_t *) "card J", (uint16_t) strlen("card J"));

    for (idx = 0; idx < p_cards_hold->cards_num; idx++)
    {
        CADE_LOG ("   Cards %d: %.*s", idx, p_cards_hold->cards[idx].card_len, p_cards_hold->cards[idx].card_name);
    }
}



// ======================== For watcher info ========================== //

/**
 * @brief: Copy watcher cards hold
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_watcher_player_info(struct player_info *p_player_info_dst, struct player_info *p_player_info_src)
{
    if (NULL == p_player_info_dst)
    {
        ERR_LOG("   Cade p_cade NULL");
        return -1;
    }
    if (NULL == p_player_info_src)
    {
        ERR_LOG("   Cade p_cade NULL");
        return -1;
    }
    memcpy(p_player_info_dst, p_player_info_src, sizeof(struct player_info));
    return OK;
}

/**
 * @brief: Add a new player for watcher
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_add_player_for_watcher(struct player_info *p_player_info, uint16_t uuid)
{
    if (NULL == p_player_info)
    {
        ERR_LOG("   add_new_card_hold p_player_info not init");
        return;
    }
    if (p_player_info->nb_players >= MAX_CADE_MEMBERS)
    {
        ERR_LOG("   add_new_card_hold reach to maximum players %d", p_player_info->nb_players);
        return;
    }

    // Check if the card already exists
    int idx;
    for (idx = 0; idx < p_player_info->nb_players; idx++)
    {
        if (uuid == p_player_info->arr_players[idx].uuid)
        {
            CADE_LOG("   Already had this uuid: %d", uuid);
            break;
        }
    }

    // Add new card
    if (idx == p_player_info->nb_players)
    {
        p_player_info->arr_players[idx].uuid = uuid;
        p_player_info->nb_players++;
    }
}






