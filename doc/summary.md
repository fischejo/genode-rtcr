Vorläufige Zusammenfassung des Modularen C/R
============================================

# Links

* [Weekly Presentationen](https://gitlab.lrz.de/rtcr_workspace/ma_johannes_fischer/master_arbeit/tree/master/04_presentation)

# Aufteilung

* **Module** Ein Modul erbt von der Klasse **Module** und stellt dem C/R eine
  Funktionalität bereit. Der Programmcode jedes **Modules** wird als statische
  Library implementiert. 
  * Beispiele:
	* rtcr_inc
	* rtcr
	* rtcr_log
  * Vorteile:
	* Portierung einfacher, da zuerst einmal nur die minimale Implementierung
      portiert werden muss.
    * C/R kann auch aus den gewünschten Modulen zusammen gelinked werden.
	* Bei einem guten modularen Design, kann die Komplexität des C/R auf Module
      verteilt werden.
* **Genode Repository** befinden sich in `genode/repos` und stellen Anwendungen
  und Libraries zur Verfügung. 
  * `rtcr` beinhaltet die Module `rtcr_log`, `rtcr_timer`, `rtcr_core`, `rtcr_ds`
  * `rtcr_inc` beinhaltet das Modul `rtcr_core_inc`
  * `rtcr_red` beinhaltet das Modul `rtcr_core_red`  
  * `rtcr_fpga` beinhaltet die Module `rtcr_core_kcap`, `rtcr_ds_cdma`
  * `rtcr_para` beinhaltet das Modul `rtcr_ds_para`

Die notwendigen Schritten zum Linken eines Modules sind in jedem Git-Repository
beschrieben. Abhängigkeiten und die Konfiguration eines Modules sind ebenfalls
dort dokumentiert.


# Multi-threading

Jedes Module läuft in einem eigenen Thread, welcher einer CPU zugeteilt werden
kann. Module können auch eigene interne Threads starten und um so die Arbeit
weiter zu parallisieren.


# Konfiguration

Zu jedem Image wird eine XML-Datei hinzugelegt, welche das Verhalten von Genode
Anwendungen definiert. Mithilfe solch einer XML Definition wird auch der C/R und
dessen Module konfiguriert. 

Konfigurationsoptionen:
* Child
  * Binary name
  * Affinity Location
  * Quota
  
* Module
  * Aktivieren/Deaktivieren
  * Affinity Location des Modul-Threads
  * Name
  * Priorität (Ein Modul mit gleicher Priorität laufen parallisiert beim
    Checkpoint. Module mit unterschiedlicher Priorität werden seriell
    ausgeführt.)

Neben den aufgelisteten Standard Konfiguration, kann jedem Modul auch eine
beliebiger XML Node übergeben werden, um so das Modul weiter zu konfigurieren.

# Implementierung eines weiteren Modules

`Rtcr::Module` befindet sich in `rtcr/include/rtcr/module.h` und stellt die
minimale Schnittstelle eines Modules zur Verfügung:

* `name()`:  Name des Modules
* `initialize()`: Wird aufgerufen sobald alle Module geladen sind und eine
  Liste der Module wird übergeben. So kann ein Module Funktionalitäten eines
  anderen Modules in anspruch nehmen.
* `checkpoint()`
* `restore()`
* `resolve_session_request` Ermöglicht dem Modul, einen Service bereit zu
  stellen.

# Module Registierung & Start

`Rtcr::Module_factory` befindet sich in `rtcr/include/rtcr/module_factory.h` und
implementiert das Factory-Pattern um ein Modul zu erzeugen. Desweiteren wird das
Registry-Pattern verwendet, um alle Module beim starten der C/R Anwendung zu
registieren. Ablauf:

1. Für jedes Modul wird eine `Module_factory` Klasse implementiert und ein `static`
   Object davon definiert.
2. C/R Anwendung startet, und Konstructor von allen `Module_factory` Objekten
   wird aufgerufen. Dies registriert jeder dieser Objekte in einer `static` Lookup-Liste.
3. Zur Laufzeit wird aus der Xml-Konfiguration alle zu ladenden Modul Namen
   ausgelesen. 
4. Mithilfe der Lookup-Liste lässt sich zu einem Modul Namen die passende
   Module-Factory finden. 
5. Diese bietet schlussendlich die Funktion `create()` an um die eigentlich
   Instance eines Moduls zu erstellen.
   
   
   
# Minimaler C/R 

Die Minimal-Implementierung des C/R befindet sich in `rtcr` und
besteht aus den Modulen:
* `rtcr_core` stellt `CPU`, `PD`, `RAM`, `ROM` & `RM` Session zur Verfügung
* `rtcr_log` stellt `LOG` Session zur Verfügung
* `rtcr_timer` stellt `Timer` Session zur Verfügung
* `rtcr_ds` stellt eine Funktion zum Kopieren von Dataspaces bereit

## Generale Core Module Implementierung

Aufgrund einer starken Verknüpfung des C/R Codes mit den Monitor-Sessions,
schien es anfangs vernünftig, die 5 wichtigsten Sessions als ein Modul
(`Core_module_abstract`) bereit zu stellen. Diese Modulklasse is in
`rtcr/include/rtcr/core_module_abstract` implementiert und erweitert die
abstrakte Klasse `Module` um Schnittstellen zur Kommunikation von notwendigen
Services (PD, CPU,RAM, ROM...).

Die `Core_module_abstract` Klasse ist also Ausgangspunkt um ein beliebiges Core
Modul zu implementieren. 


## Core Module Implementierung

Das Module `rtcr_core` basiert auf der `Core_module_abstract` Klasse und stellt
die Standard Implementierung dar. Diese ist mithilfe der Diamand-Vererbung
realisiert um trotz der starken Abhängigkeiten zwischen den einzelnen C/R
Komponenten noch eine funktionale Trennung in Klassen zu verwirklichen.

Hierzu einen Auszug aus `rtcr/include/rtcr_core/core_module_base.h`:
```C++
/**
 * The core module provides the core implementation of the Rtcr.
 *
 * It contains:
 * - CPU Session and Checkpointing/Restoring
 * - RAM Session and Checkpointing/Restoring
 * - ROM Session and Checkpointing/Restoring
 * - PD Session and Checkpointing/Restoring
 * - RM Session and Checkpointing/Restoring
 *
 * Due to the strong coupling between PD, RAM and RM sessions checkpointing,
 * this module is implemented with a diamand hierarchy. Where all classes in
 * level 2 inherits from level 1 and class Core_module (level 3) inherits from
 * all classes in level 2. This concept allows to seperate logic in container
 * classes (level 2) but there is still the possibility to share methods between
 * container classes.
 *
 *                      +---------------------+
 *                      |  Core_module_base   |                      [ LEVEL 1 ]
 *                      +---------------------+
 *
 *    +-----+       +----+      +-----+      +-----+  +----+  +----+
 *    | cpu |       | pd |      | ram |      | rom |  | rm |  | ds | [ LEVEL 2 ]
 *    +-----+       +----+      +-----+      +-----+  +----+  +----+
 *
 *                          +-------------+
 *                          | Core_module |                          [ LEVEL 3 ]
 *                          +-------------+
 *
 * The Core_module_base (level 1) defines methods which are used by one of the
 * classes in level 2-3. These methods are also defined in level 2 and 3. If
 * Core_module_cpu calls `pd_root()`, its call will be delegated to the
 * Core_module_pd class where `pd_root()` is implemented. 
 */
```

Mit solch einer Diamand-Vererbung ist es also möglich, die Funktionalitäten des
C/R in separate Klassen zu unterteilen, aber Funktionenen der einen Klasse mit
allen anderen Klassen zu teilen.

Während des Refactoring des C/R zeigte sich, dass die gemeinsame Schnittstelle
zwischen den Core-Klassen (Level 2) aus folgenden Funktionen besteht:

* `find_region_map_by_badge` implementiert bei `core_module_rm.h`
* `_prepare_region_maps` implementiert bei `core_module_rm.h`.
* `checkpoint_dataspace` implementiert bei `core_module_ram.h`.
* `{Pd,Ram,Cpu,Rm,Rom}_root` stellen die `Root_component` Objekte der Services
  zur Verfügung.


## Erweiterbarkeit des Core Modules

Neben dem vollständigen Neu schreiben des Core Moduls auf Basis von
`Core_module_abstract`, kann man auch von einzelnen Klassen (Level 2) erben und
nur einzelne Funktionen überschreiben.

Dies lässt sich anhand des `rtcr_core_inc` Moduls zeigen. Die Klasse
`Core_inc_module_ds` erbt von `Core_module_ds`

Die Erweiterbarkeit des `rtcr_core` Modules wird anhand des `rtcr_core_inc`
Modules beschrieben (`rtcr_inc/include/rtcr_core_inc`). Das dort implementierte
`Core_inc_module_ds` erbt von `Core_module_ds` (Level 2) und erweitert dies um
Funktionen für inkrementelles C/R. Desweiteren wird eine `Core_module_ds`
Klasse implementiert welche wie die `Core_module` (Level 3) von allen Level 2
Klassen erbt und damit auch von `Core_module_ds`. Auf diese Weise lassen sich
mit wenig Aufwand neue Core Module erstellen.

