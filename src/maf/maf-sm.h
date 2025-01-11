/*
 * Copyright (C) 2019-2022 by Sukchan Lee <acetcom@gmail.com>
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

#ifndef MAF_SM_H
#define MAF_SM_H

#include "event.h"

#ifdef __cplusplus
extern "C" {
#endif

void maf_state_initial(ogs_fsm_t *s, maf_event_t *e);
void maf_state_final(ogs_fsm_t *s, maf_event_t *e);
void maf_state_operational(ogs_fsm_t *s, maf_event_t *e);

void maf_nf_fsm_init(ogs_sbi_nf_instance_t *nf_instance);
void maf_nf_fsm_fini(ogs_sbi_nf_instance_t *nf_instance);

void maf_nf_state_initial(ogs_fsm_t *s, maf_event_t *e);
void maf_nf_state_final(ogs_fsm_t *s, maf_event_t *e);
void maf_nf_state_will_register(ogs_fsm_t *s, maf_event_t *e);
void maf_nf_state_registered(ogs_fsm_t *s, maf_event_t *e);
void maf_nf_state_de_registered(ogs_fsm_t *s, maf_event_t *e);
void maf_nf_state_exception(ogs_fsm_t *s, maf_event_t *e);

#define maf_sm_debug(__pe) \
    ogs_debug("%s(): %s", __func__, maf_event_get_name(__pe))

#ifdef __cplusplus
}
#endif

#endif /* MAF_SM_H */