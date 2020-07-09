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
 * @file      cade_dealer.c
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "cade_dealer.h"

static void cb_hdl_recv_cards(struct cade *p_cade)
{
    printf("################# Callback handle is called ###############\n");
    cade_show_members_info(p_cade);
}

// Create a cade dealer
int main(int argc, char *argv[])
{
    // Init
    struct cade *p_cade = cade_dealer_init(argv[1]);

    // Get poker
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 1", (uint16_t) strlen("card 1"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 2", (uint16_t) strlen("card 2"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 3", (uint16_t) strlen("card 3"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 4", (uint16_t) strlen("card 4"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 5", (uint16_t) strlen("card 5"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 6", (uint16_t) strlen("card 6"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 7", (uint16_t) strlen("card 7"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 8", (uint16_t) strlen("card 8"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 9", (uint16_t) strlen("card 9"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card 10", (uint16_t) strlen("card 10"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card J", (uint16_t) strlen("card J"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card Q", (uint16_t) strlen("card Q"));
    cade_dealer_add_poker(p_cade, (uint8_t *) "card K", (uint16_t) strlen("card K"));

    // Set callback when received card dealt
    cade_dealer_set_cb_recv_cards_success(p_cade, cb_hdl_recv_cards);

    // Set to debug info
    cade_dealer_set_cb_show_cards_info(p_cade, cade_show_members_info);

    // Run dealer
    cade_dealer_run(p_cade, atoi(argv[2]), atoi(argv[3]));

    return 0;
}






