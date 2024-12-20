#include <Arduino.h>
#include "SCServo.h"
#include <WiFi.h>
#include "WiFiClient.h"
#include "WebServer.h"
#include <string.h>
//////////////////

#define motor_forward_pin 12
#define motor_backward_pin 14

//构造对象																							
SMS_STS motor;
WebServer server(80);

String records = "";
int recordCount = 0;

void handleAction1() { 
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);
  server.send(200, "text/plain", "motor executed successfully"); }
void handleAction2() { 
  digitalWrite(12, HIGH);
  digitalWrite(13, LOW);  
  server.send(200, "text/plain", "Action 2 executed successfully"); }
void handleAction3() { 
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);    
  server.send(200, "text/plain", "Action 3 executed successfully"); }
void handleAction4() { 
  digitalWrite(25, LOW);
  digitalWrite(26, HIGH);  
  server.send(200, "text/plain", "Action 4 executed successfully"); }
void handleAction5() {
  digitalWrite(25, HIGH);
  digitalWrite(26, LOW);  
  server.send(200, "text/plain", "Action 5 executed successfully"); }
void handleAction6() {
  digitalWrite(25, HIGH);
  digitalWrite(26, HIGH);  
  server.send(200, "text/plain", "Action 6 executed successfully"); }

const char* ssid = "zcf";
const char* password = "zhangchaofu";

String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Control Panel</title>
    <script>
        // 发送请求到指定的路径
        function sendAction(action) {
            fetch(action, { method: 'POST' })
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                    alert(data); // 弹出服务器返回的信息
                })
                .catch(error => {
                    console.error('Error:', error);
                    alert('Failed to send action: ' + action);
                });
        }

        // 提交表单数据
        function submitForm(event) {
            event.preventDefault(); // 防止表单默认刷新行为
            const formData = new FormData(event.target);
            fetch('/submit', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(data => {
                console.log(data);
                alert('Form submitted successfully!');
                location.reload(); // 重新加载页面以显示最新的表格
            })
            .catch(error => {
                console.error('Error:', error);
                alert('Failed to submit form.');
            });
        }
    </script>
</head>
<body>
    <h1>ESP32 Control Panel</h1>
    
    <!-- 表单 -->
    <h2>Submit Data</h2>
    <form onsubmit="submitForm(event)">
        <label for="data1">Data 1:</label>
        <input type="text" id="data1" name="data1"><br>
        <label for="data2">Data 2:</label>
        <input type="text" id="data2" name="data2"><br>
        <label for="data3">Data 3:</label>
        <input type="text" id="data3" name="data3"><br>
        <label for="data4">Data 4:</label>
        <input type="text" id="data4" name="data4"><br>
        <label for="data5">Data 5:</label>
        <input type="text" id="data5" name="data5"><br>
        <button type="submit">Submit</button>
    </form>
    <hr>
    
    <!-- 操作按钮 -->
    <h2>Actions</h2>
    <button onclick="sendAction('/action1')">1_Forward</button>
    <button onclick="sendAction('/action2')">1_Reverse</button>
    <button onclick="sendAction('/action3')">1_Stop</button><br>
    <button onclick="sendAction('/action4')">2_Forward</button>
    <button onclick="sendAction('/action5')">2_Reverse</button>
    <button onclick="sendAction('/action6')">2_Stop</button>
    <hr>
    
    <!-- 提交记录表格 -->
    <h2>Submission Records</h2>
    <table border="1">
        <tr>
            <th>Record #</th>
            <th>Data 1</th>
            <th>Data 2</th>
            <th>Data 3</th>
            <th>Data 4</th>
            <th>Data 5</th>
        </tr>
        %RECORDS%
    </table>
</body>
</html>
)rawliteral";


void handleSubmit() {
    if (server.hasArg("data1") && server.hasArg("data2") && server.hasArg("data3") &&
        server.hasArg("data4") && server.hasArg("data5")) {
        String data1 = server.arg("data1");
        String data2 = server.arg("data2");
        String data3 = server.arg("data3");
        String data4 = server.arg("data4");
        String data5 = server.arg("data5");

        float value[6] = {0};
        value[1] = data1.toFloat();
        value[2] = data2.toFloat();
        value[3] = data3.toFloat();
        value[4] = data4.toFloat();
        value[5] = data5.toFloat();

        for(int i=1; i<6; i++)
        {
          motor.WritePosEx(i, value[i], 500, 10);
        }
        
        // 增加记录到表格
        recordCount++;
        records += "<tr><td>" + String(recordCount) + "</td><td>" + data1 + "</td><td>" +
                   data2 + "</td><td>" + data3 + "</td><td>" + data4 + "</td><td>" + data5 + "</td></tr>";

        server.send(200, "text/plain", "Data submitted successfully");
    } else {
        server.send(400, "text/plain", "Bad Request: Missing parameters");
    }
}

void handleRoot() {
    String page = html;
    page.replace("%RECORDS%", records);
    server.send(200, "text/html", page);
}

void setup() {
  //初始化串口
  Serial.begin(1000000);
  Serial2.begin(1000000, SERIAL_8N1, 16, 17);
  //串口指针
  motor.pSerial = &Serial2;

  //初始化引脚
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW); 
  digitalWrite(25, LOW);
  digitalWrite(26, LOW); 

  //初始WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //检测WiFi连接
  Serial.print("正在连接WI-FI");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("...");
  }

    Serial.println("\n连接成功");
    Serial.print("IP地址:");
    Serial.println(WiFi.localIP());

    //创建服务器
    server.on("/", handleRoot);
    server.on("/submit", HTTP_POST, handleSubmit);
    server.on("/action1", HTTP_POST, handleAction1);
    server.on("/action2", HTTP_POST, handleAction2);
    server.on("/action3", HTTP_POST, handleAction3);
    server.on("/action4", HTTP_POST, handleAction4);
    server.on("/action5", HTTP_POST, handleAction5);
    server.on("/action6", HTTP_POST, handleAction6);

    //启动http服务器
    server.begin();
    Serial.println("HTTP server started");
    
}

void loop() {
  server.handleClient();
}