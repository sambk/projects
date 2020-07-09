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
 * @file      cade_algorithm.c
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      May 13, 2020
 *
 * @brief
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>

#include "cade_declaration.h"
#include "cade_utilities.h"
#include "cade_tlv.h"
#include "cade_msg.h"
#include "cade.h"
#include "cade_table.h"
#include "cade_socket.h"
#include "cade_common.h"
#include "cade_algorithm.h"

/**
 * @brief: Add card for poker set
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
int cade_add_card_for_poker_set(struct cade *p_cade, uint8_t *card, uint16_t len)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_add_card_for_poker_set p_cade NULL");
        return ERR;
    }
    if ((0 == len) || (NULL == card))
    {
        ERR_LOG("   cade_add_card_for_poker_set card not valid");
        return ERR;
    }
    if (len > MAX_CARD_LEN)
    {
        ERR_LOG("   cade_add_card_for_poker_set card too long %d", len);
        return ERR;
    }
    if (p_cade->pokers_num >= MAX_POKER_CARDS)
    {
        ERR_LOG("   cade_add_card_for_poker_set reach to maximum poker cards %d", p_cade->pokers_num);
        return ERR;
    }

    // Check if the card already exists
    int idx;
    for (idx = 0; idx < p_cade->pokers_num; idx++)
    {
        if ((len == p_cade->poker_set[idx].card_len) && (0 == memcmp(p_cade->poker_set[idx].card_name, card, len)))
        {
            CADE_LOG("Already exists in poker set: %.*s", len, card);
            break;
        }
    }

    // Add new card
    if (idx == p_cade->pokers_num)
    {
        memcpy(p_cade->poker_set[idx].card_name, card, len);
        p_cade->poker_set[idx].card_len = len;
        p_cade->pokers_num++;
    }
    return OK;
}

/**
 * @brief: Do dealing cards business. The most important purpose is that dealing keep the least change
 *
 * @param [IN]
 *
 * @return
 *          true  : Card table has been changed
 *          false : Not change anything
 */
bool cade_do_deal_cards(struct cade *p_cade)
{
    if (NULL == p_cade)
    {
        ERR_LOG("cade_do_deal_cards p_cade NULL");
        return false;
    }

    int average, modulo, idx, jdx, nb_alive_member, nb_average_plus_one_members, pokers_num;
    struct some_cards *p_cards_hold;
    bool is_changed = false;

    // First copy current_cards to candidate_cards. Dealing cards will be calculated on candidate_card
    copy_dealer_cur_to_can_cards(&p_cade->cade_member_tbl);

    // Get the number of alive players
    nb_alive_member = get_nb_alive_players(&p_cade->cade_member_tbl);
    if (0 == nb_alive_member)
    {
        ERR_LOG("cade_do_deal_cards nb_alive_member = 0");
        return false;
    }

    pokers_num = p_cade->pokers_num;
    average = pokers_num/nb_alive_member;
    modulo = pokers_num % nb_alive_member;
    struct cards *p_poker_set = p_cade->poker_set;

    CADE_LOG("pokers_num = %d average= %d modulo = %d", pokers_num, average, modulo);
    nb_average_plus_one_members = 0;
    // If any member hold the number of cards more than average, remove it
    for (jdx = 0; jdx < p_cade->cade_member_tbl.nb_players; jdx++)
    {
        // Only deal card for alive player
        if (p_cade->cade_member_tbl.arr_cade_players[jdx].is_alive)
        {
            p_cards_hold = &p_cade->cade_member_tbl.arr_cade_players[jdx].candidate_cards;
            if (p_cards_hold->cards_num > average + 1)
            {
                idx = p_cards_hold->cards_num - 1;
                while ( ( (p_cards_hold->cards_num > average + 1) && (idx >= 0) ) ||
                        ( (p_cards_hold->cards_num == average + 1) &&
                                (nb_average_plus_one_members >= modulo) && (idx >= 0) ) )
                {
                    // Delete this card from this player
                    CADE_LOG("   Even [%.*s] is deleted from member jdx %d uuid %d", p_cards_hold->cards[idx].card_len, (char *) p_cards_hold->cards[idx].card_name,
                            jdx, p_cade->cade_member_tbl.arr_cade_players[jdx].uuid);
                    cade_delete_card_from_player(p_cards_hold, (uint8_t *) p_cards_hold->cards[idx].card_name, p_cards_hold->cards[idx].card_len);
                    is_changed = true;
                    idx--;
                }
                nb_average_plus_one_members++;
            } else if (p_cards_hold->cards_num == average + 1)
            {
                if (nb_average_plus_one_members < modulo)
                {
                    nb_average_plus_one_members++;
                } else
                {
                    idx = p_cards_hold->cards_num - 1;
                    // Delete this card from this player
                    CADE_LOG("   Odd [%.*s] is deleted from member jdx %d uuid %d", p_cards_hold->cards[idx].card_len, (char *) p_cards_hold->cards[idx].card_name,
                            jdx, p_cade->cade_member_tbl.arr_cade_players[jdx].uuid);
                    cade_delete_card_from_player(p_cards_hold, (uint8_t *) p_cards_hold->cards[idx].card_name, p_cards_hold->cards[idx].card_len);
                    is_changed = true;
                }
            }
        }
    }

    // Distribute average cards for each member
    for (idx = 0; idx < pokers_num; idx++)
    {
        if (!is_hold_by_any_player_on_candicate(&p_cade->cade_member_tbl, (uint8_t *) p_poker_set[idx].card_name, p_poker_set[idx].card_len))
        {
            nb_average_plus_one_members = 0;
            // Add this card to the player having the number of cards less than average
            for (jdx = 0; jdx < p_cade->cade_member_tbl.nb_players; jdx++)
            {
                // Only deal card for alive player
                if (p_cade->cade_member_tbl.arr_cade_players[jdx].is_alive)
                {
                    p_cards_hold = &p_cade->cade_member_tbl.arr_cade_players[jdx].candidate_cards;

                    if (p_cards_hold->cards_num < average)
                    {
                        CADE_LOG("   Even [%.*s] is dealt for member idx %d uuid %d", p_poker_set[idx].card_len, (char *) p_poker_set[idx].card_name, jdx,
                                p_cade->cade_member_tbl.arr_cade_players[jdx].uuid);
                        // Deal this card for this player
                        cade_add_card_for_player(p_cards_hold, (uint8_t *) p_poker_set[idx].card_name, p_poker_set[idx].card_len);
                        is_changed = true;
                        break;
                    } else if (p_cards_hold->cards_num > average)
                    {
                        nb_average_plus_one_members++;
                    }
                    else if (p_cards_hold->cards_num == average)
                    {
                        if (nb_average_plus_one_members < modulo)
                        {
                            CADE_LOG("   Odd [%.*s] is dealt for member idx %d uuid %d", p_poker_set[idx].card_len, (char *) p_poker_set[idx].card_name, jdx,
                                    p_cade->cade_member_tbl.arr_cade_players[jdx].uuid);
                            // Deal this card for this player
                            cade_add_card_for_player(p_cards_hold, (uint8_t *) p_poker_set[idx].card_name, p_poker_set[idx].card_len);
                            is_changed = true;
                            nb_average_plus_one_members++;
                            break;
                        }
                    }
                }
            }
        }
    }

    return is_changed;
}

// Print the status
void dump_card_hold(struct cade *p_cade)
{
    int idx, jdx;
//    CADE_LOG("\n===[Current zone]:");
//    for (idx = 0; idx < p_cade->cade_member_tbl.nb_member; idx++)
//    {
//        if (p_cade->cade_member_tbl.table_cade_member[idx].is_alive)
//        {
//            struct cade_member_info *p_member = &p_cade->cade_member_tbl.table_cade_member[idx];
//            CADE_LOG("\nMember alive %d:", idx);
//            for (jdx = 0; jdx < p_member->current_cards.cards_num; jdx++)
//            {
//                struct cards *p_cards_hold = &p_member->current_cards.cards_hold[jdx];
//                CADE_LOG("   Card: \t[%.*s]", p_cards_hold->card_len, p_cards_hold->card_name);
//            }
//        } else
//        {
//            struct cade_member_info *p_member = &p_cade->cade_member_tbl.table_cade_member[idx];
//            CADE_LOG("\nMember inactive %d:", idx);
//            for (jdx = 0; jdx < p_member->current_cards.cards_num; jdx++)
//            {
//                struct cards *p_cards_hold = &p_member->current_cards.cards_hold[jdx];
//                CADE_LOG("   Card: \t[%.*s]", p_cards_hold->card_len, p_cards_hold->card_name);
//            }
//        }
//    }
//
//    CADE_LOG("\n===[Candidate zone]:");
    for (idx = 0; idx < p_cade->cade_member_tbl.nb_players; idx++)
    {
        if (p_cade->cade_member_tbl.arr_cade_players[idx].is_alive)
        {
            struct cade_member_info *p_member = &p_cade->cade_member_tbl.arr_cade_players[idx];
            CADE_LOG("\nMember alive %d:", idx);
            for (jdx = 0; jdx < p_member->candidate_cards.cards_num; jdx++)
            {
                struct cards *p_cards_hold = &p_member->candidate_cards.cards[jdx];
                CADE_LOG("   Card: \t[%.*s]", p_cards_hold->card_len, p_cards_hold->card_name);
            }
        } else
        {
            struct cade_member_info *p_member = &p_cade->cade_member_tbl.arr_cade_players[idx];
            CADE_LOG("\nMember inactive %d:", idx);
            for (jdx = 0; jdx < p_member->candidate_cards.cards_num; jdx++)
            {
                struct cards *p_cards_hold = &p_member->candidate_cards.cards[jdx];
                CADE_LOG("   Card: \t[%.*s]", p_cards_hold->card_len, p_cards_hold->card_name);
            }
        }
    }
}

// Test cards dealing
void test_cade_dealing()
{
    struct cade s_cade, *p_cade = &s_cade;
    int idx = 0;

    // Init cade table
    memset(p_cade, 0, sizeof(struct cade));
    init_cade_member_tbl(&p_cade->cade_member_tbl);
    // Init poker set
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 1", (uint16_t) strlen("card 1"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 2", (uint16_t) strlen("card 2"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 3", (uint16_t) strlen("card 3"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 4", (uint16_t) strlen("card 4"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 5", (uint16_t) strlen("card 5"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 6", (uint16_t) strlen("card 6"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 7", (uint16_t) strlen("card 7"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 8", (uint16_t) strlen("card 8"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 9", (uint16_t) strlen("card 9"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card 10", (uint16_t) strlen("card 10"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card J", (uint16_t) strlen("card J"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card Q", (uint16_t) strlen("card Q"));
    cade_add_card_for_poker_set(p_cade, (uint8_t *) "card K", (uint16_t) strlen("card K"));




//    // Init up state for all member
//    p_cade->cade_member_tbl.nb_member = 5;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//
//    // First dealing
//    CADE_LOG("\n=========== First dealing ===========");
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    CADE_LOG("\n=========== One more player has joined ===========");
//
//    // Next dealing
//    p_cade->cade_member_tbl.nb_member++;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    CADE_LOG("\n=========== One more player has joined ===========");
//
//    // Next dealing
//    p_cade->cade_member_tbl.nb_member++;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    CADE_LOG("\n=========== One more player has joined ===========");
//
//    // Next dealing
//    p_cade->cade_member_tbl.nb_member++;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    CADE_LOG("\n=========== One more player has joined ===========");
//
//    // Next dealing
//    p_cade->cade_member_tbl.nb_member++;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    CADE_LOG("\n=========== One player has left ===========");
//
//    // Next dealing
//    p_cade->cade_member_tbl.table_cade_member[1].is_alive = false;
//    memset(&p_cade->cade_member_tbl.table_cade_member[1].current_cards, 0, sizeof(struct some_cards));
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    CADE_LOG("\n=========== Two player has left ===========");
//
//    // Next dealing
//    p_cade->cade_member_tbl.table_cade_member[0].is_alive = false;
//    memset(&p_cade->cade_member_tbl.table_cade_member[0].current_cards, 0, sizeof(struct some_cards));
//    p_cade->cade_member_tbl.table_cade_member[6].is_alive = false;
//    memset(&p_cade->cade_member_tbl.table_cade_member[6].current_cards, 0, sizeof(struct some_cards));
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);




//    // Init up state for all member
//    p_cade->cade_member_tbl.nb_member = 2;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//    p_cade->cade_member_tbl.table_cade_member[idx++].is_alive = true;
//
//    // First dealing
//    CADE_LOG("\n=========== First dealing ===========");
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    // Next dealing
//    CADE_LOG("\n=========== One player down ===========");
//    p_cade->cade_member_tbl.table_cade_member[0].is_alive = false;
//    memset(&p_cade->cade_member_tbl.table_cade_member[0].current_cards, 0, sizeof(struct some_cards));
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
//
//    // Next dealing
//    CADE_LOG("\n=========== The player come back ===========");
//    p_cade->cade_member_tbl.table_cade_member[0].is_alive = true;
//    cade_do_deal_cards(p_cade);
//    dump_card_hold(p_cade);
//    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);






    // Init up state for all member
    p_cade->cade_member_tbl.nb_players = 2;
    p_cade->cade_member_tbl.arr_cade_players[idx++].is_alive = true;
    p_cade->cade_member_tbl.arr_cade_players[idx++].is_alive = true;

    // First dealing
    CADE_LOG("\n=========== First dealing ===========");
    cade_do_deal_cards(p_cade);
    dump_card_hold(p_cade);
    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);

    // Next dealing
    CADE_LOG("\n=========== One more player has joined ===========");
    p_cade->cade_member_tbl.nb_players++;
    p_cade->cade_member_tbl.arr_cade_players[idx++].is_alive = true;
    cade_do_deal_cards(p_cade);
    dump_card_hold(p_cade);
    copy_dealer_can_to_cur_cards(&p_cade->cade_member_tbl);
}












