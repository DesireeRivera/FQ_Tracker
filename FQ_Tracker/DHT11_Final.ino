

//ESP 32_S
#if ! (ESP8266 || ESP32 )
  #error This code is intended to run on the ESP8266/ESP32 platform! Please check your Tools->Board setting
#endif

#include "Credentials.h"

#define MYSQL_DEBUG_PORT      Serial

// Debug Level from 0 to 4
#define _MYSQL_LOGLEVEL_      1

#include <MySQL_Generic.h>

#define USING_HOST_NAME     false

#if USING_HOST_NAME
  // Optional using hostname, and Ethernet built-in DNS lookup
  char server[] = "your_account.ddns.net"; // change to your server's hostname/URL
#else
  IPAddress server(195, 235, 211, 197);
#endif

uint16_t server_port = 3306;    //3306;

char default_database[] = "prbbiosoft";           //"test_arduino";
char default_table[]    = "Temperatura3";          //"test_arduino";

 

// Sample query


MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;

//DHT
#include "DHT.h"//Incluir la libreria

#define DHTPIN 22 //Definimos el pin digital donde se conecta el sensor

#define DHTTYPE DHT11 //Definimos con que tipo de sensor trabajamos ya que la libreria es compatible
                      //con otros tipos de sensores DHT
DHT dht(DHTPIN, DHTTYPE); //Para Incicializar el sensor

double t = 0.0;

void setup()
{
  Serial.begin(115200);
  dht.begin();//Inicializamos el sensorx
  while (!Serial);

  MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  // Begin WiFi section
  MYSQL_DISPLAY1("Connecting to", ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    MYSQL_DISPLAY0(".");
  }

  // print out info about the connection:
  MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());

  MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
  MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);


}

void runInsert()
{
  // Initiate the query class instance
  MySQL_Query query_mem = MySQL_Query(&conn);

  String INSERT_SQL = "INSERT INTO `prbbiosoft`.`Temperatura3` (`grados`, `ID_Parametros`) VALUES (";
  INSERT_SQL.concat(t);
  INSERT_SQL.concat(", '0002TTT');");

  if (conn.connected())
  {
    MYSQL_DISPLAY(INSERT_SQL);
    
    // Execute the query
    // KH, check if valid before fetching
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      MYSQL_DISPLAY("Insert error");
    }
    else
    {
      MYSQL_DISPLAY("Data Inserted.");
    }
  }
  else
  {
    MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}

void loop()
{
  //DHT
   delay(5000); //Esperamos 5 segundos entre medidas para evitar errores de medicion

  double h = dht.readHumidity(); //Leemos la humedad relativa

  t = dht.readTemperature();//Leemos la temperatura (en grados centigrados por defecto)

  //float f = dht.readTemperature(true);(si quisieramos leer en fahreheit

  if(isnan(h) || isnan(t)) {//Funcion de paridad, comprobamos si hay algun error en la lectura
     Serial.println("Error obtenido en los datos del sensor DHT11");
     return;
    }

  float hic = dht.computeHeatIndex(t,h,false);//Cacular el indice de calor en grados centrigrados

  //float hif = dht.computeHeatIndex(f,h); Seria para calcular en fahreheit

Serial.print("Humedad: ");
Serial.print(h);
Serial.println("%");
Serial.print("Temperatura: ");
Serial.print(t);
Serial.println(" Cº");
Serial.print("Indice de calor: ");
Serial.println(hic);
  //ESP
  MYSQL_DISPLAY("Connecting...");
  
  //if (conn.connect(server, server_port, user, password))
  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL)
  {
    delay(500);
    runInsert();
    conn.close();                     // close the connection
  } 
  else 
  {
    MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }

  MYSQL_DISPLAY("\nSleeping...");
  MYSQL_DISPLAY("================================================");
 
  delay(60000);
}
