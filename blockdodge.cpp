#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4); // Ordem da pinagem do display LCD

const int larguraTela = 16;

char blocoCima[larguraTela];
char blocoBaixo[larguraTela];

int tempoObstaculo = 0;

void setup() {
    lcd.begin(16, 2);

    // Inicia a tela sem blocos
    memset(blocoCima, ' ', larguraTela);
    memset(blocoBaixo, ' ', larguraTela);
}

void loop() {
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
    lcd.setCursor(0, 0);
    lcd.print(blocoCima);
    lcd.setCursor(0, 1);
    lcd.print(blocoBaixo);
    // Move os obstáculos/blocos para a esquerda
    memmove(blocoCima, blocoCima + 1, larguraTela - 1);
    memmove(blocoBaixo, blocoBaixo + 1, larguraTela - 1);

    // Controla a velocidade do jogo
    delay(200); // Fazer função para diminuir conforme o tempo/pontuação passe
}
