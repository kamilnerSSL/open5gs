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
 * Regression tests for the stale FAR hash preservation fix.
 *
 * Context: During an SGW handover the UPF calls
 * ogs_pfcp_far_f_teid_hash_set() to update the FAR's (TEID, peer-IP) →
 * FAR hash entry.  Before the fix, the old entry was removed immediately;
 * any GTP-U Error Indication arriving from the now-superseded SGW after
 * that point failed the hash lookup and was silently dropped.  A second
 * code path then tore down the entire session ~1-9 s later, producing a
 * runaway re-establishment loop.
 *
 * The fix:
 *   - prev_f_teid is added to ogs_pfcp_far_t.
 *   - ogs_pfcp_far_f_teid_hash_set() promotes the current entry to
 *     prev_f_teid (keeping it alive in the hash table) instead of
 *     removing it, and evicts the two-cycles-ago entry.
 *   - ogs_pfcp_far_find_by_gtpu_error_indication() checks whether the
 *     hash hit was against the current key or the stale prev entry.
 *     A stale hit returns NULL so the SMF is not incorrectly triggered.
 *   - ogs_pfcp_far_remove() also cleans up prev_f_teid.
 *
 * Tests:
 *   1. far_hash_install       – initial install; current lookup succeeds.
 *   2. far_hash_update_stale  – after one update (SGW1→SGW2): SGW1 Error
 *                               Indication is discarded (stale), SGW2 is
 *                               returned correctly.
 *   3. far_hash_two_updates   – after two updates (SGW1→SGW2→SGW3): SGW1
 *                               entry is completely evicted, SGW2 is stale
 *                               (discarded), SGW3 is returned.
 *   4. far_hash_remove        – after ogs_pfcp_far_remove(), both the
 *                               current and the prev entries are gone.
 */

#include "ogs-pfcp.h"
#include "core/abts.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/*
 * Build a minimal Error Indication payload (the portion *after* the GTP-U
 * header has been stripped by the caller).
 *
 *  Byte  0   : 16    – IE type: TEID Data I
 *  Bytes 1-4 : TEID  – big-endian
 *  Byte  5   : 133   – IE type: GTP-U Peer Address
 *  Bytes 6-7 : 0x00 0x04 – length = 4 (IPv4)
 *  Bytes 8-11: IPv4 address in network byte order
 *
 * The pkbuf is owned by the caller; free with ogs_pkbuf_free().
 */
static ogs_pkbuf_t *build_err_ind_ipv4(uint32_t teid_host, uint32_t ip_nbo)
{
    ogs_pkbuf_t *pkbuf;
    uint8_t *p;
    uint32_t teid_be = htobe32(teid_host);
    uint16_t len_be  = htobe16(OGS_IPV4_LEN);

    pkbuf = ogs_pkbuf_alloc(NULL, 12);
    ogs_assert(pkbuf);
    ogs_pkbuf_put(pkbuf, 12);
    p = pkbuf->data;

    p[0] = 16;                                  /* TEID Data I */
    memcpy(p + 1, &teid_be, 4);                 /* TEID (BE) */
    p[5] = 133;                                 /* GTP-U Peer Address */
    memcpy(p + 6, &len_be, 2);                  /* length (BE) */
    memcpy(p + 8, &ip_nbo, OGS_IPV4_LEN);      /* IPv4 addr (NBO) */

    return pkbuf;
}

/*
 * Configure a stack-allocated ogs_gtp_node_t so that
 * ogs_pfcp_far_f_teid_hash_set() can read the peer IP from it.
 * Only the addr.sin fields need to be populated.
 */
static void setup_gnode_ipv4(ogs_gtp_node_t *gnode, uint32_t ip_nbo)
{
    memset(gnode, 0, sizeof *gnode);
    gnode->addr.ogs_sa_family = AF_INET;
    gnode->addr.sin.sin_family = AF_INET;
    gnode->addr.sin.sin_addr.s_addr = ip_nbo;
}

/* ------------------------------------------------------------------ */
/* Convenience: install FAR with given TEID and peer-IP                */
/* ------------------------------------------------------------------ */
static void far_install(ogs_pfcp_far_t *far,
                        ogs_gtp_node_t *gnode,
                        uint32_t teid_host,
                        uint32_t ip_nbo)
{
    setup_gnode_ipv4(gnode, ip_nbo);
    far->outer_header_creation.teid = teid_host;
    far->gnode = gnode;
    ogs_pfcp_far_f_teid_hash_set(far);
}

/* ------------------------------------------------------------------ */
/* Test cases                                                           */
/* ------------------------------------------------------------------ */

/*
 * Test 1 – Initial install.
 *
 * After the first call to ogs_pfcp_far_f_teid_hash_set(), a lookup with
 * the correct (TEID, peer-IP) returns the FAR; prev_f_teid is empty.
 */
static void far_hash_install(abts_case *tc, void *data)
{
    ogs_pfcp_sess_t sess;
    ogs_pfcp_far_t *far;
    ogs_gtp_node_t  gnode;
    ogs_pkbuf_t    *pkbuf;
    ogs_pfcp_far_t *found;

    /* Addresses chosen to be unambiguous and easy to read in failure output */
    const uint32_t TEID = 0x00000001;
    const uint32_t IP   = inet_addr("10.0.0.1"); /* NBO */

    ogs_pfcp_pool_init(&sess);

    far = ogs_pfcp_far_add(&sess);
    ABTS_PTR_NOTNULL(tc, far);

    /* First install */
    far_install(far, &gnode, TEID, IP);

    /* prev_f_teid must be zero after the first install because there was
     * no previous entry to promote. */
    ABTS_INT_EQUAL(tc, 0, far->prev_f_teid.len);

    /* Lookup via Error Indication must succeed */
    pkbuf = build_err_ind_ipv4(TEID, IP);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);

    ABTS_PTR_EQUAL(tc, far, found);

    ogs_pfcp_far_remove(far);
    ogs_pfcp_pool_final(&sess);
}

/*
 * Test 2 – Stale entry after one update (SGW1 → SGW2).
 *
 * After calling ogs_pfcp_far_f_teid_hash_set() a second time with new
 * SGW2 parameters:
 *   - A lookup with SGW2's (TEID2, IP2) returns the FAR.
 *   - A lookup with the old SGW1's (TEID1, IP1) returns NULL (stale).
 */
static void far_hash_update_stale(abts_case *tc, void *data)
{
    ogs_pfcp_sess_t sess;
    ogs_pfcp_far_t *far;
    ogs_gtp_node_t  gnode;
    ogs_pkbuf_t    *pkbuf;
    ogs_pfcp_far_t *found;

    const uint32_t TEID1 = 0x00000010;
    const uint32_t IP1   = inet_addr("10.0.1.1");
    const uint32_t TEID2 = 0x00000020;
    const uint32_t IP2   = inet_addr("10.0.2.1");

    ogs_pfcp_pool_init(&sess);
    far = ogs_pfcp_far_add(&sess);
    ABTS_PTR_NOTNULL(tc, far);

    /* Install SGW1 path */
    far_install(far, &gnode, TEID1, IP1);

    /* Handover: update to SGW2 path */
    far_install(far, &gnode, TEID2, IP2);

    /* After one update, prev_f_teid must hold the old key */
    ABTS_TRUE(tc, far->prev_f_teid.len > 0);

    /* SGW2 (current path) lookup must succeed */
    pkbuf = build_err_ind_ipv4(TEID2, IP2);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, far, found);

    /* SGW1 (stale path) lookup must be discarded */
    pkbuf = build_err_ind_ipv4(TEID1, IP1);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, NULL, found);

    ogs_pfcp_far_remove(far);
    ogs_pfcp_pool_final(&sess);
}

/*
 * Test 3 – Two consecutive updates (SGW1 → SGW2 → SGW3).
 *
 * After the second update:
 *   - SGW3 (current) returns the FAR.
 *   - SGW2 (prev, stale) returns NULL.
 *   - SGW1 (evicted two cycles ago) is also not found (returns NULL).
 */
static void far_hash_two_updates(abts_case *tc, void *data)
{
    ogs_pfcp_sess_t sess;
    ogs_pfcp_far_t *far;
    ogs_gtp_node_t  gnode;
    ogs_pkbuf_t    *pkbuf;
    ogs_pfcp_far_t *found;

    const uint32_t TEID1 = 0x00000100;
    const uint32_t IP1   = inet_addr("10.1.0.1");
    const uint32_t TEID2 = 0x00000200;
    const uint32_t IP2   = inet_addr("10.2.0.1");
    const uint32_t TEID3 = 0x00000300;
    const uint32_t IP3   = inet_addr("10.3.0.1");

    ogs_pfcp_pool_init(&sess);
    far = ogs_pfcp_far_add(&sess);
    ABTS_PTR_NOTNULL(tc, far);

    far_install(far, &gnode, TEID1, IP1); /* cycle 1 */
    far_install(far, &gnode, TEID2, IP2); /* cycle 2 */
    far_install(far, &gnode, TEID3, IP3); /* cycle 3 */

    /* SGW3 (current) must return the FAR */
    pkbuf = build_err_ind_ipv4(TEID3, IP3);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, far, found);

    /* SGW2 (prev / stale) must be discarded */
    pkbuf = build_err_ind_ipv4(TEID2, IP2);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, NULL, found);

    /* SGW1 (two cycles ago — evicted) must not be found */
    pkbuf = build_err_ind_ipv4(TEID1, IP1);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, NULL, found);

    ogs_pfcp_far_remove(far);
    ogs_pfcp_pool_final(&sess);
}

/*
 * Test 4 – ogs_pfcp_far_remove() cleans up both hash entries.
 *
 * After installing (TEID1→TEID2 update) and then removing the FAR, both
 * the current and the prev entries must be gone from the hash table.
 */
static void far_hash_remove(abts_case *tc, void *data)
{
    ogs_pfcp_sess_t sess;
    ogs_pfcp_far_t *far;
    ogs_gtp_node_t  gnode;
    ogs_pkbuf_t    *pkbuf;
    ogs_pfcp_far_t *found;

    const uint32_t TEID1 = 0x00001000;
    const uint32_t IP1   = inet_addr("192.168.1.1");
    const uint32_t TEID2 = 0x00002000;
    const uint32_t IP2   = inet_addr("192.168.2.1");

    ogs_pfcp_pool_init(&sess);
    far = ogs_pfcp_far_add(&sess);
    ABTS_PTR_NOTNULL(tc, far);

    far_install(far, &gnode, TEID1, IP1);
    far_install(far, &gnode, TEID2, IP2);

    /* Sanity: current entry present before remove */
    pkbuf = build_err_ind_ipv4(TEID2, IP2);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, far, found);

    /* Remove the FAR — must purge both current and prev from hash */
    ogs_pfcp_far_remove(far);

    /* Current entry must be gone */
    pkbuf = build_err_ind_ipv4(TEID2, IP2);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, NULL, found);

    /* Prev (SGW1) entry must also be gone */
    pkbuf = build_err_ind_ipv4(TEID1, IP1);
    found = ogs_pfcp_far_find_by_gtpu_error_indication(pkbuf);
    ogs_pkbuf_free(pkbuf);
    ABTS_PTR_EQUAL(tc, NULL, found);

    ogs_pfcp_pool_final(&sess);
}

/* ------------------------------------------------------------------ */
/* Suite entry point                                                    */
/* ------------------------------------------------------------------ */

abts_suite *test_pfcp_far(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, far_hash_install,      NULL);
    abts_run_test(suite, far_hash_update_stale, NULL);
    abts_run_test(suite, far_hash_two_updates,  NULL);
    abts_run_test(suite, far_hash_remove,       NULL);

    return suite;
}
