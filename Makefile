PROJECT = lz4
PROJECT_DESCRIPTION = A LZ4 compression library that wraps a NIF
# The version of based on the version of the lz4 library, plus a "revision".
PROJECT_VERSION = 1.9.2.5

BUILD_DEPS = lz4_src nif_helpers
dep_lz4_src = git https://github.com/lz4/lz4 v1.9.2
dep_nif_helpers = git https://github.com/ninenines/nif_helpers 4af25bf765536496ed2b10e22eb4e6e3304b9aee
DEP_PLUGINS = nif_helpers

DEPS = host_triple
dep_host_triple = hex 0.1.0

C_SRC_OUTPUT = $(CURDIR)/priv/lz4_nif

TEST_DEPS = ct_helper
dep_ct_helper = git https://github.com/extend/ct_helper master

include erlang.mk

CFLAGS += -I $(DEPS_DIR)/lz4_src/lib -Wno-implicit-function-declaration -Wno-unused-command-line-argument
# This is required in order to build a liblz4.a that we can
# include in our shared library.
export CPPFLAGS += -shared -fPIC
LDLIBS += $(DEPS_DIR)/lz4_src/lib/liblz4.a

-include c_src/env.mk

cppcheck:
	cppcheck -f --quiet --error-exitcode=2 --enable=all --inconclusive --std=posix \
        -Ideps/lz4_src/lib/ -Ideps/nif_helpers/ -I$(ERTS_INCLUDE_DIR) c_src/

scan-build:
	make clean
	scan-build make

# Download a large file for use in compression tests.

PDF_REFERENCE = https://www.adobe.com/content/dam/Adobe/en/devnet/acrobat/pdfs/pdf_reference_1-7.pdf

test-build:: $(TEST_DIR)/lz4f_SUITE_data/pdf_reference_1-7.pdf

$(TEST_DIR)/lz4f_SUITE_data/pdf_reference_1-7.pdf:
	$(verbose) mkdir -p $(TEST_DIR)/lz4f_SUITE_data/
	$(gen_verbose) $(call core_http_get,$@,$(PDF_REFERENCE))
