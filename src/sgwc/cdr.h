/*
 * Copyright (C) 2019-2026 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SGWC_CDR_H
#define SGWC_CDR_H

#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Cause for record closing (3GPP TS 32.298 §5.1.2.2).
 * Only the values relevant to SGW-C are defined here.
 */
#define SGWC_CDR_CLOSE_NORMAL_RELEASE       0
#define SGWC_CDR_CLOSE_ABNORMAL_RELEASE     4
#define SGWC_CDR_CLOSE_MGMT_INTERVENTION   16

/*
 * Open a CDR for the session.  Sets record_opening_time and
 * local_sequence_number on sess.  Must be called after the session is
 * fully established (Create Session Response sent to MME).
 */
void sgwc_cdr_open(sgwc_sess_t *sess);

/*
 * Close the CDR and write it to the output file.
 * Must be called before sgwc_sess_remove() so that sess->gnode is still valid.
 */
void sgwc_cdr_close(sgwc_sess_t *sess, uint8_t cause);

/* Lifecycle: open/close the CDR output file. */
void sgwc_cdr_init(void);
void sgwc_cdr_final(void);

#ifdef __cplusplus
}
#endif

#endif /* SGWC_CDR_H */
