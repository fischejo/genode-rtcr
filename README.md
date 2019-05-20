# **R**eal**T**ime **C**heckpoint **R**estore 

## Documentation
All documentation is in directory `doc`.


## Modules


| **Module** | **Description** | **Optional** | **Dependencies** |
| --- | --- | --- | --- |
| `core` | Minimal implementation for a successfull checkpoint/restore | | `ds` |
| `ds` | Default memory Copying implementation | | |
| `log` | Module for checkpointing/restoring a Log session | Yes | |
| `timer` | Module for checkpointing/restoring a Timer session | Yes | |

