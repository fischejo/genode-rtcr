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
