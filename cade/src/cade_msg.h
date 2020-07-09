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
 * @file      cade_msg.h
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

#ifndef CADE_MSG_H_
#define CADE_MSG_H_

#define PROTOCOL                "CADE"
#define VERSION                 1

#define MAX_PAYLOAD_BUFF_LEN    2048

// All Message type of Cade protocol
enum cade_msg_type
{
    CADE_MSG_NOT_USED           = 0,

    CADE_MSG_NOTIFY             = 1, // payload of machine id, MULTICAST
    CADE_MSG_IDASSIGN           = 2,

    CADE_MSG_MSEARCH            = 3, // MULTICAST
    CADE_MSG_RESPONSE           = 4, // payload of cards, UNICAST

    CADE_MSG_PING               = 5,
    CADE_MSG_PONG               = 6,

    CADE_MSG_CARDDEAL           = 7, // payload of cards
    CADE_MSG_CARDDEAL_OK        = 8,
    CADE_MSG_CARDDEAL_ERR       = 9,

    CADE_MSG_CARDCOMMIT         = 10,
    CADE_MSG_CARDCOMMIT_OK      = 11,
    CADE_MSG_CARDCOMMIT_ERR     = 12,

    CADE_MSG_CARDROLLBACK       = 13,
    CADE_MSG_CARDROLLBACK_OK    = 14,
    CADE_MSG_CARDROLLBACK_ERR   = 15,

    CADE_MSG_END_MARK           = 16
};

// Cade role in card table: dealer, player, watcher
enum cade_role
{
    CADE_ROLE_NOT_USED          = 0,
    CADE_ROLE_DEALER            = 1,
    CADE_ROLE_PLAYER            = 2,
    CADE_ROLE_WATCHER           = 3
};

// Cade payload tag
enum cade_payload_tag
{
    CADE_PL_TAG_NOT_USED        = 0,
    CADE_PL_TAG_GROUP           = 1,
    CADE_PL_TAG_MACHINEID       = 2,

    CADE_PL_TAG_CARD_LST        = 3,
    CADE_PL_TAG_CARD_ADD        = 4,
    CADE_PL_TAG_CARD_DEL        = 5,

    CADE_PL_TAG_PLAYER_LST      = 6,
    CADE_PL_TAG_PLAYER_ADD      = 7,
    CADE_PL_TAG_PLAYER_DEL      = 8
};

// Struct containing cade message
struct cade_msg_encoded {
    uint8_t            protocol[4];
    uint8_t            version;
    uint8_t            role;
    uint8_t            msg_type;
    uint16_t           uuid;
    uint16_t           sequence;
    uint8_t            payload_buff[MAX_PAYLOAD_BUFF_LEN];
} __attribute__((__packed__));

// Struct containing cade message
struct cade_msg_decoded {
    uint8_t            protocol[4];
    uint8_t            version;
    uint8_t            role;
    uint8_t            msg_type;
    uint16_t           uuid;
    uint16_t           sequence;
    struct tlv_decoded pl_tlv;
} __attribute__((__packed__));

/**
 * @brief: Decapsulate cade message
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 *
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *  | Protocol                         | Version | Role    | Msg type| UUID            | Sequence number | Payloads                                |
 *  +----------------------------------+---------+---------+---------+--+-----------------+-----------------+--------------------------------------+
 *  | 4 octets                         | 1 octet | 1 octet | 1 octet | 2 octets        | 2 octets        | tag length value                        |
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *  | C  A  D  E                       | 1       | PLAYER  | HELLO   | 123             | 1               |                                         |
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 */
int cade_decap_msg(struct cade_msg_decoded *p_msg, uint8_t *msg, uint16_t msg_len);

/**
 * @brief: Encapsulate cade JOIN message
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 *
 *  +-------------+---------+---------+---------+------------+-----------------+
 *  | Protocol    | Version | Role    | Msg type| UUID       | Sequence number |
 *  +-------------+---------+---------+---------+-- ---------+-----------------+
 *  | 4 octets    | 1 octet | 1 octet | 1 octet | 2 octets   | 2 octets        |
 *  +-------------+---------+---------+---------+------------+-----------------+
 *  | C  A  D  E  | 1       | PLAYER  | HELLO   | 123        | 1               |
 *  +-------------+---------+---------+---------+------------+-----------------+
 */

int cade_encap_without_pl_msg(struct cade_msg_encoded *p_msg, uint8_t role, uint8_t msg_type, uint16_t uuid, uint16_t sequence);

/**
 * @brief: Add 1 tlv
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 */
int cade_add_tlv_msg(struct tlv_serialization *p_serialized, uint16_t tag, uint8_t *buf, uint16_t len);
/**
 * @brief: Encapsulate cade message
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 *
 *  +-------------+---------+---------+---------+------------+----------------+----------+----------+------------+------------+----------------------------------+
 *  | Protocol    | Version | Role    | Msg type| UUID       | Sequence number| Group tag|Group len | Card tag   | Card length| Card content                     |                                             |
 *  +-------------+---------+---------+---------+-- ---------+----------------+----------+----------+------------+------------+----------------------------------+
 *  | 4 octets    | 1 octet | 1 octet | 1 octet | 2 octets   | 2 octets       | 2 octets | 2 octets | 2 octets   | 2 octets   | 33 octets                        |
 *  +-------------+---------+---------+---------+------------+----------------+----------+----------+------------+------------+----------------------------------+
 *  | C  A  D  E  | 1       | PLAYER  | NOTIFY  | 123        | 1              | 1        | 37       | 2          | 6          | "card 1"                         |
 *  +-------------+---------+---------+---------+------------+----------------+----------+----------+------------+------------+----------------------------------+
 */
int cade_encap_grp_tlv_msg(struct cade_msg_encoded *p_msg, uint16_t role, uint8_t msg_type, uint16_t uuid, uint16_t sequence, struct tlv_serialization *p_serialized);

/**
 * @brief: Find tlv from tag
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct tlv *cade_find_tlv_from_tag(struct tlv_decoded *pl_tlv, uint16_t tag);

// Printf struct fields
void dump_cade_msg_struct(struct cade_msg_encoded *p_msg, uint8_t payload_len);

//Printf message
void dump_cade_msg(uint8_t *p_msg, uint16_t len);

// Dump cade struct decoded
void dump_cade_msg_decoded(struct cade_msg_decoded *p_msg);

// Unit test for encap/decap function
void test_encap_decap_msg();

#endif /* CADE_MSG_H_ */




