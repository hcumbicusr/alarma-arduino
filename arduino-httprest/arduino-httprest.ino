#include <SPI.h>

#include <Ethernet.h>

byte mac[]={0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,160);
EthernetServer server(80);

String HTTP_req; // Para guardar la petición
boolean LED2_status = 0;
boolean SECURITY_FLAG = 0;
String CURRENT_PASSWORD = "1234";

int LED_ALARMA_INACTIVA = 13; // Led rojo
int LED_ALARMA_ACTIVA = 12; // Led Verde
int SENSOR_PUERTA = 4; // Sensor magnetico
int SENSOR_MAMPARA = 5; // Sensor magnetico
int SENSOR_REJA = 6; // Sensor magnetico
int sirena = 10; // Sirena
int ESTADO_PUERTA = 0;
int ESTADO_MAMPARA = 0;
int ESTADO_REJA = 0;
String sensor_response;
String action;
String params;
String mode;
String new_password;
String password;
boolean ENCENDER_ALARMA = 0; // flag para encender o apagar alarma

void setup() {
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);
  pinMode(LED_ALARMA_INACTIVA,OUTPUT);
  pinMode(LED_ALARMA_ACTIVA,OUTPUT);
  pinMode(SENSOR_PUERTA, INPUT);
  pinMode(SENSOR_MAMPARA, INPUT);
  pinMode(SENSOR_REJA, INPUT);
  pinMode (sirena, OUTPUT);
}
void loop() {
  // ------------Sensores------------------------- 
  ESTADO_PUERTA = digitalRead(SENSOR_PUERTA);
  ESTADO_MAMPARA = digitalRead(SENSOR_MAMPARA);
  ESTADO_REJA = digitalRead(SENSOR_REJA);
  sensor_response = "";
  digitalWrite(sirena, LOW);
  if(ESTADO_PUERTA == LOW || ESTADO_MAMPARA == LOW || ESTADO_REJA == LOW ){
    if ( ESTADO_PUERTA == LOW )
      sensor_response += "{\"sensor\":\"PUERTA\",\"estado\":\"ABIERTA\"},";
    else
      sensor_response += "{\"sensor\":\"PUERTA\",\"estado\":\"CERRADA\"},";
    if ( ESTADO_MAMPARA == LOW )
      sensor_response += "{\"sensor\":\"MAMPARA\",\"estado\":\"ABIERTA\"},";
    else
      sensor_response += "{\"sensor\":\"MAMPARA\",\"estado\":\"CERRADA\"},";
    if ( ESTADO_REJA == LOW )
      sensor_response += "{\"sensor\":\"REJA\",\"estado\":\"ABIERTA\"},";
    else
      sensor_response += "{\"sensor\":\"REJA\",\"estado\":\"CERRADA\"},";
    if ( SECURITY_FLAG )
      ENCENDER_ALARMA = 1;
    else
      SECURITY_FLAG = 0;
    fnSirena();
  }else{
    sensor_response = "{\"sensor\":\"PUERTA\",\"estado\":\"CERRADA\"},{\"sensor\":\"MAMPARA\",\"estado\":\"CERRADA\"},{\"sensor\":\"REJA\",\"estado\":\"CERRADA\"},";
    fnSirena();
  }
  //Serial.println(sensor_response.lastIndexOf(","));
  if ( sensor_response.lastIndexOf(",") > -1 )
    sensor_response.remove(sensor_response.lastIndexOf(","), 1);
  //Serial.println("sensor_response::: ");
  //Serial.println(sensor_response);
  sensor_response += ",";
  //Serial.println(sensor_response);
  if ( ENCENDER_ALARMA )
    sensor_response += "\"alarma_encendida\":\"ON\",";
  else
    sensor_response += "\"alarma_encendida\":\"OFF\",";
  if ( SECURITY_FLAG )
    sensor_response += "\"alarma_activada\":\"ON\",";
  else
    sensor_response += "\"alarma_activada\":\"OFF\",";
  if ( sensor_response.lastIndexOf(",") > -1 )
    sensor_response.remove(sensor_response.lastIndexOf(","), 1);
  Serial.println(sensor_response);
  // -------------Web server------------------------ 
  EthernetClient client = server.available(); // Comprobamos si hay peticiones
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read(); // aquí viene toda la petición, separada por enter
        HTTP_req += c;
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          processRequest(client);
          
          //Serial.print(HTTP_req);
          HTTP_req = "";
          break;
        }
        if (c == '\n')
          currentLineIsBlank = true;
        else if (c != '\r')
          currentLineIsBlank = false;
      }
    } // WHile
    delay(10); // dar tiempo
    client.stop(); // Cerra conexion
  } // If client
  sensor_response = "";
}

void fnSirena() {
  if ( ENCENDER_ALARMA ) {
      digitalWrite(sirena, HIGH);
      delay(500);
      digitalWrite(sirena, LOW);
      delay(500);
    } else {
      digitalWrite(sirena, LOW);
      delay(1000);
    }
}
void processRequest(EthernetClient cl) {
  params = HTTP_req.substring(HTTP_req.indexOf("?")+1, HTTP_req.indexOf(" HTTP/1.1"));
  //Serial.println("params: "+params);
  action = params.substring(params.indexOf("ACTION=")+7, params.indexOf("&"));
  //Serial.println("action: "+action);
  password = params.substring(params.indexOf("&PASSWORD=")+10);
  //Serial.println("password: "+password);
  if ( password == CURRENT_PASSWORD ) {
    if ( action == "SECURITY" ) {
      mode = params.substring(params.indexOf("MODE=")+5);
      mode = mode.substring(0, mode.indexOf("&"));
      if (mode=="ON")
        SECURITY_FLAG = 1;
      else {
        SECURITY_FLAG = 0;
        ENCENDER_ALARMA = 0;
      }
      //Serial.println("mode: "+mode);
    } else if ( action == "PANIC" ) {
      mode = params.substring(params.indexOf("MODE=")+5);
      mode = mode.substring(0, mode.indexOf("&"));
      if (mode=="ON")
        ENCENDER_ALARMA = 1;
      else
        ENCENDER_ALARMA = 0;
      //Serial.println("mode: "+mode);
    } else if ( action == "PASSWORD" ) {
      new_password = params.substring(params.indexOf("NEW_PASSWORD=")+13);
      new_password = new_password.substring(0, new_password.indexOf("&"));
      new_password.trim();
      CURRENT_PASSWORD = new_password;
      //Serial.println("new_password: "+new_password);
    }
    cl.println("{\"message\":\"OK\"}");
  } else if ( action == "SERIAL" ) {
      cl.println(sensor_response);
  } else {
    cl.println("{\"message\":\"Clave incorrecta\"}");
  }
  
  if (SECURITY_FLAG) {
    digitalWrite(LED_ALARMA_ACTIVA, HIGH);
    digitalWrite(LED_ALARMA_INACTIVA, LOW);
  }else{
    digitalWrite(LED_ALARMA_ACTIVA, LOW);
    digitalWrite(LED_ALARMA_INACTIVA, HIGH);
  }
  //Serial.print("SECURITY_FLAG: ");
  //Serial.println(SECURITY_FLAG);
  //Serial.print("ENCENDER_ALARMA: ");
  //Serial.println(ENCENDER_ALARMA);
}
