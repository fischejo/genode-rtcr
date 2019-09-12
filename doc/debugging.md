# VERBOSITY

You can enable the macros **VERBOSE** and **DEBUG** in order to increase the
verbosity level. These are disabled by default.

In order to enable both macros for all modules, append `debug` to the SPECS
variables in `build/etc/specs.conf`:

```diff
- SPECS = genode focnados_pbxa9
+ SPECS = genode focnados_pbxa9 debug
```

# Profiling

The profiler log is activated by setting the macro `PROFILE`. This requires the
`profiler` repository to be installed in the `genode/repos` directory. In order
to enable the macro for all modules, append `profile` to the SPECS variables in
`build/etc/specs.conf`:

```diff
- SPECS = genode focnados_pbxa9
+ SPECS = genode focnados_pbxa9 profile
```
