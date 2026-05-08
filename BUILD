load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "deck",
    srcs = ["deck.cc"],
    hdrs = ["deck.h"],
)

cc_library(
    name = "ttl_pointer",
    hdrs = ["ttl_pointer.h"],
    deps = [
        "@abseil-cpp//absl/time",
    ],
)

cc_library(
    name = "game",
    srcs = [
        "game.cc",
        "player.cc",
    ],
    hdrs = [
        "game.h",
        "player.h",
    ],
    deps = [
        ":deck",
        ":ttl_pointer",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/time",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":game",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/strings",
    ],
)

cc_binary(
    name = "cli_main",
    srcs = ["cli_main.cc"],
    deps = [
        ":game",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
    ],
)

cc_binary(
    name = "replay_state",
    srcs = ["replay_state.cc"],
    deps = [
        ":game",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/strings",
    ],
)
