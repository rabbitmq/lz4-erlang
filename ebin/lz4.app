{application, 'lz4', [
	{description, "An LZ4 compression library that wraps an NIF"},
	{vsn, "1.9.4.0"},
	{modules, ['lz4_nif','lz4f']},
	{registered, []},
	{applications, [kernel,stdlib,host_triple]},
	{env, []}
]}.