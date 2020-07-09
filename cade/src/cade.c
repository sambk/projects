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
 * @file      cade.c
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

#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <math.h>

#include "cade_utilities.h"
#include "cade.h"

static int cade_add_ip_prefix(struct ip_prefix_basket *p_basket, struct ipv4_prefix *p_ipv4_prefix);

/**
 * @brief: Init baskets
 *
 * @param [IN]
 *
 * @return
 *         Poiter to struct basket : Success
 *         NULL : Failed
 */
struct baskets *cade_init_basket(void)
{
    struct baskets *p_baskets = malloc(sizeof(struct baskets));
    if (NULL != p_baskets)
    {
        memset(p_baskets, 0, sizeof(struct baskets));
        p_baskets->num_baskets = 0;
        return p_baskets;
    }
    return NULL;
}

/**
 * @brief: Destroy baskets
 *
 * @param [IN]
 *
 * @return
 */
void cade_destroy_basket(struct baskets *p_baskets)
{
    if (NULL != p_baskets)
    {
        free(p_baskets);
        p_baskets = NULL;
    }
}

/**
 * @brief: Add a new basket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_add_basket(struct baskets *p_baskets, const char *basket_name)
{
    assert(NULL != p_baskets);
    assert(NULL != basket_name);

    int idx = p_baskets->num_baskets;

    if (idx >= MAX_BASKET_NUM)
    {
        CADE_LOG("Cannot add more basket idx = %d", idx);
        return ERR;
    }

    CADE_LOG("Add new basket [%s] at index [%d]", basket_name, idx);
    strncpy((char *) p_baskets->lst_ip_prefix_baskets[idx].basket_name, basket_name, MAX_BASKET_NAME_LEN);
    strncpy((char *) p_baskets->lst_egg_baskets[idx].basket_name, basket_name, MAX_BASKET_NAME_LEN);

    p_baskets->num_baskets++;

    return OK;
}

/**
 * @brief: Add a new basket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_add_ip_prefix(struct ip_prefix_basket *p_basket, struct ipv4_prefix *p_ipv4_prefix)
{
    assert(NULL != p_basket);
    assert(NULL != p_ipv4_prefix);

    int idx = p_basket->num_ipv4_prefix;

    if (idx >= MAX_IP_PREFIX_PER_BASKET)
    {
        CADE_LOG("Cannot add more ip prefix idx = %d", idx);
        return ERR;
    }

    CADE_LOG("Add new ip prefix at index [%d]", idx);
    p_basket->lst_ipv4_prefix[idx].ip = p_ipv4_prefix->ip;
    p_basket->lst_ipv4_prefix[idx].len = p_ipv4_prefix->len;
    p_basket->num_ipv4_prefix++;

    return OK;
}

/**
 * @brief: Distribute ip prefix for baskets
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_distribute_ip_prefix_for_baskets(struct baskets *p_baskets, struct ipv4_prefix *p_ipv4_prefix)
{
    assert(NULL != p_baskets);
    assert(NULL != p_ipv4_prefix);

    int moved_bits = ceil(log(p_baskets->num_baskets) / log(2));

    struct ipv4_prefix s_smaller_ipv4_prefix, *p_smaller_ipv4_prefix = &s_smaller_ipv4_prefix;
    p_smaller_ipv4_prefix->len = p_ipv4_prefix->len + moved_bits;
    int subnet = 32 - p_smaller_ipv4_prefix->len;
    int sn_range = (int) pow(2, subnet);

    CADE_LOG("Moved_bits is %d, mask is %d --> new mask is %d sn_range is %d", moved_bits, p_ipv4_prefix->len, p_smaller_ipv4_prefix->len, sn_range);

    int ceil_subnet = 32 - p_ipv4_prefix->len;
    int ceil_sn_range = pow(2, ceil_subnet);
    uint32_t ceil_ip = p_ipv4_prefix->ip + ceil_sn_range;

    int idx = 0, jdx = 0;
    p_smaller_ipv4_prefix->ip = p_ipv4_prefix->ip;
    while (p_smaller_ipv4_prefix->ip + sn_range < ceil_ip)
    {
        CADE_LOG("ip is "PRIip" and ceil_ip is "PRIip" ", IP_DOT(p_smaller_ipv4_prefix->ip), IP_DOT(ceil_ip));
        struct ip_prefix_basket *p_basket = &p_baskets->lst_ip_prefix_baskets[jdx];

        p_smaller_ipv4_prefix->ip = p_ipv4_prefix->ip + idx * sn_range;
        cade_add_ip_prefix(p_basket, p_smaller_ipv4_prefix);

        idx++;
        jdx++;
        if (jdx == p_baskets->num_baskets)
        {
            jdx = 0;
        }
    }

    return OK;
}


// Test distribute ip prefix
void test_cade_distribute_ip_prefix_for_baskets(void)
{
    // Init basket
    struct baskets *p_baskets = cade_init_basket();
    cade_add_basket(p_baskets, "external_interface_group_1");
    cade_add_basket(p_baskets, "external_interface_group_2");
    cade_add_basket(p_baskets, "external_interface_group_3");
    cade_add_basket(p_baskets, "external_interface_group_4");
    cade_add_basket(p_baskets, "external_interface_group_5");
    cade_add_basket(p_baskets, "external_interface_group_6");
    cade_add_basket(p_baskets, "external_interface_group_7");
    cade_add_basket(p_baskets, "external_interface_group_8");
    cade_add_basket(p_baskets, "external_interface_group_9");
    cade_add_basket(p_baskets, "external_interface_group_10");
    cade_add_basket(p_baskets, "external_interface_group_11");
    cade_add_basket(p_baskets, "external_interface_group_12");
    cade_add_basket(p_baskets, "external_interface_group_13");
    cade_add_basket(p_baskets, "external_interface_group_14");
    cade_add_basket(p_baskets, "external_interface_group_15");
    cade_add_basket(p_baskets, "external_interface_group_16");
    cade_add_basket(p_baskets, "external_interface_group_17");
    cade_add_basket(p_baskets, "external_interface_group_18");
    cade_add_basket(p_baskets, "external_interface_group_19");
    cade_add_basket(p_baskets, "external_interface_group_20");
    cade_add_basket(p_baskets, "external_interface_group_21");
    cade_add_basket(p_baskets, "external_interface_group_22");
    cade_add_basket(p_baskets, "external_interface_group_23");
    cade_add_basket(p_baskets, "external_interface_group_24");
    cade_add_basket(p_baskets, "external_interface_group_25");
    cade_add_basket(p_baskets, "external_interface_group_26");
    cade_add_basket(p_baskets, "external_interface_group_27");
    cade_add_basket(p_baskets, "external_interface_group_28");
    cade_add_basket(p_baskets, "external_interface_group_29");
    cade_add_basket(p_baskets, "external_interface_group_30");
    //cade_add_basket(p_baskets, "external_interface_group_31");
    //cade_add_basket(p_baskets, "external_interface_group_32");

    // Ip prefix to be divided
    struct ipv4_prefix s_big_ipv4_prefix, *p_big_ipv4_prefix = &s_big_ipv4_prefix;
    uint32_t ip_add;
    inet_pton(AF_INET, "192.168.0.0", (struct sockaddr_in *) &ip_add);
    p_big_ipv4_prefix->ip = ntohl(ip_add);
    p_big_ipv4_prefix->len = 16;
    CADE_LOG("Big prefix is "PRIip" / %d", IP_DOT(p_big_ipv4_prefix->ip), p_big_ipv4_prefix->len);

    // Test dividing
    cade_distribute_ip_prefix_for_baskets(p_baskets, p_big_ipv4_prefix);

    // Ip prefix to be divided
    inet_pton(AF_INET, "193.168.0.0", (struct sockaddr_in *) &ip_add);
    p_big_ipv4_prefix->ip = ntohl(ip_add);
    p_big_ipv4_prefix->len = 16;
    CADE_LOG("Big prefix is "PRIip" / %d", IP_DOT(p_big_ipv4_prefix->ip), p_big_ipv4_prefix->len);

    // Test dividing
    cade_distribute_ip_prefix_for_baskets(p_baskets, p_big_ipv4_prefix);

    // Ip prefix to be divided
    inet_pton(AF_INET, "194.168.0.0", (struct sockaddr_in *) &ip_add);
    p_big_ipv4_prefix->ip = ntohl(ip_add);
    p_big_ipv4_prefix->len = 16;
    CADE_LOG("Big prefix is "PRIip" / %d", IP_DOT(p_big_ipv4_prefix->ip), p_big_ipv4_prefix->len);

    // Test dividing
    cade_distribute_ip_prefix_for_baskets(p_baskets, p_big_ipv4_prefix);

    // Print result
    int idx, jdx;
    for (idx = 0; idx < p_baskets->num_baskets; idx++)
    {
        struct ip_prefix_basket *p_basket = &p_baskets->lst_ip_prefix_baskets[idx];
        CADE_LOG("   Basket %d \tbasket name %s ", idx, p_basket->basket_name);
        for (jdx = 0; jdx < p_basket->num_ipv4_prefix; jdx++)
        {
            struct ipv4_prefix *p_ipv4_prefix = &p_basket->lst_ipv4_prefix[jdx];
            CADE_LOG("      Ip prefix "PRIip" / %d", IP_DOT(p_ipv4_prefix->ip), p_ipv4_prefix->len);
        }
    }

    // Destroy basket
    cade_destroy_basket(p_baskets);
}

/**
 * @brief: Add a new basket
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
static int cade_add_egg(struct egg_basket *p_basket, char *p_egg)
{
    assert(NULL != p_basket);
    assert(NULL != p_egg);

    int idx = p_basket->num_egg;

    if (idx >= MAX_EGGS_PER_BASKET)
    {
        CADE_LOG("Cannot add more egg idx = %d", idx);
        return ERR;
    }

    CADE_LOG("Add new egg at index [%d] name [%s]", idx, p_egg);
    strncpy((char *) p_basket->lst_egg[idx].egg_name, p_egg, MAX_BASKET_NAME_LEN);
    p_basket->lst_egg[idx].egg_len = strlen(p_egg);
    p_basket->num_egg++;

    return OK;
}

/**
 * @brief: Distribute eggs for baskets
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_distribute_eggs_for_baskets(struct baskets *p_baskets, char *lst_eggs[], uint16_t len)
{
    assert(NULL != p_baskets);
    assert(NULL != lst_eggs);

    int idx;
    for (idx = 0; idx < len; idx++)
    {
        int modulo = idx % p_baskets->num_baskets;
        CADE_LOG("modulo is %d", modulo);
        struct egg_basket *p_egg_basket = &p_baskets->lst_egg_baskets[modulo];
        cade_add_egg(p_egg_basket, lst_eggs[idx]);
    }
    return OK;
}

// Test distribute eggs
void test_cade_distribute_eggs_for_baskets(void)
{
    // Init basket
    struct baskets *p_baskets = cade_init_basket();
    cade_add_basket(p_baskets, "external_interface_group_1");
    cade_add_basket(p_baskets, "external_interface_group_2");
    cade_add_basket(p_baskets, "external_interface_group_3");
    cade_add_basket(p_baskets, "external_interface_group_4");
    cade_add_basket(p_baskets, "external_interface_group_5");

    // Available eggs
    char *lst_eggs[] = {"egg 1", "egg 2", "egg 3", "egg 4", "egg 5", "egg 6", "egg 7", "egg 8", "egg 9", "egg 10",
                    "egg 11", "egg 12", "egg 13", "egg 14", "egg 15", "egg 16", "egg 17", "egg 18", "egg 19", "egg 20",
                    "egg 21", "egg 22", "egg 23", "egg 24", "egg 25", "egg 26", "egg 27", "egg 28", "egg 29", "egg 30",
                    "egg 31", "egg 32"};

    // Test dividing
    cade_distribute_eggs_for_baskets(p_baskets, lst_eggs, 32);

    // Print result
    int idx, jdx;
    for (idx = 0; idx < p_baskets->num_baskets; idx++)
    {
        struct egg_basket *p_basket = &p_baskets->lst_egg_baskets[idx];
        CADE_LOG("   Basket %d \tbasket name [%s] size [%lu]", idx, p_basket->basket_name, sizeof(struct egg_basket));
        for (jdx = 0; jdx < p_basket->num_egg; jdx++)
        {
            struct egg *p_egg = &p_basket->lst_egg[jdx];
            CADE_LOG("      Egg: %.*s", p_egg->egg_len, p_egg->egg_name);
        }
    }

    // Destroy basket
    cade_destroy_basket(p_baskets);
}


