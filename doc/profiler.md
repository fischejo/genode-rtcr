# Profiling Rtcr

The profiler log is activated by setting the macro `-DPROFILE` during
runtime. In order to simply the profiling process, the SPEC `profiler` can be
added to the SPECS variables in `build/etc/specs.conf`:

```diff
- SPECS = genode focnados_pbxa9
+ SPECS = genode focnados_pbxa9 profiler
```

This will:

* disable debug and verbose log
* include the profiler library
* enable the profiling log
