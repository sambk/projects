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
 * @file      cade.h
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      Jun 6, 2020
 *
 * @brief
 *
 ******************************************************************************/

#ifndef CADE_H_
#define CADE_H_

#include <stdint.h>
#include <netinet/in.h>
#include "cade_declaration.h"


// Card info
struct cards
{
    uint8_t                     card_name[MAX_CARD_LEN];
    uint16_t                    card_len;
};

// Each player will hold some cards
struct some_cards
{
    struct cards                cards[MAX_CARD_NUM];
    uint16_t                    cards_num;
};

// Struct to store info about player from watcher
struct player_card_info
{
    struct some_cards           cards_hold;
    uint16_t                    uuid;
};

// Struct to store info about player from watcher
struct player_info
{
    struct player_card_info     arr_players[MAX_CADE_MEMBERS];
    uint16_t                    nb_players;
};

// Cade struct
struct cade;

/***********************************************************************
 ************** SUPPORT PREPROCESS DATA TO CREATE CARDS ****************
 ***********************************************************************/

// Support dividing ip pool to fill content of card
struct ipv4_prefix
{
    uint32_t                    ip;
    uint8_t                     len;
};

// Each basket contains some ipv4_prefix
struct ip_prefix_basket
{
    uint8_t                     basket_name[MAX_BASKET_NAME_LEN];
    struct ipv4_prefix          lst_ipv4_prefix[MAX_IP_PREFIX_PER_BASKET];
    uint16_t                    num_ipv4_prefix;
};

// Card info
struct egg
{
    uint8_t                     egg_name[MAX_BASKET_NAME_LEN];
    uint16_t                    egg_len;
};

// Each basket contains some other names
struct egg_basket
{
    uint8_t                     basket_name[MAX_BASKET_NAME_LEN];
    struct egg                  lst_egg[MAX_EGGS_PER_BASKET];
    uint16_t                    num_egg;
};

// There are some baskets
struct baskets
{
    struct ip_prefix_basket     lst_ip_prefix_baskets[MAX_BASKET_NUM];
    struct egg_basket           lst_egg_baskets[MAX_BASKET_NUM];
    uint16_t                    num_baskets;
};

/**
 * @brief: Init baskets
 *
 * @param [IN]
 *
 * @return
 *         Poiter to struct basket : Success
 *         NULL : Failed
 */
struct baskets *cade_init_basket(void);

/**
 * @brief: Destroy baskets
 *
 * @param [IN]
 *
 * @return
 */
void cade_destroy_basket(struct baskets *p_baskets);

/**
 * @brief: Add a new basket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_add_basket(struct baskets *p_baskets, const char *basket_name);

/**
 * @brief: Distribute ip prefix for baskets
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_distribute_ip_prefix_for_baskets(struct baskets *p_baskets, struct ipv4_prefix *p_ipv4_prefix);

// Test distribute ip prefix
void test_cade_distribute_ip_prefix_for_baskets(void);

/**
 * @brief: Distribute eggs for baskets
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_distribute_eggs_for_baskets(struct baskets *p_baskets, char *lst_eggs[], uint16_t len);

// Test distribute eggs
void test_cade_distribute_eggs_for_baskets(void);


#endif /* CADE_H_ */
