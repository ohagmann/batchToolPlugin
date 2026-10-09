# Entwickler-Hinweise

## Architektur

batchTool ist als BRX-Modul aufgebaut (Windows: `.brx`, Linux: `.lrx`), das als Shared Library in BricsCAD geladen wird. Bis 1.7.3 teilte es sich das Modul mit openCirt (GA-Planungsautomatisierung); seit 2.0 sind das zwei Plugins in zwei Repositories, siehe unten „Zwei Plugins, gemeinsame Dateien". Beide entstehen aus demselben Quelltext; was nur für eine Plattform gilt, steht hinter `#ifdef _WIN32` beziehungsweise `#ifndef _WIN32`. Die GUI basiert auf Qt6 (nicht MFC), was eine moderne Widget-Bibliothek und Signal/Slot-Kommunikation ermöglicht.

### Schichtenmodell

```
┌─────────────────────────────────────────┐
│  BricsCAD V26 (Host-Applikation)        │
├─────────────────────────────────────────┤
│  plugin/   BRX Entry Point              │
│            acrxEntryPoint, BATCHTOOL/BT  │
├─────────────────────────────────────────┤
│  ui/       Qt6 GUI                      │
│            MainWindow (Tabs),            │
│            AcadColorGrid,                │
│            Theming (COLORTHEME)          │
├─────────────────────────────────────────┤
│  core/     Verarbeitungslogik           │
│            DwgProcessor (BRX API),       │
│            LispProcessExecutor (SCR)     │
├─────────────────────────────────────────┤
│  data/     Datenstrukturen              │
│            ProcessingOptions,            │
│            LispScriptManager             │
└─────────────────────────────────────────┘
```

### Kernkomponenten

**BatchProcessingEngine** (`core/DwgProcessor.cpp`) – Orchestriert die Batch-Verarbeitung. Öffnet DWG-Dateien über die BRX-Seitendatenbank (AcDbDatabase), führt Text-/Attribut-/Layer-Operationen durch und speichert die Änderungen. LISP-Verarbeitung wird an den LispProcessExecutor delegiert.

**LispProcessExecutor** (`core/LispProcessExecutor.cpp`) – Ausführung für den LISP-Tab. Generiert eine SCR-Datei mit dem Standard-CAD-Batch-Pattern (`_.OPEN → load → call → _QSAVE → _.CLOSE`) und führt sie über `acedCommand(_.SCRIPT)` in der aktuellen BricsCAD-Instanz aus. Systemvariablen (FILEDIA, CMDECHO, EXPERT) werden vor der Ausführung via `acedSetVar` gesetzt und am Ende der SCR wiederhergestellt.

**MainWindow** (`ui/MainWindow.cpp`) – Qt6-Hauptfenster mit QTabWidget. Jeder Tab konfiguriert einen Verarbeitungsbereich. Die Verarbeitung wird über die BatchProcessingEngine gestartet, Fortschritt über Signals/Slots kommuniziert. Das **Processing Log** am unteren Rand ist die einzige Protokollansicht; Tabs schreiben über das Signal `logMessage` hinein statt eine eigene zu führen.

**Theming** (`ui/Theming.cpp`) – Gleicht die Oberfläche an BricsCADs Hell-/Dunkeleinstellung an. Zwei Punkte sind dabei nicht offensichtlich:

*BricsCADs Oberfläche ist MFC-basiert, nicht Qt.* Es gibt also keine Qt-Palette des Hosts zu erben. Maßgeblich ist stattdessen die Systemvariable `COLORTHEME` (0 = dunkel, 1 = hell), gelesen über `acedGetVar`.

*Der Stil muss Fusion sein.* Der ab Qt 6.8 auf Windows 11 voreingestellte `windows11`-Stil zeichnet Flächen, Rahmen und abgerundete Ecken selbst und ignoriert die Palette weitgehend – eine dunkle Palette bliebe dort wirkungslos. Fusion respektiert die Palette vollständig und zeichnet eckig, was zugleich BricsCADs Erscheinungsbild entspricht.

Farben werden nirgends fest verdrahtet. Beschriftungen bekommen über `Theming::setRole()` eine Rolle (`Muted`, `Success`, `Warning`, `ErrorBold` …), die als Qt-Property am Widget hängt. `Theming::apply()` rechnet alle Rollen neu durch – deshalb folgt auch ein bereits gebautes Fenster einem Themenwechsel. `MainWindow::applyTheme()` ergänzt das um ein Neuzeichnen des Protokolls, dessen Farben als HTML im Text stecken und sich sonst nicht mehr ändern ließen.

Aufgerufen wird `applyTheme()` bei jedem `BATCHTOOL` – ein Themenwechsel greift also nach Schließen und erneutem Öffnen des Fensters.

### Qt 6.8+ Windows-Kompatibilität

Die Datei `src/windows_fix.h` wird über `/FI` (Force-Include) in alle Kompilierungseinheiten eingebunden, einschließlich MOC-generierter Dateien. Sie löst Konflikte zwischen Qt 6.8+ internen Windows-SDK-Includes und den BRX-SDK-Headern.

### MFC-Stubs

Das Verzeichnis `src/mfc_stubs/` enthält leere Header-Dateien (`afxwin.h`, `afxext.h` etc.), die BRX-SDK-Includes befriedigen, ohne MFC-Abhängigkeiten einzuführen. Das Plugin verwendet Qt6 statt MFC.

### Linux

BricsCAD für Linux unterscheidet sich an einigen Stellen so, dass der Quelltext darauf Rücksicht nehmen muss:

**Qt ist schon da.** BricsCAD V26 für Linux bringt Qt 6.8.2 mit und hat beim Laden des Plugins bereits eine `QApplication`. Das Plugin benutzt sie mit (`BatchToolPlugin.cpp`); unter Windows erzeugt es seine eigene. Gelöscht wird sie beim Entladen in keinem Fall: ein zweites Qt-Plugin im selben Prozess (openCirt) kann sie weiter benutzen, und BricsCAD räumt beim Beenden auf. Daraus folgt: Das Plugin muss gegen genau diese Qt-Version gebaut werden, es wird ohne RPATH gelinkt (`CMAKE_SKIP_RPATH`), und außer dem BRX-Einstiegspunkt sind alle Symbole verborgen (`-fvisibility=hidden`), damit sie nicht mit gleichnamigen des Hosts kollidieren.

**Skripte starten.** `acedCommand(RTSTR, "_.SCRIPT", …)` aus dem Plugin-Fenster heraus liefert unter Linux `RTNORM`, führt aber nichts aus. Dort geht der Start über `acDocManager->sendStringToExecute` (`LispProcessExecutor::sendScriptToEditor`).

**Dateidialoge.** BricsCAD liefert keine Plattform-Themen für Qt mit; `QFileDialog` erscheint unter Linux deshalb in der Qt-eigenen Form, nicht als GTK-Dialog. BricsCADs eigene Dialoge sind GTK-Dialoge.

**LISP.** `(getenv "TEMP")` liefert `nil`, Dateimuster in `vl-directory-files` unterscheiden Groß- und Kleinschreibung, und LISP schreibt Textdateien in Windows-1252. Schwerer wiegen zwei Fehler in BricsCAD selbst, die LISP über viele Dokumente unzuverlässig machen – siehe `KNOWN_ISSUES.md` Abschnitt 2. Die Tabs Text, Attributes und Layers arbeiten deshalb ohne LISP auf der Side-Database. Wer eine Funktion neu baut, die über viele Zeichnungen läuft, schreibt sie ebenso.

**Schrift.** BricsCAD für Linux rechnet die Unterlänge einer TrueType-Schrift kleiner als BricsCAD für Windows (Arial: 0,202 statt 0,296 der Texthöhe). Unten ausgerichteter Text rutscht deshalb nach jeder Änderung unter Linux rund ein Zehntel der Texthöhe nach unten – im Editor wie in der Side-Database. openCirt gleicht das aus (`core/TextAlignment.h` dort); der Text-Tab von batchTool ersetzt Texte ohne diesen Ausgleich, hier noch offen.

**Verzeichnisse.** `QDirIterator` liefert unter Linux in beliebiger Reihenfolge, unter Windows (NTFS) nach Namen geordnet. Wo die Reihenfolge sichtbar wird oder Nummern bestimmt, ist ausdrücklich zu sortieren.

**Header.** `inc/Platform/substitutes` des BRX SDK enthält leere Ersatz-Header für `windows.h` und andere. Sie gehören nur unter Linux in den Include-Pfad; unter Windows verdecken sie die echten Header. `src/mfc_stubs` wird umgekehrt nur unter Windows gebraucht.

## Build-Konfiguration

Die Root-`CMakeLists.txt` ist die einzige Build-Datei. Sie definiert:
- Qt6-Pfad, BRX-SDK-Pfad, BricsCAD-Installationspfad
- Compiler-Flags inkl. Force-Include von windows_fix.h
- Alle Source-/Header-Dateien explizit (kein GLOB) – neue Dateien müssen in `PLUGIN_SOURCES` bzw. `PLUGIN_HEADERS` eingetragen werden
- Linker-Konfiguration gegen brx26.lib und Qt6; unter Linux gegen `libbrx26.so`, `libTD_Alloc.so`, `libTD_Root.so` der Installation und mit `--whole-archive` gegen `libdrx_entrypoint.a`

### Lokale Pfade anpassen

`CMakeLists.txt` trägt Vorgaben für beide Plattformen:

| Variable | Windows | Linux |
|---|---|---|
| `QT6_DIR` | `C:/Qt/6.8.3/msvc2022_64` | `~/.local/opt/opencirt-toolchain/Qt/6.8.2/gcc_64` |
| `BRICSCAD_DIR` | `C:/Program Files/Bricsys/BricsCAD V26 de_DE` | `/opt/bricsys/bricscad/v26` |
| `OC_SYSROOT` | – | `~/.local/opt/opencirt-toolchain/sysroot` (lokal entpackte Entwicklerpakete, optional) |

Sie lassen sich überschreiben, ohne die Datei zu ändern: `cmake -DQT6_DIR=…` oder eine gleichnamige Umgebungsvariable. Das BRX SDK wird immer unter `external/brx_sdk` erwartet.

### Build und laufendes BricsCAD

Solange BricsCAD läuft, hält es das geladene Plugin geöffnet; MSBuild kann die Datei dann nicht ersetzen und der Build bricht mit einem Linker-Fehler ab. `CLEAN_BUILD.bat` ruft deshalb in Schritt 0 `tools/Close-BricsCAD.ps1` auf. Das Skript fordert BricsCAD zunächst regulär zum Beenden auf, sodass Speichern-Rückfragen wie gewohnt erscheinen, und fragt erst nach 30 Sekunden nach hartem Beenden. Mit `-Force` beziehungsweise `CLEAN_BUILD.bat /force` entfällt die Rückfrage.

Zu beachten: zeigt der Startup-Suite-Eintrag auf `build_windows/Release/batchtool-<Version>.brx`, löscht Schritt 1 genau diesen Ordner. Zwischen Build-Start und Build-Ende sollte BricsCAD daher nicht gestartet werden.

Unter Linux gibt es diese Sperre nicht: Der Linker ersetzt die Datei, ein laufendes BricsCAD behält die geladene Fassung bis zum nächsten Start. `CLEAN_BUILD.sh` beendet BricsCAD deshalb nicht.

## Konventionen

- C++17 Standard
- Qt-Coding-Conventions (camelCase für Methoden, m_-Prefix für Member)
- LISP-Tab: LISP-Funktionsname = Dateiname ohne Extension
- SCR-Dateien: keine Leerzeilen (Leerzeile = ENTER = stört Kommandos)
- Plattformabhängiges hinter `#ifdef _WIN32` / `#ifndef _WIN32`; der Windows-Zweig bleibt dabei unverändert
- Pfade für Dateien und Anzeige über `QDir::toNativeSeparators()`, nicht über festes Ersetzen von Schrägstrich durch Backslash
- Keine festen Farbwerte in der Oberfläche – Farben über `Theming::setRole()` oder die abgeleiteten Funktionen aus `Theming.h` beziehen, sonst bricht die Themenumschaltung

## Zwei Plugins, gemeinsame Dateien

batchTool (`batchToolPlugin`) und openCirt (`openCirtPlugin`) sind seit 2.0 getrennte Repositories mit getrennten Fenstern und getrennter Verarbeitung. Gleich sind in beiden:

- `src/ui/Theming.*` – Hell-/Dunkelthema
- `src/windows_fix.h`, `src/brx_force_include.h`, `src/mfc_stubs/` – Header-Fixes für Qt 6.8+ und das BRX SDK
- der Rumpf von `CMakeLists.txt` (Pfade, Compiler- und Linker-Einstellungen), `CLEAN_BUILD.sh`, `CLEAN_BUILD.bat`, `tools/copy_plugin.cmake`, `tools/Close-BricsCAD.ps1`
- der Aufbau des Einstiegspunkts (`src/plugin/*Plugin.cpp`): Befehlsgruppe, QApplication-Behandlung

Eine Korrektur an einer dieser Dateien gehört in beide Repositories. Beide Plugins lassen sich gleichzeitig laden; sie benutzen verschiedene Befehlsgruppen (`BATCHTOOL_CMDS`, `OPENCIRT_CMDS`) und verschiedene `QSettings`.
