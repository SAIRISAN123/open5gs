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

#include "npwsiws-build.h"
#include "npwsiws-handler.h"

ogs_sbi_request_t *amf_npwsiws_build_warning_message_response(
        ogs_sbi_message_t *message, int status)
{
    ogs_sbi_request_t *request = NULL;
    ogs_sbi_message_t response_message;
    cJSON *json = NULL;

    ogs_assert(message);

    ogs_debug("Build NPWSIWS Warning Message Response");

    memset(&response_message, 0, sizeof(response_message));

    /* Set HTTP headers */
    response_message.http.content_type = (char*)"application/json";

    /* Build JSON response body */
    json = cJSON_CreateObject();
    ogs_assert(json);

    cJSON_AddStringToObject(json, "status", "SUCCESS");
    cJSON_AddStringToObject(json, "message", "Warning message processed successfully");

    /* Build SBI request */
    request = ogs_sbi_build_request(&response_message);
    ogs_assert(request);

    cJSON_Delete(json);

    return request;
}

ogs_sbi_request_t *amf_npwsiws_build_warning_message_notification(
        ogs_sbi_message_t *message, int status)
{
    ogs_sbi_request_t *request = NULL;
    ogs_sbi_message_t notification_message;
    cJSON *json = NULL;

    ogs_assert(message);

    ogs_debug("Build NPWSIWS Warning Message Notification");

    memset(&notification_message, 0, sizeof(notification_message));

    /* Set HTTP headers */
    notification_message.http.content_type = (char*)"application/json";

    /* Build JSON notification body */
    json = cJSON_CreateObject();
    ogs_assert(json);

    cJSON_AddStringToObject(json, "notificationType", "WARNING_MESSAGE_STATUS");
    cJSON_AddStringToObject(json, "status", "BROADCAST_COMPLETED");
    cJSON_AddStringToObject(json, "timestamp", "2024-01-01T00:00:00Z");

    /* Build SBI request */
    request = ogs_sbi_build_request(&notification_message);
    ogs_assert(request);

    cJSON_Delete(json);

    return request;
} 