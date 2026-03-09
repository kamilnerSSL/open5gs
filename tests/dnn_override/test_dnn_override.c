/*
 * Copyright (C) 2026 by Keith Milner.
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

#include <netinet/in.h>
#include <yaml.h>
#include "ogs-core.h"
#include "app/ogs-app.h"
#include "smf/context.h"
#include "smf/s5c-handler.h"
#include "smf/s5c-build.h"
#include "smf/metrics.h"
#include "smf/fd-path.h"

void test_setup(void);
void test_teardown(void);
void test_dnn_override(const char *request_dnn, const char *canonical_dnn);

static void build_test_config(
        char *buf, size_t len,
        const char *canonical_dnn, const char *accept_dnn)
{
    snprintf(buf, len,
        "logger:\n"
        "    level: info\n"
        "smf:\n"
        "  gtpc:\n"
        "    server:\n"
        "      - address: 127.0.0.4\n"
        "  gtpu:\n"
        "    server:\n"
        "      - address: 127.0.0.4\n"
        "  pfcp:\n"
        "    server:\n"
        "      - address: 127.0.0.4\n"
        "  session:\n"
        "    - dnn: %s\n"
        "      subnet: 10.45.0.0/16\n"
        "      accept:\n"
        "        - %s\n"
        "  dns:\n"
        "    - 8.8.8.8\n",
        canonical_dnn, accept_dnn);
}

void test_setup(void)
{
    int rv;

    rv = ogs_log_config_domain(
            ogs_app()->logger.domain, ogs_app()->logger.level);
    ogs_assert(rv == OGS_OK);

    ogs_gtp_context_init(ogs_app()->pool.nf * OGS_MAX_NUM_OF_GTPU_RESOURCE);
    ogs_pfcp_context_init();
    ogs_sbi_context_init(OpenAPI_nf_type_SMF);
    smf_metrics_init();
    smf_context_init();

    rv = ogs_gtp_xact_init();
    ogs_assert(rv == OGS_OK);
    rv = ogs_pfcp_xact_init();
    ogs_assert(rv == OGS_OK);

    rv = smf_fd_init();
    ogs_assert(rv == OGS_OK);

    rv = ogs_gtp_context_parse_config("smf", "upf");
    ogs_assert(rv == OGS_OK);

    /* Populate gtpc_addr from the parsed list for use in response building.
     * Normally this is done by OGS_SETUP_GTPC_SERVER after socket open,
     * but the test environment does not open sockets. */
    {
        ogs_socknode_t *gtpc_node =
            ogs_list_first(&ogs_gtp_self()->gtpc_list);
        if (gtpc_node)
            ogs_gtp_self()->gtpc_addr = gtpc_node->addr;
    }

    rv = ogs_pfcp_context_parse_config("smf", "upf");
    ogs_assert(rv == OGS_OK);
    rv = ogs_sbi_context_parse_config("smf", "nrf", "scp");
    ogs_assert(rv == OGS_OK);
    rv = smf_context_parse_config();
    ogs_assert(rv == OGS_OK);

    rv = ogs_pfcp_ue_pool_generate();
    ogs_assert(rv == OGS_OK);
}

void test_teardown(void)
{
    smf_fd_final();
    smf_metrics_final();
    smf_context_final();
    ogs_sbi_context_final();
    ogs_pfcp_context_final();
    ogs_gtp_context_final();

    ogs_pfcp_xact_final();
    ogs_gtp_xact_final();
}

void test_dnn_override(const char *request_dnn, const char *canonical_dnn)
{
    ogs_gtp2_message_t message;
    ogs_gtp2_create_session_request_t *req = NULL;
    char apn_buf[OGS_MAX_APN_LEN + 1];
    uint8_t imsi[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

    ogs_gtp2_f_teid_t sgw_s5c_teid;
    ogs_paa_t paa;
    uint8_t serving_network_raw[3];
    uint8_t uli_raw[1 + 5];

    ogs_gtp2_bearer_qos_t bqos;
    ogs_gtp2_f_teid_t sgw_s5u_teid;

    smf_sess_t *sess = NULL;
    uint8_t cause;

    ogs_pkbuf_t *pkbuf = NULL;

    ogs_gtp_xact_t mock_xact;
    memset(&mock_xact, 0, sizeof(mock_xact));

    ogs_info("--- test_dnn_override: request=[%s] canonical=[%s] ---",
             request_dnn, canonical_dnn);

    /* 1. Build mock Create Session Request */
    memset(&message, 0, sizeof(message));
    req = &message.create_session_request;

    req->imsi.presence = 1;
    req->imsi.len = OGS_MAX_IMSI_LEN;
    req->imsi.data = imsi;

    req->rat_type.presence = 1;
    req->rat_type.u8 = OGS_GTP2_RAT_TYPE_EUTRAN;

    req->access_point_name.presence = 1;
    req->access_point_name.len = ogs_fqdn_build(
        apn_buf, request_dnn, strlen(request_dnn));
    req->access_point_name.data = (uint8_t *)apn_buf;

    /* F-TEID for Control Plane */
    memset(&sgw_s5c_teid, 0, sizeof(sgw_s5c_teid));
    sgw_s5c_teid.ipv4 = 1;
    sgw_s5c_teid.interface_type = 6; /* OGS_GTP2_IF_TYPE_S5_S8_SGW_GTP_C */
    sgw_s5c_teid.teid = htobe32(0xdeadbeef);
    sgw_s5c_teid.addr = htonl(0x7f000001);
    req->sender_f_teid_for_control_plane.presence = 1;
    req->sender_f_teid_for_control_plane.data = &sgw_s5c_teid;

    /* PAA */
    memset(&paa, 0, sizeof(paa));
    paa.session_type = OGS_PDU_SESSION_TYPE_IPV4;
    paa.addr = htonl(0x0a2d0002);
    req->pdn_address_allocation.presence = 1;
    req->pdn_address_allocation.data = &paa;

    /* Serving Network */
    serving_network_raw[0] = 0x00; /* MCC 001 */
    serving_network_raw[1] = 0xf1;
    serving_network_raw[2] = 0x10; /* MNC 001 */
    req->serving_network.presence = 1;
    req->serving_network.data = serving_network_raw;
    req->serving_network.len = 3;

    /* ULI */
    uli_raw[0] = 0x08; /* TAI present (bit 3 in flags byte) */
    uli_raw[1] = 0x00; /* MCC 001 */
    uli_raw[2] = 0xf1;
    uli_raw[3] = 0x10; /* MNC 001 */
    uli_raw[4] = 0x00; /* TAC */
    uli_raw[5] = 0x01;
    req->user_location_information.presence = 1;
    req->user_location_information.data = uli_raw;
    req->user_location_information.len = sizeof(uli_raw);

    /* Bearer Context */
    req->bearer_contexts_to_be_created[0].presence = 1;

    /* EBI */
    req->bearer_contexts_to_be_created[0].eps_bearer_id.presence = 1;
    req->bearer_contexts_to_be_created[0].eps_bearer_id.u8 = 5;

    /* Bearer QoS */
    memset(&bqos, 0, sizeof(bqos));
    bqos.priority_level = 1;
    bqos.qci = 9;
    req->bearer_contexts_to_be_created[0].bearer_level_qos.presence = 1;
    req->bearer_contexts_to_be_created[0].bearer_level_qos.data = &bqos;

    /* F-TEID for S5/S8-U */
    memset(&sgw_s5u_teid, 0, sizeof(sgw_s5u_teid));
    sgw_s5u_teid.ipv4 = 1;
    sgw_s5u_teid.interface_type = 8; /* OGS_GTP2_IF_TYPE_S5_S8_SGW_GTP_U */
    sgw_s5u_teid.teid = htobe32(0xcafebabe);
    sgw_s5u_teid.addr = htonl(0x7f000001);
    req->bearer_contexts_to_be_created[0].s5_s8_u_sgw_f_teid.presence = 1;
    req->bearer_contexts_to_be_created[0].s5_s8_u_sgw_f_teid.data = &sgw_s5u_teid;

    /* 2. Create a session context from the request */
    sess = smf_sess_add_by_gtp2_message(&message);
    ogs_assert(sess);
    /* Replicate what smf-sm.c does before calling the handler: populate the
     * SGW-S5C TEID from the sender F-TEID in the request so the handler's
     * TEID consistency check (s5c-handler.c) does not reject the request. */
    sess->sgw_s5c_teid = be32toh(sgw_s5c_teid.teid);
    ogs_info("Session created with DNN: %s", sess->session.name);
    ogs_assert(strcmp(sess->session.name, request_dnn) == 0);

    /* 3. Handle the request, which triggers DNN override.
     * In a unit-test environment there is no connected Gx Diameter peer, so
     * the handler will return REMOTE_PEER_NOT_RESPONDING after DNN override
     * has already been applied.  The DNN override itself (steps 4-5) is
     * what this test exercises. */
    cause = smf_s5c_handle_create_session_request(sess, &mock_xact, req);
    ogs_info("Handler returned cause: %u", cause);

    /* 4. Verify the session's DNN has been updated to the canonical one */
    ogs_info("Verifying session DNN was overridden with canonical DNN");
    ogs_assert(strcmp(sess->session.name, canonical_dnn) == 0);
    ogs_assert(sess->pfcp_subnet);
    ogs_assert(strcmp(sess->pfcp_subnet->dnn, canonical_dnn) == 0);

    /* 5. Build the response and verify it can be constructed successfully.
     * smf_s5c_build_create_session_response uses sess->session.name
     * to encode the APN IE, so a non-NULL
     * return confirms the response carries the canonical DNN. */
    pkbuf = smf_s5c_build_create_session_response(
        OGS_GTP2_CREATE_SESSION_RESPONSE_TYPE, sess);
    ogs_assert(pkbuf);

    ogs_info("DNN override test passed!");

    /* Cleanup: remove the UE (which removes its sessions) so the UE list is
     * empty before smf_context_final() tries to iterate it during teardown. */
    ogs_pkbuf_free(pkbuf);
    smf_ue_remove(smf_ue_find_by_id(sess->smf_ue_id));
}

int main(int argc, char *argv[])
{
    yaml_parser_t parser;
    yaml_document_t *doc = NULL;
    char test_config[4096];

    /* DNN values under test */
    const char *canonical_dnn = "internet";
    const char *request_dnn   = "internet.mnc001.mcc001.gprs";

    ogs_core_initialize();

    /* Initialize core app context */
    ogs_app_context_init();
    ogs_app_config_init();
    ogs_app_global_conf_prepare();

    /* Build the YAML config using the chosen DNN values */
    build_test_config(test_config, sizeof(test_config),
                      canonical_dnn, request_dnn);

    /* Manually parse the YAML string into a document */
    doc = malloc(sizeof(yaml_document_t));
    ogs_assert(doc);

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser,
        (const unsigned char*)test_config, strlen(test_config));
    ogs_assert(yaml_parser_load(&parser, doc));
    yaml_parser_delete(&parser);

    /* Set the parsed document for the app to use */
    ogs_app()->document = doc;

    /* Manually parse the global and logger sections from the document */
    ogs_yaml_iter_t root_iter;
    ogs_yaml_iter_init(&root_iter, doc);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *key = ogs_yaml_iter_key(&root_iter);
        if (strcmp(key, "global") == 0) {
            ogs_app_parse_global_conf(&root_iter);
        } else if (strcmp(key, "logger") == 0) {
            ogs_yaml_iter_t logger_iter;
            ogs_yaml_iter_recurse(&root_iter, &logger_iter);
            while (ogs_yaml_iter_next(&logger_iter)) {
                ogs_app()->logger.level = ogs_yaml_iter_value(&logger_iter);
            }
        }
    }

    ogs_app_parse_local_conf("smf");

    /* Initialize event queue, timer, and pollset */
    ogs_app()->queue = ogs_queue_create(ogs_app()->pool.event);
    ogs_assert(ogs_app()->queue);
    ogs_app()->timer_mgr = ogs_timer_mgr_create(ogs_app()->pool.timer);
    ogs_assert(ogs_app()->timer_mgr);
    ogs_app()->pollset = ogs_pollset_create(ogs_app()->pool.socket);
    ogs_assert(ogs_app()->pollset);

    test_setup();
    test_dnn_override(request_dnn, canonical_dnn);
    test_teardown();

    ogs_app_terminate();

    return 0;
}
