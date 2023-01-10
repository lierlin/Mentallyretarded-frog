#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
// SSID & Password
const char *ssid = "Docimax-Dev";     // Enter your SSID here
const char *password = "Docimax@123"; // Enter your Password here

const char *mqtt_server = "172.30.199.170"; // 使用HIVEMQ 的信息中转服务
const char *TOPIC = "home/devices/onoff/";  // 订阅信息主题
const char *client_id = "emqx_test";        // 标识当前设备的客户端编号

WiFiClient espClient;           // 定义wifiClient实例
PubSubClient client(espClient); // 定义PubSubClient的实例
long lastMsg = 0;               // 记录上一次发送信息的时长

WebServer server(8011); // Object of WebServer(HTTP port, 80 is defult)

// IP Address details
IPAddress local_ip(192, 168, 10, 146);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(202, 106, 0, 20);

int digital_of_2 = 0;
int process = 0;

void web_init()
{
  delay(10);
  // 板子通电后要启动，稍微等待一下让板子点亮
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void mqtt_app_start()
{
  client.setServer(mqtt_server, 1883); // 设定MQTT服务器与使用的端口，1883是默认的MQTT端口
}

void setup()
{

  /*输出log 相关配置 我们不需要关心*/
  ESP_LOGI(TAG, "[APP] Startup..");
  /*获取空闲内存大小*/
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  /*打印当前idf的版本*/
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
  /*配置打印信息*/
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

  // web init
  web_init();
  /*mqtt开始运行*/
  mqtt_app_start();

  // put your setup code here, to run once:
  pinMode(2, OUTPUT); // GPIO2 Onboard touch switch blue LED
}
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id))
    {
      Serial.println("connected");
      // 连接成功时订阅主题
      client.subscribe(TOPIC);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
bool HIGHFirst = true;
bool LOWFirst = true;
void loop()
{
  server.handleClient();

  // put your main code here, to run repeatedly:
  if (process == 0 && touchRead(T5) <= 33)
  {
    process = 1;
    digital_of_2 = !digital_of_2;
  }
  if (process == 1 && touchRead(T5) >= 33)
  {

    process = 0;
  }

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  String json_str = "";
  if (digital_of_2 == 1)
  {
    json_str = "呱呱呱~ 再摸一下试试？？";
    Serial.println("open");
    digitalWrite(2, HIGH);

    if (HIGHFirst)
    {
      client.publish("home/status/", json_str.c_str());
      HIGHFirst = false;
      LOWFirst = true;
    }
  }
  else
  {
    json_str = "woc 你还真摸啊！";
    Serial.println("close");
    digitalWrite(2, LOW);
    if (LOWFirst)
    {
      client.publish("home/status/", json_str.c_str());
       HIGHFirst = true;
      LOWFirst = false;
    }
  }
}
