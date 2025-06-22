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

#include "npwsiws-handler.h"
#include "npwsiws-build.h"
#include "ngap-path.h"
#include "sbc-message.h"

int amf_npwsiws_handle_warning_message(
        ogs_sbi_stream_t *stream, ogs_sbi_message_t *recvmsg)
{
    int rv;
    ogs_sbi_response_t *response = NULL;
    cJSON *json = NULL;
    cJSON *item = NULL;
    
    sbc_pws_data_t sbc_pws;

    ogs_assert(stream);
    ogs_assert(recvmsg);

    ogs_debug("NPWSIWS Warning Message received");

    /* Parse the incoming JSON message */
    if (!recvmsg->part[0].pkbuf || !recvmsg->part[0].pkbuf->len) {
        ogs_error("No content in NPWSIWS message");
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream,
                OGS_SBI_HTTP_STATUS_BAD_REQUEST, recvmsg,
                "No content in message", NULL, NULL));
        return OGS_ERROR;
    }

    json = cJSON_Parse((char*)recvmsg->part[0].pkbuf->data);
    if (!json) {
        ogs_error("Failed to parse JSON content");
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream,
                OGS_SBI_HTTP_STATUS_BAD_REQUEST, recvmsg,
                "Invalid JSON content", NULL, NULL));
        return OGS_ERROR;
    }

    /* Initialize sbc_pws structure */
    memset(&sbc_pws, 0, sizeof(sbc_pws_data_t));

    /* Parse messageIdentifier */
    item = cJSON_GetObjectItem(json, "messageIdentifier");
    if (item && cJSON_IsNumber(item)) {
        sbc_pws.message_id = item->valueint;
        ogs_info("Received messageIdentifier: %d", sbc_pws.message_id);
    }

    /* Parse serialNumber */
    item = cJSON_GetObjectItem(json, "serialNumber");
    if (item && cJSON_IsNumber(item)) {
        sbc_pws.serial_number = item->valueint;
        ogs_info("Received serialNumber: %d", sbc_pws.serial_number);
    }

    /* Parse repetitionPeriod */
    item = cJSON_GetObjectItem(json, "repetitionPeriod");
    if (item && cJSON_IsNumber(item)) {
        sbc_pws.repetition_period = item->valueint;
        ogs_info("Received repetitionPeriod: %d", sbc_pws.repetition_period);
    }

    /* Parse numberOfBroadcastsRequested */
    item = cJSON_GetObjectItem(json, "numberOfBroadcastsRequested");
    if (item && cJSON_IsNumber(item)) {
        sbc_pws.number_of_broadcast = item->valueint;
        ogs_info("Received numberOfBroadcastsRequested: %d", sbc_pws.number_of_broadcast);
    }

    /* Parse dataCodingScheme */
    item = cJSON_GetObjectItem(json, "dataCodingScheme");
    if (item && cJSON_IsNumber(item)) {
        sbc_pws.data_coding_scheme = item->valueint;
        ogs_info("Received dataCodingScheme: %d", sbc_pws.data_coding_scheme);
    }

    /* Parse warningMessage */
    item = cJSON_GetObjectItem(json, "warningMessage");
    if (item && cJSON_IsString(item)) {
        size_t msg_len = strlen(item->valuestring);
        if (msg_len > sizeof(sbc_pws.message_contents) - 1) {
            msg_len = sizeof(sbc_pws.message_contents) - 1;
        }
        memcpy(sbc_pws.message_contents, item->valuestring, msg_len);
        sbc_pws.message_contents[msg_len] = '\0';
        sbc_pws.message_length = msg_len;
        ogs_info("Received warningMessage: %s", sbc_pws.message_contents);
    }

    /* Parse taiList if present */
    item = cJSON_GetObjectItem(json, "taiList");
    if (item && cJSON_IsArray(item)) {
        int array_size = cJSON_GetArraySize(item);
        sbc_pws.no_of_tai = 0;
        
        for (int i = 0; i < array_size && i < 16; i++) {
            cJSON *tai_item = cJSON_GetArrayItem(item, i);
            if (tai_item && cJSON_IsObject(tai_item)) {
                cJSON *plmn_item = cJSON_GetObjectItem(tai_item, "plmnId");
                cJSON *tac_item = cJSON_GetObjectItem(tai_item, "tac");
                
                if (plmn_item && cJSON_IsString(plmn_item) && 
                    tac_item && cJSON_IsNumber(tac_item)) {
                    
                    /* Parse PLMN ID string (format: "999-70") */
                    char *plmn_str = plmn_item->valuestring;
                    int mcc, mnc, mnc_len;
                    if (sscanf(plmn_str, "%d-%d", &mcc, &mnc) == 2) {
                        mnc_len = (mnc >= 100) ? 3 : (mnc >= 10) ? 2 : 1;
                        ogs_plmn_id_build(&sbc_pws.tai[sbc_pws.no_of_tai].plmn_id, 
                                        mcc, mnc, mnc_len);
                        sbc_pws.tai[sbc_pws.no_of_tai].tac = tac_item->valueint;
                        sbc_pws.no_of_tai++;
                        
                        ogs_info("Received TAI[%d]: PLMN=%s, TAC=%d", 
                                sbc_pws.no_of_tai-1, plmn_str, tac_item->valueint);
                    }
                }
            }
        }
    }

    cJSON_Delete(json);

    ogs_info("Processing warning message: ID=%d, Serial=%d, Message='%s', TAI_count=%d",
            sbc_pws.message_id, sbc_pws.serial_number, sbc_pws.message_contents, sbc_pws.no_of_tai);

    /* Send write replace warning request to all gNBs */
    rv = ngap_send_write_replace_warning_request(&sbc_pws);
    if (rv != OGS_OK) {
        ogs_error("Failed to send write replace warning request to gNBs");
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream,
                OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR, recvmsg,
                "Failed to broadcast warning message", NULL, NULL));
        return rv;
    }

    ogs_info("Warning message successfully broadcast to all gNBs");

    /* Send success response */
    response = ogs_sbi_response_new();
    if (!response) {
        ogs_error("Failed to create response");
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream,
                OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR, recvmsg,
                "Failed to create response", NULL, NULL));
        return OGS_ERROR;
    }

    response->status = OGS_SBI_HTTP_STATUS_OK;
    rv = ogs_sbi_server_send_response(stream, response);
    if (rv != OGS_OK) {
        ogs_error("Failed to send warning message response");
        return rv;
    }

    return OGS_OK;
} 