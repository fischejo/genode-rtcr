# Configuration of Target_child


## Name of the Child Component

The name of the child can be defined in the XML `config` Node:

```xml
<start name="rtcr_app">
	<config>
		<child name="sheep_counter" />
		...
	</config>
</start>
```

## Quota of the Child Component

As the `core` module transfers the quota from the parent component to the child
component, the qota is set as custom config of the `core` module:

```xml
<module name="core" priority="0">
	<quota>1048576</quota>
</module>
```

## Affinity of the Child Component

As the `core` module provides the CPU session, the affinity is set as custom
config of the `core` module:

```xml
<module name="core" priority="0">
	<affinity xpos="1"/>
</module>
```

Following attributes exist for the `affinity` node:
* `xpos` Sets the x position in the affinity map. For example: `xpos="1"`
  assigns the child to core `2`. `xpos="0"` is default, if no affinity node is defined.
* `ypos` Sets the y position in the affinity map. Due to patching of the
  Fiasco.OC kernel, only an affinity map with height 1 is possible. Default
  value is `ypos="0"`.
* `width` of the affinity location. Default is `0`. 
* `height` of the affinity location. Default is `0`.
