#include <Bluepad32.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// --- Configuração dos Pinos dos Motores (Adapte conforme sua conexão) ---
// Motor Esquerdo (Ponte H Canal A, ex: L298N)
const int IN1_ESQ = 26; // Pino IN1
const int IN2_ESQ = 27; // Pino IN2
const int ENA_ESQ = 14; // Pino ENA (Velocidade PWM)

// Motor Direito (Ponte H Canal B, ex: L298N)
const int IN1_DIR = 32; // Pino IN3
const int IN2_DIR = 33; // Pino IN4
const int ENB_DIR = 25; // Pino ENB (Velocidade PWM)

// --- Configuração de Failsafe e Tempo ---
const unsigned long FAILSAFE_TIMEOUT = 500; // Tempo máximo sem sinal (ms)
unsigned long lastPacketTime = 0;
unsigned long previousMillis = 0;
const long interval = 20; // 50Hz (atualização do loop)

// --- Deadzones (Zona Morta) ---
const int DEADZONE_TRIGGER = 15;
const int DEADZONE_AXIS = 30;

// Variáveis globais para armazenar os comandos
int tracaoTotal = 0;
int comandoGiro = 0;

// Sensibilidade do giro (0.1 = muito suave, 0.5 = 50% de velocidade, 1.0 = velocidade total)
const float SENSIBILIDADE_GIRO = 0.5;

// --- Comando Especial ---
// R1: gira um pouco pra direita -> vai reto -> gira mais pra esquerda.
// L1: mesma sequência, só que espelhada
enum EstadoEspecial { ESPECIAL_INATIVO, ESPECIAL_GIRO1, ESPECIAL_RETO, ESPECIAL_GIRO2 };
EstadoEspecial estadoEspecial = ESPECIAL_INATIVO;
unsigned long especialInicioEtapa = 0;
int especialSentido = 1; // +1 = R1 (começa girando pra direita), -1 = L1 (começa pra esquerda)

bool r1PressionadoAntes = false; // usados para detectar o instante em que o botão é apertado
bool l1PressionadoAntes = false; // (e não repetir o combo enquanto o botão fica segurado)

const unsigned long ESPECIAL_DURACAO_GIRO = 250; // ms de cada etapa de giro
const unsigned long ESPECIAL_DURACAO_RETO = 200; // ms da etapa reta, no meio
const int ESPECIAL_VEL_FRENTE = 150;             // velocidade de avanço durante o combo (0-255)
const int ESPECIAL_GIRO_LEVE = 60;               // intensidade do primeiro giro (leve)
const int ESPECIAL_GIRO_FORTE = 110;             // intensidade do segundo giro (mais acentuado)

// Inicia o combo. sentido = +1 (R1) ou -1 (L1).
void iniciarEspecial(int sentido) {
  estadoEspecial = ESPECIAL_GIRO1;
  especialSentido = sentido;
  especialInicioEtapa = millis();
  Serial.println("ESPECIAL: HADOKEN");
}

// Retorna uma string com os nomes dos botões atualmente pressionados no controle.
String botoesPressionados(ControllerPtr ctl) {
  String botoes = "";

  if (ctl->a())      botoes += "A ";
  if (ctl->b())      botoes += "B ";
  if (ctl->x())      botoes += "X ";
  if (ctl->y())      botoes += "Y ";
  if (ctl->l1())     botoes += "L1 ";
  if (ctl->r1())     botoes += "R1 ";
  if (ctl->thumbL()) botoes += "L3 ";
  if (ctl->thumbR()) botoes += "R3 ";

  uint8_t dpad = ctl->dpad();
  if (dpad & 0x01) botoes += "CIMA ";
  if (dpad & 0x02) botoes += "BAIXO ";
  if (dpad & 0x04) botoes += "DIREITA ";
  if (dpad & 0x08) botoes += "ESQUERDA ";

  if (botoes == "") botoes = "NENHUM";
  return botoes;
}

// Função Failsafe: Zera todas as saídas físicas dos motores IMEDIATAMENTE
void desligarMotores() {
  analogWrite(ENA_ESQ, 0);
  digitalWrite(IN1_ESQ, LOW);
  digitalWrite(IN2_ESQ, LOW);

  analogWrite(ENB_DIR, 0);
  digitalWrite(IN1_DIR, LOW);
  digitalWrite(IN2_DIR, LOW);
}

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("Dispositivo conectado no indice %d\n", i);
      myControllers[i] = ctl;
      lastPacketTime = millis(); // Reseta cronômetro ao conectar
      break;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("Failsafe: Dispositivo desconectado do indice %d\n", i);
      myControllers[i] = nullptr;
      estadoEspecial = ESPECIAL_INATIVO; // cancela qualquer combo em andamento
      desligarMotores(); // FAILSAFE 1: Desconexão declarada
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configura pinos dos motores como saídas
  pinMode(IN1_ESQ, OUTPUT);
  pinMode(IN2_ESQ, OUTPUT);
  pinMode(ENA_ESQ, OUTPUT);
  pinMode(IN1_DIR, OUTPUT);
  pinMode(IN2_DIR, OUTPUT);
  pinMode(ENB_DIR, OUTPUT);

  desligarMotores(); // Garante que começa parado

  BP32.setup(&onConnectedController, &onDisconnectedController);

  // Reconexão automática: o Bluepad32 grava as chaves de pareamento (bonding) na
  // memória não-volátil do ESP32. Como o firmware nunca chama BP32.forgetBluetoothKeys(),
  // se o chip desligar/reiniciar ele reconecta sozinho ao último controle pareado,
  // sem precisar de um novo pareamento manual.
  BP32.enableNewBluetoothConnections(true);
}

// Função para aplicar os comandos de PWM calculados nos motores físicos
void moverRobo(int velocidadeEsq, int velocidadeDir) {
  // --- Motor Esquerdo ---
  if (velocidadeEsq > 0) { // Frente
    digitalWrite(IN1_ESQ, HIGH);
    digitalWrite(IN2_ESQ, LOW);
  } else if (velocidadeEsq < 0) { // Ré
    digitalWrite(IN1_ESQ, LOW);
    digitalWrite(IN2_ESQ, HIGH);
  } else { // Parado
    digitalWrite(IN1_ESQ, LOW);
    digitalWrite(IN2_ESQ, LOW);
  }
  analogWrite(ENA_ESQ, abs(velocidadeEsq)); // Aplica a velocidade (sempre positiva)

  // --- Motor Direito ---
  if (velocidadeDir > 0) { // Frente
    digitalWrite(IN1_DIR, HIGH);
    digitalWrite(IN2_DIR, LOW);
  } else if (velocidadeDir < 0) { // Ré
    digitalWrite(IN1_DIR, LOW);
    digitalWrite(IN2_DIR, HIGH);
  } else { // Parado
    digitalWrite(IN1_DIR, LOW);
    digitalWrite(IN2_DIR, LOW);
  }
  analogWrite(ENB_DIR, abs(velocidadeDir));
}

// Avança a sequência do combo especial e aplica a velocidade correspondente aos motores.
// Não bloqueia o loop: cada chamada só olha quanto tempo já passou na etapa atual.
// Retorna false quando não há combo em andamento (o controle normal deve assumir).
bool atualizarEspecial(unsigned long agora) {
  if (estadoEspecial == ESPECIAL_INATIVO) return false;

  unsigned long decorrido = agora - especialInicioEtapa;
  int giro = 0;

  switch (estadoEspecial) {
    case ESPECIAL_GIRO1:
      giro = especialSentido * ESPECIAL_GIRO_LEVE; // R1: gira leve pra direita | L1: pra esquerda
      if (decorrido >= ESPECIAL_DURACAO_GIRO) {
        estadoEspecial = ESPECIAL_RETO;
        especialInicioEtapa = agora;
      }
      break;

    case ESPECIAL_RETO:
      giro = 0; // etapa reta, sem giro
      if (decorrido >= ESPECIAL_DURACAO_RETO) {
        estadoEspecial = ESPECIAL_GIRO2;
        especialInicioEtapa = agora;
      }
      break;

    case ESPECIAL_GIRO2:
      giro = -especialSentido * ESPECIAL_GIRO_FORTE; // inverte e reforça: R1 termina indo pra esquerda | L1 pra direita
      if (decorrido >= ESPECIAL_DURACAO_GIRO) {
        estadoEspecial = ESPECIAL_INATIVO; // combo terminou, o controle normal volta a valer no próximo ciclo
      }
      break;

    default:
      return false;
  }

  int velEsq = constrain(ESPECIAL_VEL_FRENTE + giro, -255, 255); // mesma convenção do controle normal:
  int velDir = constrain(ESPECIAL_VEL_FRENTE - giro, -255, 255); // giro positivo = esq mais rápida = vira pra direita
  moverRobo(velEsq, velDir);
  return true;
}

void loop() {
  BP32.update();
  unsigned long currentMillis = millis();

  // Executa o loop principal no intervalo definido (50Hz)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    bool controllerActive = false;

    // Percorre os controles conectados
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      ControllerPtr ctl = myControllers[i];

      if (ctl && ctl->isConnected() && ctl->isGamepad()) {
        controllerActive = true;
        lastPacketTime = currentMillis; // Atualiza o "heartbeat"

        // 1. Acelerar (Gatilho Direito) -> Mapeado para 0..255
        int rawThrottle = ctl->throttle();
        if (rawThrottle < DEADZONE_TRIGGER) rawThrottle = 0;
        int pwmAcelerador = map(rawThrottle, 0, 1023, 0, 255);

        // Intensidade do gatilho em porcentagem, para o formato de saída do desafio
        int gatilhoPercent = map(rawThrottle, 0, 1023, 0, 100);

        // 2. Freio (Gatilho Esquerdo) -> Mapeado para 0..255
        int rawBrake = ctl->brake();
        if (rawBrake < DEADZONE_TRIGGER) rawBrake = 0;
        int pwmFreio = map(rawBrake, 0, 1023, 0, 255);
        int gatilhoEsqPercent = map(rawBrake, 0, 1023, 0, 100); // só para exibição

        // Define a Tração Total (Acelerador - Freio) varia de -255 a 255
        tracaoTotal = pwmAcelerador - pwmFreio;

        // 3. Direção (Analógico Esquerdo X) -> Mapeado para -255..255
        int rawX = ctl->axisX();
        // Trata a curva suave com a deadzone
        if (abs(rawX) < DEADZONE_AXIS) {
          rawX = 0;
        } else if (rawX > 0) {
          rawX = map(rawX, DEADZONE_AXIS, 512, 0, 512); // Curva suave a partir de 0
        } else {
          rawX = map(rawX, -DEADZONE_AXIS, -512, 0, -512); // Curva suave a partir de 0
        }
        // Mapeia para PWM (-255..255) e aplica o multiplicador de sensibilidade
        comandoGiro = map(rawX, -512, 512, -255, 255) * SENSIBILIDADE_GIRO;

        // Detecta o instante em que R1/L1 é pressionado
        bool r1Agora = ctl->r1();
        bool l1Agora = ctl->l1();
        bool comboAtivo = (estadoEspecial != ESPECIAL_INATIVO);
        if (r1Agora && !r1PressionadoAntes && !comboAtivo) iniciarEspecial(1);
        else if (l1Agora && !l1PressionadoAntes && !comboAtivo) iniciarEspecial(-1);
        r1PressionadoAntes = r1Agora;
        l1PressionadoAntes = l1Agora;

        if (!atualizarEspecial(currentMillis)) {
          // Mistura a tração total com o comando de giro
          int velEsqFinal = tracaoTotal + comandoGiro; // Gira pra direita: aumenta esq
          int velDirFinal = tracaoTotal - comandoGiro; // Gira pra direita: diminui dir

          // Garante que o PWM fique rigorosamente entre -255 e 255
          velEsqFinal = constrain(velEsqFinal, -255, 255);
          velDirFinal = constrain(velDirFinal, -255, 255);

          // Aplica as velocidades calculadas aos motores físicos
          moverRobo(velEsqFinal, velDirFinal);
        }
        // Se o combo estiver ativo, atualizarEspecial() já aplicou a velocidade aos motores

        // Saída no formato pedido pelo desafio: gatilho em % (direito e esquerdo,
        Serial.printf("GATILHO DIR: %d%% GATILHO ESQ: %d%% DIRE\u00c7\u00c3O X: %d DIRE\u00c7\u00c3O Y: %d BOT\u00d5ES: %s\n",
                      gatilhoPercent, gatilhoEsqPercent, ctl->axisX(), ctl->axisY(),
                      botoesPressionados(ctl).c_str());
      }
    }

    // FAILSAFE 2: Time-out por queda brusca de sinal
    if (controllerActive && (currentMillis - lastPacketTime > FAILSAFE_TIMEOUT)) {
      Serial.println("ALERTA FAILSAFE: Queda de sinal!");
      estadoEspecial = ESPECIAL_INATIVO; // cancela qualquer combo em andamento
      desligarMotores();
    }
    // Caso nenhum controle esteja na lista
    else if (!controllerActive) {
      estadoEspecial = ESPECIAL_INATIVO;
      desligarMotores();
    }
  }
}
