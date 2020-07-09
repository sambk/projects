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
 * @file      cade_player.c
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
#include "cade_player.h"

static void cb_hdl_recv_cards(struct cade *p_cade)
{
    printf("################# Callback handle is called ###############\n");

    struct some_cards current_cards;
    int ret = cade_get_cards_hold(p_cade, &current_cards);
    if (0 == ret)
    {
        printf("Cards hold: [");

        int idx;
        for (idx = 0; idx < current_cards.cards_num; idx++)
        {
            printf("%.*s, ", current_cards.cards[idx].card_len, current_cards.cards[idx].card_name);
        }
        printf("]\n");
    }
}

// Create a cade player
int main(int argc, char *argv[])
{
    // Init
    struct cade *p_cade = cade_player_init(argv[1]);

    // Wait until getting uuid
    uint16_t uuid = cade_player_request_for_uuid(p_cade);
    printf("uuid is assigned %d\n", uuid);

    // Set callback when received card dealt
    cade_player_set_cb_recv_cards_success(p_cade, cb_hdl_recv_cards);

    // Set callback to monitor core health
    cade_player_set_cb_keep_alived_mark(p_cade, NULL);

    // Set to debug info
    cade_player_set_cb_show_cards_info(p_cade, cade_show_cards_hold);

    // Run
    cade_player_run(p_cade);
    return 0;
}




