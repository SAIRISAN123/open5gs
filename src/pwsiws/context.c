#include "context.h"
#include "ogs-sbi.h"
#include "ogs-app.h"

OGS_POOL(pwsiws_connection_pool, pwsiws_connection_t);
OGS_POOL(pwsiws_warning_pool, pwsiws_warning_t);

int __pwsiws_log_domain;

static pwsiws_context_t self;

void pwsiws_context_init(void) 
{
    memset(&self, 0, sizeof(pwsiws_context_t));
    
    ogs_list_init(&self.connection_list);
    ogs_list_init(&self.warning_list);
    ogs_list_init(&self.pws_iws_list);
    ogs_list_init(&self.pws_iws_list6);
    ogs_list_init(&self.sbcap_list);

    /* Initialize pools */
    ogs_pool_init(&pwsiws_connection_pool, 32);
    ogs_pool_init(&pwsiws_warning_pool, 64);

    self.amf_sbi = NULL;
}

void pwsiws_context_final(void) 
{
    pwsiws_connection_remove_all();
    pwsiws_warning_remove_all();

    if (self.amf_sbi) {
        // TODO: Find the correct way to free the SBI client
        // ogs_sbi_client_free(self.amf_sbi);
        self.amf_sbi = NULL;
    }
}

pwsiws_context_t *pwsiws_self(void) 
{ 
    return &self; 
}

int pwsiws_context_parse_config(void)
{
    // Hardcode SBCAP server configuration for now
    ogs_sockaddr_t *addr = NULL;
    ogs_socknode_t *node = NULL;
    char buf[OGS_ADDRSTRLEN];
    
    // Create SBCAP server configuration
    addr = ogs_calloc(1, sizeof(ogs_sockaddr_t));
    ogs_assert(addr);
    
    // Set to 127.0.0.199:29168 as configured in YAML
    addr->sa.sa_family = AF_INET;
    addr->sin.sin_addr.s_addr = inet_addr("127.0.0.199");
    addr->sin.sin_port = htons(29168);
    
    node = ogs_socknode_add(&self.sbcap_list, AF_INET, addr, NULL);
    ogs_assert(node);
    
    ogs_info("PWS-IWS SBCAP server configured: [%s]:%d", 
            OGS_ADDR(addr, buf), OGS_PORT(addr));
    ogs_info("PWS-IWS configured as SBCAP server");
    ogs_info("SBCAP server list count: %d", ogs_list_count(&self.sbcap_list));
    
    return OGS_OK;
}

int pwsiws_context_nf_info(void) 
{ 
    return OGS_OK; 
}

pwsiws_connection_t *pwsiws_connection_add(ogs_sock_t *sock, ogs_sockaddr_t *addr) 
{ 
    pwsiws_connection_t *connection = NULL;
    char buf[OGS_ADDRSTRLEN];

    ogs_assert(sock);
    ogs_assert(addr);

    ogs_pool_alloc(&pwsiws_connection_pool, &connection);
    if (!connection) {
        ogs_error("Failed to allocate PWS-IWS connection");
        return NULL;
    }

    memset(connection, 0, sizeof(pwsiws_connection_t));

    connection->sctp.sock = sock;
    connection->addr = ogs_calloc(1, sizeof(ogs_sockaddr_t));
    ogs_assert(connection->addr);
    memcpy(connection->addr, addr, sizeof(ogs_sockaddr_t));

    ogs_list_add(&self.connection_list, connection);

    ogs_info("PWS-IWS connection added: [%s]:%d", 
            OGS_ADDR(connection->addr, buf), OGS_PORT(connection->addr));

    return connection;
}

void pwsiws_connection_remove(pwsiws_connection_t *connection) {}
void pwsiws_connection_remove_all(void) {}
pwsiws_connection_t *pwsiws_connection_find_by_addr(ogs_sockaddr_t *addr) { return NULL; }
pwsiws_connection_t *pwsiws_connection_find_by_id(ogs_pool_id_t id) { return NULL; }
pwsiws_warning_t *pwsiws_warning_add(pwsiws_connection_t *connection) { return NULL; }
void pwsiws_warning_remove(pwsiws_warning_t *warning) {}
void pwsiws_warning_remove_all(void) {}
pwsiws_warning_t *pwsiws_warning_find_by_id(ogs_pool_id_t id) { return NULL; }
pwsiws_warning_t *pwsiws_warning_find_by_warning_id(uint32_t warning_id) { return NULL; }
uint32_t pwsiws_warning_id_alloc(void) { return 0; }
uint32_t pwsiws_message_id_alloc(void) { return 0; } 