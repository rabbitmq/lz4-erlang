// @todo Copyright.

#ifndef __LZ4_ERLANG_H__
#define __LZ4_ERLANG_H__

#include <lz4frame.h>

// List of atoms used by this NIF.
//
// @todo We don't use threads so perhaps we should make nif_helpers
// better by splitting concerns into threads/not and have nif_helpers
// decide when to create the _nif_thread_ret atom or not.

#define NIF_ATOMS(A) \
    A(ok) \
    A(done) \
    A(_nif_thread_ret_)

// List of resources used by this NIF.

#define NIF_RESOURCES(R) \
    R(LZ4F_cctx) \
    R(LZ4F_dctx)

// List of functions defined in this NIF.
// @todo compress_frame_bound, compress_frame

#define NIF_FUNCTION_NAME(f) lz4_erlang_ ## f
#define NIF_FUNCTIONS(F) \
    F(lz4f_compress_frame, 2) \
    F(lz4f_create_compression_context, 0) \
    F(lz4f_compress_begin, 2) \
    F(lz4f_compress_update, 2) \
    F(lz4f_flush, 1) \
    F(lz4f_compress_end, 1) \
    F(lz4f_create_decompression_context, 0) \
    F(lz4f_get_frame_info, 2) \
    F(lz4f_decompress, 2)

#include "nif_helpers.h"

NIF_ATOMS(NIF_ATOM_H_DECL)
NIF_RESOURCES(NIF_RES_H_DECL)
NIF_FUNCTIONS(NIF_FUNCTION_H_DECL)

#endif
