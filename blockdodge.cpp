#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4); // ordem da pinagem do display LCD

const int botao = 2;
const int larguraTela = 16;

char blocoCima[larguraTela + 1];
char blocoBaixo[larguraTela + 1];
String nomeJogador;

bool botaoApertado = false;
bool jogadorEmCima = false;
bool novoJogador = true;

int tempoObstaculo = 0;
int quantidadeVidas = 3;

void botaoPressionado() {
    botaoApertado = true;
}


// Verifica se o jogador colidiu com um obstáculo
void verificarColisao(){
	if (jogadorEmCima && blocoCima[1] == '#' || 
	    !jogadorEmCima && blocoBaixo[1] == '#') {

      quantidadeVidas--;
      
      // O jogador perdeu
      if (quantidadeVidas == 0){
        lcd.clear();
    	lcd.setCursor(15, 0);
      	lcd.print("Game Over!");
        
        // Desliza o texto para a esquerda
        for (int i = 0; i < 12; i++) {
            lcd.scrollDisplayLeft(); 
            delay(150); 
        }
        
        // Espera o jogador pressionar o botão para recomeçar
        while (!botaoApertado){
        }
        
        quantidadeVidas = 3;
        
      }
    }
}

// Inicio do jogo, com um novo jogador


void setup() {
    // Função de interrupção que verifica em tempo real se o botão foi pressionado. Evita a verificação dentro do loop, na qual causa conflito com a movimentação dos blocos
    attachInterrupt(digitalPinToInterrupt(botao), botaoPressionado, FALLING);    
    lcd.begin(16, 2);

    // Inicia a tela sem blocos
    memset(blocoCima, ' ', larguraTela);
    memset(blocoBaixo, ' ', larguraTela);
  
  	// Iniciar a comunicação com o monitor serial
  	Serial.begin(9600);
}

void loop() {
    	// Inicio de um novo jogo, com um novo jogador
    if (novoJogador) {
      Serial.println("Bem vindo ao jogo Block-Dodge");
      Serial.println("Vamos ver quem se esquiva melhor...");
      Serial.println("Tu vais ter 3 vidas.");
      Serial.println("Digite o nome do jogador: ");
      
      // Espero o jogador escrever o nome
      while(Serial.available() == 0){
      }
      
      if (Serial.available()) {  
        nomeJogador = Serial.readString();  
        Serial.println(nomeJogador);  
      }
    }
  	novoJogador = false;
  
  
    if (botaoApertado) {
        jogadorEmCima = !jogadorEmCima;
        botaoApertado = false;
    }
  
  	// Verifica se o jogador colidiu com um obstáculo
	verificarColisao();
    
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
