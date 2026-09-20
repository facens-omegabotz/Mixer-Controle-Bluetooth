# Desafio Mixer controle HID Bluetooth

## 💭 Objetivo

O objetivo principal do desafio é desenvolver um firmware para a placa ESP32-DEVKIT-V1 que seja capaz de receber sinais analógicos vindos do controle e convertê-los em texto para sinalizar direcionamento, intensidade e, se em um controle Bluetooth de videogame, os botões pressionados.

## 🔀 Como submeter entradas no desafio

Para registrar o seu código, faça uma branch neste repositório e, quando terminado, um pull request. A branch deve ter o seu nome. A estrutura de branches do repositório deve ser como ilustrada abaixo.

```
  └── Mixer Controle Bluetooth/
    ├── main/
    ├── <seu_nome>/
    └── ...
```

## ✅ Requisitos funcionais

- O firmware deve receber sinais vindos de um controle HID Bluetooth;
- O firmware deve ser capaz de indicar os valores que o controle Bluetooth envia;
- O firmware deve possuir a capacidade de interpretar uma zona morta onde o robô deve estar parado;
- O firmware deve ser capaz de indicar a intensidade com a qual um gatilho de sua escolha é pressionado;
- O firmware deve ser capaz de indicar valores que mostrem a eventual direção que um robô tomaria;
- Na eventualidade de desligamento do chip, o firmware deve ser capaz de conectar-se automaticamente ao controle desejado;
- **ADICIONAL:** O firmware deve ser capaz de interpretar sequências de inputs como um comando especial.

### Exemplos de saída esperados

Os exemplos de saída não devem limitar o que deve ser mostrado. Eles são mostrados como uma ilustração de eventual saída visível por um monitor serial.

```shell
GATILHO: 56% DIREÇÃO X: -25 DIREÇÃO Y: 45
GATILHO: 56% DIREÇÃO X: -25 DIREÇÃO Y: 45 
GATILHO: 56% DIREÇÃO X: -25 DIREÇÃO Y: 45
ESPECIAL: HADOKEN
GATILHO: 56% DIREÇÃO X: -25 DIREÇÃO Y: 45
GATILHO: 56% DIREÇÃO X: -25 DIREÇÃO Y: 45
GATILHO: 56% DIREÇÃO X: -25 DIREÇÃO Y: 45
```

## Materiais auxiliares

### 📽️ Vídeos

- [Usando um controle de PS4 para pilotar seu robô!](https://youtu.be/hXP_kQ_EbkA?si=DOoTi1pM-5TxfQ1z)
- [Control Motors, Servos and LEDs with a Game Controller & ESP32 (em inglês)](https://youtu.be/Laa93Wj7f-I?si=rSn0PxJesIaj3Qw8)

### 🔗 Links

- [Documentação Bluepad32](https://bluepad32.readthedocs.io/en/latest/)
- [ESP32-DEVKIT-V1 pin reference (em inglês)](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [PlatformIO](https://platformio.org/)

### 🗂️ Repositórios

- [PS4ControllerESP32_Robot](https://github.com/nickcastros/PS4ControllerESP32_Robot)
- [RiscaFaca2021](https://github.com/nickcastros/RiscaFaca2021)