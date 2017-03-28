// @todo Copyright.

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

NIF_FUNCTION(lz4f_compress_frame)
{
    LZ4F_preferences_t preferences;
    size_t dstCapacity, dstSize;
    ErlNifBinary srcBin, dstBin;

    BADARG_IF(!enif_inspect_binary(env, argv[0], &srcBin));

    memset(&preferences, 0, sizeof(preferences));

    // @todo prefs

    dstCapacity = LZ4F_compressFrameBound(srcBin.size, &preferences);

    // @todo Better error.
    BADARG_IF(!enif_alloc_binary(dstCapacity, &dstBin));

    dstSize = LZ4F_compressFrame(dstBin.data, dstCapacity, srcBin.data, srcBin.size, &preferences);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
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
        // @todo Better error.
        return enif_make_badarg(env);

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

    memset(&preferences, 0, sizeof(preferences));

    // @todo prefs

    // @todo Better error.
    BADARG_IF(!enif_alloc_binary(LZ4F_HEADER_SIZE_MAX, &dstBin));

    dstSize = LZ4F_compressBegin(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, LZ4F_HEADER_SIZE_MAX, &preferences);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
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

    // @todo Better error.
    BADARG_IF(!enif_alloc_binary(dstCapacity, &dstBin));

    // We pass NULL because we can't guarantee that the source binary
    // data will remain for future calls. It may be garbage collected.
    dstSize = LZ4F_compressUpdate(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, dstCapacity, srcBin.data, srcBin.size, NULL);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
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

    // @todo Better error.
    BADARG_IF(!enif_alloc_binary(dstCapacity, &dstBin));

    // We pass NULL because we can't guarantee that the source binary
    // data will remain for future calls. It may be garbage collected.
    dstSize = LZ4F_flush(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, dstCapacity, NULL);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
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

    // @todo Better error.
    BADARG_IF(!enif_alloc_binary(dstCapacity, &dstBin));

    // We pass NULL because we can't guarantee that the source binary
    // data will remain for future calls. It may be garbage collected.
    dstSize = LZ4F_compressEnd(NIF_RES_GET(LZ4F_cctx, cctx_res),
        dstBin.data, dstCapacity, NULL);

    if (LZ4F_isError(dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
    }

    if (!enif_realloc_binary(&dstBin, dstSize)) {
        enif_release_binary(&dstBin);
        // @todo Better error.
        return enif_make_badarg(env);
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
        // @todo Better error.
        return enif_make_badarg(env);

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

// So apparently we need to call this repeatedly until there is
// no input remaining. This means we need to create an iolist(),
// or more specifically a list of binary().

//<pre><b>size_t LZ4F_decompress(LZ4F_dctx* dctx,
//                                   void* dstBuffer, size_t* dstSizePtr,
//                                   const void* srcBuffer, size_t* srcSizePtr,
//                                   const LZ4F_decompressOptions_t* dOptPtr);

    return atom_ok;
}
