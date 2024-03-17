# Dokumentation des Projektes

## Projektrstruktur

- `./src`
  - beinhaltet den Quellcode
- `./boards`
  - beinhaltet die Devicetree overlays
- `./prj.conf`
  - beinhaltet die Kernel-Konfiguration
- `./doc`
  - beinhaltet die Dokumtation des C Codes
- ./west
  - beinhaltet Metadaten für das Build Tool

## Installation der benötigten Tool

Zur Installation der Tools unter Linux liegen 3 Skripte bei,
die sukzessive ausgeführt werden müssen.

- `./install-zephyr-sdk.sh`
  - Installiert das Zephyr SDK
- `./create-venv.sh`
  - Richtet eine virtuelle Python Umgebung mit dem von Zephyr
  genutzten build Tool "west" ein
- `./west_init.sh`
  - Initialisiert den West workspace

Alternativ kann auch der
  [offizielle Installationsguide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
  genutzt werden.

- Außerdem werden Cmake, Python und Python-venv benötigt.

## Eintragen der Passwörter

- Aus Sicherheitsgründen sind die Benutzerpasswörter für MQTT entfernt worden.
- Diese müssen in den Dateien src/mm_config.h und mqtt-py/mqtt.py eintgetragen werden.

## Kompilieren und installieren der Firmware

- Das STM Nucleo-F756zg entwicklunsboard muss mit dem PC über USB erbunden werden.
- Mit `source .venv/bin/activate` wird die Python Umgebung mit den build tools eingebunden.
- Mit `west build -b nucleo_f756zg -p` wird der Programmcode für ein Board mit LTE-Modem kompiliert
- Mit `west build -b nucleo_f756zg -p -D -- -DOVERLAY_CONF=wifi.conf` wird der Programmcode für die Platine mit WLAN Modul kompiliert
- Anschließend kann das Programm mit `west flash` auf den
  Microcontroller übertragen werden

## Konsolenlog und Debugging

- mit `picocom /dev/ttyACM0 --baud=115200` wird der Konsolenoutput des
  Microcontrollers angezeigt.
- Mit `west debug` kann außerdem eine gdb Sitzung zum Debugging gestartet werden.

## Empfangen der Fotos über MQTT

- In mqtt-py liegt ein kleines Python tool, das es ermöglicht die Bilder
  über den MQTT Sever zu empfangen.
- Dieses lässt sich mit `python mqtt-py/mqtt.py` ausführen.
- Anschließend meldet sich das Tool am MQTT-Server an und speichert das
  neuste .jpeg in den lokalen ordner.

## Erstellen der Codedokumentation

- Mit dem tool `doxygen` kann die Dokumentation erstellt werden
- Diese liegt dann im Verzeichnis ./doc in den Dateiformaten .html und als Latex-Dokument
