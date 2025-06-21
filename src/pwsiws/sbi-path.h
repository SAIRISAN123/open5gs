/*
 * Copyright (C) 2019-2025 by Sukchan Lee <acetcom@gmail.com>
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

#ifndef PWSIWS_SBI_PATH_H
#define PWSIWS_SBI_PATH_H

#include "ogs-sbi.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PWS-IWS SBI Server Functions */
int pwsiws_sbi_open(void);
void pwsiws_sbi_close(void);

/* PWS-IWS SBI Server Callback Functions */
int pwsiws_sbi_server_callback(ogs_sbi_request_t *request, void *data);

/* PWS-IWS SBI Path Functions */
int pwsiws_sbi_discover_and_send(ogs_sbi_service_type_e service_type,
        ogs_sbi_request_t *(*build)(pwsiws_warning_t *warning, void *data),
        pwsiws_warning_t *warning, ogs_sbi_xact_t *xact, void *data);

/* PWS-IWS SBI Client Functions */
int pwsiws_sbi_client_open(void);
void pwsiws_sbi_client_close(void);

/* PWS-IWS SBI Client Callback Functions */
int pwsiws_sbi_client_callback(ogs_sbi_response_t *response, void *data);

#ifdef __cplusplus
}
#endif

#endif /* PWSIWS_SBI_PATH_H */ 