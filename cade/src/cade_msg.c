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
 * @file      cade_msg.c
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

/**
 *                         CADE Message format
 *
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *  | Protocol                         | Version | Role    | Msg type| ID              | Sequence number | Payloads                                |
 *  +----------------------------------+---------+---------+---------+--+-----------------+-----------------+--------------------------------------+
 *  | 4 octets                         | 1 octet | 1 octet | 1 octet | 2 octets        | 2 octets        | tag length value                        |
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *  | C  A  D  E                       | 1       | PLAYER  | HELLO   | 123             | 1               |                                         |
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include "cade_tlv.h"
#include "cade_msg.h"
#include "cade_utilities.h"

#define HEADER_LEN (strlen("CADE") + 1 + 1 + 1 + 2 + 2)

static int cade_encap_msg(struct cade_msg_encoded *p_msg, uint8_t role, uint8_t msg_type, uint16_t uuid, uint16_t sequence, uint8_t *payload, uint16_t payload_len);

char *txt_code_msg_type[] =
{
    [CADE_MSG_NOT_USED]           = "CADE_MSG_NOT_USED",
    [CADE_MSG_NOTIFY]             = "CADE_MSG_NOTIFY",
    [CADE_MSG_IDASSIGN]           = "CADE_MSG_IDASSIGN",
    [CADE_MSG_MSEARCH]            = "CADE_MSG_MSEARCH",
    [CADE_MSG_RESPONSE]           = "CADE_MSG_RESPONSE",
    [CADE_MSG_PING]               = "CADE_MSG_PING",
    [CADE_MSG_PONG]               = "CADE_MSG_PONG",
    [CADE_MSG_CARDDEAL]           = "CADE_MSG_CARDDEAL",
    [CADE_MSG_CARDDEAL_OK]        = "CADE_MSG_CARDDEAL_OK",
    [CADE_MSG_CARDDEAL_ERR]       = "CADE_MSG_CARDDEAL_ERR",
    [CADE_MSG_CARDCOMMIT]         = "CADE_MSG_CARDCOMMIT",
    [CADE_MSG_CARDCOMMIT_OK]      = "CADE_MSG_CARDCOMMIT_OK",
    [CADE_MSG_CARDCOMMIT_ERR]     = "CADE_MSG_CARDCOMMIT_ERR",
    [CADE_MSG_CARDROLLBACK]       = "CADE_MSG_CARDROLLBACK",
    [CADE_MSG_CARDROLLBACK_OK]    = "CADE_MSG_CARDROLLBACK_OK",
    [CADE_MSG_CARDROLLBACK_ERR]   = "CADE_MSG_CARDROLLBACK_ERR"
};

/**
 * @brief: Encapsulate cade message
 *
 * @param [IN]
 *
 * @return
 * Length of message : Success
 *                -1 : Failed
 */
/*  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *  | Protocol                         | Version | Role    | Msg type| UUID            | Sequence number | Payloads                                |
 *  +----------------------------------+---------+---------+---------+--+-----------------+-----------------+--------------------------------------+
 *  | 4 octets                         | 1 octet | 1 octet | 1 octet | 2 octets        | 2 octets        | tag length value                        |
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 *  | C  A  D  E                       | 1       | PLAYER  | HELLO   | 123             | 1               |                                         |
 *  +----------------------------------+---------+---------+---------+-----------------+-----------------+-----------------------------------------+
 */
static int cade_encap_msg(struct cade_msg_encoded *p_msg, uint8_t role, uint8_t msg_type, uint16_t uuid, uint16_t sequence, uint8_t *payload, uint16_t payload_len)
{
    if (NULL == p_msg)
    {
        ERR_LOG("   [ERROR] cade_encap_msg p_msg is NULL!!!");
        return ERR;
    }

    int msg_len = 0;

    // Fill information for payload
    if (payload_len < MAX_PAYLOAD_BUFF_LEN)
    {
        if ((NULL != payload) && (payload_len > 0))
        {
            memcpy(p_msg->payload_buff, payload, payload_len);
            msg_len += payload_len;
        }
    } else
    {
        ERR_LOG("   [ERROR] Payload buffer not enough!!!");
        return ERR;
    }

    // Fill information for fixed fields
    memcpy(p_msg->protocol, PROTOCOL, 4);
    p_msg->version = VERSION;
    p_msg->role = role;
    p_msg->msg_type = msg_type;
    p_msg->uuid = uuid;
    p_msg->sequence = sequence;
    msg_len += HEADER_LEN;

    return msg_len;
}

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
int cade_decap_msg(struct cade_msg_decoded *p_msg, uint8_t *msg, uint16_t msg_len)
{
    if (NULL == p_msg)
    {
        ERR_LOG("   [ERROR] cade_decap_msg p_msg is NULL!!!");
        return ERR;
    }

    if ((NULL == msg) || (msg_len < HEADER_LEN))
    {
        if ((NULL != msg) && (msg_len > 0))
        {
            DUMP_MSG("   [ERROR] Not a valid message length!!!", msg, msg_len);
        }
        return ERR;
    }

    struct tlv_decoded *p_tlv_decoded = &p_msg->pl_tlv;
    cade_tlv_decode_init(p_tlv_decoded);

    // Decapsulate message
    if (0 == memcmp(msg, PROTOCOL, 4))
    {
        memcpy(p_msg->protocol, msg, 4);
        p_msg->version  = msg[4];
        p_msg->role     = msg[5];
        p_msg->msg_type = msg[6];
        p_msg->uuid     = (msg[8] << 8) | msg[7];
        p_msg->sequence = (msg[10] << 8) | msg[9];

        if (msg_len > HEADER_LEN)
        {
            cade_group_tlv_decode(p_tlv_decoded, msg + HEADER_LEN, msg_len - HEADER_LEN);
        }
    } else
    {
        DUMP_MSG("   [ERROR] Not a valid protocol message!!!", msg, msg_len);
        return ERR;
    }

    return OK;
}

/**
 * @brief: Encapsulate cade ASSIGNID message
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 *
 *  +-------------+---------+---------+---------+------------+----------------+
 *  | Protocol    | Version | Role    | Msg type| UUID       | Sequence number|
 *  +-------------+---------+---------+---------+-- ---------+----------------+
 *  | 4 octets    | 1 octet | 1 octet | 1 octet | 2 octets   | 2 octets       |
 *  +-------------+---------+---------+---------+------------+----------------+
 *  | C  A  D  E  | 1       | DEALER  | ASSIGN  | 123        | 1              |
 *  +-------------+---------+---------+---------+------------+----------------+
 */

int cade_encap_without_pl_msg(struct cade_msg_encoded *p_msg, uint8_t role, uint8_t msg_type, uint16_t uuid, uint16_t sequence)
{
    return cade_encap_msg(p_msg, role, msg_type, uuid, sequence, NULL, 0);
}

/**
 * @brief: Add 1 tlv
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 */
int cade_add_tlv_msg(struct tlv_serialization *p_serialized, uint16_t tag, uint8_t *buf, uint16_t len)
{
    if (NULL == buf)
    {
        return -1;
    }
    return cade_tlv_encode(p_serialized, tag, len, buf);
}
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
int cade_encap_grp_tlv_msg(struct cade_msg_encoded *p_msg, uint16_t role, uint8_t msg_type, uint16_t uuid, uint16_t sequence, struct tlv_serialization *p_serialized)
{
    if (NULL == p_serialized)
    {
        return -1;
    }
    cade_group_tlv_encode(p_serialized, CADE_PL_TAG_GROUP);
    return cade_encap_msg(p_msg, role, msg_type, uuid, sequence, p_serialized->tlv_buf, p_serialized->tlv_len);
}

/**
 * @brief: Find tlv from tag
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct tlv *cade_find_tlv_from_tag(struct tlv_decoded *pl_tlv, uint16_t tag)
{
    if (NULL == pl_tlv)
    {
        return NULL;
    }

    int idx;
    for (idx = 0; idx < pl_tlv->tlv_num; idx++)
    {
        if (tag == pl_tlv->arr_tlv[idx].tag)
        {
            return &pl_tlv->arr_tlv[idx];
        }
    }
    return NULL;
}

// Dump cade messgage encoded
void dump_cade_msg(uint8_t *p_msg, uint16_t len)
{
    CADE_LOG("Cade message:");
    CADE_LOG("Len: \t\t%d", len);
    DUMP_MSG("Message:\t", ((uint8_t *) p_msg), len);
}

// Dump cade struct encoded
void dump_cade_msg_struct(struct cade_msg_encoded *p_msg, uint8_t payload_len)
{
    assert(NULL != p_msg);
    CADE_LOG("Protocol: \t%.*s", 4, p_msg->protocol);
    CADE_LOG("Version: \t%d", p_msg->version);
    CADE_LOG("Role: \t\t%d", p_msg->role);
    CADE_LOG("Type: \t\t%d", p_msg->msg_type);
    CADE_LOG("Uuid: \t\t%d", p_msg->uuid);
    CADE_LOG("Sequence: \t%d", p_msg->sequence);
    DUMP_MSG("Payload:\t", ((uint8_t *) p_msg->payload_buff), payload_len);
}

// Dump cade struct decoded
void dump_cade_msg_decoded(struct cade_msg_decoded *p_msg)
{
    assert(NULL != p_msg);
    CADE_LOG("Protocol: \t%.*s", 4, p_msg->protocol);
    CADE_LOG("Version: \t%d", p_msg->version);
    CADE_LOG("Role: \t\t%d", p_msg->role);
    CADE_LOG("Type: \t\t%d", p_msg->msg_type);
    CADE_LOG("Uuid: \t\t%d", p_msg->uuid);
    CADE_LOG("Sequence: \t%d", p_msg->sequence);
    dump_tlv_decoded(&p_msg->pl_tlv);
}

// Unit test encap/decap cade message function
void test_encap_decap_msg()
{
    struct cade_msg_encoded s_cade_msg, *p_msg = &s_cade_msg;
    int len;
    struct cade_msg_decoded s_cade_msg_decoded, *p_cade_msg_decoded = &s_cade_msg_decoded;
    struct tlv_serialization s_serialized, s_grp_serialized, *p_serialized= &s_serialized, *p_grp_serialized= &s_grp_serialized;
    struct tlv_decoded s_tlv_decoded, *p_tlv_decoded = &s_tlv_decoded;

//    CADE_LOG("\n======== Encode: ============");
//    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
//    len = cade_encap_msg(p_msg, CADE_ROLE_PLAYER, CADE_MSG_NOTIFY, 5, generate_sequence(), NULL, 0);
//    dump_cade_msg((uint8_t *) p_msg, len);
//
//    CADE_LOG("\n======== Decode: ============");
//    p_cade_msg_decoded = &s_cade_msg_decoded;
//    memset(p_cade_msg_decoded, 0, sizeof(struct cade_msg_decoded));
//    cade_decap_msg(p_cade_msg_decoded, (uint8_t *) p_msg, len);
//    dump_cade_msg_decoded(p_cade_msg_decoded);
//
//
//    CADE_LOG("\n\n======== Encode: ============");
//    // Tlv payload
//    cade_tlv_encode_init(p_serialized);
//    cade_tlv_encode(p_serialized, 3, 3, (uint8_t *) "aaa");
//    cade_tlv_encode(p_serialized, 3, 4, (uint8_t *) "bbbb");
//    cade_tlv_encode(p_serialized, 3, 5, (uint8_t *) "ccccc");
//    cade_tlv_encode(p_serialized, 3, 6, (uint8_t *) "dddddd");
//    cade_group_tlv_encode(p_serialized, 9);
//
//    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
//    len = cade_encap_msg(p_msg, CADE_ROLE_WATCHER, CADE_MSG_IDASSIGN, 6, generate_sequence(), p_serialized->tlv_buf, p_serialized->tlv_len);
//    dump_cade_msg((uint8_t *) p_msg, len);
//
//    CADE_LOG("\n======== Decode: ============");
//    memset(p_cade_msg_decoded, 0, sizeof(struct cade_msg_decoded));
//    cade_decap_msg(p_cade_msg_decoded, (uint8_t *) p_msg, len);
//    dump_cade_msg_decoded(p_cade_msg_decoded);
//
//
//    CADE_LOG("\n\n======== Encode: ============");
//    // Tlv payload
//    cade_tlv_encode_init(p_serialized);
//    cade_add_tlv_msg(p_serialized, 1, (uint8_t *) "card 1", (uint16_t) strlen("card 1"));
//    cade_add_tlv_msg(p_serialized, 2, (uint8_t *) "card 2", (uint16_t) strlen("card 2"));
//    cade_add_tlv_msg(p_serialized, 3, (uint8_t *) "card 3", (uint16_t) strlen("card 3"));
//    cade_add_tlv_msg(p_serialized, 4, (uint8_t *) "card 4", (uint16_t) strlen("card 4"));
//
//    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
//    len = cade_encap_grp_tlv_msg(p_msg, 1, 1, 1, 6, p_serialized);
//    dump_cade_msg((uint8_t *) p_msg, len);
//
//    CADE_LOG("\n======== Decode: ============");
//    memset(p_cade_msg_decoded, 0, sizeof(struct cade_msg_decoded));
//    cade_decap_msg(p_cade_msg_decoded, (uint8_t *) p_msg, len);
//    dump_cade_msg_decoded(p_cade_msg_decoded);





    CADE_LOG("\n\n======== Encode: ============");
    // Tlv payload
    cade_tlv_encode_init(p_grp_serialized);

    cade_tlv_encode_init(p_serialized);
    cade_add_tlv_msg(p_serialized, 1, (uint8_t *) "card 1", (uint16_t) strlen("card 1"));
    cade_add_tlv_msg(p_serialized, 2, (uint8_t *) "card 2", (uint16_t) strlen("card 2"));
    cade_add_tlv_msg(p_serialized, 3, (uint8_t *) "card 3", (uint16_t) strlen("card 3"));
    cade_add_tlv_msg(p_serialized, 4, (uint8_t *) "card 4", (uint16_t) strlen("card 4"));
    cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, 0x09);

    cade_tlv_encode_init(p_serialized);
    cade_add_tlv_msg(p_serialized, 5, (uint8_t *) "card 5", (uint16_t) strlen("card 5"));
    cade_add_tlv_msg(p_serialized, 6, (uint8_t *) "card 6", (uint16_t) strlen("card 6"));
    cade_add_tlv_msg(p_serialized, 7, (uint8_t *) "card 7", (uint16_t) strlen("card 7"));
    cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, 0x09);

    cade_tlv_encode_init(p_serialized);
    cade_add_tlv_msg(p_serialized, 5, (uint8_t *) "card 10", (uint16_t) strlen("card 10"));
    cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, 0x09);

    memset(p_msg, 0, sizeof(struct cade_msg_encoded));
    len = cade_encap_grp_tlv_msg(p_msg, 1, 1, 1, 6, p_grp_serialized);
    dump_cade_msg((uint8_t *) p_msg, len);

    CADE_LOG("\n======== Decode: ============");
    memset(p_cade_msg_decoded, 0, sizeof(struct cade_msg_decoded));
    cade_decap_msg(p_cade_msg_decoded, (uint8_t *) p_msg, len);
    dump_cade_msg_decoded(p_cade_msg_decoded);

    CADE_LOG("\nDecode deeeper:");
    int idx;
    for (idx = 0; idx < p_cade_msg_decoded->pl_tlv.tlv_num; idx++)
    {
        CADE_LOG("\nTlv tag %d:", p_cade_msg_decoded->pl_tlv.arr_tlv[idx].tag);
        cade_tlv_decode_init(p_tlv_decoded);
        cade_tlv_decode(p_tlv_decoded, (uint8_t *) p_cade_msg_decoded->pl_tlv.arr_tlv[idx].value, p_cade_msg_decoded->pl_tlv.arr_tlv[idx].length);
        dump_tlv_decoded(p_tlv_decoded);
    }
}







