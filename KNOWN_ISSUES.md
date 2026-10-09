# Bekannte Probleme

Hier stehen Probleme, die beim Einsatz von batchTool auftreten, deren Ursache aber nicht im Plugin liegt, sondern in BricsCAD oder im Betriebssystem. Abschnitt 1 betrifft Windows, Abschnitt 2 Linux. Für Änderungen und behobene Fehler im Plugin selbst siehe [CHANGELOG.md](CHANGELOG.md), für Bedienfehler die [Bedienungsanleitung](sample_project/BEDIENUNGSANLEITUNG.md), Abschnitt 12.

---

## 1. GDI-Objekt-Leck in BricsCAD V26 – Absturz nach vielen geöffneten Dokumenten

**Betrifft:** BricsCAD V26 unter Windows, beobachtet mit V26.2.07 (Stand September 2026). Ob neuere Versionen betroffen sind, ist nicht geprüft.

**Betroffen ist der LISP-Tab:** er öffnet jede Zeichnung im Editor. Die Tabs Text, Attributes und Layers bearbeiten die Zeichnungen als Side-Database und öffnen keine Dokumente. (Bis Version 1.6 traf es auch den Gesamtlauf von openCirt, der seit 2.0 ein eigenes Plugin ist und ohne Editor arbeitet.)

### Symptom

Ein LISP-Lauf über sehr viele Zeichnungen oder mehrere Läufe in derselben BricsCAD-Sitzung brechen ab: BricsCAD meldet „Fehler beim Ausführen von _open" und beendet sich (APPCRASH). Im Windows-Ereignisprotokoll steht dazu ein Eintrag `GDIObjectLeak` für `bricscad.exe`. Bei einem entsprechend großen Projekt kann es bereits den ersten Lauf treffen.

### Ursache

BricsCAD gibt bei jedem Öffnen und Schließen eines Dokuments etwa zwei GDI-Objekte nicht wieder frei – rund eines je `OPEN` und eines je neu angelegter Datei. Sie bleiben bis zum Beenden von BricsCAD belegt. Das Verhalten ist in einem leeren Benutzerprofil ohne Add-ons reproduzierbar; BatchTool/openCirt, Startup-LISPs und andere Erweiterungen sind daran unbeteiligt.

Windows begrenzt GDI-Objekte auf 10.000 je Prozess. Ein LISP-Lauf öffnet jede Zeichnung einmal, mehrere Skripte nacheinander entsprechend öfter. Beobachtet wurde das Leck mit dem Gesamtlauf von openCirt bis Version 1.6: bei einem Projekt mit rund 3.200 Dateiöffnungen steht BricsCAD nach dem Lauf bei etwa 7.200 GDI-Objekten; der zweite Lauf überschreitet die Grenze. Faustformel für den Bedarf eines Laufs: 2 × Anzahl der Dateiöffnungen plus die Grundlast der Sitzung (einige hundert Objekte).

### Beobachten

Task-Manager → Reiter „Details" → Rechtsklick auf einen Spaltenkopf → „Spalten auswählen" → „GDI-Objekte" einblenden. Der Wert für `bricscad.exe` steigt während des Laufs und fällt danach nicht mehr.

### Abhilfe 1: BricsCAD vor jedem großen LISP-Lauf neu starten (Regel)

Nach jedem LISP-Lauf über viele Zeichnungen BricsCAD beenden und neu starten, bevor der nächste Lauf gestartet wird. Das genügt, solange ein einzelner Lauf unter dem Limit bleibt.

### Abhilfe 2: Windows-Limit je Prozess erhöhen (Puffer)

Für sehr große Projekte oder mehrere Läufe hintereinander lässt sich das Limit je Prozess über die Registry anheben. Erfordert Administratorrechte und einen Neustart von Windows.

| | |
|---|---|
| Schlüssel | `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows` |
| Wert | `GDIProcessHandleQuota` (REG_DWORD) |
| Standard | 10000 |
| Zulässig | 256 bis 65536 |
| Empfehlung | 20000 – deckt zwei bis drei Läufe der oben genannten Größe |

Setzen (Eingabeaufforderung als Administrator, anschließend Windows neu starten):

```
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v GDIProcessHandleQuota /t REG_DWORD /d 20000 /f
```

Prüfen:

```
reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v GDIProcessHandleQuota
```

Zurücksetzen: denselben `reg add`-Befehl mit `/d 10000` ausführen und neu starten.

Hinweise:

- Der Wert gilt für jeden Prozess, nicht nur für BricsCAD. Die gesamte Windows-Sitzung hat maximal 65.536 GDI-Handles; Werte weit über 20.000 erlauben einem einzelnen Prozess, die übrigen auszuhungern.
- `USERProcessHandleQuota` im selben Schlüssel (Standard 10000, zulässig 200 bis 18000) betrifft User-Objekte, nicht GDI-Objekte. Nur anheben, wenn im Task-Manager auch die Spalte „USER-Objekte" an das Limit kommt.
- Die Erhöhung beseitigt das Leck nicht, sie verschiebt nur die Grenze. Der Neustart vor jedem Lauf bleibt die verlässliche Regel.

### Was nicht hilft

- `SDI=1` zur Laufzeit setzen: `OPEN` öffnet weiterhin zusätzliche Dokumente, das Leck bleibt.
- Plugin entladen oder Startup-Suite leeren: das Leck tritt auch ohne jede Erweiterung auf.

---

## 2. BricsCAD V26 für Linux: LISP über viele Zeichnungen ist unzuverlässig

**Betrifft:** BricsCAD V26 unter Linux, beobachtet mit V26.2.07 (Stand 29.09.2026). Die Versionshinweise bis V26.2.08 nennen keine Korrektur. Unter Windows treten beide Fehler nicht auf.

**Betroffen ist der LISP-Tab.** Die Tabs Text, Attributes und Layers kommen ohne LISP aus.

### Symptom

- Im **LISP-Tab** laufen leichte Skripte über beliebig viele Zeichnungen. Rechenintensive Skripte brechen ab der dritten Zeichnung ab; in der Befehlszeile von BricsCAD steht `out of LISP 'Heap' memory at [gc]`.
- LISP-Code, der mit vla-Funktionen arbeitet, scheitert in einzelnen Zeichnungen mit `Automation Error DISP_E_UNKNOWNNAME` – welche es trifft, wechselt von Lauf zu Lauf.

### Ursache

Zwei voneinander unabhängige Fehler in der LISP-Umgebung von BricsCAD für Linux:

1. **LISP-Heap.** Jedes Dokument hat seine eigene LISP-Umgebung. Nur in den ersten beiden Dokumenten einer Sitzung, deren LISP-Code eine Speicherbereinigung auslöst, gelingt diese. Ab dem dritten bricht LISP mit „out of LISP 'Heap' memory" ab.
2. **vla-Objekte.** Nach einigen geöffneten und wieder geschlossenen Dokumenten liefern `vla-Open` und `vlax-ename->vla-object` gelegentlich ein Objekt des falschen Typs – ein frisch geöffnetes Dokument gilt dann zum Beispiel als Attribut.

Beide Fehler lassen sich ohne das Plugin in einem leeren Benutzerprofil nachstellen. Die Fehlerberichte für Bricsys mit Skripten zum Nachstellen liegen unter [docs/bricscad-linux-bugs](docs/bricscad-linux-bugs/README.md).

### Was unter Linux geht und was nicht

| Funktion | Linux |
|---|---|
| Tabs Text, Attributes, Layers | ja – sie bearbeiten die Zeichnungen ohne LISP |
| LISP-Tab | nur mit leichten Skripten zuverlässig |

### Abhilfe

Für den LISP-Tab gibt es im Plugin keine. Rechenintensive Skripte über viele Zeichnungen laufen unter Windows; die Zeichnungsordner sind zwischen beiden Systemen austauschbar.

### Was nicht hilft

- Die Einstellungen der LISP-Umgebung (`liblispex.so.cfg`, `LISPINIT`), `SDI=1`, ein ausdrückliches `(gc)` oder `vlax-release-object` ändern nichts.
- `NEXTFIBERWORLD=0`: danach führt BricsCAD Skripte, die beim Start mit `-b` übergeben werden, nicht mehr aus. Das trifft den PDF-Publish.
