/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
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

#include "namf-build.h"
#include "ngap-build.h"

ogs_sbi_request_t *pwsiws_nonuen2_comm_build_nonuen2_message_transfer(
        pwsiws_warning_t *warning, pwsiws_nonuen2_message_transfer_param_t *param)
{
    int i;
    pwsiws_connection_t *connection = NULL;

    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_header_t header;

    /* For Non-UE N2, we'll use a simplified approach since OpenAPI structures don't exist yet */
    ogs_sbi_http_message_t http_message;

    ogs_assert(warning);
    connection = pwsiws_connection_find_by_id(warning->connection_id);
    ogs_assert(connection);

    ogs_assert(param);
    ogs_assert(param->state);
    ogs_assert(param->n2smbuf);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NAMF_COMM;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_NON_UE_N2_MESSAGES;

    /* Set up HTTP message for Non-UE N2 transfer */
    memset(&http_message, 0, sizeof(http_message));
    http_message.headers = ogs_hash_make();
    ogs_sbi_header_set(http_message.headers, OGS_SBI_CONTENT_TYPE, 
                       OGS_SBI_CONTENT_NGAP_TYPE);

    /* Add the NGAP message as a part */
    http_message.part[http_message.num_of_part].pkbuf = param->n2smbuf;
    http_message.part[http_message.num_of_part].content_id =
        (char *)OGS_SBI_CONTENT_NGAP_SM_ID;
    http_message.part[http_message.num_of_part].content_type =
        (char *)OGS_SBI_CONTENT_NGAP_TYPE;
    http_message.num_of_part++;

    /* Add failure notification URI if requested */
    if (param->nonuen2_failure_txf_notif_uri == true) {
        server = ogs_sbi_server_first();
        ogs_assert(server);

        memset(&header, 0, sizeof(header));
        header.service.name = (char *)OGS_SBI_SERVICE_NAME_NPWS_CALLBACK;
        header.api.version = (char *)OGS_SBI_API_V1;
        header.resource.component[0] =
                (char *)OGS_SBI_RESOURCE_NAME_N1_N2_FAILURE_NOTIFY;
        
        char *failure_uri = ogs_sbi_server_uri(server, &header);
        ogs_assert(failure_uri);
        
        /* Add as custom header for failure notification */
        ogs_sbi_header_set(http_message.headers, OGS_SBI_CUSTOM_CALLBACK, failure_uri);
        ogs_free(failure_uri);
    }

    /* Add skip indicator if requested */
    if (param->skip_ind == true) {
        ogs_sbi_header_set(http_message.headers, "Skip-Indicator", "true");
    }

    /* Build the request */
    request = ogs_sbi_request_new();
    ogs_assert(request);

    /* Copy header and HTTP message to request */
    memcpy(&request->h, &message.h, sizeof(ogs_sbi_header_t));
    memcpy(&request->http, &http_message, sizeof(ogs_sbi_http_message_t));

    /* Clean up */
    for (i = 0; i < http_message.num_of_part; i++)
        if (http_message.part[i].pkbuf)
            ogs_pkbuf_free(http_message.part[i].pkbuf);

    ogs_hash_destroy(http_message.headers);

    return request;
}

ogs_sbi_request_t *pwsiws_nonuen2_callback_build_warning_status(
        pwsiws_warning_t *warning, void *data)
{
    /* TODO: Implement warning status notification when OpenAPI structures are available */
    ogs_warn("Warning status notification not implemented yet");
    return NULL;
}