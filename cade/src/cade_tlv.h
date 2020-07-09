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
 * @file      cade_tlv.h
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

#ifndef CADE_TLV_H_
#define CADE_TLV_H_

#define MAX_TLV_TOTAL_BUF   2048
#define MAX_TLV_LEN         1024
#define MAX_TLV_ELEMENTS    64

// TLV with 2 bytes of tag, 2 bytes of length, limited by MAX_TLV_LEN buffer
struct tlv
{
    uint16_t        tag;
    uint16_t        length;
    uint8_t         value[MAX_TLV_LEN];
} __attribute__((__packed__));

// Containing buffer and length of tlv encoded with limited by MAX_TLV_TOTAL_BUF
struct tlv_serialization
{
    uint8_t         tlv_buf[MAX_TLV_TOTAL_BUF];
    uint16_t        tlv_len;
};

// Containing an array of tlv with limited by MAX_TLV_ELEMENTS
struct tlv_decoded
{
    struct tlv      arr_tlv[MAX_TLV_ELEMENTS];
    uint16_t        tlv_num;
    uint16_t        group_tag;
} __attribute__((__packed__));;

/**
 * @brief: Tag length value init for encoding
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_encode_init(struct tlv_serialization *tlv_serialized);

/**
 * @brief: Tag length value init for decoding
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_decode_init(struct tlv_decoded *tlv_decoded);

/**
 * @brief: Tag length value encode
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_encode(struct tlv_serialization *tlv_serialized, uint16_t tag, uint16_t length, uint8_t *val_buf);

/**
 * @brief: Add a serialization into a group of serialization with a group tag
 *
 * @param [IN]
 *
 * @return
 *          Length of message : Success
 *         -1                 : Failed
 */
int cade_tlv_encode_from_serialized(struct tlv_serialization *p_grp_serialized, struct tlv_serialization *p_serialized, uint16_t grp_tag);

/**
 * @brief: Tag length value decode
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_tlv_decode(struct tlv_decoded *tlv_decoded, uint8_t *buf, uint16_t len);

/**
 * @brief: Group tlv buffer into one tag, length
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_group_tlv_encode(struct tlv_serialization *tlv_serialized, uint16_t tag);

/**
 * @brief: Tag length value decode with one group tag
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_group_tlv_decode(struct tlv_decoded *tlv_decoded, uint8_t *buf, uint16_t total_len);

/**
 * @brief: Print tlv value
 *
 * @param [IN]: p_tlv_decoded
 *
 * @return: void
 */
void dump_tlv_decoded(struct tlv_decoded *p_tlv_decoded);

// Test encode/decode
void test_encode_decode();

#endif /* CADE_TLV_H_ */




