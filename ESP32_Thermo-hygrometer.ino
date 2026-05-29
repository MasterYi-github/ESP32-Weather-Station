#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_AHTX0.h>

// ========== 你的 WiFi 信息 ==========
const char* ssid = "REDMIYI";         // 你的热点名称
const char* password = "liyi123456";  // 你的热点密码
// ===================================

WebServer server(80);                 // 网页服务器，端口 80
Adafruit_AHTX0 aht;

float temperature = 0;
float humidity = 0;

// LED 引脚（大部分 ESP32 开发板的板载 LED 在 GPIO 2）
const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. 初始化 LED 引脚
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // 初始熄灭
  
  // 2. 初始化 AHT20 传感器
  if (!aht.begin()) {
    Serial.println("AHT20 初始化失败，请检查接线");
    while (1);
  }
  Serial.println("AHT20 传感器已就绪");
  
  // 3. 连接 WiFi
  Serial.print("正在连接 WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi 连接成功！");
    Serial.print("📡 IP 地址: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi 连接失败，请检查热点");
    while (1);
  }
  
  // 4. 配置网页服务器路由
  server.on("/", handleRoot);           // 主页：显示温湿度 + 控制按钮
  server.on("/led/on", handleLedOn);    // 开灯命令
  server.on("/led/off", handleLedOff);  // 关灯命令
  
  server.begin();
  Serial.println("🌐 网页服务器已启动");
  Serial.println("请在浏览器访问上面的 IP 地址");
}

void loop() {
  server.handleClient();  // 持续监听浏览器请求
  
  // 每隔 2 秒读取一次温湿度
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 2000) {
    lastRead = millis();
    sensors_event_t humidityEvent, tempEvent;
    aht.getEvent(&humidityEvent, &tempEvent);
    temperature = tempEvent.temperature;
    humidity = humidityEvent.relative_humidity;
    
    // 串口打印调试信息
    Serial.print("温度: ");
    Serial.print(temperature);
    Serial.print(" °C   湿度: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}

// ========== 网页服务器处理函数 ==========

// 主页：显示温湿度 + 两个控制按钮
void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>温湿度监测 + 远程控制</title>";
  html += "<meta http-equiv='refresh' content='2'>";  // 每2秒自动刷新
  html += "<style>";
  html += "body { text-align: center; margin-top: 50px; font-family: Arial; }";
  html += "button { font-size: 24px; padding: 10px 20px; margin: 10px; cursor: pointer; }";
  html += ".on { background-color: #4CAF50; color: white; border: none; border-radius: 5px; }";
  html += ".off { background-color: #f44336; color: white; border: none; border-radius: 5px; }";
  html += "h1 { color: #333; }";
  html += ".temp { color: red; font-size: 32px; font-weight: bold; }";
  html += ".hum { color: blue; font-size: 32px; font-weight: bold; }";
  html += "</style>";
  html += "</head>";
  
  html += "<body>";
  html += "<h1>🌡️ 温湿度监测站</h1>";
  html += "<p class='temp'>温度: " + String(temperature, 1) + " °C</p>";
  html += "<p class='hum'>湿度: " + String(humidity, 1) + " %</p>";
  html += "<hr>";
  html += "<h2>💡 远程控制 LED</h2>";
  html += "<button class='on' onclick=\"window.location.href='/led/on'\">🔆 开灯</button>";
  html += "<button class='off' onclick=\"window.location.href='/led/off'\">💡 关灯</button>";
  html += "<p><small>数据每 2 秒自动更新</small></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// 开灯命令处理
void handleLedOn() {
  digitalWrite(ledPin, HIGH);
  Serial.println("💡 LED 已开启");
  
  // 返回一个简单页面，并自动跳回主页
  String response = "<!DOCTYPE html><html>";
  response += "<head><meta charset='UTF-8'>";
  response += "<meta http-equiv='refresh' content='1;url=/'>";
  response += "</head>";
  response += "<body style='text-align:center; margin-top:50px;'>";
  response += "<h2>✅ LED 已开启</h2>";
  response += "<p>1秒后自动返回...</p>";
  response += "</body></html>";
  
  server.send(200, "text/html", response);
}

// 关灯命令处理
void handleLedOff() {
  digitalWrite(ledPin, LOW);
  Serial.println("💡 LED 已关闭");
  
  String response = "<!DOCTYPE html><html>";
  response += "<head><meta charset='UTF-8'>";
  response += "<meta http-equiv='refresh' content='1;url=/'>";
  response += "</head>";
  response += "<body style='text-align:center; margin-top:50px;'>";
  response += "<h2>✅ LED 已关闭</h2>";
  response += "<p>1秒后自动返回...</p>";
  response += "</body></html>";
  
  server.send(200, "text/html", response);
}