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
 * @file      cade_tlv.c
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

#ifndef CADE_TLV_C_
#define CADE_TLV_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include "cade_utilities.h"
#include "cade_tlv.h"

/**
 * @brief: Print tlv value
 *
 * @param [IN]: p_tlv_decoded
 *
 * @return: void
 */
void dump_tlv_decoded(struct tlv_decoded *p_tlv_decoded)
{
    if (NULL == p_tlv_decoded)
    {
        return;
    }

    CADE_LOG("Tlv group tag %d: ", p_tlv_decoded->group_tag);
    int idx;
    for (idx = 0; idx < p_tlv_decoded->tlv_num; idx++)
    {
        CADE_LOG("Tlv element %d: ", idx);
        CADE_LOG("   Tag: \t%d", p_tlv_decoded->arr_tlv[idx].tag);
        CADE_LOG("   Length: \t%d", p_tlv_decoded->arr_tlv[idx].length);
        DUMP_MSG("   Value: \t", ((uint8_t *) p_tlv_decoded->arr_tlv[idx].value), p_tlv_decoded->arr_tlv[idx].length);
    }
}

/**
 * @brief: Just memset 0 for struct tlv_serialized
 *
 * @param [IN]: tlv_serialized
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_encode_init(struct tlv_serialization *tlv_serialized)
{
    if (NULL == tlv_serialized)
    {
        ERR_LOG("   [ERROR] cade_tlv_encode tlv_serialized is NULL!!!");
        return ERR;
    }

    memset(tlv_serialized, 0, sizeof(struct tlv_serialization));
    tlv_serialized->tlv_len = 0;
    return OK;
}

/**
 * @brief: Just memeset 0 for struct tlv_decoded
 *
 * @param [IN]: tlv_decoded
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_decode_init(struct tlv_decoded *tlv_decoded)
{
    if (NULL == tlv_decoded)
    {
        ERR_LOG("   [ERROR] cade_tlv_decode_init tlv_decoded is NULL!!!");
        return ERR;
    }

    memset(tlv_decoded, 0, sizeof(struct tlv_decoded));
    tlv_decoded->tlv_num = 0;
    return OK;
}

/**
 * @brief: Append a more tlv
 *
 * @param [IN]
 *  tag:     Tag
 *  length:  Length
 *  val_buf: Value
 * @param [OUT]
 *  tlv_serialized: Buffer struct containing tlv encoded
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_encode(struct tlv_serialization *tlv_serialized, uint16_t tag, uint16_t length, uint8_t *val_buf)
{
    if (NULL == tlv_serialized)
    {
        ERR_LOG("   [ERROR] cade_tlv_encode tlv_serialized is NULL!!!");
        return ERR;
    }

    // Check length of a tlv
    if (MAX_TLV_LEN < length)
    {
        ERR_LOG("   [ERROR] cade_tlv_encode buffer not enough [%d]", length);
        return ERR;
    }

    // Check length of total
    int added_len = length + 4;
    if (added_len + tlv_serialized->tlv_len > MAX_TLV_TOTAL_BUF)
    {
        ERR_LOG("   [ERROR] cade_tlv_encode toal buffer not enough [%d]", added_len + tlv_serialized->tlv_len);
        return ERR;
    }

    // Group into a tlv
    struct tlv s_tlv;
    struct tlv *p_tlv = &s_tlv;
    memset(p_tlv, 0, sizeof(struct tlv));
    p_tlv->tag = tag;
    p_tlv->length = length;
    memcpy(p_tlv->value, val_buf, length);

    // Copy tlv group to a serialized buffer
    memcpy(tlv_serialized->tlv_buf + tlv_serialized->tlv_len, p_tlv, added_len);
    tlv_serialized->tlv_len += added_len;

    return OK;
}


/**
 * @brief: Add a serialization into a group of serialization with a group tag
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 */
int cade_tlv_encode_from_serialized(struct tlv_serialization *p_grp_serialized, struct tlv_serialization *p_serialized, uint16_t grp_tag)
{
    return cade_tlv_encode(p_grp_serialized, grp_tag, p_serialized->tlv_len, p_serialized->tlv_buf);
}

/**
 * @brief: Append a more tlv
 *
 * @param [IN]
 *  tag:     Tag
 *  length:  Length
 *  val_buf: Value
 * @param [OUT]
 *  tlv_serialized: Buffer struct containing tlv encoded
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_group_tlv_encode(struct tlv_serialization *tlv_serialized, uint16_t tag)
{
    uint8_t *tlv_buf;
    uint16_t tlv_new_len, origin_len;

    if (NULL == tlv_serialized)
    {
        ERR_LOG("   [ERROR] cade_group_tlv_encode tlv_serialized is NULL!!!");
        return ERR;
    }

    // Check length for safe
    if (tlv_serialized->tlv_len + 4 > MAX_TLV_TOTAL_BUF)
    {
        ERR_LOG("   [ERROR] cade_group_tlv_encode buffer not enough [%d]", tlv_serialized->tlv_len + 4);
        return ERR;
    }

    // Update total length
    origin_len = tlv_serialized->tlv_len;
    tlv_serialized->tlv_len += 4;
    tlv_buf = tlv_serialized->tlv_buf;
    tlv_new_len = tlv_serialized->tlv_len;

    // Shift elements forward 4 bytes
    int idx;
    for (idx = tlv_new_len; idx >= 4; idx--)
    {
        tlv_buf[idx] = tlv_buf[idx - 4];
    }

    // Insert tag
    tlv_buf[0] = tag;
    tlv_buf[1] = tag << 8;

    // Insert length
    tlv_buf[2] = origin_len;
    tlv_buf[3] = origin_len << 8;

    return OK;
}

/**
 * @brief: Tag length value decode
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_decode(struct tlv_decoded *tlv_decoded, uint8_t *buf, uint16_t total_len)
{
    if (NULL == tlv_decoded)
    {
        ERR_LOG("   [ERROR] cade_tlv_decode tlv_decoded is NULL!!!");
        return ERR;
    }

    if (NULL == buf)
    {
        ERR_LOG("   [ERROR] cade_tlv_decode buf is NULL!!!");
        return ERR;
    }

    int tlv_num = 0, idx = 0, ret = OK, len;

    do
    {
        // Tag
        tlv_decoded->arr_tlv[tlv_num].tag = (buf[idx + 1] << 8) | buf[idx];
        idx += 2;

        // Length
        len = (buf[idx + 1] << 8) | buf[idx];
        idx += 2;
        if (len > MAX_TLV_LEN)
        {
            ERR_LOG("   [ERROR] cade_tlv_decode buffer not enough [%d]", len);
            return ERR;
        }

        if (len > total_len - idx)
        {
            DUMP_MSG("   [ERROR] cade_tlv_decode Not valid tlv: ", buf, total_len);
            ret = ERR;
        }
        tlv_decoded->arr_tlv[tlv_num].length = len;

        // Value
        memcpy(tlv_decoded->arr_tlv[tlv_num].value, buf + idx, tlv_decoded->arr_tlv[tlv_num].length);
        idx += tlv_decoded->arr_tlv[tlv_num].length;

        tlv_num++;
        if (tlv_num >= MAX_TLV_ELEMENTS)
        {
            ERR_LOG("   [ERROR] cade_tlv_decode reach maximum elements %d", tlv_num);
            break;
        }
    } while ((idx < total_len) &&  (OK == ret));

    // Number of tlv
    tlv_decoded->tlv_num = tlv_num;
    return ret;
}

/**
 * @brief: Tag length value decode with one group tag
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_group_tlv_decode(struct tlv_decoded *tlv_decoded, uint8_t *buf, uint16_t total_len)
{
    if (NULL == tlv_decoded)
    {
        ERR_LOG("   [ERROR] cade_group_tlv_decode tlv_decoded is NULL!!!");
        return ERR;
    }

    if (NULL == buf)
    {
        ERR_LOG("   [ERROR] cade_group_tlv_decode buf is NULL!!!");
        return ERR;
    }

    int idx = 0, len;

    // Group tag
    tlv_decoded->group_tag = (buf[idx + 1] << 8) | buf[idx];
    idx += 2;

    // Group length
    len = (buf[idx + 1] << 8) | buf[idx];
    idx += 2;
    if (len > MAX_TLV_TOTAL_BUF - 4)
    {
        ERR_LOG("   [ERROR] cade_tlv_decode group buffer not enough [%d]", len);
        return ERR;
    }

    if (len > total_len - idx)
    {
        DUMP_MSG("   [ERROR] cade_tlv_decode group Not valid tlv: ", buf, total_len);
        return ERR;
    }

    return cade_tlv_decode(tlv_decoded, buf + 4, len);
}

void test_encode_decode()
{
//    struct tlv_serialization s_serialized;
//    struct tlv_serialization *p_serialized = &s_serialized;
//    cade_tlv_encode_init(p_serialized);
//
//    cade_tlv_encode(p_serialized, 3, 3, (uint8_t *) "aaa");
//    cade_tlv_encode(p_serialized, 3, 4, (uint8_t *) "bbbb");
//    cade_tlv_encode(p_serialized, 3, 5, (uint8_t *) "ccccc");
//    cade_tlv_encode(p_serialized, 3, 6, (uint8_t *) "dddddd");
//    cade_group_tlv_encode(p_serialized, 9);
//    DUMP_MSG("Encode:\t", ((uint8_t *) p_serialized->tlv_buf), p_serialized->tlv_len);
//
//    CADE_LOG("===========================================================================");
//
//    CADE_LOG("Decode:");
//    struct tlv_decoded s_tlv_decoded;
//    struct tlv_decoded *p_tlv_decoded = &s_tlv_decoded;
//    cade_tlv_decode_init(p_tlv_decoded);
//
//    cade_group_tlv_decode(p_tlv_decoded, p_serialized->tlv_buf, p_serialized->tlv_len);
//    //cade_tlv_decode(p_tlv_decoded, p_serialized->tlv_buf, p_serialized->tlv_len);
//    dump_tlv_decoded(p_tlv_decoded);







    struct tlv_serialization s_serialized, s_grp_serialized;
    struct tlv_serialization *p_serialized = &s_serialized, *p_grp_serialized = &s_grp_serialized;

    cade_tlv_encode_init(p_grp_serialized);

    cade_tlv_encode_init(p_serialized);
    cade_tlv_encode(p_serialized, 3, 3, (uint8_t *) "aaa");
    cade_tlv_encode(p_serialized, 3, 4, (uint8_t *) "bbbb");
    cade_tlv_encode(p_serialized, 3, 5, (uint8_t *) "ccccc");
    cade_tlv_encode(p_serialized, 3, 6, (uint8_t *) "dddddd");
    cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, 0x09);

    cade_tlv_encode_init(p_serialized);
    cade_tlv_encode(p_serialized, 3, 3, (uint8_t *) "eee");
    cade_tlv_encode(p_serialized, 3, 4, (uint8_t *) "ffff");
    cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, 0x09);

    cade_tlv_encode_init(p_serialized);
    cade_tlv_encode(p_serialized, 3, 3, (uint8_t *) "ggg");
    cade_tlv_encode(p_serialized, 3, 4, (uint8_t *) "hhhh");
    cade_tlv_encode_from_serialized(p_grp_serialized, p_serialized, 0x09);

    cade_group_tlv_encode(p_grp_serialized, 8);
    DUMP_MSG("Encode:\t", ((uint8_t *) p_grp_serialized->tlv_buf), p_grp_serialized->tlv_len);


    CADE_LOG("\nDecode:");
    struct tlv_decoded s_tlv_decoded, s_sub_tlv_decoded;
    struct tlv_decoded *p_tlv_decoded = &s_tlv_decoded, *p_sub_tlv_decoded = &s_sub_tlv_decoded;
    cade_tlv_decode_init(p_tlv_decoded);
    cade_group_tlv_decode(p_tlv_decoded, p_grp_serialized->tlv_buf, p_grp_serialized->tlv_len);
    dump_tlv_decoded(p_tlv_decoded);

    // Decode deeper
    CADE_LOG("\n\nDecode deeper:");
    int idx;
    for (idx = 0; idx < p_tlv_decoded->tlv_num; idx++)
    {
        CADE_LOG("\nTlv tag %d:", p_tlv_decoded->arr_tlv[idx].tag);
        cade_tlv_decode_init(p_sub_tlv_decoded);
        cade_tlv_decode(p_sub_tlv_decoded, p_tlv_decoded->arr_tlv[idx].value, p_tlv_decoded->arr_tlv[idx].length);
        dump_tlv_decoded(p_sub_tlv_decoded);
    }
}

#endif /* CADE_TLV_C_ */










