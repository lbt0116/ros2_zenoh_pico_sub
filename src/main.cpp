//
// Copyright (c) 2022 ZettaScale Technology
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

static volatile std::sig_atomic_t g_should_exit = 0;

extern "C" void handle_sigint(int) { g_should_exit = 1; }

static const char* sample_kind_to_str(SampleKind kind) {
    switch (kind) {
        case SampleKind::Z_SAMPLE_KIND_PUT: return "PUT";
        case SampleKind::Z_SAMPLE_KIND_DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

int main(int /*argc*/, char** /*argv*/) {
#ifdef ZENOHCXX_ZENOHC
    init_log_from_env_or("error");
#endif

    std::signal(SIGINT, handle_sigint);

    const KeyExpr keyexpr("rt/chatter");

    std::cout << "Opening session (zenoh-pico backend)..." << std::endl;
    auto config = Config::create_default();
    auto session = Session::open(std::move(config));

    auto data_handler = [](const Sample& sample) {
        std::cout << ">> [ROS2 Sub] " << sample_kind_to_str(sample.get_kind())
                  << " ('" << sample.get_keyexpr().as_string_view() << "' : '"
                  << sample.get_payload().as_string() << "')";
        auto attachment = sample.get_attachment();
        if (attachment.has_value()) {
            std::cout << " (att: " << attachment->get().as_string() << ")";
        }
        std::cout << std::endl;
    };

    std::cout << "Declaring subscriber on '" << keyexpr.as_string_view() << "'..." << std::endl;
    auto sub = session.declare_subscriber(keyexpr, data_handler, closures::none);

    std::cout << "Press CTRL-C to quit..." << std::endl;
    while (!g_should_exit) {
        std::this_thread::sleep_for(200ms);
    }

    return 0;
}


