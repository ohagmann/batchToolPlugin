# Changelog

Alle wesentlichen Änderungen am batchTool-Plugin werden in dieser Datei dokumentiert.

Format basiert auf [Keep a Changelog](https://keepachangelog.com/de/1.1.0/).
Versionierung: Bump bei Änderungen am Plugin-Binary (C++/GUI). Kein Bump bei reinen Änderungen an Dokumentation oder Repo-Konfiguration.

batchTool ist aus dem Plugin batchTool/openCirt 1.7.3 hervorgegangen (Repository `batchTool_openCirt`, archiviert; die Historie bis 1.7.3 steht in dessen CHANGELOG). Die GA-Planungsautomatisierung openCirt ist seit 2.0.0 ein eigenes Plugin (Repository `openCirtPlugin`).

## [2.0.0] - 2026-10-09

### Added
- **Ergebnis je Datei im Protokoll.** Hinter „File processed successfully" steht jetzt, was in der Datei passiert ist, z.B. `| Text: 1 von 10 Texten ersetzt | Attribute: 1 von 133 ersetzt`. Ließen sich Objekte nicht zum Schreiben öffnen (gesperrter Layer o.ä.), steht das mit Anzahl und Fehlercode dabei; bisher wurden solche Objekte stumm übersprungen.

### Changed
- **Eigenes Plugin.** `batchtool-<Version>.brx` (Windows) bzw. `batchtool-<Version>.lrx` (Linux) mit dem Befehl `BATCHTOOL` und der Kurzform `BT`. Die Kurzform ist ein echter Befehl und braucht keinen Eintrag in der `default.pgp`. openCirt und batchTool lassen sich gleichzeitig laden.
- Fenstertitel „batchTool <Version>", Über-Dialog mit den Befehlen.
- Die fertige Datei liegt nach dem Build unter `dist/windows/` bzw. `dist/linux/` (bisher im Beispielprojekt von openCirt).

### Removed
- Tab openCirt (jetzt eigenes Plugin). Die Tabs General, Text, Attribute, Layer und LISP sind unverändert.

### Technisches
- Einstiegspunkt `src/plugin/BatchToolPlugin.cpp` (Befehlsgruppe `BATCHTOOL_CMDS`). Die unter Windows selbst angelegte `QApplication` bleibt beim Entladen stehen, damit ein zweites Qt-Plugin (openCirt) sie weiter benutzen kann; BricsCAD räumt beim Beenden auf.
- Einstellungen weiterhin unter `QSettings("BatchProcessing", "BricsCAD_Plugin")`; bisherige Einstellungen bleiben erhalten.
- CMake-Ziel `batchtool_plugin`, Versionsmakro `PLUGIN_VERSION`, Ablage über `tools/copy_plugin.cmake`.
- `src/ui/Theming.*`, `src/windows_fix.h` und `src/mfc_stubs/` sind in beiden Repositories gleich; eine Korrektur dort gehört in beide.

### Geprüft (Linux)
- Ordner mit 8 Zeichnungen des openCirt-Beispielprojekts (Vorlagen und Quellzeichnung), Text-Tab (`DECKBLATT` → `BTTEST-TEXT`) und Attribute-Tab (Tag `OC_BEZEICHNUNG`, `Ventilator` → `BTTEST`, unsichtbare Attribute eingeschlossen) mit 1.7.3 und 2.0.0 auf zwei identischen Kopien: beide ersetzen 4 Texte und 1 Attribut, verarbeiten 8 Dateien, legen 8 Sicherungen an; die Auszüge aller Zeichnungen (`fulldump`, 2.224 Attribute) sind byteidentisch. Tabs und Feldreihenfolge wie in 1.7.3.
- openCirt und batchTool 2.0.0 in einer Sitzung geladen, batchTool entladen und erneut geladen: BricsCAD läuft weiter.
- Windows-Build am 2026-10-09 vom Anwender gebaut (MSVC 19.44, Qt 6.8.3): `batchtool-2.0.0.brx` liegt unter `dist/windows/`; Plugin unter Windows geladen.
