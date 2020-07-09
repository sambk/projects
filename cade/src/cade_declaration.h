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
 * @file      cade_declaration.h
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

#ifndef CADE_DECLARATION_H_
#define CADE_DECLARATION_H_

#define DBG_CARDS_HOLD

// Multicast ip/port define
#define SSDP_ADDR                       "239.255.255.250"
#define SSDP_PORT                       1901
#define UCAST_PORT                      1902

// Max support length
#define MAXLINE                         2048
#define MACHINEID_LEN                   40

#define MAX_CARD_LEN                    64
#define MAX_CARD_NUM                    128
#define MAX_CADE_MEMBERS                64
#define MAX_POKER_CARDS                 64

#define MAX_IP_PREFIX_PER_BASKET        256
#define MAX_EGGS_PER_BASKET             32
#define MAX_BASKET_NAME_LEN             64
#define MAX_BASKET_NUM                  64

// Time define
#define CADE_SEC_WAIT_CARDDEAL_REP      1
#define CADE_MILI_SEC_MSEARCH_FREQ      100
#define CADE_SEC_MSEARCH_DURATION       3
#define CADE_MILI_SEC_POLL_TIMEOUT      10
#define CADE_MILI_SEC_NOTIFY_FREQ       100
#define CADE_SEC_SHOW_CARD_DEBUG_FREQ   5
#define CLOCKS_PER_MILI_SEC             (CLOCKS_PER_SEC/1000)
#define TIME_PING_SEND_SEC              0
#define TIME_PING_SEND_NSEC             500000000 /* nanosecond*/
#define NB_PING_TIMEOUT                 3

#endif /* CADE_DECLARATION_H_ */
