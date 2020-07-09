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
 * @file      cade_member.h
 *
 * @author    sampv@viettel.com.vn
 *
 * @version   3.0
 *
 * @date      May 29, 2020
 *
 * @brief
 *
 ******************************************************************************/

#ifndef CADE_MEMBER_H_
#define CADE_MEMBER_H_


/**
 * @brief: Init resource for cade member instance
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
struct cade *cade_member_init(enum cade_role role, const char *if_addr);

/**
 * @brief: Wait for uuid. This function will block until getting uuid
 *
 * @param [IN]
 *
 * @return
 *       uuid : Success
 */
uint16_t cade_member_request_for_uuid(struct cade *p_cade);

/**
 * @brief: Run cade member business
 *
 * @param [IN]
 *
 * @return
 *          0 : Success
 *         -1 : Failed
 */
void cade_member_run(struct cade *p_cade);

#endif /* CADE_MEMBER_H_ */
