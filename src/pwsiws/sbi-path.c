#include "sbi-path.h"
#include "ogs-sbi.h"
#include "context.h"

int pwsiws_sbi_open(void) 
{
    // Start all SBI servers using the standard SBI context
    int count = 0;
    ogs_sbi_server_t *server = NULL;
    ogs_list_for_each(&ogs_sbi_self()->server_list, server) {
        count++;
    }
    ogs_info("pwsiws_sbi_open: Starting %d SBI servers", count);
    if (ogs_sbi_server_start_all(pwsiws_sbi_server_callback) != OGS_OK) {
        ogs_error("Failed to start PWS-IWS SBI servers");
        return OGS_ERROR;
    }
    return OGS_OK;
}

void pwsiws_sbi_close(void) 
{
    ogs_sbi_server_stop_all();
    ogs_sbi_server_remove_all();
}

int pwsiws_sbi_server_callback(ogs_sbi_request_t *request, void *data) 
{ 
    ogs_info("PWS-IWS SBI server callback received");
    return OGS_OK; 
}

int pwsiws_sbi_discover_and_send(ogs_sbi_service_type_e service_type, 
        ogs_sbi_request_t *(*build)(pwsiws_warning_t *warning, void *data),
        pwsiws_warning_t *warning, ogs_sbi_xact_t *xact, void *data) 
{ 
    return OGS_OK; 
}

int pwsiws_sbi_client_open(void) 
{ 
    return OGS_OK; 
}

void pwsiws_sbi_client_close(void) 
{
}

int pwsiws_sbi_client_callback(ogs_sbi_response_t *response, void *data) 
{ 
    return OGS_OK; 
} 