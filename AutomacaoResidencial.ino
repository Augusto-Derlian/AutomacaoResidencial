/*
Programa de automação residencial. Caixa d'água lâmpada e tomada
*/

//Os pinos dos sensores de nível são pullUp internamente.
#define Pino_Tanque_Nivel_Baixo                 0 //Tanque Nivel Baixo
#define Pino_Caixa_Nivel_Baixo                  4 //Caixa Nivel Baixo
#define Pino_Caixa_Nivel_Alto                   5 //Caixa Nivel Alto

#define Tempo_Entre_Analises_Do_Nivel_De_Agua   5100 //deve ser a cada 5.1s
#define Pino_Bomba_Dagua                        11
#define Pino_Erro                               9

#define Pino_Buzzer                             12
#define Pino_Botao_Buzzer                       13
#define Tempo_Buzzer_Ligado                     4000

#define Numero_Maximo_De_Deteccoes_No_Deboucing 4
#define Tempo_Maximo_Entre_Palmas               400
#define Tempo_De_Debouncing                     200
#define Pino_Tomada                             A0
#define Pino_Lampada                            A5
#define Pino_Sensor_De_Som                      8

bool               State[3] =                                {B111};
unsigned long      Previous_Millis =                         0;

unsigned long long Tempo_Em_Que_O_Botao_Foi_Precionado =     0;

boolean            temPalmas =                               false;
boolean            Primeiro_Som =                            0;
short int          Palmas =                                  0;
short              Conta_Som_Durante_Debouncing =            0;
unsigned long long Tempo_Primeiro_Som_Do_Ultimo_Debouncing = 0;

void setup()
{
  // put your setup code here, to run once:
  pinMode(Pino_Bomba_Dagua, OUTPUT);
  pinMode(Pino_Erro, OUTPUT);
  pinMode(Pino_Caixa_Nivel_Baixo, INPUT_PULLUP);
  pinMode(Pino_Caixa_Nivel_Alto, INPUT_PULLUP);
  pinMode(Pino_Tanque_Nivel_Baixo, INPUT_PULLUP);

//O módulo relé é acionado com nivel LOW.
  digitalWrite(Pino_Bomba_Dagua, HIGH); //NA
  digitalWrite(Pino_Erro, LOW); //LED

  pinMode(Pino_Buzzer, OUTPUT);
  pinMode(Pino_Botao_Buzzer, INPUT_PULLUP);

  digitalWrite(Pino_Buzzer, LOW);

  //Serial.begin(9600);
  pinMode(Pino_Sensor_De_Som, INPUT);
  pinMode(Pino_Lampada, OUTPUT);
  pinMode(Pino_Tomada, OUTPUT);

  digitalWrite(Pino_Lampada, HIGH); //HIGH por causa do rele
  digitalWrite(Pino_Tomada, HIGH);
}

void Caixa_Dagua();
void Avalia_Buzzer();
void Conta_Palmas();
void Blink_Erro( unsigned short Quantidade_De_Piscadas );

void loop()
{ //loop  
  Caixa_Dagua();
  Avalia_Buzzer();
  Conta_Palmas();
}

void Caixa_Dagua()
{//Caixa_Dagua 
  if ( millis() - Previous_Millis > Tempo_Entre_Analises_Do_Nivel_De_Agua )
  {
      bool niveis[3] = {B0, B0, B0};
      niveis[2] = digitalRead(Pino_Caixa_Nivel_Baixo);
      niveis[1] = digitalRead(Pino_Caixa_Nivel_Alto);
      niveis[0] = digitalRead(Pino_Tanque_Nivel_Baixo);
      /*  1 = tem água.
      B111 = 7
      B110 = 6
      B101 = 5
      B100 = 4 
      B011 = 3
      B010 = 2
      B001 = 1
      B000 = 0
      */
      switch ( (unsigned short) niveis )
             {
              case 7: // Todos as portas em nível alto, ou seja, sensores abertos e submersos. Caixa e tanque cheios.
              digitalWrite(Pino_Bomba_Dagua, HIGH); //Desliga Bomba. Rele deve estar como NA.
              break;

              case 6: //Caixa cheia e tanque vazio.
              digitalWrite(Pino_Bomba_Dagua, HIGH); //Desliga Bomba. Rele deve estar como NA.
              Blink_Erro(2);
              break;

              case 5:
              //Somente a boia de nível da caixa d'água alto não está aberta=1=submersa. Caixa contém pouco líquido. Pode estar enchendo ou esvaziando.
              //Do nothing
              break;

              case 4: // Caixa ainda com líquido e tanque vazio.
              digitalWrite(Pino_Bomba_Dagua, HIGH); //Desliga Bomba. Rele deve estar como NA.
              Blink_Erro(2);
              break;

              case 3: //Somente a boia de nível da caixa d'água baixo fechada, não submersa. Erro porque aboia nível alto da caixa está aberta e a de nível baixo fechada.
              digitalWrite(Pino_Bomba_Dagua, HIGH); //Desliga Bomba. Rele deve estar como NA.
              Blink_Erro(1);
              break;

              case 2: //Tanque vazio. Erro porque o sensor nível alto está fechado e o baixo aberto.
              digitalWrite(Pino_Bomba_Dagua, HIGH); //Desliga Bomba. Rele deve estar como NA.
              Blink_Erro(1);
              Blink_Erro(2);
              break;

              case 1: //Caixa Vazia. Tanque cheio. Somente nessa condição a bomba d'água é ligada.
              digitalWrite(Pino_Bomba_Dagua, LOW); //Liga Bomba. Rele deve estar como NA.
              break;

              default: // B000. Tudo vazio.
              digitalWrite(Pino_Bomba_Dagua, HIGH); //Desliga Bomba. Rele deve estar como NA.
              Blink_Erro(2);
              break;
             }
      Previous_Millis = millis();
  }
  else
  { //Do nothing
  }
}

void Blink_Erro( unsigned short Quantidade_De_Piscadas )
{// 1 Piscada: Erro na caixa d'água.
 // 2 piscadas: Erro no tanque.
 // 3 Piscadas: Erro no quarto.
  unsigned short i = 0;
  do
  { i++;
    digitalWrite(Pino_Erro, (State[0])? HIGH : LOW);
    State[0] = !State[0];
    delay(500); //500ms
  } while (i < Quantidade_De_Piscadas*2 );
  delay (500); //500ms
}

void Avalia_Buzzer()
{
  if( !digitalRead(Pino_Botao_Buzzer) )
  {
    Tempo_Em_Que_O_Botao_Foi_Precionado = millis();
  }
  if( (millis() - Tempo_Em_Que_O_Botao_Foi_Precionado < Tempo_Buzzer_Ligado) && (millis() > 4000) )
  {//millis() > 4000 elimina o toque ao ligar.
    digitalWrite(Pino_Buzzer, HIGH);
  }
  else
  {
    digitalWrite(Pino_Buzzer, LOW);
  }
}

void Conta_Palmas()
{
 temPalmas = digitalRead(Pino_Sensor_De_Som);
 //Serial.println( temPalmas );
 if( temPalmas == 1 && Conta_Som_Durante_Debouncing == 0 )
 { Primeiro_Som = 1;
   Tempo_Primeiro_Som_Do_Ultimo_Debouncing = millis();
 }
 if( Primeiro_Som )
 { if( temPalmas )
   { if( millis() - Tempo_Primeiro_Som_Do_Ultimo_Debouncing < Tempo_De_Debouncing )
      { Conta_Som_Durante_Debouncing++;
      }
   }
    //Se já passou o tempo de debouncing:
   if( millis() - Tempo_Primeiro_Som_Do_Ultimo_Debouncing > Tempo_De_Debouncing )
    { Primeiro_Som = 0;
      if( Conta_Som_Durante_Debouncing <= Numero_Maximo_De_Deteccoes_No_Deboucing )//Garante que um som contínuo (múltiplas detecções) não seja trigger
      { Palmas++;
      }
      else //Detecto ruino (barulho continuo, caracterizado po multiplas detecções durante o tempo de debouncing). Descarto as palmas acumuladas.
      { Palmas = 0;
      }
      Conta_Som_Durante_Debouncing = 0;
      Tempo_Primeiro_Som_Do_Ultimo_Debouncing = millis();
    }
 }
 if ( millis() - Tempo_Primeiro_Som_Do_Ultimo_Debouncing > Tempo_Maximo_Entre_Palmas )
 { switch ( Palmas )
   {
     case 0:// zero palmas
            break;
     case 1:
            Palmas = 0;
            break;
                
     case 2://duas palmas
            Palmas = 0;
            digitalWrite( Pino_Lampada, (State[1])? HIGH : LOW );
            State[1] = !State[1];
            break;
                
     case 3:
            digitalWrite(Pino_Tomada, ( State[2])? HIGH : LOW );
            State[2] = !State[2];
            Palmas = 0;
            break;

     default:
            // Do nothing
            break;
    }
  }
}
