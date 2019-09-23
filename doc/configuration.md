# Configuration 

## Child Component

A child is initialized with a label, which corresponds to an `child` node in the
XML config. Following parameters exists:

* `name` Name of the child
* `quota` A portion of memory which is transfered to the child. The current
	maximum is 140000 bytes (136Kb). 
* `xpos` and `ypos` assign the child to a specific CPU core. Default is `0`
* `caps` the number of assigned capabilities.

```xml
<start name="rtcr_app">
	<config>
		<child name="sheep_counter" quota="140000"  caps='1000' xpos="1" ypos="1"/>
		...
	</config>
</start>
```

## Rtcr
The checkpoint mechanism consists of several threads each checkpointing a part
of session. It is possible to assign a thread to a CPU by the `checkpointable`
node. Existing threads:

* `cpu_session`
* `pd_session`
* `rom_session`
* `rm_session`
* `log_session`
* `timer_session`
* `capability_mapping`
* `ram_dataspaces`

Parameters:

* `name` of the checkpointable
* `xpos` and `ypos` assign the thread to a specific CPU core. Default is `0`

```xml
<start name="rtcr_app">
	<config>
		<checkpointable name="cpu_session" xpos="1" ypos="1"/>
		...
	</config>
</start>
```

In order to run all threads in parallel, it is necessary to activate it with the
`checkpoint` node. For debugging purposes, this flag is deactivated by default.

```xml
<start name="rtcr_app">
	<config>
       <checkpoint parallel="true"/>
		...
	</config>
</start>
```
