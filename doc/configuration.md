# Configuration 

## Child Component

A child is initialized with a label, which corresponds to an `child` node in the
XML config. Following parameters exists:

* `name` Name of the child
* `quota` A portion of memory which is transfered to the child. Default is `512 KB`
* `xpos` and `ypos` assign the child to a specific CPU core. Default is `0`

```xml
<start name="rtcr_app">
	<config>
		<child name="sheep_counter" quota="1048576"  xpos="1" ypos="1"/>
		...
	</config>
</start>
```

## Rtcr
The checkpoint mechanism consists of several threads each checkpointing a
session. It is possible to assign a thread to a CPU by the `checkpointable` node. Existing threads:

* `cpu_session`
* `pd_session`
* `ram_session`
* `rom_session`
* `rm_session`
* `log_session`
* `timer_session`
* `capability_mapping`

Parameters:

* `name` of the checkpointable
* `xpos` and `ypos` assign the thread to a specific CPU core. Default is `0`

```xml
<start name="rtcr_app">
	<config>
		<checkpointable name="ds_session" xpos="1" ypos="1"/>

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


If the module factory for loading a module is used, the module can be specified
in a `module` node.
```xml
<start name="rtcr_app">
	<config>
       <module name="base"/>
		...
	</config>
</start>
```
