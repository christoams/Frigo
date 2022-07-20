/**
 * capteur de temperature DS18B20 sur un bus 1-Wire(Broche 10).
 *mis par defaut sur 30°
 *delais de 30 secondes entre chaque lecture de temperature
 *déclanche la broche 3 => darlingtone pour pouvoir controler le ventilateur, ne pas oublier de mettre une diode de roue libre pour éviter d'abimer le microcontroleur
  */
 
/* Dépendance pour le bus 1-Wire */
#include <OneWire.h>
 
 
/* Broche du bus 1-Wire */
const byte BROCHE_ONEWIRE = 10;

/* Code de retour de la fonction getTemperature() */
enum DS18B20_RCODES {
  READ_OK,  // Lecture ok
  NO_SENSOR_FOUND,  // Pas de capteur
  INVALID_ADDRESS,  // Adresse reçue invalide
  INVALID_SENSOR  // Capteur invalide (pas un DS18B20)
};


/* Création de l'objet OneWire pour manipuler le bus 1-Wire */
OneWire ds(BROCHE_ONEWIRE);
 
 
/**
 * Fonction de lecture de la température via un capteur DS18B20.
 */
byte getTemperature(float *temperature, byte reset_search) {
  byte data[9], addr[8];
  // data[] : Données lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire détecté
  
  /* Reset le bus 1-Wire ci nécessaire (requis pour la lecture du premier capteur) */
  if (reset_search) {
    ds.reset_search();
  }
 
  /* Recherche le prochain capteur 1-Wire disponible */
  if (!ds.search(addr)) {
    // Pas de capteur
    return NO_SENSOR_FOUND;
  }
  
  /* Vérifie que l'adresse a été correctement reçue */
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // Adresse invalide
    return INVALID_ADDRESS;
  }
 
  /* Vérifie qu'il s'agit bien d'un DS18B20 */
  if (addr[0] != 0x28) {
    // Mauvais type de capteur
    return INVALID_SENSOR;
  }
 
  /* Reset le bus 1-Wire et sélectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de température et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la température en degré Celsius */
  *temperature = ((data[1] << 8) | data[0]) * 0.0625; 
  
  // Pas d'erreur
  return READ_OK;
}

float TempDeclenchement = 30; //temperature de déclenchement du ventilo
 
 
/** Fonction setup() **/
void setup() {
 pinMode(3, OUTPUT); //sortie déclencement ventilo
 pinMode(LED_BUILTIN, OUTPUT);
  /* Initialisation du port série */
  Serial.begin(9600);
}
 
 
/** Fonction loop() **/
void loop() {
  float temperature;
   
  /* Lit la température ambiante à ~1Hz */
  if (getTemperature(&temperature, true) != READ_OK) {
    Serial.println(F("Erreur de lecture du capteur"));
    return;
  }

  /* Affiche la température */
  Serial.print(F("Temperature : "));
  Serial.print(temperature, 2);
  //Serial.write(176); // Caractère degré
  Serial.write('C');
  Serial.println();

  if (temperature > TempDeclenchement ){
    digitalWrite(3, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Ventilo on");
  }
  else {
  digitalWrite(3, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Ventilo off");
  }
  delay(30000); //delais 30s
}
