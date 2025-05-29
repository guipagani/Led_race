#include <Adafruit_NeoPixel.h>

// === Fitas ===
#define PINO_FITA1 6
#define NUM_LEDS1 330
#define PINO_FITA2 7
#define NUM_LEDS2 130

Adafruit_NeoPixel fita1(NUM_LEDS1, PINO_FITA1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fita2(NUM_LEDS2, PINO_FITA2, NEO_GRB + NEO_KHZ800);

// === Botões ===
#define BOTAO_J1 A1
#define BOTAO_J2 A4
#define BOTAO_J3 A3
#define BOTAO_J4 A2

// === Jogadores ===
int posJ[4] = {0, 0, 0, 0};
int voltas[4] = {0, 0, 0, 0};
bool ativo[4] = {false, false, false, false};

const int TOTAL_VOLTAS = 2;

// === Controle de vitória ===
bool venceu = false;
int vencedor = -1;
unsigned long tempoPisca = 0;
bool estadoPisca = false;
int contaPisca = 0;
bool esperando = true;

// === Debounce ===
int estadoBotao[4] = {HIGH, HIGH, HIGH, HIGH};
int ultimoEstadoBotao[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long ultimoTempoBotao[4] = {0, 0, 0, 0};
const unsigned long debounceDelay = 5;

// === Rainbow ===
int offsetRainbow = 0;

void setup() {
  fita1.begin();
  fita2.begin();
  fita1.clear();
  fita2.clear();
  pintarFita2();
  fita1.show();
  fita2.show();

  pinMode(BOTAO_J1, INPUT_PULLUP);
  pinMode(BOTAO_J2, INPUT_PULLUP);
  pinMode(BOTAO_J3, INPUT_PULLUP);
  pinMode(BOTAO_J4, INPUT_PULLUP);

  Serial.begin(9600);

  // Animação inicial
  fita1.fill(fita1.Color(255, 255, 255));
  fita2.fill(fita2.Color(255, 255, 255));
  fita1.show();
  fita2.show();
  delay(500);
  fita1.clear();
  fita2.clear();
  pintarFita2();
  fita1.show();
  fita2.show();
}

void loop() {
  pintarFita2(); // Mantém as marcações da fita2 sempre acesas

  int botoes[4] = {
    digitalRead(BOTAO_J1),
    digitalRead(BOTAO_J2),
    digitalRead(BOTAO_J3),
    digitalRead(BOTAO_J4)
  };

  if (esperando) {
    efeitoRainbow();
    if (botoes[0] == LOW || botoes[1] == LOW || botoes[2] == LOW || botoes[3] == LOW) {
      esperando = false;
      semaforo();
      fita1.clear();
      fita1.show();
    }
    delay(15);
    return;
  }

  if (venceu) {
    piscar();
    return;
  }

  for (int i = 0; i < 4; i++) {
    processaBotao(botoes[i], i);
  }

  atualizarFitas();
}

void processaBotao(int leitura, int j) {
  if (leitura != ultimoEstadoBotao[j]) {
    ultimoTempoBotao[j] = millis();
  }
  if (millis() - ultimoTempoBotao[j] > debounceDelay) {
    if (leitura == LOW && estadoBotao[j] == HIGH) {
      if (!ativo[j]) ativo[j] = true;
      avancar(j);
    }
    estadoBotao[j] = leitura;
  }
  ultimoEstadoBotao[j] = leitura;
}

void avancar(int j) {
  posJ[j] += 5;
  if (posJ[j] < NUM_LEDS1) return;
  if (posJ[j] < NUM_LEDS1 + NUM_LEDS2) return;

  posJ[j] -= (NUM_LEDS1 + NUM_LEDS2);
  voltas[j]++;

  if (voltas[j] >= TOTAL_VOLTAS) {
    venceu = true;
    vencedor = j;
    tempoPisca = millis();
    contaPisca = 0;
    estadoPisca = false;
    Serial.print("Jogador ");
    Serial.print(j + 1);
    Serial.println(" venceu!");
  }
}

void atualizarFitas() {
  fita1.clear();
  fita2.clear();
  pintarFita2(); // Mantém as áreas coloridas na fita2

  byte cores[4][3] = {
    {0, 255, 0},     // Verde
    {0, 0, 255},     // Azul
    {255, 150, 0},   // Amarelo
    {255, 0, 0}      // Vermelho
  };

  for (int i = 0; i < 4; i++) {
    if (ativo[i])
      desenharRastro(posJ[i], cores[i][0], cores[i][1], cores[i][2]);
  }

  fita1.show();
  fita2.show();
}

void desenharRastro(int pos, byte r, byte g, byte b) {
  if (pos < NUM_LEDS1) {
    desenharFita(fita1, NUM_LEDS1, pos, r, g, b);
  } else {
    desenharFita(fita2, NUM_LEDS2, pos - NUM_LEDS1, r, g, b);
  }
}

void desenharFita(Adafruit_NeoPixel &fita, int num, int pos, byte r, byte g, byte b) {
  for (int i = 0; i < 5; i++) {
    int p = (pos + i) % num;
    adicionarCor(fita, p, r, g, b);
  }
  for (int i = 1; i <= 5; i++) {
    int p = pos - i;
    if (p < 0) p += num;
    float fator = 1.0 - (float)i / 6.0;
    adicionarCor(fita, p, r * fator, g * fator, b * fator);
  }
}

void adicionarCor(Adafruit_NeoPixel &fita, int p, byte r, byte g, byte b) {
  uint32_t c = fita.getPixelColor(p);
  byte r1 = (c >> 16) & 0xFF;
  byte g1 = (c >> 8) & 0xFF;
  byte b1 = c & 0xFF;
  fita.setPixelColor(p, fita.Color(min(r + r1, 255), min(g + g1, 255), min(b + b1, 255)));
}

void pintarFita2() {
  for (int i = 0; i < NUM_LEDS2; i++) {
    int led = NUM_LEDS2 - 1 - i;
    if (led >= 50 && led <= 70) {
      fita2.setPixelColor(led, fita2.Color(0, 0, 255));  // Azul
    } else if (led >= 105 && led <= 120) {
      fita2.setPixelColor(led, fita2.Color(0, 255, 0));  // Verde
    } else {
      fita2.setPixelColor(led, fita2.Color(255, 1, 7));  // Rosa
    }
  }
}

void piscar() {
  if (millis() - tempoPisca >= 150) {
    tempoPisca = millis();
    estadoPisca = !estadoPisca;

    byte cores[4][3] = {
      {0, 255, 0},
      {0, 0, 255},
      {255, 150, 0},
      {255, 0, 0}
    };

    if (estadoPisca) {
      fita1.fill(fita1.Color(cores[vencedor][0], cores[vencedor][1], cores[vencedor][2]));
      fita2.fill(fita2.Color(cores[vencedor][0], cores[vencedor][1], cores[vencedor][2]));
    } else {
      fita1.clear();
      pintarFita2();
    }

    fita1.show();
    fita2.show();

    if (estadoPisca) contaPisca++;
    if (contaPisca > 10) {
      venceu = false;
      esperando = true;
      for (int i = 0; i < 4; i++) {
        posJ[i] = 0;
        voltas[i] = 0;
        ativo[i] = false;
      }
      fita1.clear();
      fita2.clear();
      pintarFita2();
      fita1.show();
      fita2.show();
    }
  }
}

void efeitoRainbow() {
  offsetRainbow = (offsetRainbow + 2) % 256;
  int arco = NUM_LEDS1 / 3;
  fita1.clear();

  for (int a = 0; a < 3; a++) {
    for (int i = 0; i < arco; i++) {
      int led = a * arco + i;
      int hue = (i * 256 / arco + offsetRainbow) & 255;
      uint32_t cor = fita1.ColorHSV(hue * 256);
      fita1.setPixelColor(led, cor);
    }
  }
  fita1.show();
}

void semaforo() {
  // Vermelho
  fita1.fill(fita1.Color(255, 0, 0));
  fita1.show();
  delay(800);

  // Amarelo
  fita1.fill(fita1.Color(255, 150, 0));
  fita1.show();
  delay(800);

  // Verde (2 segundos)
  fita1.fill(fita1.Color(0, 255, 0));
  fita1.show();
  delay(2000);

  // Apagar
  fita1.clear();
  fita1.show();
}
