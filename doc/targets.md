# Libraries

This repository provides several libraries:

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


# Applications

* [sheep_counter](src/app/sheep_counter/target.mk) provides a child application
  which can be be checkpointed and restored.

* [rtcr_app](src/app/rtcr_app/target.mk) provides an example for using the
  `rtcr` library. It depends on the library `rtcr`.


# Run Scripte

* [run/rtcr_seq](run/rtcr_seq.run) provides an Run Script including `rtcr_test` and
  `sheep_counter`. This exmaple checkpoints and restores the application
  `sheep_counter`. All modules are executed sequentially.

* [run/rtcr](run/rtcr.run) Same as `run/rtcr` but all modules are
  executed in parallel.

  
