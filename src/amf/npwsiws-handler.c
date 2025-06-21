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
#include "ngap-handler.h"
#include "npwsiws-build.h"

int amf_npwsiws_handle_warning_message(
        ogs_sbi_stream_t *stream, ogs_sbi_message_t *message)
{
    int rv;
    ogs_sbi_response_t *response = NULL;
    ogs_sbi_message_t response_message;
    char buf[OGS_ADDRSTRLEN];

    cJSON *json = NULL;
    cJSON *messageIdentifier = NULL;
    cJSON *serialNumber = NULL;
    cJSON *warningAreaList = NULL;
    cJSON *repetitionPeriod = NULL;
    cJSON *numberOfBroadcastsRequested = NULL;
    cJSON *warningType = NULL;
    cJSON *warningSecurityInfo = NULL;
    cJSON *dataCodingScheme = NULL;
    cJSON *warningMessageContents = NULL;
    cJSON *concurrentWarningMessageInd = NULL;
    cJSON *warningAreaCoordinates = NULL;

    ogs_assert(stream);
    ogs_assert(message);

    ogs_debug("NPWSIWS Warning Message Request");

    if (!message->http.content_type) {
        ogs_error("No HTTP content");
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No HTTP content", NULL, NULL));
        return OGS_ERROR;
    }

    json = cJSON_Parse(message->http.content_type);
    if (!json) {
        ogs_error("Cannot parse JSON");
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "Cannot parse JSON", NULL, NULL));
        return OGS_ERROR;
    }

    /* Parse JSON fields */
    messageIdentifier = cJSON_GetObjectItem(json, "messageIdentifier");
    serialNumber = cJSON_GetObjectItem(json, "serialNumber");
    warningAreaList = cJSON_GetObjectItem(json, "warningAreaList");
    repetitionPeriod = cJSON_GetObjectItem(json, "repetitionPeriod");
    numberOfBroadcastsRequested = cJSON_GetObjectItem(json, "numberOfBroadcastsRequested");
    warningType = cJSON_GetObjectItem(json, "warningType");
    warningSecurityInfo = cJSON_GetObjectItem(json, "warningSecurityInfo");
    dataCodingScheme = cJSON_GetObjectItem(json, "dataCodingScheme");
    warningMessageContents = cJSON_GetObjectItem(json, "warningMessageContents");
    concurrentWarningMessageInd = cJSON_GetObjectItem(json, "concurrentWarningMessageInd");
    warningAreaCoordinates = cJSON_GetObjectItem(json, "warningAreaCoordinates");

    /* Validate required fields */
    if (!messageIdentifier || !serialNumber || !warningMessageContents) {
        ogs_error("Missing required fields");
        cJSON_Delete(json);
        ogs_assert(true ==
            ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "Missing required fields", NULL, NULL));
        return OGS_ERROR;
    }

    ogs_debug("    MessageIdentifier[%d]", messageIdentifier->valueint);
    ogs_debug("    SerialNumber[%d]", serialNumber->valueint);
    if (repetitionPeriod)
        ogs_debug("    RepetitionPeriod[%d]", repetitionPeriod->valueint);
    if (numberOfBroadcastsRequested)
        ogs_debug("    NumberOfBroadcastsRequested[%d]", numberOfBroadcastsRequested->valueint);
    if (warningType)
        ogs_debug("    WarningType[%d]", warningType->valueint);
    if (dataCodingScheme)
        ogs_debug("    DataCodingScheme[%d]", dataCodingScheme->valueint);
    if (warningMessageContents)
        ogs_debug("    WarningMessageContents[%s]", warningMessageContents->valuestring);
    if (concurrentWarningMessageInd)
        ogs_debug("    ConcurrentWarningMessageInd[%d]", concurrentWarningMessageInd->valueint);

    /* Broadcast warning to all gNBs */
    amf_gnb_t *target_gnb = NULL;
    int broadcast_count = 0;
    
    ogs_list_for_each(&amf_self()->gnb_list, target_gnb) {
        if (target_gnb->state.ng_setup_success) {
            /* TODO: Check if gNB serves area in warningAreaList */
            /* For now, broadcast to all connected gNBs */
            
            ogs_debug("Broadcasting warning to gNB[%s]", 
                OGS_ADDR(target_gnb->sctp.addr, buf));
            
            /* Call the NGAP warning broadcast function */
            rv = ngap_broadcast_warning_to_gnb(target_gnb, 
                messageIdentifier->valueint,
                serialNumber->valueint,
                warningMessageContents->valuestring,
                repetitionPeriod ? repetitionPeriod->valueint : 0,
                numberOfBroadcastsRequested ? numberOfBroadcastsRequested->valueint : 1,
                warningType ? warningType->valueint : 0,
                dataCodingScheme ? dataCodingScheme->valueint : 0,
                concurrentWarningMessageInd ? concurrentWarningMessageInd->valueint : 0);
            
            if (rv == OGS_OK) {
                broadcast_count++;
            } else {
                ogs_error("Failed to broadcast warning to gNB[%s]", 
                    OGS_ADDR(target_gnb->sctp.addr, buf));
            }
        }
    }

    ogs_info("Warning message broadcasted to %d gNBs", broadcast_count);

    /* Send success response using build function */
    response = ogs_sbi_build_response(message, OGS_SBI_HTTP_STATUS_OK);
    ogs_assert(response);

    rv = ogs_sbi_server_send_response(stream, response);
    if (rv != true) {
        ogs_error("ogs_sbi_server_send_response() failed");
        cJSON_Delete(json);
        return OGS_ERROR;
    }

    cJSON_Delete(json);
    return OGS_OK;
} 