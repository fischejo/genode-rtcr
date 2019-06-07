# Rtcr - *Real Time Checkpoint Restore*

## Genode Repository Dependencies
This Genode repository depends on following repositories
1. `profiler`


## Modules

| **Module** | **Description** | **Optional** | **Dependencies** |
| --- | --- | --- | --- |
| `core` | Minimal implementation for a successfull checkpoint/restore | | `ds` |
| `ds` | Default memory Copying implementation | | |
| `log` | Module for checkpointing/restoring a Log session | Yes | |
| `timer` | Module for checkpointing/restoring a Timer session | Yes | |


## Documentation
All documentation is in directory [doc](doc).
