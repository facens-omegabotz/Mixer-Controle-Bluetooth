/*  
  Desafio Mixer - Controle HID Bluetooth (ESP32-DEVKIT-V1 + Bluepad32)
  Funciona com DualSense (PS5), PS4, Xbox, etc.

  Pareamento DualSense: segure CREATE + PS até a luz piscar rápido.
  Especiais (D-pad + X): HADOKEN = 2,3,6 | SHORYUKEN = 6,2,3  (notação numpad)
*/
#include <Bluepad32.h>

const int      ZONA_MORTA   = 40;   // eixos: -511..512
const uint32_t PERIODO_MS   = 50;   // 20 linhas/s na serial
const uint32_t JANELA_COMBO = 1000; // ms entre direções do combo

ControllerPtr ctl = nullptr;
String seq = "";                    // direções recentes em notação numpad
uint32_t tSeq = 0, tPrint = 0;
char dirAnt = '5';
bool aAnt = false;

void onConnected(ControllerPtr c)    { if (!ctl) { ctl = c; Serial.println("Controle conectado"); } }
void onDisconnected(ControllerPtr c) { if (ctl == c) { ctl = nullptr; Serial.println("Controle desconectado, aguardando..."); } }

int eixo(int v) {                   // zona morta + escala -100..100
  if (abs(v) < ZONA_MORTA) return 0;
  int m = constrain(map(abs(v), ZONA_MORTA, 512, 0, 100), 0, 100);
  return v < 0 ? -m : m;
}

char direcao(uint8_t d) {           // D-pad -> dígito numpad ('5' = neutro)
  int x = ((d & DPAD_RIGHT) ? 1 : 0) - ((d & DPAD_LEFT) ? 1 : 0);
  int y = ((d & DPAD_UP) ? 1 : 0) - ((d & DPAD_DOWN) ? 1 : 0);
  return "123456789"[(y + 1) * 3 + (x + 1)] ;  // linha de baixo=1,2,3 | meio=4,5,6 | cima=7,8,9
}

void processa() {
  uint32_t agora = millis();

  // --- sequência de inputs ---
  char d = direcao(ctl->dpad());
  if (d != dirAnt) {
    dirAnt = d;
    if (d != '5') { seq += d; tSeq = agora; if (seq.length() > 6) seq.remove(0, 1); }
  }
  if (agora - tSeq > JANELA_COMBO) seq = "";

  bool a = ctl->a();                // X no DualSense
  if (a && !aAnt) {
    if (seq.endsWith("236"))      { Serial.println("ESPECIAL: CHOQUE DO TROVÃO");   seq = ""; }
    else if (seq.endsWith("623")) { Serial.println("ESPECIAL: LANÇA-CHAMAS"); seq = ""; }
  }
  aAnt = a;

  // --- telemetria ---
  if (agora - tPrint < PERIODO_MS) return;
  tPrint = agora;
  int x = eixo(ctl->axisX());
  int y = eixo(-ctl->axisY());      // cima = positivo
  int gatilho = map(ctl->throttle(), 0, 1023, 0, 100);  // R2
  Serial.printf("GATILHO: %d%% DIRECAO X: %d DIRECAO Y: %d%s\n", gatilho, x, y, (!x && !y) ? " [PARADO]" : "");
}   

void setup() {
  Serial.begin(115200);
  BP32.setup(&onConnected, &onDisconnected);
  // Não use forgetBluetoothKeys(): as chaves na NVS permitem a reconexão automática.
}
void loop() {
  if (BP32.update() && ctl && ctl->isConnected() && ctl->hasData()) processa();
  delay(1);
}
