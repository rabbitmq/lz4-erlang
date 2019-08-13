// Copyright (c) 2017-Present Pivotal Software, Inc.  All rights reserved.
//
// This package, the LZ4 binding for Erlang, is double-licensed under the Mozilla
// Public License 1.1 ("MPL") and the Apache License version 2
// ("ASL"). For the MPL, please see LICENSE-MPL-RabbitMQ. For the ASL,
// please see LICENSE-APACHE2.
//
// This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND,
// either express or implied. See the LICENSE file for specific language governing
// rights and limitations of this software.
//
// If you have any questions regarding licensing, please contact us at
// info@rabbitmq.com.

#include "lz4_erlang.h"
#include <string.h>

void dtor_LZ4F_cctx(ErlNifEnv* env, void* obj)
{
    LZ4F_freeCompressionContext(NIF_RES_GET(LZ4F_cctx, obj));
}

void dtor_LZ4F_dctx(ErlNifEnv* env, void* obj)
{
    LZ4F_freeDecompressionContext(NIF_RES_GET(LZ4F_dctx, obj));
}

int opts_to_lz4f_preferences(ErlNifEnv*, ERL_NIF_TERM, LZ4F_preferences_t*);
int opts_to_lz4f_preferences(ErlNifEnv* env, ERL_NIF_TERM opts, LZ4F_preferences_t* preferences)
{
    ERL_NIF_TERM info, value;
    ErlNifUInt64 ull;

    if (enif_get_map_value(env, opts, atom_frame_info, &info)) {
        if (!enif_is_map(env, info))
            return 0;

        if (enif_get_map_value(env, info, atom_block_size_id, &value)) {
            if (value == atom_default)
                preferences->frameInfo.blockSizeID = LZ4F_default;
            else if (value == atom_max64KB)
                preferences->frameInfo.blockSizeID = LZ4F_max64KB;
            else if (value == atom_max256KB)
                preferences->frameInfo.blockSizeID = LZ4F_max256KB;
            else if (value == atom_max1MB)
                preferences->frameInfo.blockSizeID = LZ4F_max1MB;
            else if (value == atom_max4MB)
                preferences->frameInfo.blockSizeID = LZ4F_max4MB;
            else
                return 0;
        }

        if (enif_get_map_value(env, info, atom_block_mode, &value)) {
            if (value == atom_linked)
                preferences->frameInfo.blockMode = LZ4F_blockLinked;
            else if (value == atom_independent)
                preferences->frameInfo.blockMode = LZ4F_blockIndependent;
            else
                return 0;
        }

        if (enif_get_map_value(env, info, atom_content_checksum, &value)) {
            if (value == atom_true)
                preferences->frameInfo.contentChecksumFlag = 1;
            else if (value != atom_false)
                return 0;
        }

        if (enif_get_map_value(env, info, atom_frame_type, &value)) {
            if (value == atom_frame)
                preferences->frameInfo.frameType = LZ4F_frame;
            else if (value == atom_skippable_frame)
                preferences->frameInfo.frameType = LZ4F_skippableFrame;
            else
                return 0;
        }

        if (enif_get_map_value(env, info, atom_content_size, &value)) {
            if (!enif_get_uint64(env, value, &ull))
                return 0;

            preferences->frameInfo.contentSize = ull;
        }
    }

    if (enif_get_map_value(env, opts, atom_compression_level, &value)) {
        if (!enif_get_int(env, value, &preferences->compressionLevel))
            return 0;
    }

    if (enif_get_map_value(env, opts, atom_auto_flush, &value)) {
        if (value == atom_true)
            preferences->autoFlush = 1;
        else if (value != atom_false)
            return 0;
    }

    return 1;
}

NIF_FUNCTION(lz4f_compress_frame)
{
    LZ4F_preferences_t preferences;
    size_t dstCapacity, dstSize;
    ErlNifBinary srcBin, dstBin;

    BADARG_IF(!enif_inspect_binary(env, argv[0], &srcBin));
    BADARG_IF(!enif_is_map(env, argv[1]));

    memset(&preferences, 0, sizeof(preferences));
    if (!opts_to_lz4f_preferences(env, argv[1], &preferences))
        return enif_make_badarg(env);

    dstCapacity = LZ4F_compressFrameBound(srcBin.size, &preferences);

    if (!enif_alloc_binary(dstCapacity, &dstBin))
        return enif_raise_exception(env, atom_enomem);

    dstSize = LZ4F_compressFrame(dstBin.data, dstCapacity, srcBin.data, srcBin.size, &preferences);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(dstSize)));
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, atom_enomem);
    }

    return enif_make_binary(env, &dstBin);
}

NIF_FUNCTION(lz4f_create_compression_context)
{
    LZ4F_cctx* cctx;
    LZ4F_errorCode_t result;
    ERL_NIF_TERM term;

    result = LZ4F_createCompressionContext(&cctx, LZ4F_VERSION);

    if (LZ4F_isError(result))
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(result)));

    NIF_RES_TO_TERM(LZ4F_cctx, cctx, term);

    return term;
}

NIF_FUNCTION(lz4f_compress_begin)
{
    void* cctx_res;
    LZ4F_preferences_t preferences;
    size_t dstSize;
    ErlNifBinary dstBin;

    BADARG_IF(!enif_get_resource(env, argv[0], res_LZ4F_cctx, &cctx_res));
    BADARG_IF(!enif_is_map(env, argv[1]));

    memset(&preferences, 0, sizeof(preferences));
    if (!opts_to_lz4f_preferences(env, argv[1], &preferences))
        return enif_make_badarg(env);

    if (!enif_alloc_binary(LZ4F_HEADER_SIZE_MAX, &dstBin))
        return enif_raise_exception(env, atom_enomem);

    dstSize = LZ4F_compressBegin(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, LZ4F_HEADER_SIZE_MAX, &preferences);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(dstSize)));
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, atom_enomem);
    }

    return enif_make_binary(env, &dstBin);
}

NIF_FUNCTION(lz4f_compress_update)
{
    void* cctx_res;
    size_t dstCapacity, dstSize;
    ErlNifBinary srcBin, dstBin;

    BADARG_IF(!enif_get_resource(env, argv[0], res_LZ4F_cctx, &cctx_res));
    BADARG_IF(!enif_inspect_binary(env, argv[1], &srcBin));

    // @todo We pass NULL because we don't currently keep the preferences
    // setup when the user began the compression. It might be done later
    // as an optimization.
    dstCapacity = LZ4F_compressBound(srcBin.size, NULL);

    if (!enif_alloc_binary(dstCapacity, &dstBin))
        return enif_raise_exception(env, atom_enomem);

    // We pass NULL because we can't guarantee that the source binary
    // data will remain for future calls. It may be garbage collected.
    dstSize = LZ4F_compressUpdate(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, dstCapacity, srcBin.data, srcBin.size, NULL);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(dstSize)));
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, atom_enomem);
    }

    return enif_make_binary(env, &dstBin);
}

NIF_FUNCTION(lz4f_flush)
{
    void* cctx_res;
    size_t dstCapacity, dstSize;
    ErlNifBinary dstBin;

    BADARG_IF(!enif_get_resource(env, argv[0], res_LZ4F_cctx, &cctx_res));

    // We pass 0 to get the upper bound for this operation.
    //
    // @todo We pass NULL because we don't currently keep the preferences
    // setup when the user began the compression. It might be done later
    // as an optimization.
    dstCapacity = LZ4F_compressBound(0, NULL);

    if (!enif_alloc_binary(dstCapacity, &dstBin))
        return enif_raise_exception(env, atom_enomem);

    // We pass NULL because we can't guarantee that the source binary
    // data will remain for future calls. It may be garbage collected.
    dstSize = LZ4F_flush(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, dstCapacity, NULL);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(dstSize)));
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, atom_enomem);
    }

    return enif_make_binary(env, &dstBin);
}

NIF_FUNCTION(lz4f_compress_end)
{
    void* cctx_res;
    size_t dstCapacity, dstSize;
    ErlNifBinary dstBin;

    BADARG_IF(!enif_get_resource(env, argv[0], res_LZ4F_cctx, &cctx_res));

    // We pass 0 to get the upper bound for this operation.
    //
    // @todo We pass NULL because we don't currently keep the preferences
    // setup when the user began the compression. It might be done later
    // as an optimization.
    dstCapacity = LZ4F_compressBound(0, NULL);

    if (!enif_alloc_binary(dstCapacity, &dstBin))
        return enif_raise_exception(env, atom_enomem);

    // We pass NULL because we can't guarantee that the source binary
    // data will remain for future calls. It may be garbage collected.
    dstSize = LZ4F_compressEnd(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, dstCapacity, NULL);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(dstSize)));
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        return enif_raise_exception(env, atom_enomem);
    }

    return enif_make_binary(env, &dstBin);
}

NIF_FUNCTION(lz4f_create_decompression_context)
{
    LZ4F_dctx* dctx;
    LZ4F_errorCode_t result;
    ERL_NIF_TERM term;

    result = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);

    if (LZ4F_isError(result))
        return enif_raise_exception(env, enif_make_atom(env, LZ4F_getErrorName(result)));

    NIF_RES_TO_TERM(LZ4F_dctx, dctx, term);

    return term;
}

NIF_FUNCTION(lz4f_get_frame_info)
{
    // @todo

    return atom_ok;
}

NIF_FUNCTION(lz4f_decompress)
{
    void* dctx_res;
    ERL_NIF_TERM head, reversed;
    size_t dstSize, srcRead, srcSize;
    ErlNifBinary srcBin, dstBin;

    BADARG_IF(!enif_get_resource(env, argv[0], res_LZ4F_dctx, &dctx_res));
    BADARG_IF(!enif_inspect_binary(env, argv[1], &srcBin));

    srcRead = 0;
    srcSize = srcBin.size;

    head = enif_make_list(env, 0);

    while (srcSize) {
        dstSize = 65536; // Arbitrary maximum size of chunk.

        if (!enif_alloc_binary(dstSize, &dstBin)) {
            return enif_raise_exception(env, atom_enomem);
        }

        LZ4F_decompress(NIF_RES_GET(LZ4F_dctx, dctx_res),
            dstBin.data, &dstSize,
            srcBin.data + srcRead, &srcSize,
            NULL);

        if (LZ4F_isError(dstSize)) {
            enif_release_binary(&dstBin);
            return enif_raise_exception(env, atom_enomem);
        }

        if (!enif_realloc_binary(&dstBin, dstSize)) {
            enif_release_binary(&dstBin);
            return enif_raise_exception(env, atom_enomem);
        }

        head = enif_make_list_cell(env, enif_make_binary(env, &dstBin), head);

        srcRead += srcSize;
        srcSize = srcBin.size - srcRead;
    }

    // We don't check the return value because no error can occur
    // according to the function's source code.
    enif_make_reverse_list(env, head, &reversed);

    return reversed;
}
