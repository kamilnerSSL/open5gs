/*
 * Copyright (C) 2026 by Open5GS Contributors
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

/*
 * Tests for 3GPP PS Data Off PCO container handling.
 *
 * Covers the PCO wire-format layer (ogs_pco_build / ogs_pco_parse) that
 * underpins the SMF's smf_pco_build() handling of OGS_PCO_ID_3GPP_PS_DATA_OFF
 * (PCO container ID 0x0017, TS 24.008 §10.5.6.3).
 *
 * Wire format of a single PS Data Off container inside a PCO IE:
 *
 *   Byte 0        : 0x80  — ext=1, spare=0000, config_protocol=000
 *   Bytes 1–2     : 0x00 0x17  — container ID (big-endian)
 *   Byte 3        : 0x01  — container length (1 octet of content)
 *   Byte 4        : status — bit0: 1 = PS Data Off activated, 0 = deactivated
 */

#include "ogs-gtp.h"
#include "core/abts.h"

/* PCO wire bytes for PS Data Off activated (status = 0x01) */
static const uint8_t pco_ps_data_off_on[]  = { 0x80, 0x00, 0x17, 0x01, 0x01 };
/* PCO wire bytes for PS Data Off deactivated (status = 0x00) */
static const uint8_t pco_ps_data_off_off[] = { 0x80, 0x00, 0x17, 0x01, 0x00 };

/*
 * Test 1: Parse a raw PCO buffer containing only the PS Data Off container
 * with the activated status (bit 0 = 1).
 */
static void pco_ps_data_off_parse_activated(abts_case *tc, void *data)
{
    ogs_pco_t pco;
    int size;

    size = ogs_pco_parse(&pco, (unsigned char *)pco_ps_data_off_on,
                         sizeof(pco_ps_data_off_on));

    ABTS_INT_EQUAL(tc, (int)sizeof(pco_ps_data_off_on), size);
    ABTS_INT_EQUAL(tc, 1, pco.num_of_id);
    ABTS_INT_EQUAL(tc, OGS_PCO_ID_3GPP_PS_DATA_OFF, pco.ids[0].id);
    ABTS_INT_EQUAL(tc, 1, pco.ids[0].len);
    ABTS_PTR_NOTNULL(tc, pco.ids[0].data);
    /* bit 0 set → PS Data Off is active */
    ABTS_TRUE(tc, (((uint8_t *)pco.ids[0].data)[0] & 0x01) == 0x01);
}

/*
 * Test 2: Parse a raw PCO buffer containing only the PS Data Off container
 * with the deactivated status (bit 0 = 0).
 */
static void pco_ps_data_off_parse_deactivated(abts_case *tc, void *data)
{
    ogs_pco_t pco;
    int size;

    size = ogs_pco_parse(&pco, (unsigned char *)pco_ps_data_off_off,
                         sizeof(pco_ps_data_off_off));

    ABTS_INT_EQUAL(tc, (int)sizeof(pco_ps_data_off_off), size);
    ABTS_INT_EQUAL(tc, 1, pco.num_of_id);
    ABTS_INT_EQUAL(tc, OGS_PCO_ID_3GPP_PS_DATA_OFF, pco.ids[0].id);
    ABTS_INT_EQUAL(tc, 1, pco.ids[0].len);
    ABTS_PTR_NOTNULL(tc, pco.ids[0].data);
    /* bit 0 clear → PS Data Off is not active */
    ABTS_TRUE(tc, (((uint8_t *)pco.ids[0].data)[0] & 0x01) == 0x00);
}

/*
 * Test 3: Build a PCO containing PS Data Off (activated), verify the produced
 * wire bytes match the expected format exactly.
 */
static void pco_ps_data_off_build_activated(abts_case *tc, void *data)
{
    ogs_pco_t pco;
    uint8_t buf[OGS_MAX_PCO_LEN];
    uint8_t status = 0x01; /* PS Data Off activated */
    int size;

    memset(&pco, 0, sizeof(pco));
    pco.ext = 1;
    pco.configuration_protocol =
        OGS_PCO_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE;
    pco.num_of_id = 1;
    pco.ids[0].id  = OGS_PCO_ID_3GPP_PS_DATA_OFF;
    pco.ids[0].len  = 1;
    pco.ids[0].data = &status;

    size = ogs_pco_build(buf, OGS_MAX_PCO_LEN, &pco);

    ABTS_INT_EQUAL(tc, (int)sizeof(pco_ps_data_off_on), size);
    ABTS_TRUE(tc, memcmp(buf, pco_ps_data_off_on, size) == 0);
}

/*
 * Test 4: Build a PCO containing PS Data Off (deactivated), verify wire bytes.
 */
static void pco_ps_data_off_build_deactivated(abts_case *tc, void *data)
{
    ogs_pco_t pco;
    uint8_t buf[OGS_MAX_PCO_LEN];
    uint8_t status = 0x00; /* PS Data Off deactivated */
    int size;

    memset(&pco, 0, sizeof(pco));
    pco.ext = 1;
    pco.configuration_protocol =
        OGS_PCO_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE;
    pco.num_of_id = 1;
    pco.ids[0].id  = OGS_PCO_ID_3GPP_PS_DATA_OFF;
    pco.ids[0].len  = 1;
    pco.ids[0].data = &status;

    size = ogs_pco_build(buf, OGS_MAX_PCO_LEN, &pco);

    ABTS_INT_EQUAL(tc, (int)sizeof(pco_ps_data_off_off), size);
    ABTS_TRUE(tc, memcmp(buf, pco_ps_data_off_off, size) == 0);
}

/*
 * Test 5: Round-trip — build then parse PS Data Off (activated).
 * Simulates the UE building its PCO request and the SMF parsing it.
 */
static void pco_ps_data_off_roundtrip_activated(abts_case *tc, void *data)
{
    ogs_pco_t tx, rx;
    uint8_t buf[OGS_MAX_PCO_LEN];
    uint8_t status = 0x01;
    int built, parsed;

    memset(&tx, 0, sizeof(tx));
    tx.ext = 1;
    tx.configuration_protocol =
        OGS_PCO_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE;
    tx.num_of_id = 1;
    tx.ids[0].id  = OGS_PCO_ID_3GPP_PS_DATA_OFF;
    tx.ids[0].len  = 1;
    tx.ids[0].data = &status;

    built = ogs_pco_build(buf, OGS_MAX_PCO_LEN, &tx);
    ABTS_TRUE(tc, built > 0);

    parsed = ogs_pco_parse(&rx, buf, built);
    ABTS_INT_EQUAL(tc, built, parsed);
    ABTS_INT_EQUAL(tc, 1, rx.num_of_id);
    ABTS_INT_EQUAL(tc, OGS_PCO_ID_3GPP_PS_DATA_OFF, rx.ids[0].id);
    ABTS_INT_EQUAL(tc, 1, rx.ids[0].len);
    ABTS_PTR_NOTNULL(tc, rx.ids[0].data);
    ABTS_TRUE(tc, (((uint8_t *)rx.ids[0].data)[0] & 0x01) == 0x01);
}

/*
 * Test 6: Round-trip — build then parse PS Data Off (deactivated).
 */
static void pco_ps_data_off_roundtrip_deactivated(abts_case *tc, void *data)
{
    ogs_pco_t tx, rx;
    uint8_t buf[OGS_MAX_PCO_LEN];
    uint8_t status = 0x00;
    int built, parsed;

    memset(&tx, 0, sizeof(tx));
    tx.ext = 1;
    tx.configuration_protocol =
        OGS_PCO_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE;
    tx.num_of_id = 1;
    tx.ids[0].id  = OGS_PCO_ID_3GPP_PS_DATA_OFF;
    tx.ids[0].len  = 1;
    tx.ids[0].data = &status;

    built = ogs_pco_build(buf, OGS_MAX_PCO_LEN, &tx);
    ABTS_TRUE(tc, built > 0);

    parsed = ogs_pco_parse(&rx, buf, built);
    ABTS_INT_EQUAL(tc, built, parsed);
    ABTS_INT_EQUAL(tc, 1, rx.num_of_id);
    ABTS_INT_EQUAL(tc, OGS_PCO_ID_3GPP_PS_DATA_OFF, rx.ids[0].id);
    ABTS_INT_EQUAL(tc, 1, rx.ids[0].len);
    ABTS_PTR_NOTNULL(tc, rx.ids[0].data);
    ABTS_TRUE(tc, (((uint8_t *)rx.ids[0].data)[0] & 0x01) == 0x00);
}

/*
 * Test 7: PCO with PS Data Off alongside other containers.
 * Simulates a realistic UE PCO that also requests IPv4 DNS and the
 * PS Data Off feature, and verifies that parsing correctly identifies
 * the PS Data Off container within a multi-container PCO.
 */
static void pco_ps_data_off_mixed_containers(abts_case *tc, void *data)
{
    ogs_pco_t tx, rx;
    uint8_t buf[OGS_MAX_PCO_LEN];
    uint8_t ps_status = 0x01;
    int built, parsed;
    int i, found_ps_data_off = 0;

    memset(&tx, 0, sizeof(tx));
    tx.ext = 1;
    tx.configuration_protocol =
        OGS_PCO_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE;
    tx.num_of_id = 2;

    /* Container 0: IPv4 DNS server request (no data) */
    tx.ids[0].id  = OGS_PCO_ID_DNS_SERVER_IPV4_ADDRESS_REQUEST;
    tx.ids[0].len  = 0;
    tx.ids[0].data = NULL;

    /* Container 1: PS Data Off, activated */
    tx.ids[1].id  = OGS_PCO_ID_3GPP_PS_DATA_OFF;
    tx.ids[1].len  = 1;
    tx.ids[1].data = &ps_status;

    built = ogs_pco_build(buf, OGS_MAX_PCO_LEN, &tx);
    ABTS_TRUE(tc, built > 0);

    parsed = ogs_pco_parse(&rx, buf, built);
    ABTS_INT_EQUAL(tc, built, parsed);
    ABTS_INT_EQUAL(tc, 2, rx.num_of_id);

    for (i = 0; i < rx.num_of_id; i++) {
        if (rx.ids[i].id == OGS_PCO_ID_3GPP_PS_DATA_OFF) {
            found_ps_data_off = 1;
            ABTS_INT_EQUAL(tc, 1, rx.ids[i].len);
            ABTS_PTR_NOTNULL(tc, rx.ids[i].data);
            ABTS_TRUE(tc,
                (((uint8_t *)rx.ids[i].data)[0] & 0x01) == 0x01);
        }
    }

    ABTS_TRUE(tc, found_ps_data_off);
}

/*
 * Test 8: smf_pco_build echoes PS Data Off back.
 *
 * smf_pco_build() takes the raw UE PCO buffer and builds the network's
 * response PCO.  For PS Data Off, the network acknowledges support by
 * echoing the container with the same status byte.  Verify that the
 * response PCO contains the PS Data Off container with identical data.
 *
 * This test operates purely on ogs_pco_t structures so it does not
 * require the full SMF subsystem to be initialised.
 */
static void pco_ps_data_off_smf_response_format(abts_case *tc, void *data)
{
    /*
     * Simulate what smf_pco_build produces for the response:
     *   - Parse the UE's raw PCO
     *   - Build the SMF's response with the same PS Data Off container
     * The actual smf_pco_build function follows exactly this pattern for
     * the OGS_PCO_ID_3GPP_PS_DATA_OFF case.
     */
    ogs_pco_t ue_req, smf_resp;
    uint8_t ue_buf[OGS_MAX_PCO_LEN];
    uint8_t resp_buf[OGS_MAX_PCO_LEN];
    ogs_pco_t parsed_resp;
    int ue_size, resp_size, parsed_size;

    /* 1. Parse the UE request */
    ue_size = ogs_pco_parse(&ue_req,
                            (unsigned char *)pco_ps_data_off_on,
                            sizeof(pco_ps_data_off_on));
    ABTS_INT_EQUAL(tc, (int)sizeof(pco_ps_data_off_on), ue_size);
    ABTS_INT_EQUAL(tc, 1, ue_req.num_of_id);
    ABTS_INT_EQUAL(tc, OGS_PCO_ID_3GPP_PS_DATA_OFF, ue_req.ids[0].id);

    /* 2. Build the SMF response — echo the container back (same id/len/data) */
    memset(&smf_resp, 0, sizeof(smf_resp));
    smf_resp.ext = 1;
    smf_resp.configuration_protocol =
        OGS_PCO_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE;
    smf_resp.num_of_id = 1;
    smf_resp.ids[0].id   = ue_req.ids[0].id;
    smf_resp.ids[0].len  = ue_req.ids[0].len;
    smf_resp.ids[0].data = ue_req.ids[0].data;

    resp_size = ogs_pco_build(resp_buf, OGS_MAX_PCO_LEN, &smf_resp);
    ABTS_TRUE(tc, resp_size > 0);

    /* 3. The response wire bytes must match the original UE request bytes */
    ABTS_INT_EQUAL(tc, (int)sizeof(pco_ps_data_off_on), resp_size);
    ABTS_TRUE(tc, memcmp(resp_buf, pco_ps_data_off_on, resp_size) == 0);

    /* 4. Parse the response and verify the PS Data Off status is preserved */
    parsed_size = ogs_pco_parse(&parsed_resp, resp_buf, resp_size);
    ABTS_INT_EQUAL(tc, resp_size, parsed_size);
    ABTS_INT_EQUAL(tc, 1, parsed_resp.num_of_id);
    ABTS_INT_EQUAL(tc, OGS_PCO_ID_3GPP_PS_DATA_OFF, parsed_resp.ids[0].id);
    ABTS_INT_EQUAL(tc, 1, parsed_resp.ids[0].len);
    ABTS_TRUE(tc, (((uint8_t *)parsed_resp.ids[0].data)[0] & 0x01) == 0x01);
}

abts_suite *test_pco(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, pco_ps_data_off_parse_activated, NULL);
    abts_run_test(suite, pco_ps_data_off_parse_deactivated, NULL);
    abts_run_test(suite, pco_ps_data_off_build_activated, NULL);
    abts_run_test(suite, pco_ps_data_off_build_deactivated, NULL);
    abts_run_test(suite, pco_ps_data_off_roundtrip_activated, NULL);
    abts_run_test(suite, pco_ps_data_off_roundtrip_deactivated, NULL);
    abts_run_test(suite, pco_ps_data_off_mixed_containers, NULL);
    abts_run_test(suite, pco_ps_data_off_smf_response_format, NULL);

    return suite;
}
