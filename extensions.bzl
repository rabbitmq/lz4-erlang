load(
    "@bazel_tools//tools/build_defs/repo:http.bzl",
    "http_file",
)
load(
    "@bazel_tools//tools/build_defs/repo:git.bzl",
    "new_git_repository",
)
load(
    "@bazel_tools//tools/build_defs/repo:utils.bzl",
    "maybe",
)

LZ4_SRC_BUILD_FILE_CONTENT = """load(
    "@rules_erlang//:generlang.bzl",
    "generlang",
)

exports_files([
    "lib/lz4.h",
    "lib/lz4frame.h",
])

generlang(
    name = "static_library",
    srcs = glob(
        [
            "lib/**/*",
            "Makefile.inc",
        ],
        exclude = ["lib/liblz4.a"],
    ),
    outs = [
        "lib/liblz4.a",
    ],
    cmd = "LIB_DIR=$(dirname $(location lib/Makefile)); make -C $LIB_DIR liblz4.a MOREFLAGS=-fPIC && cp $LIB_DIR/liblz4.a $@",
    visibility = ["//visibility:public"],
)
"""

NIF_HELPERS_BUILD_FILE_CONTENT = """exports_files([
    "nif_helpers.h",
    "nif_helpers.c",
])
"""

def _external_deps(_ctx):
    new_git_repository(
        name = "lz4_src",
        build_file_content = LZ4_SRC_BUILD_FILE_CONTENT,
        remote = "https://github.com/lz4/lz4",
        tag = "v1.9.2",
    )

    maybe(
        repo_rule = new_git_repository,
        name = "nif_helpers",
        build_file_content = NIF_HELPERS_BUILD_FILE_CONTENT,
        commit = "4af25bf765536496ed2b10e22eb4e6e3304b9aee",
        remote = "https://github.com/ninenines/nif_helpers",
    )

    maybe(
        repo_rule = http_file,
        name = "pdf_reference",
        urls = ["https://opensource.adobe.com/dc-acrobat-sdk-docs/pdfstandards/pdfreference1.7old.pdf"],
    )

external_deps = module_extension(
    implementation = _external_deps,
)
