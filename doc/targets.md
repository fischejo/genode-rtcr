# Libraries

This repository provides several libraries:

* [rtcr](lib/mk/rtcr.mk) is the rtcr library which provides the
  checkpoint/restore functionality

* [rtcr_serializer](lib/mk/rtcr_serializer.mk) provides a protobuf serialization for a
  checkpointed target child. 


# Applications

* [sheep_counter](src/app/sheep_counter/target.mk) provides a child application
  which can be be checkpointed and restored.

* [rtcr_app](src/app/rtcr_app/target.mk) provides an example for using the
  `rtcr` library. It depends on the library `rtcr`.


# Run Scripte

* [run/rtcr](run/rtcr.run) provides an Run Script including `rtcr_test` and
  `sheep_counter`. 
  
* [run/rtcr_multicore](run/rtcr_multicore.run) Same as `run/rtcr` but is configured for
  dual core CPUs. The child is running on `CPU 0`, while the checkpointing is
  executed on `CPU 1`.

* [run/rtcr_serializer](run/rtcr_serializer.run) Same as `run/rtcr` but
  serializes the child after checkpointing.





  
