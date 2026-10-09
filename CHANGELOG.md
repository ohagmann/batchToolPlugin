# Changelog

Alle wesentlichen Änderungen am batchTool-Plugin werden in dieser Datei dokumentiert.

Format basiert auf [Keep a Changelog](https://keepachangelog.com/de/1.1.0/).
Versionierung: Bump bei Änderungen am Plugin-Binary (C++/GUI). Kein Bump bei reinen Änderungen an Dokumentation oder Repo-Konfiguration.

batchTool ist aus dem Plugin batchTool/openCirt 1.7.3 hervorgegangen (Repository `batchTool_openCirt`, archiviert; die Historie bis 1.7.3 steht in dessen CHANGELOG). Die GA-Planungsautomatisierung openCirt ist seit 2.0.0 ein eigenes Plugin (Repository `openCirtPlugin`).

## [2.0.0] - 2026-10-09

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
- Ordner mit 8 Zeichnungen des openCirt-Beispielprojekts (Vorlagen und Quellzeichnung), Text-Tab und Attribute-Tab aktiv, mit 1.7.3 und 2.0.0 auf zwei identischen Kopien: beide verarbeiten 8 Dateien, legen 8 Sicherungen an, die Attributauszüge (`fulldump`, 2.224 Attribute) sind byteidentisch. Tabs und Feldreihenfolge wie in 1.7.3.
- openCirt und batchTool 2.0.0 in einer Sitzung geladen, batchTool entladen und erneut geladen: BricsCAD läuft weiter.
- **Offen:** Im Test haben Text- und Attributersetzung weder mit 1.7.3 noch mit 2.0.0 etwas ersetzt (Ziele im Modellbereich, auch mit „Process invisible attributes"): die Verarbeitung meldet 8 Dateien ohne Fehler und 0 Ersetzungen. Die Engine (`DwgProcessor`) ist seit 1.7.3 unverändert, die Ursache liegt also nicht in der Teilung; siehe KNOWN_ISSUES.md Abschnitt 3.
- Windows-Build steht aus.
