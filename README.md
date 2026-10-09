# batchTool – DWG-Stapelverarbeitung für BricsCAD

Ein BRX-Plugin (C++/Qt6) für die automatisierte Massenverarbeitung von DWG-Dateien in BricsCAD V26 unter Windows und Linux. Statt Zeichnungen einzeln zu öffnen und manuell zu bearbeiten, können wiederkehrende Aufgaben über beliebig viele Dateien in einem Durchgang erledigt werden: Texte ersetzen, Attributwerte ändern, Layer-Operationen ausführen und eigene LISP-Skripte anwenden.

Bis Version 1.7.3 war batchTool mit der GA-Planungssoftware openCirt in einem Plugin (batchTool/openCirt). Seit 2.0.0 sind es zwei Plugins: **batchTool** (dieses Repository) und **[openCirt](https://github.com/ohagmann/openCirtPlugin)** für die Planungsunterlagen der Gebäudeautomation nach VDI 3814. Beide lassen sich gleichzeitig in BricsCAD laden.

> ## ⚠️ Hinweis: Bildschirmflackern (Photosensitivität)
>
> Bei Läufen im LISP-Tab werden Zeichnungen in schneller Folge im sichtbaren BricsCAD-Fenster geöffnet, verarbeitet, gespeichert und geschlossen. Dabei entsteht ein **rasches, großflächiges Flackern** des Bildschirms.
>
> Solche schnellen Hell-Dunkel-Wechsel können bei Menschen mit **photosensitiver Epilepsie** Anfälle auslösen und auch bei nicht betroffenen Personen Unwohlsein, Kopfschmerzen oder Augenbelastung verursachen. Viele Betroffene wissen nichts von ihrer Empfindlichkeit, bis ein Anfall auftritt.
>
> **Empfehlung:** Während eines laufenden LISP-Batch-Vorgangs nicht dauerhaft auf den Bildschirm schauen, das Fenster minimieren oder den Arbeitsplatz verlassen. Personen mit bekannter Photosensitivität sollten den Lauf nicht beobachten.
>
> Die Tabs Text, Attributes und Layers bearbeiten die Zeichnungen als Side-Database ohne Bildaufbau.

## Features

Das Plugin bietet fünf Funktionsbereiche als Tabs im Hauptfenster:

**General** – Quellordner, Dateifilter, Backup-Konfiguration
**Text** – Suchen/Ersetzen in DBText und MText (inkl. Regex, Mehrfachersetzung)
**Attributes** – Blockattribute gezielt ändern (nach Block, Tag, Sichtbarkeit filterbar)
**Layers** – Layer löschen, umbenennen, einfrieren, Farbe/Linientyp/Transparenz ändern
**LISP** – Eigene LISP-Skripte automatisiert auf alle DWG-Dateien anwenden

## Voraussetzungen

Windows:

- **BricsCAD V26** (Windows, 64-Bit)
- **BRX SDK V26** (separat von Bricsys zu beziehen, siehe unten)
- **Qt 6.8+** (MSVC 2022, 64-Bit)
- **CMake 3.20+**
- **Visual Studio 2022** (MSVC v143 Toolset)

Linux:

- **BricsCAD V26** (64-Bit, getestet mit V26.2.07 unter Ubuntu)
- **BRX SDK V26** – dasselbe SDK wie unter Windows, die Header sind plattformneutral
- **Qt 6.8.2** (gcc_64) – genau die Version, die BricsCAD mitbringt. Das Plugin läuft im BricsCAD-Prozess und benutzt dessen Qt-Bibliotheken; das SDK wird nur zum Bauen gebraucht
- **CMake 3.20+**, **Ninja**, **g++** mit C++17
- OpenGL-Entwicklerdateien (`libgl-dev` oder gleichwertig), die Qt beim Konfigurieren verlangt

### BRX SDK

Das BRX SDK ist proprietär und wird von Bricsys bereitgestellt. Es ist nicht Teil dieses Repositories. Nach dem Bezug muss das SDK unter `external/brx_sdk/` abgelegt werden, sodass die Struktur wie folgt aussieht:

```
external/
  brx_sdk/
    inc/         ← Header-Dateien
    inc64/
    lib64/       ← brx26.lib etc.
    docs/
```

Das SDK kann über das Bricsys Developer Network bezogen werden: https://www.bricsys.com/en-eu/developers

Liegt das SDK an anderer Stelle (etwa ein gemeinsames für openCirt und batchTool), zeigt `BRX_SDK_DIR` darauf: als CMake-Variable (`cmake -DBRX_SDK_DIR=…`) oder als Umgebungsvariable. Unter Linux genügt auch eine Verknüpfung `external/brx_sdk`; unter Windows muss der Ordner kopiert sein oder `BRX_SDK_DIR` gesetzt werden.

## Build

```cmd
CLEAN_BUILD.bat
```

Das Skript führt folgende Schritte aus:
1. Beendet laufende BricsCAD-Instanzen – solange BricsCAD läuft, hält es das geladene Plugin geöffnet und der Build scheitert am Linker. Zuerst wird BricsCAD regulär zum Beenden aufgefordert (Speichern-Rückfragen erscheinen wie gewohnt), erst nach 30 Sekunden folgt eine Rückfrage zum harten Beenden. `CLEAN_BUILD.bat /force` überspringt diese Rückfrage
2. Löscht alte Build-Artefakte
3. CMake-Konfiguration (Visual Studio 17 2022, x64, Release)
4. MSBuild-Kompilierung

Das fertige Plugin liegt anschließend unter `build_windows\Release\batchtool-<Version>.brx` (die Version stammt aus dem obersten Abschnitt von `CHANGELOG.md`, z.B. `batchtool-2.0.0.brx`) und wird zusätzlich nach `dist/windows/` gelegt; ältere Stände dort werden entfernt.

### Linux

```sh
./CLEAN_BUILD.sh
```

Das Skript löscht `build_linux`, konfiguriert mit CMake (Generator Ninja, Release) und baut. Das fertige Plugin liegt unter `build_linux/Release/batchtool-<Version>.lrx` und wird zusätzlich nach `dist/linux/` gelegt; ältere Stände dort werden entfernt. Ein laufendes BricsCAD wird nicht beendet; es behält die geladene Fassung, die neue gilt nach dem nächsten Start.

Alles Nötige lässt sich ohne Systemrechte im Benutzerverzeichnis einrichten. Vorgabe ist `~/.local/opt/opencirt-toolchain` (die Toolchain ist dieselbe wie für openCirt):

```sh
TC=~/.local/opt/opencirt-toolchain
python3 -m venv $TC/venv
$TC/venv/bin/pip install cmake ninja aqtinstall
$TC/venv/bin/aqt install-qt linux desktop 6.8.2 linux_gcc_64 -O $TC/Qt
```

Fehlen die OpenGL-Entwicklerdateien im System, genügt es, die Pakete herunterzuladen und nach `$TC/sysroot` zu entpacken (`apt-get download libgl-dev libglx-dev libopengl-dev libegl-dev libgles-dev libglvnd-dev libvulkan-dev libxkbcommon-dev libx11-dev x11proto-dev`, dann je Paket `dpkg -x <paket>.deb $TC/sysroot`).

### Manuelle Build-Schritte

```cmd
mkdir build_windows
cd build_windows
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Qt- und BricsCAD-Pfade anpassen

Die Vorgaben stehen in der Root-`CMakeLists.txt`:

| Variable | Windows | Linux |
|---|---|---|
| `QT6_DIR` | `C:/Qt/6.8.3/msvc2022_64` | `~/.local/opt/opencirt-toolchain/Qt/6.8.2/gcc_64` |
| `BRICSCAD_DIR` | `C:/Program Files/Bricsys/BricsCAD V26 de_DE` | `/opt/bricsys/bricscad/v26` |
| `OC_SYSROOT` | – | `~/.local/opt/opencirt-toolchain/sysroot` (optional) |

Abweichende Pfade lassen sich ohne Änderung der Datei setzen: beim Aufruf mit `cmake -DQT6_DIR=… -DBRICSCAD_DIR=…` oder über gleichnamige Umgebungsvariablen.

## Installation in BricsCAD

### Einmalig (zum Testen)

1. BricsCAD starten
2. Befehl: `APPLOAD`
3. Zur Datei `batchtool-<Version>.brx` (Linux: `batchtool-<Version>.lrx`) navigieren und laden
4. In der Kommandozeile erscheint: *"batchTool <Version> geladen. Befehl: BATCHTOOL (Kurzform BT)"*

### Automatisch bei jedem Start

1. `APPLOAD` aufrufen
2. Unten auf *"Inhalt..."* (Startup Suite) klicken
3. `batchtool-<Version>.brx` (Linux: `batchtool-<Version>.lrx`) zur Startup Suite hinzufügen. Nach einem Versionswechsel den Eintrag auf die neue Datei umstellen – der Dateiname trägt die Version.

## Befehle

| Befehl | Beschreibung |
|---|---|
| `BATCHTOOL` | Öffnet das Hauptfenster |
| `BT` | Kurzform von `BATCHTOOL` |

## Bedienung

### Grundsätzlicher Ablauf

1. Im Tab **General** den Quellordner mit DWG-Dateien auswählen
2. In einem oder mehreren Tabs die gewünschten Operationen konfigurieren
3. **Start** klicken
4. Fortschritt im **Processing Log** am unteren Fensterrand beobachten – dort laufen die Meldungen aller Tabs zusammen

Das Fenster übernimmt BricsCADs Hell-/Dunkeleinstellung. Maßgeblich ist die Systemvariable `COLORTHEME`; sie wird bei jedem Aufruf von `BATCHTOOL` neu gelesen. Nach einem Themenwechsel genügt es also, das Fenster zu schließen und den Befehl erneut aufzurufen.

### Tab: General

Legt fest, welche Dateien verarbeitet werden: Quellordner, Include/Exclude-Filter, Unterordner-Option und Backup-Einstellungen (Speicherort, Zeitstempel, alte Backups löschen).

### Tab: Text

Suchen/Ersetzen in allen Textobjekten. Unterstützt Regex, Groß-/Kleinschreibung, ganze Wörter und eine Ersetzungstabelle für mehrere Paare gleichzeitig. Texttypen (einzeilig, mehrzeilig, Bemaßung, Leader) sind einzeln aktivierbar.

### Tab: Attributes

Ändert Attributwerte in Block-Referenzen. Filterbar nach Blockname und Attribut-Tag. Ein leeres Suchfeld überschreibt den kompletten Attributwert. Optionen für unsichtbare, konstante und verschachtelte Attribute.

### Tab: Layers

Layer-Operationen werden als Liste definiert und in Reihenfolge ausgeführt. Schnelloperationen: Löschen (inkl. Entitäten), Einfrieren, Farbe ändern (AutoCAD-Farbgrid), Umbenennen. Die Layer-Analyse scannt alle DWGs und listet vorhandene Layer auf.

### Tab: LISP

Führt LISP-Skripte automatisiert auf alle DWGs aus. Das Plugin erzeugt eine SCR-Datei und führt sie über `_.SCRIPT` in der aktuellen BricsCAD-Instanz aus.

**Wichtige Konvention:** Der Funktionsname im LISP-Skript muss dem Dateinamen (ohne `.lsp`) entsprechen.

```
Datei: sk3.lsp → muss Funktion (defun sk3 ...) enthalten
```

Das Plugin generiert pro DWG:
```
_.OPEN "datei.dwg"
(progn (load "sk3.lsp")(princ))
(progn (sk3)(princ))
_QSAVE
_.CLOSE
```

LISP-Vorlage:
```lisp
;;; mein_skript.lsp
(defun mein_skript ( / )
  (command "_.LAYER" "_Make" "Neu" "")
  (princ "\nmein_skript: Fertig.\n")
  (princ)
)
```

Hinweise:
- Das abschließende `(princ)` verhindert unerwünschte Ausgaben in die Kommandozeile.
- Die aktuell geöffnete Zeichnung darf nicht in der Batch-Liste enthalten sein.
- Während der Verarbeitung BricsCAD nicht manuell bedienen.

## Projektstruktur

```
├── CMakeLists.txt              Root-Build-Konfiguration
├── CLEAN_BUILD.bat             Build-Skript Windows
├── CLEAN_BUILD.sh              Build-Skript Linux
├── KNOWN_ISSUES.md             Bekannte Probleme in BricsCAD (Windows: GDI-Objekt-Leck, Linux: LISP) – betreffen den LISP-Tab
├── LICENSE                     BSL 1.1 Lizenz
├── .gitignore
├── dist/
│   ├── windows/                Kompiliertes Plugin batchtool-<Version>.brx
│   └── linux/                  Kompiliertes Plugin batchtool-<Version>.lrx
├── docs/
│   ├── DEVELOPMENT.md          Entwickler-Hinweise
│   └── bricscad-linux-bugs/    Fehlerberichte zu BricsCAD für Linux, mit Skripten zum Nachstellen
├── tools/
│   ├── Close-BricsCAD.ps1      Beendet BricsCAD vor dem Build (Windows)
│   └── copy_plugin.cmake       Legt das fertige Plugin nach dist/
├── external/
│   └── brx_sdk/                BRX SDK (nicht im Repository)
└── src/
    ├── windows_fix.h           Qt 6.8+ / Windows SDK Kompatibilität
    ├── brx_force_include.h     BRX Platform-Header
    ├── core/
    │   ├── DwgProcessor.cpp/h      DWG-Verarbeitungslogik (Text, Attribute, Layer)
    │   └── LispProcessExecutor.cpp/h   In-Process LISP-Ausführung via _.SCRIPT
    ├── data/
    │   ├── ProcessingOptions.h         Datenstrukturen und Optionen
    │   └── ProcessingOptionsImpl.cpp   LISP Script Manager
    ├── mfc_stubs/              Leere MFC/ATL-Stubs (Qt-basiert, kein MFC; nur Windows)
    ├── plugin/
    │   └── BatchToolPlugin.cpp/h   BRX Entry Point, Befehle BATCHTOOL und BT
    └── ui/
        ├── MainWindow.cpp/h            Hauptfenster mit Tab-Verwaltung
        ├── Theming.cpp/h               Hell-/Dunkelthema aus BricsCAD COLORTHEME
        └── widgets/
            └── AcadColorGrid.cpp/h     AutoCAD-Farbauswahl-Widget
```

## Hinweise

- Vor dem ersten produktiven Einsatz immer mit aktivierten Backups arbeiten.
- LISP-Skripte vorher manuell an einer einzelnen DWG testen.
- Layer-Analyse vor Layer-Operationen durchführen, um Tippfehler zu vermeiden.
- Unter Linux läuft der LISP-Tab nur mit leichten Skripten zuverlässig – Ursache sind Fehler in BricsCAD für Linux, siehe [KNOWN_ISSUES.md](KNOWN_ISSUES.md) Abschnitt 2. Die Tabs Text, Attributes und Layers laufen unter Windows und Linux gleich.
- Mehrere Tabs können gleichzeitig aktiviert sein. Die Verarbeitung erfolgt in der Reihenfolge: Text → Attribute → Layer → LISP.

## Lizenz

Business Source License 1.1 (BSL 1.1) – siehe [LICENSE](LICENSE) und [ADDITIONAL_TERMS](ADDITIONAL_TERMS).

Kurzfassung: Nutzung für interne Zwecke, kommerzielle Projekte und Dienstleistungen ist erlaubt. Verkauf als eigenständiges Produkt, SaaS-Angebote und proprietäre Forks sind untersagt. Ab dem Change Date (2030-03-02) wird die Software unter AGPLv3 verfügbar. Nutzung auf eigenes Risiko – vor jedem Batch-Lauf Backups erstellen!

## Technologie

Entwickelt mit BRX SDK V26, Qt 6, C++17, CMake.
