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

#include "ogs-app.h"
#include "smf/context.h"
#include "smf/s5c-handler.h"
#include "smf/s5c-build.h"

#include "lib/gtp/v2/message.h"

static const char *test_config =
"logger:\n"
"    level: info\n"
"smf:\n"
"  sbi:\n"
"    server:\n"
"      - address: 127.0.0.4\n"
"        port: 7777\n"
"    client:\n"
"      nrf:\n"
"        - address: 127.0.0.1\n"
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
"    - dnn: internet\n"
"      subnet: 10.45.0.0/16\n"
"      accept:\n"
"        - internet.mnc001.mcc001.gprs\n"
"  dns:\n"
"    - 8.8.8.8\n";

void test_setup(void)
{
    int rv;

    rv = ogs_app_init(NULL, "test.yml");
    ogs_assert(rv == OGS_OK);

    rv = ogs_app_load_yaml_from_string(test_config);
    ogs_assert(rv == OGS_OK);

    ogs_log_init();

    ogs_gtp_context_init(ogs_app()->pool.nf);
    ogs_pfcp_context_init();
    ogs_sbi_context_init(OpenAPI_nf_type_SMF);
    smf_context_init();

    rv = ogs_gtp_xact_init();
    ogs_assert(rv == OGS_OK);
    rv = ogs_pfcp_xact_init();
    ogs_assert(rv == OGS_OK);

    rv = ogs_gtp_context_parse_config("smf", "upf");
    ogs_assert(rv == OGS_OK);
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
    smf_context_final();
    ogs_sbi_context_final();
    ogs_pfcp_context_final();
    ogs_gtp_context_final();

    ogs_pfcp_xact_final();
    ogs_gtp_xact_final();

    ogs_app_final();
}

void test_dnn_replacement(void)
{
    ogs_gtp2_message_t message;
    ogs_gtp2_create_session_request_t *req = NULL;
    char apn_buf[OGS_MAX_APN_LEN + 1];
    uint8_t imsi[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };

    smf_sess_t *sess = NULL;
    uint8_t cause;

    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp2_message_t rsp_msg;
    ogs_gtp2_create_session_response_t *rsp = NULL;
    char apn_out[OGS_MAX_APN_LEN + 1];

    ogs_gtp_xact_t mock_xact;
    memset(&mock_xact, 0, sizeof(mock_xact));

    ogs_info("--- test_dnn_replacement ---");

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
        apn_buf, "internet.mnc001.mcc001.gprs",
        strlen("internet.mnc001.mcc001.gprs"));
    req->access_point_name.data = apn_buf;

    /* 2. Create a session context from the request */
    sess = smf_sess_add_by_gtp2_message(&message);
    ogs_assert(sess);
    ogs_assert(strcmp(sess->session.name, "internet.mnc001.mcc001.gprs") == 0);

    /* 3. Handle the request, which should trigger the DNN replacement */
    cause = smf_s5c_handle_create_session_request(sess, &mock_xact, req);
    ogs_assert(cause == OGS_GTP2_CAUSE_REQUEST_ACCEPTED);

    /* 4. Verify the session's DNN has been updated to the canonical one */
    ogs_info("Verifying session DNN was replaced with canonical DNN");
    ogs_assert(strcmp(sess->session.name, "internet") == 0);
    ogs_assert(sess->pfcp_subnet);
    ogs_assert(strcmp(sess->pfcp_subnet->dnn, "internet") == 0);

    /* 5. Build the response and verify it contains the canonical DNN */
    pkbuf = smf_s5c_build_create_session_response(
        OGS_GTP2_CREATE_SESSION_RESPONSE_TYPE, sess);
    ogs_assert(pkbuf);

    ogs_gtp2_parse_msg(&rsp_msg, pkbuf);
    rsp = &rsp_msg.create_session_response;

    ogs_info("Verifying response contains canonical DNN");
    ogs_assert(rsp->apn.presence == 1);
    memset(apn_out, 0, sizeof(apn_out));
    ogs_fqdn_parse(apn_out, (const char*)rsp->apn.data, rsp->apn.len);
    ogs_assert(strcmp(apn_out, "internet") == 0);

    ogs_info("DNN replacement test passed!");

    /* Cleanup */
    ogs_pkbuf_free(pkbuf);
    smf_sess_remove(sess);
}

int main(int argc, char *argv[])
{
    test_setup();
    test_dnn_replacement();
    test_teardown();

    return 0;
}