# Targets

This repository provides several targets:

* [rtcr](lib/mk/rtcr.mk) is the rtcr library which provides the
  checkpoint/restore functionality
  
* [rtcr_core](lib/mk/rtcr_core.mk) provides a minimal implementation of the core
  module. Required by library `rtcr`.

* [rtcr_ds](lib/mk/rtcr_ds.mk) provides a module implementation for handling
  dataspaces. Required by library `rtcr`.
  
* [rtcr_timer](lib/mk/rtcr_timer.mk) provides a  module implementation for the
  timer session. Optionally required by library `rtcr`, but included by default.

* [rtcr_log](lib/mk/rtcr_log.mk) provides a  module implementation for the
  log session. Optionally required by library `rtcr`, but included by default.

* [rtcr_test](src/app/rtcr_test/target.mk) provides an example for using the
  `rtcr` library.

* [sheep_counter](src/app/sheep_counter/target.mk) provides a child application
  which is checkpointed by `rtcr_test`.

