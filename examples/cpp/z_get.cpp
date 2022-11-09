//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>

#include <stdio.h>
#include <string.h>

#include "zenohcpp.h"
#include "zenohcpp_0.h"

using namespace zenoh_0;

int main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    if (argc > 1) {
        expr = argv[1];
    }
    auto keyexpr = zenoh::keyexpr(expr);
    if (!zenoh::check(keyexpr)) {
        printf("%s is not a valid key expression", expr);
        exit(-1);
    }
    zenoh::OwnedConfig config;
    if (argc > 2) {
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, argv[2])) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[2], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    zenoh::OwnedSession s(std::move(config));
    if (!s.check()) {
        printf("Unable to open session!\n");
        exit(-1);
    }

    printf("Sending Query '%s'...\n", expr);
    auto opts = zenoh::GetOptions().target(Z_QUERY_TARGET_ALL);
    auto channel = zenoh::reply_fifo_new(16);
    s.get(keyexpr, "",  channel.take_send(), opts);

    z_owned_reply_t reply = z_reply_null();
    for(z_call(channel.recv(), &reply); z_check(reply); z_call(channel.recv(), &reply)) {
        if (z_reply_is_ok(&reply)) {
            z_sample_t sample = z_reply_ok(&reply);
            char *keystr = z_keyexpr_to_string(sample.keyexpr);
            printf(">> Received ('%s': '%.*s')\n", keystr, (int)sample.payload.len, sample.payload.start);
            free(keystr);
        } else {
            printf("Received an error\n");
        }
    }
    return 0;
}