/******************************************************************************
 * 
Project GEII Etude et Rea
groupe 2 : SAYOURI MARWANE ET SHU AKAOGI
Notre projet:
utlise la librairie tinygps++
une liaison serie entre l'arduino et le gps et une communication SPI avec la carte micro SD pour enregistrer les informations
liaison serie avec les pin 2 TX et 3 RX
 Et le pin 8 pour la SD
 
Pour regarder la trajectoir il suffit de crée un fichier .txt
******************************************************************************/

#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>

#define ARDUINO_USD_CS 8 // uSD CS pin 8

// Defenition de notre fichier log

#define LOG_FILE_PREFIX "gpslog" // Le nom du fichier.
#define MAX_LOG_FILES 100 // le nombre maximum de fichier qu'on peut creer.
#define LOG_FILE_SUFFIX "txt" // Suffix du log file.
char logFileName[13]; // Char string pour enregistrer le nom du fichier.

// pour l'enregistrement des données

#define LOG_COLUMN_COUNT 8
char * log_col_names[LOG_COLUMN_COUNT] = {
  "longitude", "latitude", "altitude", "vitesse", "course", "date", "temps", "satellites"
}; // log_col_names est imprimer en haut de la page pour pouvoir connaitre les données.


// le debit d'enregistrement des données

#define LOG_RATE 5000 // chaque 5s
unsigned long lastLog = 0; // initialiser


// TinyGPS Definition 

TinyGPSPlus tinyGPS; // On creer un objet tinyGPS
#define GPS_BAUD 9600 // la frequence du module gps


// configuration de la liaison gps-arduino //

#include <SoftwareSerial.h>
#define ARDUINO_GPS_RX 3 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 2 // GPS RX, Arduino TX pin
SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); 



#define gpsPort ssGPS  
#define SerialMonitor Serial

void setup()
{
  SerialMonitor.begin(9600);
  gpsPort.begin(GPS_BAUD);
  
  // La configuration de la carte SD

  SerialMonitor.println("en cours de preparation de la carte SD.");
  // Voir si la carte SD est bien brancher:
  if (!SD.begin(ARDUINO_USD_CS))
  {
    SerialMonitor.println("Error en initialison la carte SD.");
  }
  updateFileName(); // A chaque fois qu'on commence, creer a nouveau fichier en incrementant le nombre
  printHeader(); // ecrire un titre en haut du nouveau fichier
}

void loop()
{
  if ((lastLog + LOG_RATE) <= millis())
  { // si c'etait LOG_RATE milliseconds depuis le dernier enregistrement:
    if (tinyGPS.location.isUpdated()) // si les infos sont valid
    {
      if (logGPSData()) //  données enregistree
      {
        SerialMonitor.println("GPS logged.");
        lastLog = millis(); // mis a jour de la derniere variable lastLog
      }
      else // si on reussi pas a enregister
      { // Print error:
        SerialMonitor.println("Pas reussi a enregistrer les données du gps.");
      }
    }
    else // si Les données ne sont pas valid
    {
      // Print un message d'erreur. Peut etre on a pas assez de satellites.
      SerialMonitor.print("No GPS data. Sats: ");
      SerialMonitor.println(tinyGPS.satellites.value());
    }
  }

  // si on enregistre pas les données on continue la recherche
  while (gpsPort.available())
    tinyGPS.encode(gpsPort.read());
}

byte logGPSData()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Ouvrire le fichier log

  if (logFile)
  { // imprimer longitude, latitude, altitude, speed, course
    // degrees, date, temps et le nombre de satellite.
    logFile.print(tinyGPS.location.lng(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.location.lat(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.altitude.feet(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.speed.mph(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.course.deg(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.date.value());
    logFile.print(',');
    logFile.print(tinyGPS.time.value());
    logFile.print(',');
    logFile.print(tinyGPS.satellites.value());
    logFile.println();
    logFile.close();

    return 1; // Return success
  }

  return 0; // si le fichier ne s'ouvre pas, return fail
}

// printHeader()--- print les huite noms de colonnes en haut de notre fichier log
void printHeader()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // ouvrir le fichier log
  if (logFile) // si le fichier est ouvert on imprime les noms de colonnes
  {
    int i = 0;
    for (; i < LOG_COLUMN_COUNT; i++)
    {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1) // si c'est n'importe quelle colonne sauf la dernier colonne...
        logFile.print(','); // print virgule
      else
        logFile.println(); // print une nouvelle ligne
    }
    logFile.close(); // fermer le fichier
  }
}

// updateFileName() - cherche dans les fichiers log deja existant sur la carte memoire,
// et crée un nouveau fichier avec un index de fichier incrémenté.
void updateFileName()
{
  int i = 0;
  for (; i < MAX_LOG_FILES; i++)
  {
    memset(logFileName, 0, strlen(logFileName)); // vider la chaine logFileName
    // puis le mettre a "gpslogXX.csv":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) // si un fichier n'existe pas
    {
      break; //sortie de la boucle
    }
    else
    {
      SerialMonitor.print(logFileName);
      SerialMonitor.println(" existe");
    }
  }
  SerialMonitor.print("Nom du fichier: ");
  SerialMonitor.println(logFileName); // Debug print le nom du fichier
}   
