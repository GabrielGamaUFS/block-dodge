#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4); // ordem da pinagem do display LCD

const int botao = 2;
const int larguraTela = 16;

char blocoCima[larguraTela + 1];
char blocoBaixo[larguraTela + 1];

bool botaoApertado = false;
bool jogadorEmCima = false;

int tempoObstaculo = 0;

void botaoPressionado() {
    botaoApertado = true;
}

void setup() {
    // Função de interrupção que verifica em tempo real se o botão foi pressionado. Evita a verificação dentro do loop, na qual causa conflito com a movimentação dos blocos
	attachInterrupt(digitalPinToInterrupt(botao), botaoPressionado, FALLING);    
    lcd.begin(16, 2);

    // Inicia a tela sem blocos
    memset(blocoCima, ' ', larguraTela);
    memset(blocoBaixo, ' ', larguraTela);
}

void loop() {
    if (botaoApertado) {
        jogadorEmCima = !jogadorEmCima;
        botaoApertado = false;
    }

    // Move os obstáculos/blocos para a esquerda
    memmove(blocoCima, blocoCima + 1, larguraTela - 1);
    memmove(blocoBaixo, blocoBaixo + 1, larguraTela - 1);

    // Adiciona um novo obstáculo/bloco aleatório ao array, se tempo == 0
    if (tempoObstaculo == 0) {
        if (random(2) == 0) {
            blocoCima[larguraTela - 1] = '#';
            blocoBaixo[larguraTela - 1] = ' ';
        }
        else {
            blocoCima[larguraTela - 1] = ' ';
            blocoBaixo[larguraTela - 1] = '#';
        }
        // Define o intervalo de espaços entre os obstáculos
        tempoObstaculo = 4; // Fazer função para diminuir conforme o tempo/pontuação passe
    }
    else {
        blocoCima[larguraTela - 1] = ' ';
        blocoBaixo[larguraTela - 1] = ' ';
        tempoObstaculo--;
    }

    // Desenha no display
    lcd.clear();
    lcd.setCursor(0, jogadorEmCima ? 0 : 1);
    lcd.print('X'); // formato do jogador
    lcd.setCursor(1, 0);
    lcd.print(blocoCima);
    lcd.setCursor(1, 1);
    lcd.print(blocoBaixo);

    // Controla a velocidade do jogo
    delay(200); // Fazer função para diminuir conforme o tempo/pontuação passe
}
