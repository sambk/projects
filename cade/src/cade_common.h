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
 * @file      cade_common.h
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

#ifndef CADE_COMMON_H_
#define CADE_COMMON_H_

// Struct cade
struct cade
{
    /*****************************
     *  Variable for init
     *  **************************/

    // Socket
    struct cade_sock            sock;

    // Role
    uint8_t                     role;



    /*****************************
     *  Variable for runtime
     *  **************************/

    // For encoding
    struct cade_msg_encoded     encoded_msg;

    // For decoding
    struct cade_msg_decoded     decoded_msg;



    /*****************************
     *  For dealer
     *  **************************/

    // Cade table
    struct cade_member_tbl      cade_member_tbl;
    pthread_mutex_t             tbl_lock;

    // Poker cards set for dealer
    struct cards                poker_set[MAX_POKER_CARDS];
    uint16_t                    pokers_num;
    bool                        is_got_poker_set;



    /*****************************
     *  For both players and members
     *  **************************/

    // uuid and machineid
    uint16_t                    uuid;
    uint8_t                     machineid[MACHINEID_LEN];
    uint16_t                    machineid_len;

    // State for CARDDEAL transaction
    enum cade_state             state;

    // Callback function to be called when receiving carddeal successfully
    void                        (*cb_hdl_recv_cards_success)(struct cade *);
    // Callback function to be called to mark core health
    void                        (*cb_hdl_keep_alived_mark)(void);
    // Callback function to be called to show cards hold
    void                        (*cb_hdl_show_cards_info)(struct cade *);



    /*****************************
     *  For player only
     *  **************************/

    // Cards hold for player
    struct some_cards           current_cards;
    struct some_cards           candidate_cards;




    /*****************************
     *  For watcher only
     *  **************************/

    // Player info for watcher
    struct player_info          current_player_info;
    struct player_info          candidate_player_info;
};

/**
 * @brief: Usually machine-id is 32-byte-string like "ebb4f0a9c3474b7c99ac8f5a6ad85b86"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int get_machine_id(uint8_t *machine_id, uint16_t max_len);

/**
 * @brief: Free cade memory
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_destroy(struct cade *p_cade);

/**
 * @brief: Send message on multicast "sending socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_send_msg_on_mcast_s_sock(struct cade *p_cade, int len);

/**
 * @brief: Send message on multicast "receiving socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_send_msg_on_mcast_r_sock(struct cade *p_cade, int len);

/**
 * @brief: Send message on unicast socket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_send_msg_on_unicast_sock(struct cade *p_cade, int len, uint32_t ip, uint16_t port);

/**
 * @brief: Reply message on unicast socket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_rep_msg_on_unicast_sock(struct cade *p_cade, int len);

/**
 * @brief: Receive message on sending socket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_recv_msg_on_mcast_s_sock(struct cade *p_cade);

/**
 * @brief: Receive message on "receiving socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_recv_msg_on_mcast_r_sock(struct cade *p_cade);

/**
 * @brief: Receive message on "unicast socket"
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_recv_msg_on_unicast_sock(struct cade *p_cade);


/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void set_cb_recv_cards_success(struct cade *p_cade, void (*cb_hdl_recv_cards)(struct cade *));

/**
 * @brief: Set callback handle to monitor core health
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void set_cb_keep_alived_mark(struct cade *p_cade, void (*cb_keep_alived_mark)(void));

/**
 * @brief: Set callback handle to be called to show cards hold
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void set_cb_show_cards(struct cade *p_cade, void (*cb_hdl_show_cards_info)(struct cade *));

#endif /* CADE_COMMON_H_ */
