#include <Adafruit_NeoPixel.h>

// Configurações da fita
#define PINO_FITA 6
#define NUM_LEDS 300

Adafruit_NeoPixel fita = Adafruit_NeoPixel(NUM_LEDS, PINO_FITA, NEO_GRB + NEO_KHZ800);

// Botões dos jogadores
#define BOTAO_JOGADOR1 A1  // Azul
#define BOTAO_JOGADOR2 A2  // Vermelho
#define BOTAO_JOGADOR3 A3  // Amarelo
#define BOTAO_JOGADOR4 A4  // Verde

// Posições e voltas dos jogadores
int posicaoJogador1 = 0;
int posicaoJogador2 = 0;
int posicaoJogador3 = 0;
int posicaoJogador4 = 0;

int voltasJogador1 = 0;
int voltasJogador2 = 0;
int voltasJogador3 = 0;
int voltasJogador4 = 0;

const int TOTAL_VOLTAS = 3;

// Ativação dos jogadores
bool ativoJogador1 = false;
bool ativoJogador2 = false;
bool ativoJogador3 = false;
bool ativoJogador4 = false;

// Estados dos botões
int estadoBotao1 = HIGH;
int estadoBotao2 = HIGH;
int estadoBotao3 = HIGH;
int estadoBotao4 = HIGH;

int ultimoEstadoBotao1 = HIGH;
int ultimoEstadoBotao2 = HIGH;
int ultimoEstadoBotao3 = HIGH;
int ultimoEstadoBotao4 = HIGH;

unsigned long ultimoDebounceTempo1 = 0;
unsigned long ultimoDebounceTempo2 = 0;
unsigned long ultimoDebounceTempo3 = 0;
unsigned long ultimoDebounceTempo4 = 0;

const unsigned long debounceDelay = 2;

// Controle de vitória
bool venceu = false;
int jogadorVencedor = 0;
unsigned long tempoUltimoPisca = 0;
bool estadoPisca = false;
int contadorPisca = 0;

// Controle de estado geral
bool esperandoInicio = true;

void setup() {
  fita.begin();
  fita.clear();
  desenharFaixaRosa();
  fita.show();

  pinMode(BOTAO_JOGADOR1, INPUT_PULLUP);
  pinMode(BOTAO_JOGADOR2, INPUT_PULLUP);
  pinMode(BOTAO_JOGADOR3, INPUT_PULLUP);
  pinMode(BOTAO_JOGADOR4, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  desenharFaixaRosa(); // Faixa rosa sempre acesa

  int leitura1 = digitalRead(BOTAO_JOGADOR1);
  int leitura2 = digitalRead(BOTAO_JOGADOR2);
  int leitura3 = digitalRead(BOTAO_JOGADOR3);
  int leitura4 = digitalRead(BOTAO_JOGADOR4);

  // Estado aguardando início
  if (esperandoInicio) {
    if (leitura1 == LOW || leitura2 == LOW || leitura3 == LOW || leitura4 == LOW) {
      esperandoInicio = false;
      semaforo();
      fita.clear();
      desenharFaixaRosa();
      fita.show();
    }
    return;
  }

  if (venceu) {
    piscarVencedor();
    return;
  }

  // Processa botões dos jogadores
  processaBotao(leitura1, estadoBotao1, ultimoEstadoBotao1, ultimoDebounceTempo1, ativoJogador1, 1);
  processaBotao(leitura2, estadoBotao2, ultimoEstadoBotao2, ultimoDebounceTempo2, ativoJogador2, 2);
  processaBotao(leitura3, estadoBotao3, ultimoEstadoBotao3, ultimoDebounceTempo3, ativoJogador3, 3);
  processaBotao(leitura4, estadoBotao4, ultimoEstadoBotao4, ultimoDebounceTempo4, ativoJogador4, 4);
}

void processaBotao(int leitura, int &estado, int &ultimoEstado, unsigned long &ultimoTempo, bool &ativo, int jogador) {
  if (leitura != ultimoEstado) {
    ultimoTempo = millis();
  }
  if ((millis() - ultimoTempo) > debounceDelay) {
    if (leitura == LOW && estado == HIGH) {
      if (!ativo) ativo = true;
      avancarJogador(jogador);
      atualizarFita();
    }
    estado = leitura;
  }
  ultimoEstado = leitura;
}

void avancarJogador(int jogador) {
  int* posicao;
  int* voltas;

  switch (jogador) {
    case 1: posicao = &posicaoJogador1; voltas = &voltasJogador1; break;
    case 2: posicao = &posicaoJogador2; voltas = &voltasJogador2; break;
    case 3: posicao = &posicaoJogador3; voltas = &voltasJogador3; break;
    case 4: posicao = &posicaoJogador4; voltas = &voltasJogador4; break;
  }

  *posicao += 5;

  // Pular a faixa rosa entre 140 e 160
  if (*posicao > 139 && *posicao < 161) {
    *posicao = 161;
  }

  if (*posicao >= NUM_LEDS) {
    *posicao -= NUM_LEDS;
    (*voltas)++;
  }

  if (*voltas >= TOTAL_VOLTAS) {
    iniciarVitoria(jogador);
  }
}

void atualizarFita() {
  fita.clear();

  if (ativoJogador1) desenharRastro(posicaoJogador1, 0, 0, 255);      // Azul
  if (ativoJogador2) desenharRastro(posicaoJogador2, 255, 0, 0);      // Vermelho
  if (ativoJogador3) desenharRastro(posicaoJogador3, 255, 150, 0);    // Amarelo (laranja)
  if (ativoJogador4) desenharRastro(posicaoJogador4, 0, 255, 0);      // Verde

  desenharFaixaRosa(); // Desenha faixa rosa sempre após limpar e atualizar corredores

  fita.show();
}

void desenharRastro(int posicao, byte r, byte g, byte b) {
  for (int i = 0; i < 5; i++) {
    int pixel = (posicao + i) % NUM_LEDS;
    adicionarCor(pixel, r, g, b);
  }

  for (int i = 1; i <= 5; i++) {
    int pixel = posicao - i;
    if (pixel < 0) pixel += NUM_LEDS;

    float fatorBrilho = 1.0 - (float)i / 6.0;
    byte r_dim = (byte)(r * fatorBrilho);
    byte g_dim = (byte)(g * fatorBrilho);
    byte b_dim = (byte)(b * fatorBrilho);
    adicionarCor(pixel, r_dim, g_dim, b_dim);
  }
}

void adicionarCor(int pixel, byte r, byte g, byte b) {
  uint32_t corAtual = fita.getPixelColor(pixel);

  byte r1 = (corAtual >> 16) & 0xFF;
  byte g1 = (corAtual >> 8) & 0xFF;
  byte b1 = corAtual & 0xFF;

  byte rFinal = min(r1 + r, 255);
  byte gFinal = min(g1 + g, 255);
  byte bFinal = min(b1 + b, 255);

  fita.setPixelColor(pixel, fita.Color(rFinal, gFinal, bFinal));
}

void desenharFaixaRosa() {
  // Faixa rosa entre os LEDs 140 e 160
  for (int i = 140; i <= 160; i++) {
    fita.setPixelColor(i, fita.Color(255,7,8)); // Rosa
  }
}

void iniciarVitoria(int jogador) {
  venceu = true;
  jogadorVencedor = jogador;
  tempoUltimoPisca = millis();
  contadorPisca = 0;
  estadoPisca = false;
  Serial.print("Jogador ");
  Serial.print(jogador);
  Serial.println(" venceu!");
}

void piscarVencedor() {
  unsigned long agora = millis();
  if (agora - tempoUltimoPisca >= 300) {
    tempoUltimoPisca = agora;
    estadoPisca = !estadoPisca;

    if (estadoPisca) {
      uint32_t cor;
      switch (jogadorVencedor) {
        case 1: cor = fita.Color(0, 0, 255); break;        // Azul
        case 2: cor = fita.Color(255, 0, 0); break;        // Vermelho
        case 3: cor = fita.Color(255, 150, 0); break;      // Amarelo
        case 4: cor = fita.Color(0, 255, 0); break;        // Verde
      }
      for (int i = 0; i < NUM_LEDS; i++) {
        fita.setPixelColor(i, cor);
      }
    } else {
      fita.clear();
      desenharFaixaRosa();  // Mantém a faixa rosa acesa durante o piscar
    }
    fita.show();

    if (!estadoPisca) {
      contadorPisca++;
      if (contadorPisca >= 5) {
        venceu = false;
        resetarCorrida();
      }
    }
  }
}

void resetarCorrida() {
  posicaoJogador1 = posicaoJogador2 = posicaoJogador3 = posicaoJogador4 = 0;
  voltasJogador1 = voltasJogador2 = voltasJogador3 = voltasJogador4 = 0;
  ativoJogador1 = ativoJogador2 = ativoJogador3 = ativoJogador4 = false;
  esperandoInicio = true;
  fita.clear();
  desenharFaixaRosa();  // Faixa rosa acesa após reset
  fita.show();
}

void pintarForaFaixaRosa(uint32_t cor) {
  // Pinta LEDs de 0 a 139
  fita.fill(cor, 0, 140);
  // Pinta LEDs de 161 a 299 (total 139 LEDs)
  fita.fill(cor, 161, NUM_LEDS - 161);
}

void semaforo() {
  pintarForaFaixaRosa(fita.Color(255, 0, 0));    // Vermelho
  desenharFaixaRosa();
  fita.show();
  delay(1030);

  pintarForaFaixaRosa(fita.Color(255, 150, 0));  // Amarelo
  desenharFaixaRosa();
  fita.show();
  delay(1030);

  pintarForaFaixaRosa(fita.Color(0, 255, 0));    // Verde
  desenharFaixaRosa();
  fita.show();
  delay(1030);

  // Limpa fora faixa rosa e desenha faixa rosa no final
  fita.clear();
  desenharFaixaRosa();
  fita.show();
}