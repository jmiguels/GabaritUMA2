/*
Esquema de ligações
                                                       
    |--------------------------------|                
    | Arduino                        |                 
    | ESP32 30P                      |                 
    |                                |                 
    |                                |                        |---------------------|
    |                            GND |------------------------|GND                  |
    |                            SCL |------------------------|SCL                  |
    |                            SDA |------------------------|SDA                  |
    |                            3V3 |------------------------|VCC (2.4V to 3.6V)   |
    |                                |                        |---------------------|      
    |                                |                            
    |                                |                        |---------------------|
    |                         GPIO02 |-->---------------------|IN Relay1            |
    |                         GPIO15 |-->---------------------|IN Relay2            |
    |                                |                        |---------------------|  
    |                                |                        |---------------------|
    |                         GPIO27 |--<---------------------|OUT OpenButton       |
    |                         GPIO26 |--<---------------------|OUT CloseButton      |
    |                                |                        |---------------------|  
    |                                |                        |---------------------|
    |                         GPIO35 |-->---------------------|AnalogIn1            |
    |                         GPIO32 |--<---------------------|Servo1 (antigo AnalogIn2)
    |                         GPIO33 |--<---------------------|EnergyCut (AnalogIn3)|
    |                                |                        |---------------------|  
  --|USB                             |
    |--------------------------------|             

**********************************************************************/

#include <Servo.h> // https://randomnerdtutorials.com/esp32-servo-motor-web-server-arduino-ide/
                   // Biblioteca "ServoESP32"de Jaroslav Paral
                   
#include <Wire.h>  // Necessário para utilizar a biblioteca SparkFun_APDS9960.h
#include <SparkFun_APDS9960.h> //https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor

//*********** WS2812B **************
//Utilizado para LEDS endereçaveis
#include <Adafruit_NeoPixel.h>  //Utilizado para LEDS endereçaveis

#define NUM          10
#define LED_WS2813   19
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM,LED_WS2813, NEO_GRB + NEO_KHZ800);
//**********************************


String DeviceTypeStr="GABARIT_CTRL";
String FirmwareVersion="V1.2 - 22-09-2022 - JMSantos";
String VersionStr= DeviceTypeStr + " - " + FirmwareVersion;

// Definição dos pinos de uControlador utilizados
#define Esp32LED      5  // GPIO5  - LED interno do ESP32
#define Relay1        2  // GPIO2
#define Relay2        15 // GPIO15
#define OpenButton    26 // GPIO26
#define CloseButton   27 // GPIO27
#define AnalogIn1     35 // GPIO35 THIS PIN IS INPUT ONLY !!!
#define Servo1        32 // GPIO32
#define EnergyCut     33 // GPIO33 AnalogIn3

hw_timer_t * timer = NULL; //******** Timer Interrupt definitions

#define RelayPulseDuration        500000   //pulse duration for the relay activation in micro seconds
#define InterruptActivationDelay  1000000  //Delay to activate interrupts again after a command
int LastCommandWasOpen = true;

Servo servo_1;  // create servo object to control a servo

//***** Color sensor SparkFun_APDS9960
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;
//**********************************

//********* CLInterface ************
String Commandstr = "";
int CommandStrIsComplete=0;
bool Echo=false;   //Configure Echo of CLI
//**********************************

void IRAM_ATTR InterruptToOpen()
{
  Open();
}

void IRAM_ATTR InterruptToClose()
{  
  if(Close()==1)
  {
    Serial.println("Emergency pressed");
  }
}

void TimerInterruptToFinishPulse()
{
  timerEnd(timer); //Destroi o timer
  digitalWrite(Relay1,LOW);
  digitalWrite(Relay2,LOW);
  //Timer setup
  timer = timerBegin(0, 80, true); // Timer 0 programado comum prescale de 80 (o relógio é de 80MHz). countup (true) ou countdown (false)
  timerAttachInterrupt(timer, &ActivateInterruptsAgain, true); //Definição da rotina de serviço ao interrupt. edge (true) or level (false)  
  timerAlarmWrite(timer, InterruptActivationDelay, true); //configurar o timer interrupt TimerInterruptToFinishPulse() para daqui a 1s
  timerAlarmEnable(timer); 
}

void ActivateInterruptsAgain()
{
  timerEnd(timer); //Destroi o timer
  EnableButtonInterrupts();
}

void EnableButtonInterrupts()
{
  attachInterrupt(OpenButton, InterruptToOpen, RISING);
  attachInterrupt(CloseButton, InterruptToClose, FALLING);   
}

void DetachButtonInterrupts()
{
  detachInterrupt(OpenButton);
  detachInterrupt(CloseButton);  
}

////////////////////////////////
void setup() 
{
  int n;
  
  Serial.begin(115200); // Inicialização da porta serie para debug
  delay(100);
  pinMode(Esp32LED, OUTPUT);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(EnergyCut, OUTPUT);
  pinMode(OpenButton, INPUT_PULLUP);
  pinMode(CloseButton, INPUT_PULLUP);
  servo_1.attach(Servo1);  // attaches the servo on pin Servo1 to the servo object
  Serial.print(F("Servo configured on pin "));
  Serial.println(Servo1);

  pixels.begin();   //Utilizado para LEDS endereçaveis
  
  digitalWrite(Relay1,LOW);
  digitalWrite(Relay2,LOW);

  init_APDS9960();

  CommandStrIsComplete=0;
  Commandstr = "";

  Serial.print("RED ");
  pixels.setPixelColor(8,pixels.Color(0,255,0));    
  pixels.show();

  Serial.print(F("\r\n\n************************\n\r"));
  Serial.println(VersionStr);
  EnableButtonInterrupts();
  Serial.print(">");
  Open();
}

////////////////////////////////
////////////////////////////////
////////////////////////////////
void loop()
{   
  CLInerface(); // Processamento do CLI via porta serie
  
  if((!LastCommandWasOpen)&&(digitalRead(OpenButton)==LOW)) //ALTERAR PARA HIGH SE EMERGENGY BUTTON FOR NC
  {
    Open();
    LastCommandWasOpen=true;
  }
}

void DoNothing()
{
}

void Open()
{
  DetachButtonInterrupts();
  //Timer setup
  timer = timerBegin(0, 80, true); // Timer 0 programado comum prescale de 80 (o relógio é de 80MHz). countup (true) ou countdown (false)
  timerAttachInterrupt(timer, &TimerInterruptToFinishPulse, true); //Definição da rotina de serviço ao interrupt. edge (true) or level (false)  
  timerAlarmWrite(timer, RelayPulseDuration, true); //configurar o timer interrupt TimerInterruptToFinishPulse() para daqui a 0.5s
  timerAlarmEnable(timer); 
  
  digitalWrite(Relay2,LOW);  //desativa primeiro o rele1
  digitalWrite(Relay1,HIGH);  //ativa o rele2
  LastCommandWasOpen=true;
}

int Close()
{
  DetachButtonInterrupts();
  if(digitalRead(OpenButton)==HIGH) //Emergency Button not pressed //ALTERAR PARA LOW SE EMERGENGY BUTTON FOR NC
  {
    //Timer setup
    timer = timerBegin(0, 80, true); // Timer 0 programado comum prescale de 80 (o relógio é de 80MHz). countup (true) ou countdown (false)
    timerAttachInterrupt(timer, &TimerInterruptToFinishPulse, true); //Definição da rotina de serviço ao interrupt. edge (true) or level (false)  
    timerAlarmWrite(timer, RelayPulseDuration, true); //configurar o timer interrupt TimerInterruptToFinishPulse() para daqui a 0.5s
    timerAlarmEnable(timer); 
  
    digitalWrite(Relay1,LOW);   //desativa primeiro o rele1
    digitalWrite(Relay2,HIGH);  //ativa o rele2
    LastCommandWasOpen=false;
    return(0);
  }
  else
  {
    //Timer setup
    timer = timerBegin(0, 80, true); // Timer 0 programado comum prescale de 80 (o relógio é de 80MHz). countup (true) ou countdown (false)
    timerAttachInterrupt(timer, &ActivateInterruptsAgain, true); //Definição da rotina de serviço ao interrupt. edge (true) or level (false)  
    timerAlarmWrite(timer, InterruptActivationDelay, true); //configurar o timer interrupt TimerInterruptToFinishPulse() para daqui a 1s
    timerAlarmEnable(timer); 
    return(1);
  }
}
