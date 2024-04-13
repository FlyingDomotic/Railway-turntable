# Railway turntable control / Contrôle d'un pont tournant ferroviaire

[English version and French version in the same document]

Controls a railway turntable with an ESP8266, a stepper, rotation sensor through a web interface

[ Versions françaises et anglaises dans le même document]

Contrôle un pont tournant ferroviaire au travers d'un ESP8266, avec un moteur pas à pas, un capteur de rotation, via une interface Web

## What's for?/A quoi ça sert ?

This code has been written to help a FabLab's member to implement a railway turntable for it HO scale railway, just clicking on an image of turntable. Initially build around one RPI 4, with a WaveShare's RS485 CAN HAT, an ACCNT QY3806-485RTU rotary encoder, a NMEA 29 stepper with a DM556 driver, and power supply to feed them, RPI 4 and CNA HAT where replaced by an ESP8266 with a simple RS485/TTL interface.

On command side, idea was to have an image or turntable, and just click on track where we want to rotate turntable.

Ce code a été élaboré à la demande d'un membre du FabLab, qui souhaitait commander un pont tournant sur un circuit ferroviaire à l'échelle HO, en cliquant sur une image du pont. Initialement construit autour d'un RPI 4, avec une interface RS485 CAN HAT de chez waveshare, un capteur de rotation modbus QY3806-485RTU de chez ACCNT, un moteur pas à pas NMEA 29 avec un driver DM556, et les alimentations nécessaires pour tout ça, le RPI 4 et le CAN HAT ont été remplacés par un ESP8266 avec une interface RS485/TTL de base.

Côté commande, l'idée est d'avoir une image du pont et de cliquer sur la voie vers laquelle on souhaite orienter le pont.

## Driving choices / Choix retenus

Idea is to create a web server, which will display a turntable image, and detect clicks (or pushes on a touch screen). Clicks close to a point on image will rotate turntable. Rotation angle will be computed from current angle (from rotary encoder), and angle associated to the point.

Code, initially written in Python, has been rewritten in C++. It easily manages web server, GPIO to drive DM556, as well as modbus rotary encoder connected to RS485 interface. An HTML file will display turntable image and return click relative position (0%-100%), allowing to reduce/enlarge image without impact.

A page to manage flash content will be added. Another one will be used to set all parameters (only GPIO will be defined at compile time, as we don't care changing them). Some debug tools have also been added.

API will allow setting turntable in front of a specified track, and request information about position.

English and French languages are supported by interface, and will automatically been chosen depending on received support languages sent by browser. Other languages can be added through a simple JSON file.

L'idée est de créer un serveur Web, qui affichera une image du pont et détectera les clics (ou les appuis sur un écran tactile). Les clics proches d'un point sur l'image déclencheront la rotation du pont. L'angle de rotation sera déterminé à partir de l'angle actuel (récupéré par le capteur de position) et l'angle associé au point.

Initialement écrit en Python, le code a été réécrit en C++. Il gère facilement un serveur Web, les GPIO pour commander le DM556, ainsi que la partie modbus du capteur de rotation. Un fichier HTML affichera l'image et retournera la position relative (0%-100%) en x et y de l'image cliquée, pour permettre d'agrandir ou réduire l'image sans impact.

Une page gérant le contenu de la flash sera ajoutée, ainsi qu'une autre pour définir l'ensemble des paramètres (seuls les GPIO sont définis à la compilation, vu le peu d'intérêt de les changer).

Une API permettent de positionner le pont en face d'une voie données, une autre de retourner la position courante.

Le Français et l'Anglais sont supportés par le serveur, et seront choisis automatiquement en fonction de la liste des langages supportés envoyés par le navigateur. D'autres langues peuvent être ajoutés en créant un simple fichier JSON.

## Interfaces

Connection between ESP and rotary encoder is done by a simple RS485/TTL interface. A small interface, based on 4N25 optocoupler, with a limitation resistor on LED side, has been designed to connect 24V DM556 driver to 3.3V ESP.

An optional optocoupler is also available to connect a LED to signal turntable rotation.

An optional MP3 reader, based on XY-V17B/DY-SV5W chip. It will allow to play a start sound during a settable time, a rotation sound, as long as turntable turns, and a stop sound during a settable time (all optional).

La connexion entre l'ESP et le capteur de rotation est réalisée par un module RS485/TTL simple. Une petite interface à base d'opto coupleurs 4N25, avec une résistance de limitation côté LED, a été réalisée pour connecter le DM556 alimenté en 24V, alors que l'ESP est en 3,3V.

Un opto coupleur est également disponible pour connecter une LED de rotation (optionnel).

Un lecteur MP3, basé sur un chip XY-V17B/DY-SV5W peut également être ajouté (optionnel). Il permet de jouer un son de démarrage pendant une durée paramétrable, un son de rotation tant que le pont tourne, et un son d'arrêt pendant une durée paramétrable (tous optionnels).

## Network/Réseau

Network connection is done using Wifi. If you specify a SSID in setup, when module will try to connect to it during a minute. Should it fail (or no SSID has been specified), then a local access point, named "PontTournant_XXXXXX" (where XXXXXX is an hexadecimal number from ESP chip ID) will be created (using password specified in settings).

La connexion au réseau est faite au travers du Wifi. Si vous spécifiez un SSID dans la configuration, le module essaiera de se connecter pendant une minute. S'il n'y arrive pas (ouu qu'il n'y a pas de SSID de défini), un point d'accès nommé "PontTournant_XXXXXX" (où XXXXXX représente un nombre hexadécimal basé sur l'Id du chip de l'ESP) sera crée (en utilisant le mot de passe donné dans la configuration).

## Prerequisites/Prérequis

VSCodium (or Visual Studio Code) with PlatformIO should be installed. You may also use Arduino IDE, as long as you read platforio.ini file to get the list of required libraries.

Vous devez avoir installé VSCodium (ou Visual Studio Code) avec PlatformIO. Vous pouvez également utiliser l'IDE Arduino, à condition d'extraire la liste des librairies requises indiquée dans le fichier platformio.ini.

## Installation

Follow these steps:

1. Clone repository in folder where you want to install it
```
cd <where_ever_you_want>
git clone https://github.com/FlyingDomotic/railwayTurntable.git railwayTurntable
```
2. Make ESP connections:
   - Stepper enable optocoupler is connected to D0 (optional as long as you force it permanently to low)
   - Stepper pulse optocoupler is connected to D1
   - Stepper direction optocoupler is connected to D2
   - RS485 RX signal is connected to D5
   - RS485 TX signal is connected to D6
   - Optional MP3 module (one line protocol pin 4) is connected to D7 (Dip switches 1 & 3 should be set to 1, 2 to 0)
   - Optional rotation LED optocoupler is connected to D8
3. Compile and load code into ESP.
4. Load data folder into ESP flash.
5. Start ESP and connect to Wifi SSID "PontTournant_XXXXXX" (where XXXXXX represent ESP chip ID)
6. Connect to http://192.168.4.1/setup to set proper parameters (including your own Wifi SSID/Password if needed). See hereunder for details.

If needed, you can connect on serial/USB ESP link to see debug messages (at 74880 bds).

Suivez ces étapes :

1. Clonez le dépôt GitHub dans le répertoire où vous souhaitez l'installer
```
cd <là_où_vous_voulez_l'installer>
git clone https://github.com/FlyingDomotic/railwayTurntable.git railwayTurntable
```
2. Connecter l'ESP:
   - L'opto coupleur "stepper enable" à D0 (optionnel si vous forcez le signal à l'état bas de façon permanente)
   - L'opto coupleur "pulse" à D1
   - L'opto coupleur "direction" à D2
   - Le signal RX du module RS485 à D5
   - Le signal TX du module RS485 à D6
   - La pinoche 4 (one line protocol) du module MP3 optionnel à D7 (les micro interrupteurs 1 et 3 doivent être à 1, le 2 à 0)
   - l'opto coupleur de la LED de rotation optionnelle à D8.
3. Compiler et charger le code dans l'ESP.
4. Charger le contenu du répertoire "data" dans la flash de l'ESP.
5. Démarrer l'ESP et se connecter au SSID Wifi SSID "PontTournant_XXXXXX" (où XXXXXX représente l'ID du chip ESP)
6. Connectez-vous à http://192.168.4.1/setup pour définir les paramètres de votre configuration (incluant SSID et mot de passe de votre Wifi personnel si besoin). Voir ci-dessous pour les détails.

Si besoin, vous pouvez vous connecter sur le lien série/USB de lESP pour voir les messages de déverminage (à 74880 bds).

## Settings/Paramètres

### Define turntable settings/Define turntable settings

#### Define stepper parameters/Définit les paramètres du moteur pas à pas
  - Degrees per step: Give number of degrees corresponding to one step. For example, a stepper with 200 steps per revolution will make 200/360 = 1,8° per step.
  - Micro-steps per step: Give count of micro-steps per step. Depends on driver.  Look at driver's doc and copy value choosen on driver.
  - Delay between commands (µs: Give minimal delay between 2 commands. Given in driver's doc.
  - Turntable RPM: Give desired turntable rotation speed. Often, between 1 and 2 RPM.

  - Degrés par pas : Indiquer le nombre de degrés correspondant à 1 pas. Par exemple, un moteur avec 200 pas par tour fera 200/360 = 1,8° par pas.
  - Micro-pas par pas : Indiquer le nombre de micro-pas par pas. Dépend des réglages du driver. Voir la doc du driver et reporter la valeur choisie sur le driver.
  - Délai entre commandes (µs) : Indiquer le délai minimum entre 2 envois de commandes. Indiqué dans la doc du driver.
  - Tours par minute du pont : Indiquer la vitesse de rotation souhaitée du pont. En général, entre 1 et 2 tours par minute.

#### Define encoder parameters/Définir les paramètres de l'encodeur
  - Use encoder: Tick to use encoder. If unselected, system should be manually positioned on track #1 when starting. Turntable angle will be computed instead of being read on encoder.
  - Slave id: Encoder's RS485 bus slave ID. See encoder's doc. Often 1.
  - Register id: Encoder's register number containing rotation angle. See encoder's doc. Often 0.

  - Utiliser l'encodeur : Cocher pour utiliser l'encodeur. Si décoché, le système doit être positionné manuellement sur la voie 1 au lancement. L'angle du pont sera calculé au lieu d'être lu sur l'encodeur.
  - Numéro de l'esclave : Numéro de l'esclave sur le bus RS485. Voir la doc de l'encodeur. Couramment 1.
  - Numéro du registre : Numéro du registre contenant l'angle de rotation. Voir la doc de l'encodeur. Souvent 0.

#### Define sensitive zones parameters/Définir les paramètres des zones sensibles
  - Sensitive zones radius: Give sensitive zones radius, in image percentage. 0% maps centers, 50% image width.
  - Sensitive zones width: Give sensitive zone width, in image percentage. Value like 4 are common.

  - Rayon des zones sensibles : Indique le rayon des zones sensibles , en % de l'image. 0% correspond au centre, 50% à la largeur de l'image.
  - Largeur des zones sensibles : Indique la largeur de la zone sensible, en % de l'image. Des valeurs autour de 4 sont courantes.

#### Define MP3 reader parameters/Définir les paramètres du module MP3
  - Activate sound: Tick to activate sound globally.
  - Sound volume: Give desired sound level (between 0 and 30).
  - Before rotation sound index: MP3 index to play before rotation. Set to zero if not used.
  - Before rotation sound duration (s): Duration (in seconds) of before rotation sound.
  - During rotation sound index: MP3 index to play during rotation. Set to zero if not used.
  - After rotation sound index: MP3 index to play after rotation. Set to zero if not used.
  - After rotation sound duration (s): Duration (in seconds) of after rotation sound.

  - Activer le son : Cocher pour activer le son globalement.
  - Volume sonore : Indiquer le volume sonore désiré (entre 0 et 30).
  - Index son avant rotation : Index du MP3 à lire avant la rotation. Mettre à zéro si non utilisé.
  - Durée son avant rotation (s) : Durée (en secondes) de lecture du son avant la rotation.
  - Index son pendant rotation : Index du MP3 à lire pendant la rotation. Mettre à zéro si non utilisé.
  - Index son après rotation : Index du MP3 à lire avant la rotation. Mettre à zéro si non utilisé.
  - Durée son après rotation (s) : Durée (en secondes) de lecture du son après la rotation.

#### Define network parameters/Définir les paramètres réseau
  - Wifi SSID: Give Wifi existing SSId to use. If empty a "PontTournant_XXXXXX" private access point will be created.
  - Password: Give Wifi password to use (existing one if SSDI not empty, created access point one else).
  - Module name: Give module name.

  - SSID Wifi : Indiquer le SSID du réseau existant à utiliser. Si vide, un réseau "PontTournant_XXXXXX" sera crée.
  - Mot de passe : Indiquer le mot de passe du réseau Wifi à utiliser (ou à créer si SSID vide).
  - Nom du module : Indiquer le nom à donner au module.

#### Define trace parameters/Définir les paramètres de trace
  - Display code traces: Tick to display code's traces on ESP serial link. Used to debug.
  - Display modules traces: Tick to display modules' traces on ESP serial link. Used to debug.

  - Affichage des traces du code : Cocher pour afficher les traces du code sur le lien série de l'ESP. Utilisé pour déverminer.
  - Affichage des traces des modules : Cocher pour afficher les traces des modules sur le lien série de l'ESP. Utilisé pour déverminer.

#### Stall track #1/Caler la voie 1
  - Stall track#1 on turn table using +/- 1/500 buttons.
  - Click on "<--" of track #1 to save global stall.

  - Caler la voie 1 sur le le plateau avec les touches +/- 1/500.
  - Cliquer sur "<--" de la voie voie 1 pour mémoriser le calage global.

#### Stall image on track #1/Caler l'image sur la voie 1
  - Stall image  on screen over track #1 using +/- 1/500 buttons.
  - Click on "Set image on track #1" to save image position.

  - Caler l'image sur la voie 1 avec les touches +/- 1/500.
  - Cliquer sur "Calage image sur voie 1" pour mémoriser la position de l'image.

#### Input track count into "Track count"/Saisir le nombre de voies dans la zone "Nombre de voies"
  - List of tracks will be adapted giving this count.

  - La liste des voies va s'adapter en fonction de ce nombre.

#### Stall all tracks (except#1 already stalled/Caler toutes les voies (sauf la 1 déjà calée)
  - Stall turntable in front on desired track
  - Click on "<--" of desired track  to save position.

  - Caler l'entrée du pont  en face de la voie.
  - Cliquer sur le bouton "<--" de la voie correspondante pour la mémoriser.

### Image used by Web interface setting/Image utilisée par l'interface Web
We have now to setup web interface. Take a photo of turntable, in top view. Convert it in PNG format, adjust size to display it in one part in your browser, ensuring that center of turntable is exactly in center of image. Name it "imagePont.png".

Then, duplicate image and name it "plaque.png". Select the center part (the rotating one), and remove external part. A good idea is to use GIMP, and make a rounded rectangle with 100% radius, then inverting selection, and cutting selection.

Copy both files in data folder, then download data folder into ESP.

When used, first image will be displayed as is, and second one will be overwritten in center, rotated the same angle as turntable.

Reste à paramétrer l'interface Web. Prendre une photo du pont, vue de dessus. La convertir en PNG, ajuster la taille pour qu'elle soit affichable en une seule fois dans votre navigateur, en s'assurant que le centre de la plaque est exactement au centre de l'image. La nommer "imagePont.png".

Dupliquer ensuite cette image en la nommant "plaque.png". Sélectionner le centre de l'image (la partie tournante), et supprimer la partie externe. Une façon de faire ça est d'utiliser GIMP, de faire une sélection rectangulaire arrondie, avec un rayon de 100%, d'inverser la sélection et de la supprimer.

Copier les 2 fichier dans le répertoire data, et le charger dans la flash de l'ESP.

## Available URL/URL disponibles

  - / : root page (displays turntable allowing clicking on tracks)/page d'accueil (affiche le pont et permet de cliquer sur les voies)
  - /status : returns status in JSON format/Retourne l'état sous forme JSON
  - /goto/<nnn> : turns turntable in front of track <nnn>/tourne le pont en face de la voie <nnn>
  - /setup : display setup page/affiche la page de configuration
  - /edit : manage and edit flie system/gère et édite le système de fichier
  - /settings : returns settings in JSON format/Retourne la configuration au format JSON
  - /debug : display internal variables to debug/Affiche les variables internes pour déverminer