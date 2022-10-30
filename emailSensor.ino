#include <WiFiManager.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#endif
#include <ESP_Mail_Client.h>

#define PIN_BUZZER D4
#define PIN_ALTO D2
#define PIN_SENSOR D3
// Configurações da rede WiFi à se conectar
//#define WIFI_SSID "NOME DA REDE"
//#define WIFI_PASSWORD "SENHA DA REDE"
// Configurações do SMTP host
#define SMTP_HOST "smtp.gmail.com" // SMTP host
#define SMTP_PORT esp_mail_smtp_port_587
// As credenciais de login
#define AUTOR_EMAIL "SEU EMAIL"
#define AUTOR_SENHA "SENHA DO ESMAIL"
// E-mail do destinatário
#define DESTINATARIO_NOME "ADM Alarme"
#define DESTINATARIO_EMAIL "EMAIL DESTINATARIO"
// O objeto de sessão SMTP usado para envio de e-mail
SMTPSession smtp;
// Variáveis Globais
bool permiteEnvioEmail = false; // controle se deve ou não enviar e-mail. Este
//                                  controle permite que somente um e-mail seja
//                                  enviado a cada abertura da porta, mesmo que o
//                                  tempo de alarme pré-defindo seja ultrapassado.
// Função de retorno de chamada para obter o status de envio de e-mail
void smtpCallback(SMTP_Status status);
int val;
bool enviaEmail_TXT(String nomeRemetente,
                    String emailRemetente,
                    String senhaRemetente,
                    String assunto,
                    String nomeDestinatario,
                    String emailDestinatario,
                    String messageTXT,
                    String stmpHost,
                    int stmpPort); // Função de envio de e-mail
void setup() {
  // Inicia Serial
  Serial.begin(115200);

  //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
 
  Serial.println();
  delay(1000);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  pinMode(PIN_ALTO, OUTPUT);
  digitalWrite(PIN_ALTO, HIGH);
  pinMode(PIN_SENSOR, INPUT_PULLUP);
  
//  Inicia conexão WiFi
//  Serial.println("Conectando à rede WiFi");
//  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//  while (WiFi.status() != WL_CONNECTED) // Enquanto o status de conexão ao WiFi
    //                                      não for bem sucedido, ...
  //{
    //Serial.print(".");
    //delay(200);
  //}
  // Mostra na Serial que a conexão está realizada
  //Serial.println("\nWiFi conectado.");
 // Serial.println();
 // digitalWrite(PIN_BUZZER, HIGH);
 // delay(1000);
 // digitalWrite(PIN_BUZZER, LOW);
  // Habilita a depuração via porta serial:
  //    0: nenhuma depuração
  //    1: deburação básica
  smtp.debug(0);
  // Define a função de retorno de chamada para obter os resultados de envio
  smtp.callback(smtpCallback);
}
void loop(){
    val = digitalRead(PIN_SENSOR);
    if (val==LOW){          
      permiteEnvioEmail == true;       
        enviaEmail_TXT("ESP32 Alarme de Porta",
                       AUTOR_EMAIL,
                       AUTOR_SENHA,
                       "Alarme da Porta Ativado",
                       DESTINATARIO_NOME,
                       DESTINATARIO_EMAIL,
                       "Foi detectado a abertura da porta",
                       SMTP_HOST,
                       SMTP_PORT); // envia o e-mail com os dados em parâmetros
        permiteEnvioEmail = false; // não deixa enviar e-mail
      
    } else // se não, ...
    {
      permiteEnvioEmail = false; // deixa enviar e-mail
    }
    delay(100); // pausa de 0,1 segundos
  }

bool enviaEmail_TXT(String nomeRemetente,
                    String emailRemetente,
                    String senhaRemetente,
                    String assunto,
                    String nomeDestinatario,
                    String emailDestinatario,
                    String messageTXT,
                    String stmpHost,
                    int stmpPort) {
  // Objeto para declarar os dados de configuração da sessão
  ESP_Mail_Session session;
  // Defina os dados de configuração da sessão
  session.server.host_name = stmpHost;
  session.server.port = stmpPort;
  session.login.email = emailRemetente;
  session.login.password = senhaRemetente;
  session.login.user_domain = "";
  // Defina o tempo de configuração do NTP
  session.time.ntp_server = F("time.google.com"); // Utilizado o NTP do Google:
  //                                         https://developers.google.com/time
  session.time.gmt_offset = -3;
  session.time.day_light_offset = 0;
  // Instanciação do objeto da classe de mensagem
  SMTP_Message message;
  // Definição os cabeçalhos das mensagens
  message.sender.name = nomeRemetente;
  message.sender.email = emailRemetente;
  message.subject = assunto;
  message.addRecipient(nomeDestinatario, emailDestinatario);
  message.text.content = messageTXT.c_str();
  // O conjunto de caracteres de mensagem de texto html, por exemplo:
  //  us-ascii
  //  utf-8
  //  utf-7
  // O valor padrão é utf-8
  message.text.charSet = "utf-8";
  // A codificação de transferência de conteúdo. Ex:
  //  enc_7bit ou "7bit" (não codificado)
  //  enc_qp ou "quoted-printable" (codificado)
  //  enc_base64 ou "base64" (codificado)
  //  enc_binary ou "binary" (não codificado)
  //  enc_8bit ou "8bit" (não codificado)
  //  O valor padrão é "7bit"
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  // A prioridade da mensagem:
  //  esp_mail_smtp_priority_high ou 1
  //  esp_mail_smtp_priority_normal ou 3
  //  esp_mail_smtp_priority_low ou 5
  //  O valor padrão é esp_mail_smtp_priority_low
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  // As notificações de status de entrega, Ex:
  //  esp_mail_smtp_notify_never
  //  esp_mail_smtp_notify_success
  //  esp_mail_smtp_notify_failure
  //  esp_mail_smtp_notify_delay
  //  O valor padrão é esp_mail_smtp_notify_never
  message.response.notify = esp_mail_smtp_notify_success |
                            esp_mail_smtp_notify_failure |
                            esp_mail_smtp_notify_delay;
  // Conecte-se ao servidor com a configuração da sessão
  if (!smtp.connect(&session))
    return false;
  // Começa a enviar e-mail e fecha a sessão
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Erro ao enviar e-mail, " + smtp.errorReason());
    return false;
  }
  return true;
}
/* Função de retorno de chamada para obter o status de envio de e-mail */
void smtpCallback(SMTP_Status status) {
  /* Imprima o status atual */
  Serial.println(status.info());
  /* Imprima o resultado do envio */
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Mensagem enviada com sucesso: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Falha na mensagem enviada: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;
    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Obter o item de resultado */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      
       ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", asctime(localtime(&ts)));
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
  }
}
