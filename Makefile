PROJECT = lz4
PROJECT_DESCRIPTION = New project
PROJECT_VERSION = 0.1.0

BUILD_DEPS = lz4_src
dep_lz4_src = git https://github.com/lz4/lz4 v1.7.5

C_SRC_OUTPUT = $(CURDIR)/priv/lz4_nif

include erlang.mk

CFLAGS += -I $(DEPS_DIR)/lz4_src/lib
LDLIBS += -L $(DEPS_DIR)/lz4_src/lib -llz4
