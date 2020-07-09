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
 * @file      cade_table.h
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

#ifndef CADE_TABLE_H_
#define CADE_TABLE_H_

// State machine
enum cade_state
{
    CADE_STATE_NOT_USED             = 0,
    CADE_STATE_NORMAL               = 1,

    // For dealer
    CADE_STATE_SENT_CARDDEAL        = 2,
    CADE_STATE_RECV_CARDDEAL_OK     = 3,
    CADE_STATE_RECV_CARDDEAL_ERR    = 4,
    CADE_STATE_SENT_COMMIT          = 5,
    CADE_STATE_RECV_COMMIT_OK       = 6,
    CADE_STATE_RECV_COMMIT_ERR      = 7,
    CADE_STATE_SENT_ROLLBACK        = 8,
    CADE_STATE_RECV_ROLLBACK_OK     = 9,
    CADE_STATE_RECV_ROLLBACK_ERR    = 10,

    // For player
    CADE_STATE_SENT_NOTIFY          = 11,
    CADE_STATE_SENT_REPLY_CARDDEAL  = 12
};

// Struct to store info from players
struct cade_member_info
{
    // Info from members
    uint8_t                     role;
    uint32_t                    ip_addr;
    uint16_t                    port;   // Network order
    uint8_t                     machineid[MACHINEID_LEN];
    uint16_t                    machineid_len;

    // Info assigned by dealer
    uint16_t                    uuid;

    // Memeber state
    bool                        is_alive;
    bool                        is_in_carddeal;
    enum cade_state             state;
    uint16_t                    ping_timeout_counter;

    // Cards hold to store player info
    struct some_cards           current_cards;
    struct some_cards           candidate_cards;

    // Cards hold to store watcher info
    struct player_info          current_player_info;
    struct player_info          candidate_player_info;
};

struct cade_member_tbl
{
    struct cade_member_info     arr_cade_players[MAX_CADE_MEMBERS];
    uint16_t                    nb_players;

    struct cade_member_info     arr_cade_watchers[MAX_CADE_MEMBERS];
    uint16_t                    nb_watchers;
};

// Add member type
enum cade_adding_member_type
{
    CADE_ADDING_NOTIFY = 0,
    CADE_ADDING_RESPONSE = 1
};

/**
 * @brief: Init cade table to store information of members
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void init_cade_member_tbl(struct cade_member_tbl *cade_member_tbl);

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
        uint8_t *machineid, uint16_t machineid_len, enum cade_adding_member_type adding_type, uint16_t uuid);

/**
 * @brief: Count the number of alive members
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_alive_players(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Count the number of alive watchers
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_alive_watchers(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Get uuid from machine id
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
int get_uuid_from_machineid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint8_t *machineid, uint16_t machineid_len);

/**
 * @brief: Get member from machine id
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
struct cade_member_info *get_member_from_machineid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint8_t *machineid, uint16_t machineid_len);

/**
 * @brief: Get member from uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 *         -1 : Failed
 */
struct cade_member_info *get_member_from_uuid(struct cade_member_info *lst_cade_member_info, uint16_t nb_member, uint16_t uuid);

/**
 * @brief: Count the number of CARDDEAL sent
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_carrdeal_sent(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Count the number of CARDDEAL reply
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_carrdeal_reply(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Remove all cards
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int remove_cards_hold(struct some_cards *p_cards_hold);

/**
 * @brief: Remove all players info
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int remove_players_info(struct player_info *p_player_info);

/**
 * @brief: Count the number of PING reply (PONG)
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int check_pong_mark_if_alive_all_members(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Count the number of COMMIT reply
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_commit_reply(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Count the number of ROLLBACK reply
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_rollback_reply(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Count the number of PING reply (PONG)
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_nb_pong_players(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Finalize if the carddeal business is success or failed or out of synchronous
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int finalize_carddeal_transaction(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Check if there is all CARDDEAL_OK reply
 *
 * @param [IN]
 *
 * @return
 *          true  : all CARDDEAL_OK reply
 *          false : Failed
 */
bool is_all_state_carddeal_ok(struct cade_member_tbl *p_cade_member_tbl);

// Printf all member of cade member table
void dump_cade_member_tbl(struct cade_member_tbl *p_cade_member_tbl);
// Test cade member table
void test_cade_member_tbl();

/**
 * @brief: Add a new card for player
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_add_card_for_player(struct some_cards *p_cards_hold, uint8_t *card, uint16_t len);

/**
 * @brief: Delete a card from player
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_delete_card_from_player(struct some_cards *p_cards_hold, uint8_t *card, uint16_t len);

/**
 * @brief: Add a new player for watcher
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_add_player_for_watcher(struct player_info *p_player_info, uint16_t uuid);

/**
 * @brief: Check if one player not having card or not
 *
 * @param [IN]
 *
 * @return
 *          true  : one player have not had cards
 *          false :
 */
bool is_one_player_not_having_card(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Check if the card is hold by any player
 *
 * @param [IN]
 *
 * @return
 *          true  : one player have not had cards
 *          false :
 */
bool is_hold_by_any_player_on_candicate(struct cade_member_tbl *p_cade_member_tbl, uint8_t *card, uint16_t len);

/**
 * @brief: Check if the card is hold by this player
 *
 * @param [IN]
 *
 * @return
 *          true  : this player have this card
 *          false :
 */
bool is_hold_by_this_player(struct some_cards *p_cards_hold, uint8_t *card, uint16_t len);

/**
 * @brief: Copy dealer current to candidate cards
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_dealer_cur_to_can_cards(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Copy dealer candidate to current cards
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_dealer_can_to_cur_cards(struct cade_member_tbl *p_cade_member_tbl);

/**
 * @brief: Copy player cards hold
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_player_cards_hold(struct some_cards *p_cards_dst, struct some_cards *p_candidate_src);

/**
 * @brief: Copy watcher cards hold
 *
 * @param [IN]
 *
 * @return
 *           0 : Success
 *          -1 : Failed
 */
int copy_watcher_player_info(struct player_info *p_player_info_dst, struct player_info *p_player_info_src);

/**
 * @brief: Check if there is the same between current cards and candidate cards
 *
 * @param [IN]
 *
 * @return
 *          true  : if there is difference
 *          false :
 */
bool is_same_current_candidate(struct cade_member_info *p_cade_member_info);

// Test adding cards
void test_cade_add_cards();

#endif /* CADE_TABLE_H_ */





