/*                            
                          P R O J E C T  R A T U B O X
                  R A W A T  T U M B U H A N  D A L A M  B O X
                             
led indikator wifi connect pin1
led indikator mode auto aktif pin2
relay 1 lampu pin3
relay 2 waterpump pin4
sensor kelembabantanah pin analog0
sensor suhubox pin5
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define  ONE_WIRE_BUS D8 

String identitas = "264149504"; //identitas user telegram nya
String sandi = "elektrojaya"; //password bot nya

// pengaturan wifi
char ssid[] = "rtbx";   
char password[] = "wifiratubox"; 

// token bot telegram dan mode client
#define BOTtoken "445355149:AAFb1xvIZjcfcpQ26CVsOvmFMtM8bFGHnTg"
String test_photo_url = "https://ratubox.xyz/wp-content/uploads/2017/11/cropped-logoratubox.png";
WiFiClientSecure client;  
UniversalTelegramBot bot(BOTtoken, client);
bool statussandi = false; //kondisi kunci
bool stsand = false;

int Bot_mtbs = 2000; //intval waktu scan 
long Bot_lasttime;  
bool manual = true;
const int ledkonek = D1;
const int ledmode = D2;
const int relaylampu = D3;
const int relaypump = D4;
const int sensorlembab = A0;
OneWire sensor(ONE_WIRE_BUS); 
DallasTemperature sensorsuhu(&sensor);
char temperatureCString[6];
char temperatureFString[6];

float ambilsuhu();
int ambillembab();
void modeauto();
void manualsiram();
void manualpanas();

void handleNewMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    
    if (text == "AKTIF") {
      digitalWrite(ledmode, HIGH);   
      manual = false;
      bot.sendMessage(identitas,"RATUBOX dalam modem AUTOMATIS");
    }

    if (text == "MATI") {
      manual = true;
      digitalWrite(ledmode, LOW);    
      String keyboardJson = "[[\"AKTIF\", \"MATI\"],[\"STATUS\",\"DATA\"],[\"PENGHANGAT\", \"SIRAM\", \"OFF_PENGHANGAT\"],[\"LOCK\"]]";
      bot.sendMessageWithReplyKeyboard(identitas, "RATUBOX dalam mode MANUAL", "", keyboardJson, true);
    }

    if (manual == true) {
      if (text == "SIRAM"){
        manualsiram();
        delay(20);
      }else if (text == "PENGHANGAT"){
        manualpanas();
        delay(20);
      }
    }

    if (text == "STATUS") {
      if(manual == false){
        bot.sendMessage(identitas, "MODE : AUTOMATIS", "");
      } else {
        bot.sendMessage(identitas, "MODE : MANUAL", "");
      }
    }

    if (text == "DATA") {
      float suhu = ambilsuhu();
      String ssuhu = String(suhu);
      delay(20);
      int lembab = 100 - ambillembab();
      String slembab = String(lembab);
      bot.sendMessage(identitas,"REPORT DATA SENSOR REAL TIME :  \nSuhu : ");
      bot.sendMessage(identitas,ssuhu);
      bot.sendMessage(identitas,"\nKelembaban : ");
      bot.sendMessage(identitas,slembab);
      Serial.print("kirimdata");
    }

    if (text == "LOCK") {
        statussandi = false;
        stsand = false;
    }

    if (text == "OFF_PENGHANGAT") {
        manualpanasmoff();
    }
    

    if (text == "HAI") {
      String welcome = "HAI  " + from_name + " SELAMAT DATANG DI BOT RATUBOX" ",\n";
      welcome += "BOT INI AKAN MEMBANTU ANDA UNTUK BERINTERAKSI DENGAN RATUBOX\n\n";
      welcome += "AKTIF : UNTUK MENYALAKAN RATUBOX SECARA OTOMATIS\n";
      welcome += "MATI : UNTUK MENJALANKAN MODE MANUAL\n";
      welcome += "STATUS : UNTUK MELIHAT MODE SAAT INI\n";
      welcome += "DATA : UNTUK CEK DATA SENSOR\n";
      bot.sendMessage(identitas, welcome, "Markdown");
      String keyboardJson = "[[\"AKTIF\", \"MATI\"],[\"STATUS\",\"DATA\"],[\"PENGHANGAT\", \"SIRAM\", \"OFF_PENGHANGAT\"],[\"LOCK\"]]";
      bot.sendMessageWithReplyKeyboard(identitas, "PILIHLAH MENU BERIKUT", "", keyboardJson, true);
    }
  }
}


void setup() {
  Serial.begin(115200);
  sensorsuhu.begin();
  pinMode(ledmode, OUTPUT); 
  pinMode(ledkonek, OUTPUT);
  pinMode(relaypump, OUTPUT);
  pinMode(relaylampu, OUTPUT);
  pinMode(sensorlembab, INPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ledkonek,HIGH);
    delay(250);
    digitalWrite(ledkonek,LOW);
    delay(250); 
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(10);
  digitalWrite(ledkonek,HIGH);
  digitalWrite(ledmode, LOW); // initialize pin as off
  digitalWrite(relaylampu,LOW);
  digitalWrite(relaypump, LOW);

}

void loop() {
  while (statussandi == false){    
    //kirim pesan bot untuk verifikasi user
    bot.sendPhoto(identitas, test_photo_url);
    bot.sendMessage(identitas, "KATA SANDI : ");
    while (stsand == false){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      for (int i=0; i<numNewMessages; i++) {
          String text = bot.messages[i].text;
          if (text == sandi){
            statussandi = true;
            stsand = true;
          }else{
            bot.sendMessage(identitas, "Sandi salah !!");
          }
          delay(20);
      }
      delay(200);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    }
    delay(200);
    String keyboardJson = "[[\"HAI\"]]";
    bot.sendMessageWithReplyKeyboard(identitas, "RATUBOX SIAP DIGUNAKAN", "", keyboardJson, true);
  }
  if (manual == false){
      modeauto();
  }
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Bot_lasttime = millis();
  }
}

float ambilsuhu() {
   float temp;
   do {
    sensorsuhu.requestTemperatures(); 
    temp = sensorsuhu.getTempCByIndex(0);
    delay(100);
   } while (temp == 85.0 || temp == (-127.0));
   Serial.println("ambilsuhu");
   return temp;
}

int ambillembab(){
   int lembab = analogRead(sensorlembab);
   Serial.print("ambillembab");
   lembab = 1000 - lembab;
   int datalem = map(lembab,550,10,0,100);
   Serial.println(datalem);
   return datalem;
}

void modeauto() {
    float t = ambilsuhu();
    int l =100 - ambillembab();
    if( t <= 27 || t >30 ){
         if (t <= 27){
            digitalWrite(relaylampu,HIGH);
            bot.sendMessage(identitas,"Suhu telalu rendah, penghangat menyala !", "");
         }
         else if( t > 30){
            digitalWrite(relaylampu,LOW);
            bot.sendMessage(identitas,"Suhu sudah cukup, penghangat dimatikan !", "");
         }
    }else{
         digitalWrite(relaylampu,LOW);
         bot.sendMessage(identitas,"Suhu sudah pada kondisi ideal", "");
    }
    if ( l < 70  || l >=70 ) {
         if (l <70){
            digitalWrite(relaypump,HIGH);
            bot.sendMessage(identitas,"Kelembaban tanah dibawah 80%, pompa menyala !", "");
            delay (10000);
            digitalWrite(relaypump,LOW);
         }
         else if (l>70) {
            digitalWrite(relaypump,LOW);
            bot.sendMessage(identitas,"Kelembaban tanah cukup", "");
        }
    }else{
        digitalWrite(relaypump,LOW);
        bot.sendMessage(identitas,"Kelembaban tanah cukup", "");
    }
    delay (30000);
}

void manualsiram() {    
    digitalWrite(relaypump,HIGH);
    bot.sendMessage(identitas, "Penyiram nyala!", "");
    int batas = 100 - ambillembab();
    if (batas >= 80){
      digitalWrite(relaypump,LOW); 
    }
    delay(5000);
    digitalWrite(relaypump,LOW);
    Serial.println("manualsiram");
}

void manualpanas() {
    bot.sendMessage(identitas, "Penghangat nyala!", "");
    digitalWrite(relaylampu,HIGH);
    delay(2000);
    int batas = ambilsuhu();
    if (batas <= 30){
      digitalWrite(relaypump,LOW); 
    }
}

void manualpanasmoff() {
    bot.sendMessage(identitas, "Penghangat Dimatikan!", "");
    digitalWrite(relaylampu,LOW); 
}
