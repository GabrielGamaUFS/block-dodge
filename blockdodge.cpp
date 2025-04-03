#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4); // ordem da pinagem do display LCD

const int botao = 2;
const int larguraTela = 16;

unsigned long tempoSom = 0; 
bool tocandoSom = false;

char blocoCima[larguraTela + 1];
char blocoBaixo[larguraTela + 1];
char nomeJogador[6];

bool botaoApertado = false;
bool jogadorEmCima = false;

int tempoObstaculo = 0;
int intervaloEntreObstaculos = 4;
int quantidadeVidas = 3;
int som = 9; //Pino do buzzer

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
	if (jogadorEmCima && blocoCima[0] == '#' || 
	    !jogadorEmCima && blocoBaixo[0] == '#') {

      quantidadeVidas--;
      Serial.println("Iiiiiiiiiih...");
      
      // Inicia o som
      tone(som, 400);
      tempoSom = millis(); // Salva o tempo em que o som começou
      tocandoSom = true;   
      
      // O jogador perdeu
      if (quantidadeVidas == 0)
        computarFimDeJogo();
    }
}

// Verifica se já passou tempo suficiente para parar o som
void atualizarSom() {
    if (tocandoSom && millis() - tempoSom >= 10) { // Som toca por 10ms
        noTone(som);
        tocandoSom = false;
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
  	if(pontuacaoJogador > pontuacaoMaximaPermitida)
      pontuacaoJogador = 9999;
  
	posicaoRankingJogador = atualizarRankingJogador();
  
    Serial.println("\nRanking dos 5 melhores jogadores:");
    exibirRanking();

    Serial.print("\nSua pontuacao foi: ");
    Serial.println(pontuacaoJogador);

    if(posicaoRankingJogador != -1) {
      Serial.println("Voce entrou pro ranking!");
      salvarRankingEEPROM(posicaoRankingJogador);
    }

    // Espera o jogador pressionar o botão para recomeçar
  	while (!botaoApertado)
      	atualizarSom();

    quantidadeVidas = 3;
    pontuacaoJogador = 0;
    delayJogo = 200;
  	intervaloEntreObstaculos = 4;
}

// Itera o array de pontuações do ranking que veio da EEPROM
// Verifica se o usuário entrou no ranking e salva o registro
// no array em caso afirmativo
// Retorna a posicao do array de ranking que o usuário entrou
// ou -1
int atualizarRankingJogador() {
  char nomeJogadorSubstituido[6];
  int pontuacaoJogadorSubstituido;
  
  for(int i = 0; i < 5; i++) {
    if(pontuacaoJogador > pontuacoesRanking[i]) {
      strcpy(nomeJogadorSubstituido, nomesRanking[i]);
      pontuacaoJogadorSubstituido = pontuacoesRanking[i];
      
      pontuacoesRanking[i] = pontuacaoJogador;
      strcpy(nomesRanking[i], nomeJogador);
      
      cascatearAtualizacaoRanking(i + 1, nomeJogadorSubstituido, pontuacaoJogadorSubstituido);

      return i;
  	}
  }
  return -1;
}

void cascatearAtualizacaoRanking(int posicao, char* nome, int pontuacao) {
  char nomeJogadorSubstituido[6];
  int pontuacaoJogadorSubstituido;
  
  while(posicao < 5) {
    // Salva dados que serão substituídos
     pontuacaoJogadorSubstituido = pontuacoesRanking[posicao];
     strcpy(nomeJogadorSubstituido, nomesRanking[posicao]);
    
    // Substitui pelo novo jogador a ocupar o ranking
  	 pontuacoesRanking[posicao] = pontuacao;
     strcpy(nomesRanking[posicao], nome);
    
    // O jogador que foi substituido ocupará a posição seguinte
     pontuacao = pontuacaoJogadorSubstituido;
     strcpy(nome, nomeJogadorSubstituido);
            
     posicao++;
  }
}

void salvarRankingEEPROM(int posicao) {
  while(posicao < 5) {
    EEPROM.put(pontuacaoJogadorRankAddr[posicao], pontuacoesRanking[posicao]);
    EEPROM.put(nomeJogadorRankAddr[posicao], nomesRanking[posicao]);
    posicao++;
  }
}

// Inicio do jogo
void setup() {
    // Função de interrupção que verifica em tempo real se o botão foi pressionado. Evita a verificação dentro do loop, na qual causa conflito com a movimentação dos blocos
    attachInterrupt(digitalPinToInterrupt(botao), botaoPressionado, FALLING);    
    lcd.begin(16, 2);
  	
  	// Aviso para o jogador inserir o nome no monitor serial
  	lcd.setCursor(0, 0);
  	lcd.print("     ABRA O"); 
  	lcd.setCursor(0, 1);
  	lcd.print(" MONITOR SERIAL");
  
  	// Iniciar a comunicação com o monitor serial
  	Serial.begin(9600);
  
  	// Pino do som
  	pinMode(9, OUTPUT);
  
  	// Iniciar ranking na memória EEPROM, caso não tenha já sido iniciado
  	if (EEPROM.read(memoriaInicializadaAddr) != 0xAA)
      inicializarRanking();
  
  	criarCopiaRankingMemoria();
  
  	Serial.println("Bem vindo ao jogo Block-Dodge");
  	Serial.println("Vamos ver quem se esquiva melhor...");
  	Serial.println("Tu vais ter 3 vidas.");
  	Serial.println("Digite o nome do jogador (ate 5 letras): ");

  	// Espera o jogador escrever o nome
  	while(Serial.available() == 0){}

  	if (Serial.available()) {
      Serial.readBytes(nomeJogador, 5);
      nomeJogador[5] = '\0';
      Serial.println(nomeJogador);  
  	}
  
  	// Espera o jogador apertar o botão para começar
  	lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("clique no botao!"); 
  	
  	while(!botaoApertado) {}
  
  	// Inicia a tela sem blocos
    memset(blocoCima, ' ', larguraTela);
    memset(blocoBaixo, ' ', larguraTela);
}

void loop() {
  	atualizarSom(); 
  
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
