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
 * Test binary entry point for PFCP FAR hash tests.
 *
 * A separate main is needed because ogs_pfcp_context_init() requires
 * ogs_app()->pool.sess to be non-zero, which is set by
 * ogs_app_global_conf_prepare().  The standard unit-test main does not
 * call ogs_app_global_conf_prepare(), so these tests live in their own
 * binary.
 */

#include "ogs-pfcp.h"
#include "core/abts.h"

abts_suite *test_pfcp_far(abts_suite *suite);

const struct testlist {
    abts_suite *(*func)(abts_suite *suite);
} alltests[] = {
    {test_pfcp_far},
    {NULL},
};

static void terminate(void)
{
    ogs_pfcp_context_final();

    ogs_pkbuf_default_destroy();

    ogs_core_terminate();
}

int main(int argc, const char *const argv[])
{
    int rv, i, opt;
    ogs_getopt_t options;
    struct {
        char *log_level;
        char *domain_mask;
    } optarg;
    const char *argv_out[argc + 3]; /* '-e error' is always added */

    abts_suite *suite = NULL;
    ogs_pkbuf_config_t config;

    rv = abts_main(argc, argv, argv_out);
    if (rv != OGS_OK) return rv;

    memset(&optarg, 0, sizeof(optarg));
    ogs_getopt_init(&options, (char **)argv_out);

    while ((opt = ogs_getopt(&options, "e:m:")) != -1) {
        switch (opt) {
        case 'e':
            optarg.log_level = options.optarg;
            break;
        case 'm':
            optarg.domain_mask = options.optarg;
            break;
        case '?':
        default:
            fprintf(stderr, "%s: should not be reached\n", OGS_FUNC);
            return OGS_ERROR;
        }
    }

    ogs_core_initialize();

    ogs_pkbuf_default_init(&config);
    ogs_pkbuf_default_create(&config);

    /*
     * ogs_pfcp_context_init() uses ogs_app()->pool.sess (and pool.nf) to
     * size its internal pools.  ogs_app_global_conf_prepare() sets these
     * to their default values (1024 UEs, etc.) so the pools are non-zero.
     */
    ogs_app_global_conf_prepare();

    ogs_pfcp_context_init();

    atexit(terminate);

    rv = ogs_log_config_domain(optarg.domain_mask, optarg.log_level);
    if (rv != OGS_OK) return rv;

    for (i = 0; alltests[i].func; i++)
        suite = alltests[i].func(suite);

    return abts_report(suite);
}
