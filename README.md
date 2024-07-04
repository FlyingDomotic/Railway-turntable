# Railway turntable control / Contrôle d'un pont tournant ferroviaire

[English version and French version in the same document]

Controls a railway turntable with an ESP8266, a stepper, rotation sensor through a web interface (fully asynchronous)

[ Versions françaises et anglaises dans le même document]

Contrôle un pont tournant ferroviaire au travers d'un ESP8266, avec un moteur pas à pas, un capteur de rotation, via une interface Web (complètement asynchrone)

## What's for?/A quoi ça sert ?

This code has been written to help a FabLab's member to implement a railway turntable for its HO scale railway, just clicking on an image of turntable. It's build around an ESP8266, a simple RS485/TTL interface connected to an ACCNT QY3806-485RTU rotary encoder, a NMEA 29 stepper with a DM556 driver, a MP3 sound module to complete the feeling and power supply to feed them all.

On command side, idea was to have an image or turntable, and just click on track where we want to rotate turntable. Of course, commands may also be sent to embedded web server to get turntable state and position, as well as changing them. As soon as railway structure will be completely defined, DCC support will be added.

Code can support up to 36 different tracks. All settings are done through a graphical interface with feedback. They're saved into a JSON file that can be either backup and/or manually edited, if needed. This includes stepper, encoder and MP3 module characteristics, values and offsets, RPM, inertia, sound played during the different phases... Extended list can be found latter on in this document.

Ce code a été élaboré à la demande d'un membre du FabLab, qui souhaitait commander un pont tournant sur un circuit ferroviaire à l'échelle HO, en cliquant sur une image du pont. Il est construit autour d'un ESP8266, d'une interface RS485 de base connectée à un capteur de rotation MODBUS QY3806-485RTU de chez ACCNT, d'un moteur pas à pas NMEA 29 avec un driver DM556, et des alimentations nécessaires pour tout ça.

Côté commande, l'idée est d'avoir une image du pont et de cliquer sur la voie vers laquelle on souhaite orienter le pont. Évidement, des commandes peuvent être envoyés au serveur web embarqué pour récupérer son état et sa position, mais aussi pour les modifier. Dès que la structure du réseau sera complètement définie, le support d'une interface DCC sera également ajouté.

Le code support jusqu'à 36 voies. L'ensemble du paramétrage est réalisé au travers d'une interface graphique interactive. Il est sauvegardé dans un fichier JSON qui peut être sauvegardé et/ou modifié manuellement, si besoin. Ceci concerne les caractéristiques, valeurs et décalages du moteur pas à pas, de l'encodeur et du module MP3, la vitesse de rotation, l'inertie et les sons associés aux différentes phases de la rotation. La liste complète figure plus bas.

## Driving choices / Choix retenus

Idea is to create a web server, which will display a turntable image, and detect clicks (or pushes on a touch screen). Clicks close to a point on image will rotate turntable. Rotation angle will be computed from current angle (from rotary encoder), and angle associated to the point.

Code, initially written in Python, has been rewritten in C++. It easily manages web server, GPIO to drive DM556, as well as MODBUS rotary encoder connected to RS485 interface. An HTML file will display turntable image and return click relative position (0%-100%), allowing to reduce/enlarge image without impact.

A page to manage flash content will be added. Another one will be used to set all parameters (only GPIO will be defined at compile time, as we don't care changing them). Some debug tools have also been added.

API will allow setting turntable in front of a specified track, and request information about position.

English and French languages are supported by interface, and will automatically been chosen depending on received support languages sent by browser. Other languages can be added through a simple JSON file.

L'idée est de créer un serveur Web, qui affichera une image du pont et détectera les clics (ou les appuis sur un écran tactile). Les clics proches d'un point sur l'image déclencheront la rotation du pont. L'angle de rotation sera déterminé à partir de l'angle actuel (récupéré par le capteur de position) et l'angle associé au point.

Initialement écrit en Python, le code a été réécrit en C++. Il gère facilement un serveur Web, les GPIO pour commander le DM556, ainsi que la partie MODBUS du capteur de rotation. Un fichier HTML affichera l'image et retournera la position relative (0%-100%) en x et y de l'image cliquée, pour permettre d'agrandir ou réduire l'image sans impact.

Une page gérant le contenu de la flash sera ajoutée, ainsi qu'une autre pour définir l'ensemble des paramètres (seuls les GPIO sont définis à la compilation, vu le peu d'intérêt de les changer).

Une API permettent de positionner le pont en face d'une voie données, une autre de retourner la position courante.

Le Français et l'Anglais sont supportés par le serveur, et seront choisis automatiquement en fonction de la liste des langages supportés envoyés par le navigateur. D'autres langues peuvent être ajoutés en créant un simple fichier JSON.

## Interfaces

Connection between ESP and rotary encoder is done by a simple RS485/TTL interface. A small interface, based on 4N25 optocoupler, with a limitation resistor on LED side, has been designed to connect 24V DM556 driver to 3.3V ESP.

An optional optocoupler is also available to connect a LED to signal turntable rotation. It can driver/be replaced by a relay if current is important.

An optional MP3 reader, based on DFPlayer Mini MP3 Player cr hip. It will allow to play a start sound during a settable time, a rotation sound, as long as turntable turns, and a stop sound during a settable time (all optional).

To make turntable more realistic, inertia is simulated when starting rotation and/or another when stoppoing it.

La connexion entre l'ESP et le capteur de rotation est réalisée par un module RS485/TTL simple. Une petite interface à base d'optocoupleurs 4N25, avec une résistance de limitation côté LED, a été réalisée pour connecter le DM556 alimenté en 24V, alors que l'ESP est en 3,3V.

Un optocoupleur est également disponible pour connecter une LED de rotation (optionnel). Il peut commander/être remplacé par un relai si le courant à commuter est important.

Un lecteur MP3, basé sur un chip DFPlayer Mini MP3 Player peut également être ajouté (optionnel). Il permet de jouer un son de démarrage pendant une durée paramétrable, un son de rotation tant que le pont tourne, et un son d'arrêt pendant une durée paramétrable (tous optionnels).

Pour rendre le pont tournant plus réaliste, une inertie est simulée lorsqu'on démarre le pont et/ou une seconde lorsqu'on l'arrête.

## Network/Réseau

Network connection is done using Wifi. If you specify a SSID in setup, when module will try to connect to it during a minute. Should it fail (or no SSID has been specified), then a local access point, named "PontTournant_XXXXXX" (where XXXXXX is an hexadecimal number from ESP chip ID) will be created (using password specified in settings).

La connexion au réseau est faite au travers du Wifi. Si vous spécifiez un SSID dans la configuration, le module essaiera de se connecter pendant une minute. S'il n'y arrive pas (ouu qu'il n'y a pas de SSID de défini), un point d'accès nommé "PontTournant_XXXXXX" (où XXXXXX représente un nombre hexadécimal basé sur l'Id du chip de l'ESP) sera crée (en utilisant le mot de passe donné dans la configuration).

## Prerequisites/Prérequis

VSCodium (or Visual Studio Code) with PlatformIO should be installed. You may also use Arduino IDE, as long as you read platformio.ini file to get the list of required libraries.

Vous devez avoir installé VSCodium (ou Visual Studio Code) avec PlatformIO. Vous pouvez également utiliser l'IDE Arduino, à condition d'extraire la liste des librairies requises indiquée dans le fichier platformio.ini.

## Installation

Follow these steps:

1. Clone repository in folder where you want to install it
```
cd <where_ever_you_want>
git clone https://github.com/FlyingDomotic/Railway-turntable railwayTurntable
```
2. Make ESP connections:
   - Stepper enable optocoupler is connected to D0 (optional as long as you force it permanently to low)
   - Stepper pulse optocoupler is connected to D7
   - Stepper direction optocoupler is connected to D8
   - RS485 RX signal is connected to D5
   - RS485 TX signal is connected to D6
   - Optional rotation LED optocoupler (or relay) is connected to D3
   - Optional MP3 module (serial) is connected to D1 (TX) and D2 (RX)
3. Compile and load code into ESP.
4. Load data folder into ESP flash.
5. Start ESP and connect to Wifi SSID "PontTournant_XXXXXX" (where XXXXXX represent ESP chip ID)
6. Connect to http://192.168.4.1/setup to set proper parameters (including your own Wifi SSID/Password if needed). See hereunder for details.

If needed, you can connect on serial/USB ESP link to see debug messages (at 74880 bds).

Suivez ces étapes :

1. Clonez le dépôt GitHub dans le répertoire où vous souhaitez l'installer
```
cd <là_où_vous_voulez_l'installer>
git clone https://github.com/FlyingDomotic/Railway-turntable railwayTurntable
```
2. Connecter l'ESP:
   - L'optocoupleur "stepper enable" à D0 (optionnel si vous forcez le signal à l'état bas de façon permanente)
   - L'optocoupleur "pulse" à D7
   - L'optocoupleur "direction" à D8
   - Le signal RX du module RS485 à D5
   - Le signal TX du module RS485 à D6
   - Le module MP3 optionnel à D1 (TX) et D2 (RX)
   - l'optocoupleur (ou le relais) de la LED de rotation optionnelle à D3.
3. Compiler et charger le code dans l'ESP.
4. Charger le contenu du répertoire "data" dans la flash de l'ESP.
5. Démarrer l'ESP et se connecter au SSID Wifi SSID "PontTournant_XXXXXX" (où XXXXXX représente l'ID du chip ESP)
6. Connectez-vous à http://192.168.4.1/setup pour définir les paramètres de votre configuration (incluant SSID et mot de passe de votre Wifi personnel si besoin). Voir ci-dessous pour les détails.

Si besoin, vous pouvez vous connecter sur le lien série/USB de l'ESP pour voir les messages de déverminage (à 74880 bds).

##Schema/Schéma

A schema of possible implementation can be found in schema.jpg. In this implementation, LED is replaced by a blinking module powered by 20V AC, so optocoupler is replaced by a relay.

Un schéma d'une implementation possible est disponible dans le fichier schema.jpg. Dans celle-ci, la LED est remplacée par un module clignotant alimenté en 20V alternatif, l'optocoupleur est donc remplacé par un relais.

## Settings/Paramètres

### Define turntable settings/Define turntable settings

#### Define stepper parameters/Définit les paramètres du moteur pas à pas
  - Degrees per step: Give number of degrees corresponding to one step. For example, a stepper with 200 steps per revolution will make 200/360 = 1,8° per step.
  - Micro-steps per step: Give count of micro-steps per step. Depends on driver.  Look at driver's doc and copy value choosen on driver.
  - Stepper reduction factor: Number of rotation of stepper for one rotation of turntable (1.0 if no reduction, 3.0 if 3 stepper steps are needed to get one turntable step).
  - Delay between commands (µs): Give minimal delay between 2 commands. Given in driver's doc.
  - Turntable RPM: Give desired turntable rotation speed. Often, between 1 and 2 RPM.
  - Inertia factor when starting: start rotation with speed equal to RPM divided by this number when simulating inertia. Set to 1 to ignore starting inertia.
  - Angle before full speed: angle, in degree, to accelerate from initial speed to normal RPM. Set to zero to ignore starting inertia.
  - Inertia factor when stopping: end rotation with speed equal to RPM divided by this number when simulating inertia. Set to 1 to ignore stopping inertia.
  - Angle before slowing down: angle, in degree, to decelerate from normal RPM to ending speed. Set to zero to ignore ending inertia.
  - Adjust position: permanently read turntable position and realign it with required angle when selected.

  - Degrés par pas : Indiquer le nombre de degrés correspondant à 1 pas. Par exemple, un moteur avec 200 pas par tour fera 200/360 = 1,8° par pas.
  - Micro-pas par pas : Indiquer le nombre de micro-pas par pas. Dépend des réglages du driver. Voir la doc du driver et reporter la valeur choisie sur le driver.
  - Réduction moteur : nombre de tours du moteur pour un tour du pont (1.0 s'il n'y a pas de réducteur, 3.0 si 3 pas du moteur sont nécessaires pour avoir un pas sur le pont).
  - Délai entre commandes (µs) : Indiquer le délai minimum entre 2 envois de commandes. Indiqué dans la doc du driver.
  - Tours par minute du pont : Indiquer la vitesse de rotation souhaitée du pont. En général, entre 1 et 2 tours par minute.
  - Facteur inertiel au démarrage : démarre la rotation avec une vitesse égale à la vitesse du pont divisée par ce nombre. Mettre à 1 pour ignorer l'inertie au démarrage.
  - Angle avant pleine vitesse : angle, en degrés, pour accélérer de la vitesse de démarrage à la vitesse normale. Mettre à 0 pour ignorer l'inertie au démarrage.
  - Facteur inertiel à l'arrêt : termine la rotation avec une vitesse égale à la vitesse du pont divisée par ce nombre. Mettre à 1 pour ignorer l'inertie à l'arrêt.
  - Angle avant ralentissement : angle, en degrés, pour freiner de la vitesse normale à la vitesse d'arrêt. Mettre à 0 pour ignorer l'inertie au démarrage.
  - Inverser le sens du moteur : cocher pour inverser le sens de rotation fu moteur ;-)
  - Ajuster la position : lit en permanence la position du pont et la réaligne avec l'angle demandé si coché.

#### Define encoder parameters/Définir les paramètres de l'encodeur
  - Use encoder: Tick to use encoder. If unselected, system should be manually positioned on track #1 when starting. Turntable angle will be computed instead of being read on encoder.
  - Slave id: Encoder's RS485 bus slave ID. See encoder's doc. Often 1.
  - Register id: Encoder's register number containing rotation angle. See encoder's doc. Often 0.
  - Encoder angle: Give encoder offset angle to align turntable to track #1.
  - Clockwise increment: tick when encoder increments angle clock-wisely. Invert it if image turns a way and turntable the opposite one. 

  - Utiliser l'encodeur : Cocher pour utiliser l'encodeur. Si décoché, le système doit être positionné manuellement sur la voie 1 au lancement. L'angle du pont sera calculé au lieu d'être lu sur l'encodeur.
  - Numéro de l'esclave : Numéro de l'esclave sur le bus RS485. Voir la doc de l'encodeur. Couramment 1.
  - Numéro du registre : Numéro du registre contenant l'angle de rotation. Voir la doc de l'encodeur. Souvent 0.
  - Angle de l'encodeur : Donner l'angle de l'encodeur pour aligner le pont sur la voie 1.
  - Incrément sens horaire : cocher si l'encodeur incrémente l'angle dans le sens horaire. Inverser si le pont et l'image du pont tournent en sens opposés.

#### Define sensitive zones parameters/Définir les paramètres des zones sensibles
  - Sensitive zones radius: Give sensitive zones radius, in image percentage. 0% maps centers, 50% image width.
  - Sensitive zones width: Give sensitive zone width, in image percentage. Value like 4 are common.
  - Show track number in circle: display track number in circle in normal (/) view, always displayed in (/setup)

  - Rayon des zones sensibles : Indique le rayon des zones sensibles , en % de l'image. 0% correspond au centre, 50% à la largeur de l'image.
  - Largeur des zones sensibles : Indique la largeur de la zone sensible, en % de l'image. Des valeurs autour de 4 sont courantes.
  - Afficher le numéro de voie dans un rond : ne concerne que la vue normale (/), toujours affiché dans (/setup) 

#### Define MP3 reader parameters/Définir les paramètres du module MP3
  - Activate sound: Tick to activate sound globally.
  - Before rotation sound index: MP3 index to play before rotation. Set to zero if not used.
  - Before rotation sound duration (s): Duration (in seconds) of before rotation sound.
  - Before rotation sound volume: Give desired before rotation sound level (between 0 and 30).
  - Test before rotation sound: play sound specified in "Before rotation sound index" for "Before rotation sound duration" seconds.
  - During rotation sound index: MP3 index to play during rotation. Set to zero if not used.
  - During rotation sound volume: Give desired during rotation sound level (between 0 and 30).
  - Test during rotation sound: play sound specified in "During rotation sound index" for 5 seconds.
  - After rotation sound index: MP3 index to play after rotation. Set to zero if not used.
  - After rotation sound duration (s): Duration (in seconds) of after rotation sound.
  - After rotation sound volume: Give desired after rotation sound level (between 0 and 30).
  - Test after rotation sound: play sound specified in "After rotation sound index" for "After rotation sound duration" seconds.

Note: MP3 files should be written in /MP3 folder on SD card. Filenames should be from 0001.mp3 to 9999.mp3

  - Activer le son : Cocher pour activer le son globalement.
  - Index son avant rotation : Index du MP3 à lire avant la rotation. Mettre à zéro si non utilisé.
  - Durée son avant rotation (s) : Durée (en secondes) de lecture du son avant la rotation.
  - Volume son avant rotation : Indiquer le volume sonore avant rotation désiré (entre 0 et 30).
  - Tester le son avant rotation : jour le son spécifié dans "Index son avant rotation" pendant "Durée son avant rotation" secondes.
  - Index son pendant rotation : Index du MP3 à lire pendant la rotation. Mettre à zéro si non utilisé.
  - Volume son pendant rotation : Indiquer le volume sonore pendant avant rotation désiré (entre 0 et 30).
  - Tester le son pendant rotation : jour le son spécifié dans "Index son pendant rotation" pendant 5 secondes.
  - Index son après rotation : Index du MP3 à lire avant la rotation. Mettre à zéro si non utilisé.
  - Durée son après rotation (s) : Durée (en secondes) de lecture du son après la rotation.
  - Volume son après rotation : Indiquer le volume sonore après rotation désiré (entre 0 et 30).
  - Tester le son après rotation : jour le son spécifié dans "Index son après rotation" pendant "Durée son après rotation" secondes.

Note: Les fichiers MP3 doivent être écrits dans le répertoire /MP3 de la carte SD. Les noms doivent être compris entre 0001.mp3 et 9999.mp3.

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
  - Display Java traces: tick to display JavaScript traces. Used to debug.

  - Affichage des traces du code : Cocher pour afficher les traces du code sur le lien série de l'ESP. Utilisé pour déverminer.
  - Affichage des traces des modules : Cocher pour afficher les traces des modules sur le lien série de l'ESP. Utilisé pour déverminer.
  - Affichage des traces Java : Cocher pour afficher les traces JavaScript. Utilisé pour déverminer.

#### Stall track #1/Caler la voie 1
  - Stall track#1 on turn table using +/- 1/500 buttons.
    - If (real) turntable turns clock-wisely when hitting any of the "+" buttons, invert "Invert stepper rotation".
    - If (real) turntable and image on browser are turning in opposite way, invert "Clockwise increment".
  - Click on "<--" of track #1 to save global stall (double check current angle just under image and value of line, that should be close, it's easy to change the wrong line ;-)

  - Caler la voie 1 sur le le plateau avec les touches +/- 1/500.
    - Si le pont (réel) tourne dans le sens des aiguilles d'une montre lorsqu'on appuie sur un des boutons "+", inverser "Inverser le sens du moteur"
    - Si le pont (réel) et son image sur le navigateur tournent en sens opposé, inverser "Incrément sens horaire".
  - Cliquer sur "<--" de la voie voie 1 pour mémoriser le calage global (bien vérifier l'angle affiché sous l'image et celui de la ligne, il est facile de modifier la mauvaise ligne ;-)

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
We have now to setup web interface. Take a photo of turntable, in top view. Convert it in PNG format, adjust size to display it in one part in your browser, ensuring that center of turntable is exactly in center of image. Name it "imagePont.png". To limit image size, it could be a good idea to save it with 8 or 16 bits palette. Don't forget to set transparency if needed.

Then, duplicate image and name it "plaque.png". Select the center part (the rotating one), and remove external part. A good idea is to use GIMP, and make a rounded rectangle with 100% radius, then inverting selection, and cutting selection. Again, to limit image size, it could be a good idea to save it with 8 or 16 bits palette. Don't forget to set (mandatory) transparency.

Copy both files in data folder, then download data folder into ESP. Take care of image size. Too large, you won't be able to load them into flash (limited around 1 MB).

When used, first image will be displayed as is, and second one will be overwritten in center, rotated the same angle as turntable.

Reste à paramétrer l'interface Web. Prendre une photo du pont, vue de dessus. La convertir en PNG, ajuster la taille pour qu'elle soit affichable en une seule fois dans votre navigateur, en s'assurant que le centre de la plaque est exactement au centre de l'image. La nommer "imagePont.png". Pour limiter la taille de l'image, il peut être habile de l'enregistrer avec une palette 8 ou 16 bits. Ne pas oublier la transparence si utilisée.

Dupliquer ensuite cette image en la nommant "plaque.png". Sélectionner le centre de l'image (la partie tournante), et supprimer la partie externe. Une façon de faire ça est d'utiliser GIMP, de faire une sélection rectangulaire arrondie, avec un rayon de 100%, d'inverser la sélection et de la supprimer. Encore une fois, il peut être habile de l'enregistrer avec une palette 8 ou 16 bits. Ne pas oublier la transparence obligatoire.

Copier les 2 fichier dans le répertoire data, et le charger dans la flash de l'ESP. Attention à la taille des images : trop importantes, elles ne pourront pas être chargées en flash (limitée autour d'un MO).

## Available URL/URL disponibles

  - / : root page (displays turntable allowing clicking on tracks)/page d'accueil (affiche le pont et permet de cliquer sur les voies)
  - /status : returns status in JSON format/Retourne l'état sous forme JSON
  - /goto/<nnn> : turns turntable in front of track <nnn>/tourne le pont en face de la voie <nnn>
  - /setup : display setup page/affiche la page de configuration
  - /edit : manage and edit flie system/gère et édite le système de fichier
  - /settings : returns settings in JSON format/Retourne la configuration au format JSON
  - /tstsnd/<x> : play sound, 1 being before rotation, 2 during rotation and 3 after rotation/Jour le son, 1 étant avant rotation, 2 pendant rotation et 3 après rotation.
  - /debug : display internal variables to debug/Affiche les variables internes pour déverminer
  
## Backups/Sauvegardes

You may use /settings to get a copy of settings.json file, which contains parameters. Make a copy of it to be able to restore them, when copied back into data folder, and reloaded in flash. You can also use /edit to download/upload this file.

Vous pouvez utiliser /settings pour obtenir une copie du fichier settings.json, qui contient les paramètres. Faites une copie de ce fichier pour être capable de le recharger en le copiant dans le répertoire data, et en rechargeant la flash. Vous pouvez aussi utiliser /edit pour récupérer ou recharger ce fichier.
