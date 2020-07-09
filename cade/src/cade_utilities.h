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
 * @file      cade_utilities.h
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

#ifndef CADE_UTILITIES_H_
#define CADE_UTILITIES_H_

#include <stdint.h>

//#define DBG_LOG(...) printf(__VA_ARGS__);printf("\n");
#define DBG_LOG(...) do {} while (0)
#define ERR_LOG(...) printf(__VA_ARGS__);printf("\n");
#define CADE_LOG(...) printf(__VA_ARGS__);printf("\n");
#define CADE_N_LOG(...) printf(__VA_ARGS__);
#define ERR -1
#define OK 0
#define DUMP_MSG(_title_, _buf_, _len_) printf(_title_);for(int i = 0; i < _len_; i++){printf("%02X", _buf_[i]);}printf("\n");

#define PRIip                                       "%i.%i.%i.%i"
#define IP_DOT(x)                                   (((x)>>24) & 0xFF),(((x)>>16) & 0xFF),(((x)>>8) & 0xFF),((x) & 0xFF)

#define NB_R_SOCK 2

uint16_t generate_sequence();

#endif /* CADE_UTILITIES_H_ */



