// @todo Copyright.

#include "lz4_erlang.h"

NIF_ATOMS(NIF_ATOM_DECL)
NIF_RESOURCES(NIF_RES_DECL)

int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info);
int upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info);
void unload(ErlNifEnv* env, void* priv_data);

int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    NIF_ATOMS(NIF_ATOM_INIT)
    NIF_RESOURCES(NIF_RES_INIT)

    return 0;
}

int upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info)
{
    *priv_data = *old_priv_data;

    return 0;
}

void unload(ErlNifEnv* env, void* priv_data)
{
}

static ErlNifFunc nif_funcs[] = {
    NIF_FUNCTIONS(NIF_FUNCTION_ARRAY)
};

ERL_NIF_INIT(lz4_nif, nif_funcs, load, NULL, upgrade, unload)
