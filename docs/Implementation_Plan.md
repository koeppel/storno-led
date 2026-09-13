# ESP32 Light Tubes: Implementierungsplan

## Ziel

Die Light Tubes werden mit einem ESP32, Arduino/C++ und PlatformIO gesteuert.
Die 24-V-WS2814-RGBW-Strips erhalten ihre Daten ueber FastLED. DMX512 ist die
primaere Steuerung; ein OLED und Rotary Encoder ermoeglichen die lokale
Konfiguration. WLAN und MQTT gehoeren nicht zum ersten Stand.

Die geplante Stromtopologie fuehrt 60 V von einem zentralen 1200-W-Netzteil zu
jeder Tube. In jeder der zehn Tubes wandelt ein lokaler Step-Down-Wandler auf
24 V fuer die LEDs. Jede der acht Tubes besitzt zwei LED-Strips mit je 13
Segmenten. Ein Segment umfasst sechs physische LEDs. Die zwei Strips sind
parallel angesteuert: Sie erhalten dasselbe Datensignal und bilden damit
gemeinsam 13 logische, adressierbare Pixel pro Tube. Die Tubes werden ueber
`DOUT` nach `DIN` seriell als Daisy-Chain verbunden; eine durchgehende Kette
aller acht Tubes hat damit 104 logische Pixel. Jede Tube hat insgesamt 156 und
der Gesamtaufbau 1.248 physische LEDs. Der Datenpegelwandler und die
LED-Datenleitung muessen eine gemeinsame Bezugserde mit der lokalen 24-V-Seite
haben.

## Phase 1: PlatformIO Hello World (abgeschlossen)

Ziel: Die komplette Entwicklungsstrecke von macOS ueber USB bis zum ESP32
funktioniert nachweislich.

- PlatformIO in VS Code installieren und ein ESP32-Arduino-Environment fuer das
  vorhandene generische ESP32-Devboard anlegen (`ESP32-WROOM-32`, Aufdruck
  `HW-394`, USB-C).
- Fuer den ersten Test die 60-V-Versorgung ausgeschaltet und physisch getrennt
  lassen. Den ESP32 ausschliesslich ueber USB versorgen; erst nach erfolgreichem
  Hello-World-Test die Versorgung des Controllerboards elektrisch pruefen.
- Ein minimales `src/main.cpp` erstellen, das `Serial.begin(115200)` aufruft und
  regelmaessig `Hello from ESP32` ausgibt.
- Eine vorhandene, steuerbare Board-LED nur dann parallel blinken lassen, wenn
  ihr GPIO am konkreten Board bestaetigt ist. Die sichtbare rote `PWR`-LED ist
  lediglich eine Versorgungsanzeige und kein geeigneter Funktionstest.
- Das Projekt bauen, per USB flashen und den seriellen Monitor mit 115200 Baud
  oeffnen.

**Akzeptanzkriterien**

- [x] Der PlatformIO-Build ist erfolgreich.
- [x] Der Upload auf den ESP32 ist erfolgreich.
- [x] Nach Reset erscheint `Hello from ESP32` im seriellen Monitor.
- [x] Die Board-LED blinkt, falls sie angeschlossen und ihr GPIO bekannt ist.

**Ergebnis (2026-09-13)**

- PlatformIO IDE ist in VS Code installiert; das Projekt verwendet
  `espressif32@6.10.0`, `esp32dev` und das Arduino-Framework.
- Build und Upload auf `/dev/cu.usbserial-1440` waren erfolgreich. Der ESP32
  wurde als `ESP32-D0WD-V3` erkannt.
- Der serielle Monitor liefert bei 115200 Baud regelmaessig `Hello from ESP32`.
- Die steuerbare Board-LED ist bestaetigt: `GPIO2` (`D2`) blinkt im
  Sekundentakt. Die rote `PWR`-LED bleibt eine reine Versorgungsanzeige.

## Phase 2: Hardware-Freigabe und Projektgrundlage

Ziel: Alle hardwareabhaengigen Werte sind vor der LED- und DMX-Integration
eindeutig erfasst.

**Status (2026-09-13): in Arbeit**

- [x] ESP32-Devboard, PlatformIO-Environment und steuerbare Board-LED
  verifiziert.
- [x] Acht unabhängige LED-Datenausgaenge und ihre Reihenfolge erfasst.
- [x] Startaufbau mit einer Röhre pro Ausgang sowie Ziel von maximal drei
  Röhren je Ausgang als Daisy-Chain festgelegt.
- [x] Rotary-Encoder-Pins erfasst: `CLK = GPIO33`, `DT = GPIO34`,
  `SW = GPIO35`.
- [x] DMX-Empfangspfad `RO = GPIO3` erfasst.
- [ ] OLED per I2C-Scan nachweisen und Encoder-Eingaben praktisch testen.
- [ ] DMX-Steuerpins und die Kollisionsfreiheit von `RO` mit dem USB-UART
  elektrisch prüfen.
- [ ] Strompfad, Pegelwandler, Sicherungen sowie LED- und Wandlerlast messen
  und freigeben.

- ESP32-Variante sowie GPIOs fuer LED-Ausgaenge, DMX, OLED und Encoder erfassen.
- Den RS-485-Transceiver und seine Pins identifizieren. Der Empfangsausgang
  `RO` ist mit dem Standard-RX-Pin des ESP32 (`U0RXD`, GPIO 3) verbunden; die
  DMX-Daten werden dort eingelesen.
- Fuer den reinen Empfang `DE` auf `LOW` und `/RE` auf `LOW` setzen oder beide
  Signale, falls fest verdrahtet, mit GND verbinden. Dadurch ist der
  Treiberausgang deaktiviert und der Empfaenger aktiviert.
- Pruefen, ob der TX-Ausgang des USB-UART-Wandlers ebenfalls GPIO 3 treibt. Er
  darf den `RO`-Ausgang des RS-485-Transceivers nicht elektrisch belasten oder
  mit ihm kollidieren. Fuer den DMX-Betrieb keine serielle Diagnose ueber
  `Serial` bei 115200 Baud erwarten; DMX verwendet 250000 Baud und belegt
  UART0 als Empfangsschnittstelle.
- Pro Ausgang Pixelzahl und Zahl der angeschlossenen Roehren festlegen. Der
  Startaufbau verwendet eine Röhre pro Ausgang, maximal sind drei Röhren je
  Ausgang als `DOUT`-zu-`DIN`-Kette geplant. Fuer ein identisches Muster auf
  allen Röhren einer Kette muss der Firmware-Puffer die 13 Segmentwerte je
  Röhre wiederholen.
- Die Auslegung am finalen Aufbau pruefen: Ein lokaler Step-Down-Wandler ist in
  jeder Röhre verbaut. Die reale Stromaufnahme der zwei Strips pro Röhre messen
  und den Wandler fuer Dauerbetrieb mit Reserve auslegen oder die globale
  Helligkeit so begrenzen, dass seine Nennleistung nicht erreicht wird.
- Die Meterangabe in der Einkaufsliste gegen das Strip-Datenblatt abgleichen:
  Zwei Strips mit je $13 \times 6 = 78$ physischen LEDs ergeben 156 LEDs pro
  Röhre. Bei der bisher angenommenen Dichte von 60 LEDs/m waeren das 2,6 m
  Strip, nicht 1,5 m. Fuer die Firmware sind die bestaetigten 13 parallelen
  Segmente pro Röhre massgeblich.
- Gemeinsame Masse, 60-V-Verteilung, lokale 24-V-Einspeisung, Sicherungen,
  3,3-V- und 5-V-Versorgung sowie den 5-V-Datenpegelwandler pruefen.
- Steckverbinder und dreiadrige Leitung eindeutig dokumentieren. Wegen 60 V
  duerfen Pinbelegung, Strombelastbarkeit und Verpolschutz erst nach der
  elektrischen Freigabe verwendet werden.
- `platformio.ini` mit ESP32-Board, Arduino-Framework und festen Versionen fuer
  FastLED, Display-, Encoder- und DMX-Bibliothek anlegen.
- Eine zentrale Hardwarekonfiguration fuer alle Pins und Groessen anlegen.

**Akzeptanzkriterien**

- Pinout und Strompfad sind dokumentiert.
- Die maximale LED-Leistung jeder Tube ist gemessen oder aus dem Datenblatt
  bestaetigt und der lokale Step-Down-Wandler ist dafuer ausreichend dimensioniert.
- Abhaengigkeiten bauen erfolgreich.
- Das OLED wird bei einem I2C-Scan gefunden.
- Der Encoder liefert lesbare Dreh- und Tasteneingaben.

**Bereits bestaetigt**

- Controller: generisches ESP32-Devboard mit ESP32-WROOM-32 und USB-C
  (Platinendruck `HW-394`).
- Steuerbare Board-LED: `GPIO2` (`D2`), aktiv bei `HIGH`.
- Acht unabhängige LED-Datenausgaenge. Von links nach rechts, je Spalte zuerst
  oben und dann unten: `GPIO26`, `GPIO27`, `GPIO18`, `GPIO19`, `GPIO23`,
  `GPIO13`, `GPIO16`, `GPIO17`.
- Aufbauplanung: zunächst eine Röhre pro Ausgang; maximal drei Röhren pro
  Ausgang als Daisy-Chain. Die Daten für jede Röhre einer Kette werden im
  Firmware-Puffer identisch wiederholt.
- Rotary Encoder: `CLK = GPIO33`, `DT = GPIO34`, `SW = GPIO35`.
- Das Board kann fuer Phase 1 mit dem PlatformIO-Environment `esp32dev`
  angesprochen werden. Die finale Auswahl wird nur dann angepasst, wenn die
  tatsaechliche Flash-Groesse oder weitere Boarddetails davon abweichen.
- Auf dem Controllerboard sind Anschluesse fuer `PWR In`, `DMX In`, `OLED` und
  `Encoder` sichtbar.
- Die DMX-Schnittstelle ist ein RS-485-Transceivermodul mit den Signalen `DI`,
  `DE`, `RE` und `RO`. `RO` ist der Empfangsausgang zum ESP32; `DE` und `RE`
  bestimmen die Sende-/Empfangsrichtung. Fuer den reinen DMX-Empfang werden sie
  erst nach Pruefung der Verdrahtung fest auf Empfang konfiguriert.
- `RO` ist mit dem Standard-RX-Pin des ESP32 (`U0RXD`, GPIO 3) verbunden. Dort
  werden die DMX-Daten vom RS-485-Transceiver eingelesen.
- Fuer reinen Empfang muss `DE` auf `LOW` und `/RE` auf `LOW` liegen (alternativ
  beide fest an GND). Der Transceiver sendet dann nicht und sein Empfangsausgang
  `RO` ist aktiv.
- GPIO 3 wird zugleich vom USB-UART-Wandler als `U0RXD` verwendet. Vor dem
  DMX-Betrieb muss dessen Ausgang auf mögliche Kollisionen mit `RO` geprüft
  werden. Die USB-serielle Diagnose mit 115200 Baud ist nicht gleichzeitig mit
  DMX auf UART0 nutzbar; DMX wird mit 250000 Baud empfangen.
- Vier DIP-Schalter (`U5` bis `U8`) sind vorhanden. Ihre GPIO-Zuordnung und
  Funktion sind noch nicht bestaetigt.
- Der Controller hat einen DC/DC-Wandler mit der Kennzeichnung `60V/3A`. Seine
  tatsaechliche Ausgangsspannung fuer ESP32 und Peripherie muss vor Anschluss
  oder Messung der Signalleitungen verifiziert werden.
- Je Röhre ist ein lokaler 60-V-zu-24-V-DC/DC-Wandler vorhanden (Modulaufdruck
  `CR-6030L`), der die beiden LED-Strips dieser Röhre versorgt.

## Phase 3: LED-Smoketest mit FastLED

Ziel: Ein kurzer WS2814-RGBW-Strip reagiert korrekt und sicher auf den ESP32.

- Einen `LedOutput` fuer einen einzelnen Datenpin erstellen.
- Mit FastLED die Muster Rot, Gruen, Blau, Weiss und gedimmtes Weiss senden.
- Den kompatiblen FastLED-Chipsatz und die korrekte RGBW-Kanalreihenfolge
  bestimmen.
- Falls FastLED RGBW nicht direkt fuer den verwendeten Strip abbildet, die
  Uebertragung in einer eigenen RGBW-Adapterklasse kapseln.
- Helligkeits- und Strombegrenzung einbauen.
- Pro Röhre eine Datenkette mit 13 adressierbaren Segmenten verwenden. Die zwei
  Strips erhalten das Signal parallel; das Testmuster muss beide Seiten der
  Röhre sichtbar und gleich ansteuern.
- Fuer eine Daisy-Chain den LED-Puffer auf 13 Pixel je angeschlossener Tube
  erweitern; eine Kette mit acht Tubes verwendet 104 logische Pixel.

**Akzeptanzkriterien**

- Alle vier Farbkanäle entsprechen dem jeweiligen Testmuster.
- Es gibt keine Framefehler oder sichtbares Flackern.
- Die Strombegrenzung wirkt nachvollziehbar.

## Phase 4: Konfiguration und lokale Bedienung

Ziel: Einstellungen lassen sich ohne Computer sicher aendern und bleiben nach
einem Neustart erhalten.

- Standardwerte und Konfigurationsvalidierung implementieren.
- Einstellungen als JSON auf LittleFS oder in NVS speichern.
- Ein erweiterbares OLED-Menue mit entprelltem Rotary Encoder implementieren.
- Folgende Einstellungen anbieten: DMX-Startadresse, Modus, globale Helligkeit,
  Ausgabesynchronisierung sowie Roehren- und Pixelanzahl je Ausgang.

**Akzeptanzkriterien**

- Einstellungen koennen bearbeitet und gespeichert werden.
- Nach Neustart werden die gespeicherten Einstellungen geladen.
- Ungueltige oder fehlende Konfiguration faellt auf sichere Standardwerte zurueck.

## Phase 5: Lokale Modi und Effekte

Ziel: Die Tubes funktionieren auch ohne DMX als Hintergrundbeleuchtung.

- Einen zentralen Modus-Dispatcher implementieren.
- Statische RGBW-Farbe mit Helligkeit implementieren.
- Nichtblockierende Effekte auf Basis von `millis()` implementieren: Pulsieren
  und Farbwechsel.
- Synchronisierte Ausgaenge und getrennte Phasen je Ausgang konfigurierbar
  machen.

**Akzeptanzkriterien**

- Jeder Modus reagiert auf Menueaenderungen.
- Die Effekte blockieren weder Bedienung noch LED-Ausgabe.
- Der Betrieb bleibt mindestens 30 Minuten stabil.

## Phase 6: DMX-Empfang und Kanalmap

Ziel: Die Tubes reagieren als zuverlaessiger DMX-Empfaenger.

- Eine zur ESP32-Variante und zum RS-485-Transceiver passende Arduino-DMX-
  Receive-Bibliothek auswaehlen.
- Einen `DmxReceiver` bauen, der 512-Kanal-Snapshots mit Zeitstempel bereitstellt.
- Einen RGBW-Direktmodus mit vier DMX-Kanaelen implementieren.
- Einen erweiterten Modus mit Effekt-ID, Farbe, Helligkeit und Geschwindigkeit
  als dokumentierte Kanalmap implementieren.
- Einen DMX-Timeout mit Rueckfall auf den lokalen Modus vorsehen.

**Akzeptanzkriterien**

- Startadresse und Kanalzuordnung funktionieren mit einem DMX-Sender.
- Der RGBW-Direktmodus steuert die erwarteten Farbwerte.
- Bei Signalverlust greift der lokale Fallback innerhalb des festgelegten
  Timeouts.

## Phase 7: Systemintegration und Belastungstest

Ziel: Der vollstaendige Aufbau ist fuer den Einsatz zuverlaessig und elektrisch
sicher.

- DMX, Bedienung, Rendering und `FastLED.show()` im Hauptloop mit festen
  Intervallen zusammenfuehren.
- Serielle Diagnosen und eine sichtbare Fehleranzeige ergaenzen.
- Zuerst einen Ausgang, dann eine Röhre, eine Daisy-Chain und danach den
  vollstaendigen Aufbau testen.
- Native PlatformIO-Tests fuer Konfigurationsvalidierung, DMX-Kanalmap und
  Effektwerte ergaenzen.
- Unter realistischer Last Spannungsabfall, Datenintegritaet, Stromaufnahme und
  Erwärmung messen.

**Akzeptanzkriterien**

- Alle lokalen und DMX-Modi laufen stabil im Zielaufbau.
- Die elektrische Last bleibt innerhalb der Auslegung.
- Alle PlatformIO-Tests bestehen.

## Noch offene Hardwaredaten

- Konkrete GPIO-Belegung fuer LED-Ausgaenge, DMX, OLED und Encoder
- Verdrahtung und GPIO-Zuordnung fuer `DI`, `DE` und `RE` des
  RS-485-Transceivers
- Elektrische Verträglichkeit des USB-UART-Wandlers mit dem an GPIO 3
  angeschlossenen `RO`-Ausgang
- Zuordnung des ESP32-LED-GPIOs zur `DIN`-Leitung der ersten Tube sowie die
  `DIN`/`DOUT`-Pinbelegung fuer die Daisy-Chain
- Tatsaechliche Striplaenge und Leistungsaufnahme pro Röhre; sie weicht
  rechnerisch von der bisherigen Meterangabe ab
- Ausgangsspannung des Controller-DC/DC-Wandlers sowie seine Verbindung zu
  ESP32-Versorgung und 5-V-Datenpegelwandler
- Finale 60-V-Verteilung, Einspeisung, Steckverbinder-Pinbelegung und
  Sicherungswerte