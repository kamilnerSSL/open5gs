/*
 * Copyright (C) 2019-2026 by Sukchan Lee <acetcom@gmail.com>
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
 * SGW-C Gz offline charging — Phase 1: JSON CDR file output.
 *
 * Each completed session produces one JSON record (a single line) written to
 * the configured CDR file.  The record layout mirrors the 3GPP TS 32.298
 * S-CDR field set so that a future phase can replace the JSON serialisation
 * with ASN.1 BER encoding and GTP' transport to a CGF without restructuring
 * the data-capture layer.
 *
 * Configuration (sgwc.yaml):
 *
 *   sgwc:
 *     gz:
 *       cdr_file: /var/log/open5gs/sgwc-cdrs.jsonl
 *
 * If cdr_file is absent the default path below is used.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "cdr.h"

#define SGWC_CDR_DEFAULT_PATH   "/var/log/open5gs/sgwc-cdrs.jsonl"

static FILE *cdr_fp = NULL;

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void timestamp_str(ogs_time_t t, char *buf, size_t buflen)
{
    struct tm gmt;
    ogs_gmtime((time_t)ogs_time_sec(t), &gmt);
    ogs_strftime(buf, buflen, "%Y-%m-%dT%H:%M:%SZ", &gmt);
}

static const char *pdn_type_str(uint8_t session_type)
{
    switch (session_type) {
    case OGS_PDU_SESSION_TYPE_IPV4:    return "IPv4";
    case OGS_PDU_SESSION_TYPE_IPV6:    return "IPv6";
    case OGS_PDU_SESSION_TYPE_IPV4V6:  return "IPv4v6";
    default:                           return "unknown";
    }
}

/*
 * Build a string representation of the UE IP address from the PAA.
 * For dual-stack sessions the IPv4 address is written first, then IPv6.
 * buf must be at least OGS_ADDRSTRLEN bytes.
 */
static void ue_ip_str(const ogs_paa_t *paa, char *buf, size_t buflen)
{
    char v6[INET6_ADDRSTRLEN];

    switch (paa->session_type) {
    case OGS_PDU_SESSION_TYPE_IPV4:
        inet_ntop(AF_INET, &paa->addr, buf, buflen);
        break;
    case OGS_PDU_SESSION_TYPE_IPV6:
        inet_ntop(AF_INET6, paa->addr6, buf, buflen);
        break;
    case OGS_PDU_SESSION_TYPE_IPV4V6:
        inet_ntop(AF_INET6, paa->both.addr6, v6, sizeof(v6));
        snprintf(buf, buflen, "%s", v6);
        /* append IPv4 separated by a comma */
        {
            char v4[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &paa->both.addr, v4, sizeof(v4));
            strncat(buf, ",", buflen - strlen(buf) - 1);
            strncat(buf, v4,  buflen - strlen(buf) - 1);
        }
        break;
    default:
        snprintf(buf, buflen, "unknown");
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void sgwc_cdr_init(void)
{
    const char *path = sgwc_self()->cdr_file ?
                       sgwc_self()->cdr_file : SGWC_CDR_DEFAULT_PATH;

    cdr_fp = fopen(path, "a");
    if (!cdr_fp)
        ogs_error("Gz: failed to open CDR file '%s': %s",
                  path, strerror(errno));
    else
        ogs_info("Gz: CDR output → %s", path);
}

void sgwc_cdr_final(void)
{
    if (cdr_fp) {
        fflush(cdr_fp);
        fclose(cdr_fp);
        cdr_fp = NULL;
    }
}

void sgwc_cdr_open(sgwc_sess_t *sess)
{
    ogs_assert(sess);

    sess->record_opening_time   = ogs_time_now();
    sess->local_sequence_number = ++sgwc_self()->cdr_sequence;
}

void sgwc_cdr_close(sgwc_sess_t *sess, uint8_t cause)
{
    sgwc_ue_t    *sgwc_ue = NULL;
    ogs_time_t    close_time;
    uint32_t      duration_sec;

    char open_ts[32], close_ts[32];
    char ue_ip[OGS_ADDRSTRLEN * 2 + 2];   /* room for dual-stack */
    char sgw_addr[OGS_ADDRSTRLEN];
    char pgw_addr[OGS_ADDRSTRLEN];
    char chargchars[OGS_CHRGCHARS_LEN * 2 + 1];
    char mnc_str[6];    /* uint16_t decimal (max 5 digits) + NUL */
    int  mnc_len;

    ogs_assert(sess);

    if (!cdr_fp)
        return;

    sgwc_ue = sgwc_ue_find_by_id(sess->sgwc_ue_id);
    ogs_assert(sgwc_ue);

    if (!sess->record_opening_time) {
        /* CDR was never opened — session failed before establishment */
        return;
    }

    close_time   = ogs_time_now();
    duration_sec = (uint32_t)ogs_time_sec(close_time - sess->record_opening_time);

    timestamp_str(sess->record_opening_time, open_ts,  sizeof(open_ts));
    timestamp_str(close_time,                close_ts, sizeof(close_ts));

    ue_ip_str(&sess->paa, ue_ip, sizeof(ue_ip));

    /* SGW-C control-plane address */
    {
        ogs_sockaddr_t *sa = ogs_gtp_self()->gtpc_addr ?
                             ogs_gtp_self()->gtpc_addr :
                             ogs_gtp_self()->gtpc_addr6;
        if (sa)
            OGS_ADDR(sa, sgw_addr);
        else
            snprintf(sgw_addr, sizeof(sgw_addr), "unknown");
    }

    /* PGW control-plane address */
    if (sess->gnode)
        OGS_ADDR(&sess->gnode->addr, pgw_addr);
    else
        snprintf(pgw_addr, sizeof(pgw_addr), "unknown");

    /* Charging characteristics as hex string */
    if (sess->session.charging_characteristics_presence)
        snprintf(chargchars, sizeof(chargchars), "%02x%02x",
                 sess->session.charging_characteristics[0],
                 sess->session.charging_characteristics[1]);
    else
        snprintf(chargchars, sizeof(chargchars), "0000");

    /* MNC: preserve leading zeros for 3-digit MNCs */
    mnc_len = ogs_plmn_id_mnc_len(&sgwc_ue->serving_plmn_id);
    if (mnc_len == 3)
        snprintf(mnc_str, sizeof(mnc_str), "%03d",
                 ogs_plmn_id_mnc(&sgwc_ue->serving_plmn_id));
    else
        snprintf(mnc_str, sizeof(mnc_str), "%02d",
                 ogs_plmn_id_mnc(&sgwc_ue->serving_plmn_id));

    fprintf(cdr_fp,
        "{"
        "\"recordType\":\"SGW-CDR\","
        "\"localSequenceNumber\":%u,"
        "\"imsi\":\"%s\","
        "\"msisdn\":\"%s\","
        "\"apn\":\"%s\","
        "\"pdnType\":\"%s\","
        "\"ueIPAddress\":\"%s\","
        "\"sgwAddress\":\"%s\","
        "\"pgwAddress\":\"%s\","
        "\"chargingId\":%u,"
        "\"ratType\":%u,"
        "\"servingNetwork\":{\"mcc\":\"%03d\",\"mnc\":\"%s\"},"
        "\"chargingCharacteristics\":\"%s\","
        "\"recordOpeningTime\":\"%s\","
        "\"recordClosingTime\":\"%s\","
        "\"duration\":%u,"
        "\"causeForRecordClosing\":%u",
        sess->local_sequence_number,
        sgwc_ue->imsi_bcd,
        sgwc_ue->msisdn_bcd[0] ? sgwc_ue->msisdn_bcd : "",
        sess->session.name ? sess->session.name : "",
        pdn_type_str(sess->paa.session_type),
        ue_ip,
        sgw_addr,
        pgw_addr,
        sess->pgw_charging_id,
        sgwc_ue->rat_type,
        sgwc_ue->serving_plmn_id_presence ?
            ogs_plmn_id_mcc(&sgwc_ue->serving_plmn_id) : 0,
        sgwc_ue->serving_plmn_id_presence ? mnc_str : "00",
        chargchars,
        open_ts,
        close_ts,
        duration_sec,
        (unsigned)cause
    );

    /* Location info (conditional) */
    if (sgwc_ue->uli_presence) {
        char tai_mnc[6], ecgi_mnc[6]; /* uint16_t decimal (max 5 digits) + NUL */
        int  tai_mnc_len, ecgi_mnc_len;

        tai_mnc_len = ogs_plmn_id_mnc_len(&sgwc_ue->e_tai.plmn_id);
        if (tai_mnc_len == 3)
            snprintf(tai_mnc, sizeof(tai_mnc), "%03d",
                     ogs_plmn_id_mnc(&sgwc_ue->e_tai.plmn_id));
        else
            snprintf(tai_mnc, sizeof(tai_mnc), "%02d",
                     ogs_plmn_id_mnc(&sgwc_ue->e_tai.plmn_id));

        ecgi_mnc_len = ogs_plmn_id_mnc_len(&sgwc_ue->e_cgi.plmn_id);
        if (ecgi_mnc_len == 3)
            snprintf(ecgi_mnc, sizeof(ecgi_mnc), "%03d",
                     ogs_plmn_id_mnc(&sgwc_ue->e_cgi.plmn_id));
        else
            snprintf(ecgi_mnc, sizeof(ecgi_mnc), "%02d",
                     ogs_plmn_id_mnc(&sgwc_ue->e_cgi.plmn_id));

        fprintf(cdr_fp,
            ",\"locationInfo\":{"
            "\"tai\":{\"mcc\":\"%03d\",\"mnc\":\"%s\",\"tac\":%u},"
            "\"ecgi\":{\"mcc\":\"%03d\",\"mnc\":\"%s\",\"eci\":%u}"
            "}",
            ogs_plmn_id_mcc(&sgwc_ue->e_tai.plmn_id), tai_mnc,
            sgwc_ue->e_tai.tac,
            ogs_plmn_id_mcc(&sgwc_ue->e_cgi.plmn_id), ecgi_mnc,
            sgwc_ue->e_cgi.cell_id
        );
    }

    fprintf(cdr_fp, "}\n");
    fflush(cdr_fp);
}
