#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4); // ordem da pinagem do display LCD

const int botao = 2;
const int larguraTela = 16;

char blocoCima[larguraTela + 1];
char blocoBaixo[larguraTela + 1];
char nomeJogador[6];

bool botaoApertado = false;
bool jogadorEmCima = false;

int tempoObstaculo = 0;
int intervaloEntreObstaculos = 4;
int quantidadeVidas = 3;

const int limiteCaracteresNomeJogador = 5;
const int pontuacaoMaximaPermitida = 9999;

int pontuacaoJogador = 0;
bool pontuacaoMultiplaDeCem = false;
bool pontuacaoMultiplaDeTrezentos = false;
int delayJogo = 200;

// Mapeamento de posições na memória EEPROM
const int memoriaInicializadaAddr = 0;
const int nomeJogadorRankAddr[] = {1, 7, 13, 19, 25};
const int pontuacaoJogadorRankAddr[] = {31, 33, 35, 37, 39};

// Cópia das pontuações do ranking em memória
// para evitar acessos desnecessários à EEPROM
int pontuacoesRanking[5];

// Cópia dos nomes do ranking em memória
// para evitar acessos desnecessários à EEPROM
char nomesRanking[5][6];

void botaoPressionado() {
    botaoApertado = true;
}

// Inicializa o ranking com 5 bots com pontuação 100
void inicializarRanking() {
  char nomeBot[6] = "BOT00";
  char numeroBot[2];
  
  for(int i = 0; i < 5; i++) {
    sprintf(numeroBot, "%d", i + 1);
    nomeBot[4] = numeroBot[0];
    
    EEPROM.put(nomeJogadorRankAddr[i], nomeBot);
    EEPROM.put(pontuacaoJogadorRankAddr[i], 100);
  }
  
  EEPROM.write(memoriaInicializadaAddr, 0xAA);
}

void exibirRanking() {
  char linhaRanking[20];
  char jogador[6];
  int pontuacao;
  
  for(int i = 0; i < 5; i++) {
     strcpy(jogador, nomesRanking[i]);

     pontuacao = pontuacoesRanking[i];
    
  	 sprintf(linhaRanking, "%s - %d", jogador, pontuacao);
    
     Serial.println(linhaRanking);
  }
}

void criarCopiaRankingMemoria() {
  for(int i = 0; i < 5; i++) {
    EEPROM.get(pontuacaoJogadorRankAddr[i], pontuacoesRanking[i]);
    EEPROM.get(nomeJogadorRankAddr[i], nomesRanking[i]);
  }
}

// Verifica se o jogador colidiu com um obstáculo
void verificarColisao(){
	if (jogadorEmCima && blocoCima[1] == '#' || 
	    !jogadorEmCima && blocoBaixo[1] == '#') {

      quantidadeVidas--;
      
      // O jogador perdeu
      if (quantidadeVidas == 0)
        computarFimDeJogo();
  
    }
}

// Encerra o jogo
void computarFimDeJogo() {
	int posicaoRankingJogador; 
  
    lcd.clear();
    lcd.setCursor(15, 0);
    lcd.print("Game Over!");

    // Desliza o texto para a esquerda
    for (int i = 0; i < 12; i++) {
      lcd.scrollDisplayLeft(); 
      delay(150); 
    }
  
  	// Limita a pontuação a 9999 devido às limitações
    // de armazenamento na EEPROM
  	if(pontuacaoJogador > 9999)
      pontuacaoJogador = 9999;
  
	posicaoRankingJogador = atualizarRankingJogador();
  
    Serial.println("\nRanking dos 5 melhores jogadores:");
    exibirRanking();

    Serial.print("\nSua pontuacao foi: ");
    Serial.println(pontuacaoJogador);

    if(posicaoRankingJogador != -1) {
      Serial.println("Voce entrou pro ranking!");
      salvarPontuacaoJogador(posicaoRankingJogador);
    }

    // Espera o jogador pressionar o botão para recomeçar
    while (!botaoApertado){}

    quantidadeVidas = 3;
    pontuacaoJogador = 0;
    delayJogo = 200;
}

// Itera o array de pontuações do ranking que veio da EEPROM
// Verifica se o usuário entrou no ranking e salva o registro
// no array em caso afirmativo
// Retorna a posicao do array de ranking que o usuário entrou
// ou -1
int atualizarRankingJogador() {
  for(int i = 0; i < 5; i++) {
    if(pontuacaoJogador > pontuacoesRanking[i]) {
      pontuacoesRanking[i] = pontuacaoJogador;
      strcpy(nomesRanking[i], nomeJogador);

      return i;
  	}
  }
  return -1;
}

void salvarPontuacaoJogador(int posicao) {
  EEPROM.put(pontuacaoJogadorRankAddr[posicao], pontuacaoJogador);
  EEPROM.put(nomeJogadorRankAddr[posicao], nomeJogador);
}

// Inicio do jogo
void setup() {
    // Função de interrupção que verifica em tempo real se o botão foi pressionado. Evita a verificação dentro do loop, na qual causa conflito com a movimentação dos blocos
    attachInterrupt(digitalPinToInterrupt(botao), botaoPressionado, FALLING);    
    lcd.begin(16, 2);

    // Inicia a tela sem blocos
    memset(blocoCima, ' ', larguraTela);
    memset(blocoBaixo, ' ', larguraTela);
  
  	// Iniciar a comunicação com o monitor serial
  	Serial.begin(9600);
  
  	// Iniciar ranking na memória EEPROM, caso não tenha já sido iniciado
  	if (EEPROM.read(memoriaInicializadaAddr) != 0xAA)
      inicializarRanking();
  
  	criarCopiaRankingMemoria();
  
  	Serial.println("Bem vindo ao jogo Block-Dodge");
  	Serial.println("Vamos ver quem se esquiva melhor...");
  	Serial.println("Tu vais ter 3 vidas.");
  	Serial.println("Digite o nome do jogador: ");

  	// Espero o jogador escrever o nome
  	while(Serial.available() == 0){}

  	if (Serial.available()) {
      Serial.readBytes(nomeJogador, 5);
      nomeJogador[5] = '\0';
      Serial.println(nomeJogador);  
  	}
}

void loop() { 
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
      
      	// A cada 30 segundos, a pontuacao é múltipla de 300
      	pontuacaoMultiplaDeTrezentos = pontuacaoJogador && ((pontuacaoJogador % 300) == 0);
      	if(intervaloEntreObstaculos > 1 && pontuacaoMultiplaDeTrezentos)
          intervaloEntreObstaculos--;
      
        // Define o intervalo de espaços entre os obstáculos
        tempoObstaculo = intervaloEntreObstaculos;
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
  
    pontuacaoJogador += 2;
  
  	// A cada 10 segundos, a pontuação é múltipla de 100
    pontuacaoMultiplaDeCem = (pontuacaoJogador % 100) == 0;
  
  	// Reduz o delay o jogo em 10 a cada 10 segundos, se o delay for maior que 100
  	// Acelera a velocidade do jogo
    if(delayJogo > 100 && pontuacaoMultiplaDeCem)
	  delayJogo -= 10;

    // Controla a velocidade do jogo
    delay(delayJogo);
}
