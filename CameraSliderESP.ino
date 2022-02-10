
//#include <connectionModules.h>

////IMPORTS
//Mqtt
#include <PubSubClient.h>
//test
//Wlan Verbindung
#include <ESP8266WiFi.h>

//Literals
char* SSID = "Debi lan";
char* PASS = "HubbaBubbaGang799";

char* mqtt_adress = "192.168.0.162";
int mqtt_port = 1883;
char* mqtt_topic = "chris/led";

//MQTT infrastructure

WiFiClient espClient;
PubSubClient client(espClient);


int Schrittmotor = 12; // Pin 8 an „Step“
int Richtung = 14; // Pin7 an „Dir“


int current_position = 0;
int last_step = 0;
int MAX_STEPS = 6380;


//Reset pin config
//wenn rst durch output auf high gesetzt wird ist der slider auf position 0
int rst_pin = A0;

void setup() {
//Serial configuration
Serial.begin(115200);

//Step Motor configuration

pinMode(Schrittmotor, OUTPUT); // Pin8 muss ein Ausgang sein, damit hier ein Signal gesendet werden kann.
pinMode(Richtung, OUTPUT); // Pin7 muss ein Ausgang sein, damit hier ein Signal gesendet werden kann.
digitalWrite(Richtung, LOW); // Das Signal für die Richtung wird zunächst auf LOW gesetzt. Wenn die Drehung nicht in die gewünschte Richtung geht, wird das Signal auf HIGH geändert;
pinMode(LED_BUILTIN, OUTPUT);//Zur Anzeige das wir mit dem Server verbunden sind

//join network
 WiFi.mode(WIFI_STA);
WiFi.begin(SSID, PASS);

Serial.println("Trying to connect to ");
Serial.println(SSID);
Serial.println("...");

while (WiFi.status() != WL_CONNECTED)
{
  delay(500);
  Serial.print(".");
}
Serial.println("Wifi connected!");
printCurrentNet();

//connect to Mqtt Broker
client.setServer(mqtt_adress, mqtt_port);
//subscribe to topic
client.subscribe(mqtt_topic);
client.setCallback(callback);
digitalWrite(LED_BUILTIN, LOW);
}


//##############################
//Main Loop Content
//##############################
void loop() {

    //check for network connection
     while(WiFi.status() != WL_CONNECTED) {
        WiFi.begin(SSID, PASS);
        delay(1000);
        Serial.println(".");
      }
    //check connection to broker and reconnect if needed
    if (!client.connected()) {
      reconnect();
    }
    
    //loop through mqtt messages
    client.loop();

}

void printCurrentNet() {
    //print ssid
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    //print IP
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    //print signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength:");
    Serial.println(rssi);
    
}

void check_mqtt_conn() {
    //check if we are connected to the Mqtt broker
    if(!client.connected()) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print("Reconnecting...");
      
      while(!client.connected()) {
        
        //periodically check if connection is established
        if(!client.connected()) {
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
    client.subscribe(mqtt_topic);
    Serial.println("MQTT Connected...");
    digitalWrite(LED_BUILTIN, LOW);
 }
}

void reconnect() {
// Loop until we're reconnected
while (!client.connected()) {
  Serial.println("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect("")) {
    Serial.println("connected");
    client.subscribe(mqtt_topic);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}
}
    
int move_camera(int steps, int dir, int vel) {

    //delay 
  vel = 100-vel;
  int pause = 800 + vel * 5;
  Serial.print("Delay in ms: ");
  Serial.print(pause);
  Serial.print('\n');
          
  //going to the right
  if(dir == 0) {
          Serial.println("Going left");
          digitalWrite(Richtung, HIGH);
          for(int i = 0; i < steps; i++) {
            digitalWrite(Schrittmotor, HIGH);
            
            
            digitalWrite(Schrittmotor, LOW);
            delayMicroseconds(pause);
            Serial.print(i);

          }
        }
  else{
    Serial.println("Going right");
    digitalWrite(Richtung, LOW);
    for(int i = 0; i < steps; i++) {
      digitalWrite(Schrittmotor, HIGH);
      
      
      digitalWrite(Schrittmotor, LOW);
      delayMicroseconds(pause);
      Serial.print(i);

    }
          
}
return 1;
}

void read_msg(int container[], char* msg){
  char * token = strtok(msg, "/");
  int steps_length = strlen(token);
  
  int steps = atoi(msg);
  //dir is always 2 positions behind the Steps
  int dir = (int)msg[steps_length+1]-48;
  
  //printf("\nSteps:%d", steps);
  Serial.print("steps:");
  Serial.print(steps);
  Serial.print("\n");
  //printf("\nStep_length: %d", steps_length);
  Serial.print("Steps length:");
  Serial.print(steps_length);
  Serial.print("\n");
  
  if(dir == 1) {
      token = strtok(NULL, "1");
  }
  else {
      token = strtok(NULL, "0");
  }
  
  int velocity = atoi(token);
  
  
  Serial.print("Velocity:");
  Serial.print(velocity);
  Serial.print("%");
  Serial.print("\n");
  //printf("\nDirection:%d", dir);
  Serial.print("Direction:");
  Serial.print(dir);
  Serial.print("\n");

  container[0] = steps;
  container[1] = dir;
  container[2] = velocity;
  
}

void execute(int values[3]) {
  //execute a single message

    int steps = values[0];
    int dir = values[1];
    int velocity = values[2];

        Serial.print("Currently located at: ");
        Serial.print(current_position);
        Serial.print("\n");

        Serial.print("Step count = \n");
        Serial.print(steps);
        //check if movement possible
        
        int destination = 0;
        //get position for next move
        if( dir == 1 ) {destination = current_position + steps;}
        else {destination = current_position - steps; }
        //check if position is valid else adapt it
        if (destination > MAX_STEPS) {steps = MAX_STEPS - current_position;
                                      destination = MAX_STEPS;}
        else if(destination < 0) {steps = current_position;
                                  destination = 0;}


        Serial.print("\nAction--> |From:");
        Serial.print(current_position);
        Serial.print("|To:");
        Serial.print(destination);
        Serial.print("|Actual steps:");
        Serial.print(steps);
        Serial.print('\n');
        //Action
        move_camera(steps, dir, velocity);

        current_position = destination;
}

int read_multiple(String msg) {
  //read a concatenated string of Commands
  
  //remove head
  msg.remove(0,1);
  //get index of second head
  int index = msg.indexOf('[');
  int index_end = msg.indexOf(']') - 2;

  //IF no head is present we abort the process
  if(!index) {
    return -1;
  }

  //create string for head content and allocate space for it
  String size_s = String();
  size_s.reserve(3);

  for (int i = 0 ; i < index ; i++) {
    //get chars until index of "[" -> head content
    size_s += msg[i];
  }
  //cut off head/tail
  msg.remove(0,index+1);
  msg.remove(index_end, msg.length() - index_end);
  int size = size_s.toInt();

  //convert to c string  
  int buffer_length = msg.length()+1;
  char buffer[buffer_length];

  msg.toCharArray(buffer, buffer_length);
  //Kontrolle der beiden Formate
  Serial.println(buffer);
  Serial.println(msg);

  int end = -1;
  int delay_end = 0;
  int start = end+1;
 
  for (int j = 0 ; j < size; j++) {
    //seperate single command
    if(end != -1) {start = delay_end + 1;}
    //Den ersten Endpunkt der command liste finden
    end = msg.indexOf(";", end+1);
    delay_end = msg.indexOf("!", end);


    Serial.print("start:");
    Serial.print(start);
    Serial.print("\n");
    Serial.print("End:");
    Serial.print(end);
    Serial.print("\n");

    int command_length = end - start;
    char command[command_length+1] = "";
    //this value defines the sleep after our command (max duration is 99 seconds)
    char delay_after[delay_end - end] = "";

    //Generation of Command in char[] format 
    for(int z = 0 ; z < command_length; z++) {
      command[z] = buffer[start + z];
    }
    //Generation of delay
    for(int d = 0; d< delay_end - end -1; d++) {
      delay_after[d] = buffer[end +1 + d];
    }
    delay_after[-1] = '\n';

    //allocate space for our command values
    int vals[3];
    //read and execute
    Serial.println("------------------------");
    Serial.print("Executin command number: ");
    Serial.print(j + 1);
    Serial.print('/');
    Serial.print(size);
    Serial.print('\n');
    Serial.print("Command: ");
    Serial.print(command);
    Serial.print("\n");
    Serial.print("Delay after Command: ");
    Serial.print(atoi(delay_after));
    Serial.print("s");
    Serial.print('\n');

    //Read the command and execute it
    read_msg(vals, command);
    execute(vals);

    //delay right here
    Serial.println("Starting delay...");
    delay(atoi(delay_after) * 1000);
  }
  return 1;

}

int reset_position() {
  //set direction to left
  digitalWrite(Richtung, HIGH);
  //digitalWrite(output_pin, LOW);
  Serial.println("Starting reset");

  for(int i = 0 ; i < MAX_STEPS ; i++) {

        if(analogRead(rst_pin) >= 500) {
          break;
        }

        digitalWrite(Schrittmotor, HIGH);
        
        digitalWrite(Schrittmotor, LOW);
        delayMicroseconds(2000);

        /*if(i%1000 == 0) {
          delay(500);
        }*/
        Serial.println(i);
  }
  current_position = 0;
  return 1;
}

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("\n\nReceived messsage[");
    Serial.print(topic);
    Serial.print(" | length: ");
    Serial.print(length);
    Serial.print("]");

    //print incoming message
    Serial.print("-->");

    //get string and char* instance of payload
    String msgShow = String("");
    msgShow = String((char*)payload).substring(0, length);
    char* msg = (char*)payload;
    Serial.println(msgShow);

    //Die Eichung um dem Slider auf den Anfang zu setzten
    if(msgShow == "rst") {
      if(reset_position() == 1) {
        Serial.println("Position is now set to 0!");
      }
      else {
        Serial.println("Position could not be reset!");
      }
    }

    //check msg integrity

    if (msgShow[0] == '[') {
      if (true) {
        int status = read_multiple(msg);
        if (status == 1) {Serial.println("Multi Message was absorbed.");}
        else if (status == -1) {Serial.println("Multi Message could not be read."); }
      }
      else {Serial.println("Multi Message could not be read. Wrong Format {error: no end}");}

      
    }
    }
    






        
