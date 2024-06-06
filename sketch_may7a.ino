#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

const char* WIFI_SSID = "WiFi VAZQZ";
const char* WIFI_PASS = "12345678909";
const int LED_PIN = 4; // Pin del LED

WebServer server(80);

bool ledState = false; // Estado inicial del LED (apagado)

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }

  // Captura OK, enviar imagen
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}
 
void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgMid()
{
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}

void handlePersonCount() {
  if (server.hasArg("num_persons")) {
    int num_persons = server.arg("num_persons").toInt(); // Obtener el número de personas detectadas
    if (num_persons > 0) {
      digitalWrite(LED_PIN, HIGH); // Encender el LED si se detectan más de una persona
      ledState = true;
    } else {
      digitalWrite(LED_PIN, LOW); // Apagar el LED si no se detectan personas o solo una
      ledState = false;
    }
  }
  server.send(200, "text/plain", ""); // Respuesta HTTP vacía
}

 
 
void  setup(){
  Serial.begin(115200);
  Serial.println();
  digitalWrite(LED_PIN, LOW);
  pinMode(LED_PIN, OUTPUT); // Configurar el pin del LED como salida
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
 
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");
  Serial.println("  /person-count?count=<num_persons>");

  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam-mid.jpg", handleJpgMid);
  server.on("/person-count", HTTP_GET, handlePersonCount);
 
  server.begin();
}
 
void loop()
{
  server.handleClient();
}
