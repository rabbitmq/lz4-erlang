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

def _external_deps(ctx):
    new_git_repository(
        name = "lz4_src",
        build_file = "//:BUILD.lz4_src",
        remote = "https://github.com/lz4/lz4",
        tag = "v1.9.2",
    )

    maybe(
        repo_rule = new_git_repository,
        name = "nif_helpers",
        build_file = "//:BUILD.nif_helpers",
        commit = "4af25bf765536496ed2b10e22eb4e6e3304b9aee",
        remote = "https://github.com/ninenines/nif_helpers",
    )

    maybe(
        repo_rule = http_file,
        name = "pdf_reference",
        urls = ["https://www.adobe.com/content/dam/Adobe/en/devnet/acrobat/pdfs/pdf_reference_1-7.pdf"],
    )

external_deps = module_extension(
    implementation = _external_deps,
)
