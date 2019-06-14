# Configuration of a Module

A module is loaded during runtime, if a valid XML `module` node is added to the
XML `config` node of the component.

```xml
<start name="rtcr_app">
	<config>
		<module name="core"/>
		...
	</config>
</start>
```

## Name

A module has a `name` attribute which is used for identifying and loading the
correct module during runtime.

```xml
<module name="core" />
```

## Deactivation

A module can be disabled by setting the attribute `disable` to `true`.

```xml
<module name="core" disable="true" />
```

## Parallelization

A module can be assigned to a priority. All modules with the same priority are
executed in parallel. A module with a higher priority is executed after a module
with a lower priority. The maximum number of available priorities depends on the
variable `Rtcr::Target_child::MAX_PRIORITY`, which is `10` by default.

In order to run all modules sequentially:

```xml
<module name="core" priority="0"/>
<module name="log" priority="1"/>
<module name="timer" priority="2"/>
<module name="ds" priority="3"/>
```

Run `log` and `timer` in parallel:


```xml
<module name="core" priority="0"/>
<module name="log" priority="1"/>
<module name="timer" priority="1"/>
<module name="ds" priority="2"/>
```

If no priority is set, the module will be automatically assigned to priority
`0`.

Run all modules in parallel:

```xml
<module name="core" />
<module name="log" />
<module name="timer"/>
<module name="ds" />
```

## Custom Configuration

XML Sub-nodes of `module` node are passed to the module during construction time
and provide an consistent configuration of a module.

```xml
<module name="core_inc">
	<granularity>120</granularity>
	<quota>1048576</quota>
</module>
```
