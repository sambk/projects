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
 * @file      cade_watcher.h
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      May 5, 2020
 *
 * @brief
 *
 ******************************************************************************/

#ifndef CADE_WATCHER_H_
#define CADE_WATCHER_H_

#include "cade.h"

/**
 * @brief: Get info about player and cards hold for watcher
 *
 * @param [IN]
 *
 * @return : void
 */
int cade_get_cards_hold_by_players(struct cade *p_cade, struct player_info *current_player_info);

/**
 * @brief: Show cards hold
 *
 * @param [IN]
 *
 * @return : void
 */
void cade_show_cards_hold_by_players(struct cade *p_cade);

/**
 * @brief: Init resource for cade player instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_watcher_init(const char *if_addr);

/**
 * @brief: Init resource for cade player instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_watcher_init_uint32_ip(uint32_t if_addr);

/**
 * @brief: Wait for uuid. This function will block until getting uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
uint16_t cade_watcher_request_for_uuid(struct cade *p_cade);

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_watcher_set_cb_recv_cards_success(struct cade *p_cade, void (*cb_hdl_recv_cards)(struct cade *));

/**
 * @brief: Set callback to monitor core health
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_watcher_set_cb_keep_alived_mark(struct cade *p_cade, void (*cb_keep_alived_mark)(void));

/**
 * @brief: Set callback handle to call when receiving cards successfully
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
void cade_watcher_set_cb_show_cards_info(struct cade *p_cade, void (*cb_hdl_show_cards)(struct cade *));

/**
 * @brief: Run cade watcher business
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_watcher_run(struct cade *p_cade);

#endif /* CADE_WATCHER_H_ */
