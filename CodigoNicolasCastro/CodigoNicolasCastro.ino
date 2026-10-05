// Lib PS4
#include <PS4Controller.h>
#include <ps4.h>
#include <ps4_int.h>

//Lib pra controlar motor
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

#define leftMotorPin 25
#define rightMotorPin 26

Servo MotorEsquerdo;
Servo MotorDireito;
//01:01:01:01:01:01 Exemplo MAC Adress
void setup() {
  PS4.begin("01:01:01:01:01:01");

  Serial.begin(9600);

  while(!PS4.isConnected()){
    Serial.println("Esperando conexao com o controle...");
    delay(250);
  }

  PS4.setLed(100,0,0);
  PS4.sendToController();

  MotorEsquerdo.attach(leftMotorPin);
  MotorDireito.attach(rightMotorPin);

  MotorEsquerdo.write(90);
  MotorDireito.write(90);

  Serial.println("Conexao Estabelecida!");
}

void loop() {

  if(PS4.isConnected()){
    int LStickX = PS4.LStickX();
    int LStickY = PS4.LStickY();

    int leftMotorOutput = constrain(map(LStickY,-127,127,0,180) + map(LStickX,-127,127,0,180),0,180);
    int rightMotorOutput = constrain(map(LStickY,-127,127,0,180) - map(LStickX,-127,127,0,180),0,180);

    if(leftMotorOutput > 95 || leftMotorOutput < 85 ){
      MotorEsquerdo.write(leftMotorOutput);
    }
    else{
      MotorEsquerdo.write(90);
    }
    if(rightMotorOutput > 95 || rightMotorOutput < 85 ){
      MotorDireito.write(rightMotorOutput);
    }
    else{
      MotorDireito.write(90);
    }

  }
  else{
    MotorEsquerdo.write(90);
    MotorDireito.write(90);
  }

}
