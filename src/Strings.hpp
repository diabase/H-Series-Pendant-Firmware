/*
 * Strings.hpp
 *
 *  Created on: 27 Feb 2017
 *      Author: David
 *
 * The encoding used for this file must be UTF-8 to ensure that accented characters are displayed correctly.
 */

#ifndef SRC_STRINGS_HPP_
#define SRC_STRINGS_HPP_

#include "ecv.h"
#include "UserInterfaceConstants.hpp"

#define CSTRING const char * const array
#define Newline			"\n"
#define DegreeSymbol	"\u00B0"

constexpr unsigned int NumLanguages = 5;
constexpr unsigned int NumStatusStrings = 14;

struct StringTable
{
	// Language name
	CSTRING languageName;

	// Main page strings
	CSTRING control;
	CSTRING print;
	CSTRING console;
	CSTRING setup;
	CSTRING pendant;
	CSTRING current;
	CSTRING active;
	CSTRING standby;
	CSTRING move;
	CSTRING extrusion;
	CSTRING macro;
	CSTRING stop;

	// Print page
	CSTRING extruderPercent;
	CSTRING speed;
	CSTRING fan;
	CSTRING timeRemaining;
	CSTRING file;
	CSTRING filament;
	CSTRING layer;
	CSTRING notAvailable;
	CSTRING pause;
	CSTRING babystep;
	CSTRING resume;
	CSTRING cancel;
	CSTRING set;

	// Setup page
	CSTRING volume;
	CSTRING calibrateTouch;
	CSTRING mirrorDisplay;
	CSTRING invertDisplay;
	CSTRING theme;
	CSTRING brightnessDown;
	CSTRING brightnessUp;
	CSTRING saveSettings;
	CSTRING clearSettings;
	CSTRING saveAndRestart;
	CSTRING infoTimeout;

	// Pendant root
	CSTRING backToNormal;
	CSTRING jog;
	CSTRING offset;
	CSTRING job;

	// Pendant Jog page
	CSTRING axis;
	CSTRING movement;
	CSTRING currentLocation;
	CSTRING homing;
	CSTRING measureZ;
	CSTRING confirmMeasureZ;
	CSTRING tools;
	CSTRING probe;

	// Pendant Offset page
	CSTRING probeWorkpiece;
	CSTRING findCenterOfCavity;
	CSTRING touchOff;
	CSTRING toolOffset;
	CSTRING zeroWorkpiece;

	// Pendant Job page
	CSTRING noJob;
	CSTRING extruderShort;
	CSTRING spindleRPM;
	CSTRING heatControl;

	// Misc
	CSTRING confirmFactoryReset;
	CSTRING confirmFileDelete;
	CSTRING areYouSure;
	CSTRING touchTheSpot;
	CSTRING moveHead;
	CSTRING extrusionAmount;
	CSTRING extrusionSpeed;
	CSTRING extrude;
	CSTRING retract;
	CSTRING babyStepping;
	CSTRING currentZoffset;
	CSTRING message;
	CSTRING messages;
	CSTRING firmwareVersion;
	CSTRING response;

	// File popup
	CSTRING filesOnCard;
	CSTRING macros;
	CSTRING error;
	CSTRING accessingSdCard;
	CSTRING fileName;
	CSTRING fileSize;
	CSTRING layerHeight;
	CSTRING objectHeight;
	CSTRING filamentNeeded;
	CSTRING generatedBy;
	CSTRING lastModified;
	CSTRING estimatedPrintTime;
	CSTRING simulatedPrintTime;
	CSTRING simulate;

	// Printer status strings
	CSTRING statusValues[NumStatusStrings];

	// Colour theme names
	CSTRING colourSchemeNames[NumColourSchemes];

	// Display dimmer types
	CSTRING displayDimmingNames[(unsigned int)DisplayDimmerType::NumTypes];
};

const StringTable LanguageTables[NumLanguages] =
{
	// English
	{
		// ISO-639.1 language code
		"en",

		// Main page strings
		"Control",
		"Print",
		"Console",
		"Setup",
		"Pendant",
		"Current" THIN_SPACE DEGREE_SYMBOL "C",
		"Active" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Move",
		"Extrusion",
		"Macro",
		"STOP",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Speed ",							// note space at end
		"Fan ",								// note space at end
		"Time left: ",
		"file ",							// note space at end
		", filament ",						// note space at end
		", layer ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Resume",
		"Cancel",
		"Set",

		// Setup page
		"Volume ",							// note space at end
		"Calibrate touch",
		"Mirror display",
		"Invert display",
		"Theme",
		"Brightness -",
		"Brightness +",
		"Save settings",
		"Clear settings",
		"Save & Restart",
		"Info timeout ",					// note space at end

		// Pendant root
		"Panel",
		"Jog",
		"Offset",
		"Job",

		// Pendant Jog tab
		"Axis",
		"Movement",
		"Current",
		"Homing",
		"Measure Z",
		"Confirm Measure Z",
		"Tools",
		"Probe",

		// Pendant Offset tab
		"Probe Workpiece",
		"Find Center of Cavity",
		"Touch-off Plate",
		"Set Tool Offset",
		"Zero Axes in active WCS",

		// Pendant Job tab
		"No job running",
		"Extrd. ",
		"Spindle RPM ",
		"Heat Control",

		// Misc
		"Confirm factory reset",
		"Confirm file delete",
		"Are you sure?",
		"Touch the spot",
		"Move head",
		"Extrusion amount (mm)",
		"Speed (mm/s)",
		"Extrude",
		"Retract",
		"Baby stepping",
		"Current Z offset: ",
		"Message",
		"Messages",
		"Panel Due firmware version ",	// note space at end
		"Response",

		// File popup
		"Files on card ",				// note the space on the end
		"Macros",
		"Error ",						// note the space at the end
		" accessing SD card",			// note the space at the start
		"Filename: ",
		"Size: ",
		"Layer height: ",
		"Object height: ",
		"Filament needed: ",
		"Sliced by: ",
		"Last modified: ",
		"Estimated print time: ",
		"Simulated print time: ",
		"Simulate",

		// Printer status strings
		{
			"Connecting",
			"Idle",
			"Printing",
			"Halted",
			"Starting up",
			"Paused",
			"Busy",
			"Pausing",
			"Resuming",
			"Firmware upload",
			"Changing tool",
			"Simulating",
			"Standby",
			"Initializing",
		},

		// Theme names
		{
			"Light theme",
			"Dark theme 1",
			"Dark theme 2"
		},

		// Display dimming types
		{
			"Never dim",
			"Dim if idle",
			"Always dim"
		}
 	},

	// German
	{
		// ISO-639.1 language code
		"de",

		// Main page strings
		"Steuerung",
		"Druck",
		"Konsole",
		"Setup",
		"Handsteuerung",
		"Istwert" THIN_SPACE DEGREE_SYMBOL "C",
		"Aktiv" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Bewegung",
		"Extrusion",
		"Makro",
		"STOP",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Tempo ",							// note space at end. Was "Geschwindigkeit " but that is too long to fit in the space available.
		"Lüfter ",							// note space at end
		"Restzeit: ",
		"Datei ",							// note space at end
		", Filament ",						// note space at end
		", Layer ",							// note space at end. "Schicht" is too long.
		"n/v",
		"Pause",
		"Einzelschritt",
		"Fortsetzen",
		"Abbrechen",
		"Set",

		// Setup page
		"Lautstärke ",						// note space at end
		"Touch kalibrieren",
		"Anzeige spiegeln",
		"Anzeige umkehren",
		"Darstellung",
		"Beleuchtung  -",
		"Beleuchtung  +",
		"Einstllgen sichern",
		"Werks-Reset",
		"Sichern & Reboot",
		"Info timeout ",					// note space at end

		// Pendant root
		"Panel",
		"Jog",
		"Offset",
		"Job",

		// Pendant Jog tab
		"Achse",
		"Bewegung",
		"Aktuell",
		"Homing",
		"Z messen",
		"Confirm Measure Z",
		"Tools",
		"Probe",

		// Pendant Offset tab
		"Werkstück finden",
		"Hohlraummitte finden",
		"Tastplatte",
		"Werkzeug Offset",
		"Achsen in WCS Nullen",

		// Pendant Job tab
		"Kein laufender Job",
		"Extrd. ",
		"Spindel Umin ",
		"Heizungskontrolle",

		// Misc
		"Alle Einstellungen zurücksetzen",
		"Die Datei wird gelöscht",
		"Sind sie sicher?",
		"Bitte auf den Punkt tippen",
		"Kopf bewegen",
		"Extrusionsmenge (mm)",
		"Geschwindigkeit (mm/s)",
		"Extrudieren",
		"Zurückziehen",
		"Einzelschritte",
		"Aktueller Z-Versatz: ",
		"Nachricht",
		"Nachrichten",
		"Panel Due Firmwareversion ",	// note space at end
		"Antwort",

		// File popup
		"Dateien auf Karte ",			// note the space on the end
		"Makros",
		"Fehler ",						// note the space at the end
		" beim Zugriff auf SD-Karte",	// note the space at the start
		"Dateiname: ",
		"Größe: ",
		"Schichthöhe: ",
		"Objekthöhe: ",
		"Benötigtes Filament: ",
		"Erzeugt mit: ",
		"Letzte Änderung: ",
		"Geschätzte Druckdauer: ",
		"Errechnete Druckdauer: ",
		"Simulieren",

		// Printer status strings
		{
			"Verbinde",
			"Leerlauf",
			"Druckt",
			"Angehalten",
			"Starte",
			"Pausiert",
			"Beschäftigt",
			"Pausiere",
			"Fortsetzen",
			"Firmware-Upload",
			"Wechsle Tool",
			"Simuliert",
			"Stand-by"
		},

		// Theme names
		{
			"Anzeige hell",
			"Anzeige inv. 1",
			"Anzeige inv. 2"
		},

		// Display dimming types
		{
			"Dimmen aus",
			"Dim bei idle",				// shortened due to space limitations, ideally "Nur im Standby dimmen"
			"Dimmen ein"
		}
	},

	// French
	{
		// ISO-639.1 language code
		"fr",

		// Main page strings
		"Contrôle",
		"Imprimer",
		"Console",
		"Installation",
		"Pendant",
		"Actuel" THIN_SPACE DEGREE_SYMBOL "C",
		"Actif" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Mouvement",
		"Extrusion",
		"Macro",
		"ARRÊT",

		// Print page
		"Extrudeuse" THIN_SPACE "%",
		"Vitesse ",								// note space at end
		"Ventilo ",								// note space at end. "Ventilateur 0%" was too long to fit.
		"Temps Restant: ",
		"Fichier ",								// note space at end
		", filament ",							// note space at end
		", couche ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Reprise",
		"Annuler",
		"Set",

		// Setup page
		"Volume ",								// note space at end
		"Calibrer touch",
		"Affichage en négatif",
		"Inverser affichage",
		"Théme",
		"Luminosité -",
		"Luminosité +",
		"Sauver paramêtres",
		"Effacer paramêtres",
		"Sauvegarde & Redémarrage",
		"Info timeout ",						// note space at end

		// Pendant root
		"Panel",
		"Jog",
		"Offset",
		"Job",

		// Pendant Jog tab
		"Axis",
		"Movement",
		"Current",
		"Homing",
		"Measure Z",
		"Confirm Measure Z",
		"Tools",
		"Probe",

		// Pendant Offset tab
		"Probe Workpiece",
		"Find Center of Cavity",
		"Touch-off Plate",
		"Set Tool Offset",
		"Zero Axes in WCS",

		// Pendant Job tab
		"No job running",
		"Extrd. ",
		"Spindle RPM ",
		"Heat Control",

		// Misc
		"Confirmer le réinitialisation de l'imprimante",
		"Confirm suppression fichier",
		"Vous êtes sûre?",
		"Appuyer sur le point",
		"Mouvement de la  tête",
		"Quantité de Matière extrudée (mm)",
		"Vitesse (mm/s)",
		"Extruder",
		"Retracter",
		"Baby stepping",
		"décalage Z courant : ",
		"Message",
		"Messages",
		"Version du firmware du Panel Due ",	// note space at end
		"Réponse",

		// File popup
		"Fichier sur carte ",					// note the space on the end
		"Macros",
		"Erreur ",								// note the space at the end
		" accés SD card en cours",				// note the space at the start
		"Nom du fichier : ",
		"Taille : ",
		"Hauteur de couche: ",
		"Hauteur de l'objet: ",
		"Filament requis: ",
		"Sliced par: ",
		"Dernière modification: ",
		"Temps d'impression estimé: ",
		"Temps d'impression simulé: ",
		"Simuler",

		// Printer status strings
		{
			"Liaison en cours",					// "Connexion en cours" was too long
			"Au repos",
			"Impression",
			"Arrêt",
			"Démarrage",
			"Pause",
			"Occupé"
			"Pause",
			"Reprise",
			"Flasher firmware",
			"Changer outil",
			"Simuler",
			"En veille"
		},

		// Theme names
		{
			"Fond Blanc",
			"Fond Noir 1",
			"Fond Noir 2"
		},

		// Display dimming types
		{
			"Jamais Dim",
			"Idle Dim",
			"Toujours Dim"
		}
	},

	// Spanish
	{
		// ISO-639.1 language code
		"es",

		// Main page strings
		"Control",
		"Imprimir",
		"Consola",
		"Configuración",
		"Pendant",
		"Actual" THIN_SPACE DEGREE_SYMBOL "C",
		"Activo" THIN_SPACE DEGREE_SYMBOL "C",
		"Esperando" THIN_SPACE DEGREE_SYMBOL "C",
		"Mover",
		"Extrusión",
		"Macro",
		"PARADA",							// It could also be STOP, both are OK

		// Print page
		"Extrusor" THIN_SPACE "%",
		"Veloc. ",							// note space at end. "Velocidad" is too long.
		"Ventil. ",							// note space at end. "Ventilador" is too lonh.
		"Tiempo restante: ",
		"archivo ",							// note space at end
		", filamento ",						// note space at end
		", capa ",							// note space at end
		"n/d",								// Not available / no disponible
		"Pausa",
		"Micro paso",						// Literal translation of baby step it's very odd in spanish...
		"Resumir",
		"Cancelar",
		"Fijar",							// "Establecer" would be more correct, but it's longer.

		// Setup page
		"Volumen ",							// note space at end
		"Calibrar toque",					// this one is tricky because "touch" is very known in regard to screens...
		"Espejar pantalla",
		"Invertir pantalla",
		"Tema",
		"Brillo -",
		"Brillo +",
		"Guardar parámetros",
		"Borrar parámetros",
		"Guardar y Reiniciar",
		"Info timeout ",					// note space at end

		// Pendant root
		"Panel",
		"Jog",
		"Offset",
		"Job",

		// Pendant Jog tab
		"Axis",
		"Movement",
		"Current",
		"Homing",
		"Measure Z",
		"Confirm Measure Z",
		"Tools",
		"Probe",

		// Pendant Offset tab
		"Probe Workpiece",
		"Find Center of Cavity",
		"Touch-off Plate",
		"Set Tool Offset",
		"Zero Axes in WCS",

		// Pendant Job tab
		"No job running",
		"Extrd. ",
		"Spindle RPM ",
		"Heat Control",

		// Misc
		"Confirma restablecimiento de fábrica",
		"Confirma borrar archivo",
		"Está seguro?",
		"Tocar el punto",
		"Mover cabezal",
		"Cantidad de extrusión (mm)",
		"Velocidad (mm/s)",
		"Extruir",
		"Retraer",
		"Micro paso",
		"Separación actual de Z: ",
		"Mensaje",
		"Mensajes",
		"Panel Due versión de firmware ",	// note space at end
		"Respuesta",

		// File popup
		"Archivos en la tarjeta ",			// note the space on the end
		"Macros",
		"Error ",							// note the space at the end
		" accediendo a la tarjeta SD",		// note the space at the start
		"Nombre de archivo: ",
		"Tamaño: ",
		"Altura de capa: ",
		"Altura de objeto: ",
		"Filamento necesario: ",
		"Procesado por: ",					// there is no translation in spanish for this meaning, so I proposed to use "processed by" which is understandable
		"Última modificación: ",
		"Tiempo estimado de impresión: ",
		"Tiempo de impresión simulado: ",
		"Simular",

		// Printer status strings
		{
			"conexión",
			"en espera",					// it's more frequently use "en espera" than "ocioso", it makes more sense for a machine
			"imprimiendo",
			"detuvo",
			"empezando",
			"pausado",
			"ocupado",
			"pausando",
			"reanudando",
			"carga del firmware",
			"herramienta de cambio",
			"simulando",
			"en espera"
		},

		// Theme names
		{
			"Claro",
			"Oscuro 1",
			"Oscuro 2"
		},

		// Display dimming types
		{
			"Nunca Atenuar",
			"Atenuar en espera",
			"Siempre Atenuar",
		},
	},

	// Czech
	{
		// ISO-639.1 language code
		"cs",

		// Main page strings
		"Ovládání",
		"Tisk",
		"Konzole",
		"Nastavení",
		"Pendant",
		"Aktuální" THIN_SPACE DEGREE_SYMBOL "C",
		"Aktivní" THIN_SPACE DEGREE_SYMBOL "C",
		"Nečinná" THIN_SPACE DEGREE_SYMBOL "C",
		"Pohyb",
		"Extruder",
		"Makra",
		"STOP",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Rychl. ",							// note space at end
		"Vent. ",							// note space at end
		"Čas do konce: ",
		"soubor ",							// note space at end
		", materiál ",						// note space at end
		", vrstva ",						// note space at end
		"n/a",
		"Pozastavit",
		"Baby step",
		"Pokračovat",
		"Zrušit",
		"OK",

		// Setup page
		"Hlasitost ",						// note space at end
		"Kalibrace dotyku",
		"Zrcadlit displej",
		"Obrátit displej",
		"Motiv",
		"Podsvícení -",
		"Podsvícení +",
		"Uložit nastavení",
		"Smazat nastavení",
		"Uložit a Restart",
		"Info timeout ",					// note space at end

		// Pendant root
		"Panel",
		"Jog",
		"Offset",
		"Job",

		// Pendant Jog tab
		"Axis",
		"Movement",
		"Current",
		"Homing",
		"Measure Z",
		"Confirm Measure Z",
		"Tools",
		"Probe",

		// Pendant Offset tab
		"Probe Workpiece",
		"Find Center of Cavity",
		"Touch-off Plate",
		"Tool Offset",
		"Zero Axes in WCS",

		// Pendant Job tab
		"No job running",
		"Extrd. ",
		"Spindle RPM ",
		"Heat Control",

		// Misc
		"Skutečně obnovit tovární nastavení?",
		"Skutečně smazat?",
		"Určitě?",
		"Dotkněte se bodu",
		"Posun hlavy",
		"Množství (mm)",
		"Rychlost (mm/s)",
		"Vytlačit (extr.)",
		"Zatlačit (retr.)",
		"Baby stepping",
		"Aktuální Z offset: ",
		"Zpráva",
		"Zprávy",
		"Verze firmware Panel Due ",	// note space at end
		"Odpověď",

		// File popup
		"Soubory na kartě ",			// note the space on the end
		"Makra",
		"Chyba ",						// note the space at the end
		" přístupu ke kartě",			// note the space at the start
		"Název: ",
		"Velikost: ",
		"Výška vrstvy: ",
		"Výška objektu: ",
		"Spotřeba (mat.): ",
		"Slicer: ",
		"Poslední úprava: ",
		"Zbývající čas tisku: ",
		"Simulovaný čas tisku: ",
		"Simulace",

		// Printer status strings
		{
			"Připojování",
			"Nečinný",
			"Tiskne",
			"Zastaven",
			"Startuje",
			"Pozastaven",
			"Zaneprázdněný",
			"Pozastavuje se",
			"Pokračuje",
			"Nahrává firmware",
			"Výměna nástroje",
			"Simulace",
			"Pohotovostní"
		},

		// Theme names
		{
			"Světlý",
			"Tmavý 1",
			"Tmavý 2"
		},

		// Display dimming types
		{
			"Nikdy neztlumit jas",
			"Pohasnout při nečinnosti",
			"Pohasnout vždy"
		}
	}
};

#endif /* SRC_STRINGS_HPP_ */
