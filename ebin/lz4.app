{application, 'lz4', [
	{description, "New project"},
	{vsn, "0.1.0"},
	{modules, ['lz4_nif','lz4f']},
	{registered, []},
	{applications, [kernel,stdlib]},
	{env, []}
]}.