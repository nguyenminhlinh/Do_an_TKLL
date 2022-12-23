/*
  Arduino Mega:
   - LCD ngoài + Keyboard
   - Cửa chính: 2 servo + 5 button
   - Đèn phòng khách + 1 button
   - Cảm biến ánh sáng + 1 button
   - Code Webserver.
  Giao tiếp Serial với các phòng và WebServer
   - Phòng ngủ: Serial1
    + Đèn: 0,1
    + Điều hòa: 2,3
    + Rèm: 4,5
   - Bếp: Serial2
    + Đèn: 0,1
    + Điều hòa: 2,3
    + Báo khí gas: 4,5
   - Báo động: Serial3
    + Bật tắt chế độ báo động: 0,1
    + Gara: 2,3
    + Phát hiện xâm nhập: 4,5
   ***Lưu ý:
     - GND: CBND, 2 Servo.
*/

// Khai bao cac thu vien
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SimpleTimer.h>
#include <Keypad.h> // dung cac chan 48 46 44 42 40 38 36 34
#include <Key.h>
#include <DHT.h>
#include <WiFiEspAT.h>
#include <Servo.h>
//#include "web_script.h"

// Khai bao cac chan
#define pinServo1         22 // canh phai
#define pinServo2         24 // canh trai
#define dencong           25
#define denpk             28 // led ngoai hien
#define dieuhoapk         26 // dieu hoa phong khach
#define button_dieuhoapk  50
#define button_denpk      51
#define button_dencong    52
#define button_dongcua    53
#define CBAS              A1
#define CBND              A0
#define CBCD1             23

//#define NAME_OF_SSID      "abcd"
//#define PASSWORD_OF_SSID  "kiet12345"
#define NAME_OF_SSID      "u cant see me"
#define PASSWORD_OF_SSID  "pasSword"
WiFiServer server(80);

// HOUSE STAT:
// cua cong
byte state_cuacong = 0, state_dencong = 0;
int pos1 = 90, pos2 = 90, // cua mo khi pos1 = 180, pos 2 = 0, cua dong khi pos1 = pos2 = 90
    cuacong_flag = 2, security_lock = 0;
int dencong_flag = 0, button_dencong_flag = 0; 
// nhiet do + do am cac phong
int nhiet_do_pk = 32, do_am_pk = 70, 
    nhiet_do_pn = 31, nhiet_do_bep = 32;
// phong khach
byte state_denpk = 0, state_dieuhoapk = 0, state_lcdpk = 0;
// phong ngu
byte state_denpn = 0, state_dieuhoapn = 0, state_rem = 0, state_remauto;
// nha bep
byte state_denbep = 0, state_dieuhoabep = 0, state_gas = 0;

// Khai bao cua Keypad - PassWord cho cua chinh
const byte ROWS = 4;
const byte COLS = 4;
int correct = 0; // đếm số kí tự trong Pass đúng
int count = 0; // giới hạn kí tự của pass
int error = 0;
char keys[ROWS][COLS] =
{ 
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// password
char pass[] = {'1', '2', '3', '4', '5', '6'};
char newpass[6];
byte rowPins[ROWS] = {34, 36, 38, 40};
byte colPins[COLS] = {42, 44, 46, 48};
int clr = 0; // lcd Clear flag
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// LCD with adaptor (I2C communication: SDA = 20, SCL = 21)
LiquidCrystal_I2C lcd_cong(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
LiquidCrystal_I2C lcd_pk(0x26, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// global SimpleTimer object
SimpleTimer timer;

// khai bao servo 2 cua
Servo motor1_cuachinh, motor2_cuachinh;
// cam bien nhiet do, do am
DHT dht_pk(CBND, DHT11);

String line, docstring;
/*
//____________________________Web function________________________________
void serverRoot()
{
  String s = MAIN_page;
  server.send(200, "text/html", s);
}

void serverCong()
{
  String code = server.arg("code");
  switch(code)
  {
  case 0: // dong cua cong
    state_cuacong = 0;
    cuacong_flag = 0;
    server.send(200, "text/html", "Đóng");
    break;
  case 1: // mo cua cong
    state_cuacong = 1;
    cuacong_flag = 1;
    server.send(200, "text/html", "Mở");
    break;
  case 2: // tat den cong
    state_dencong = 0;
    button_dencong_flag = 0;
    server.send(200, "text/html", "Tắt");
    break;
  case 3: // bat den cong
    state_dencong = 1;
    button_dencong_flag = 1;
    server.send(200, "text/html", "Bật");
    break;
  default:
    server.send(200, "text/html", "failed");
    break;
  }
}

void serverPK()
{
  String code = server.arg("code");
  switch(code)
  {
  case 0: // tat den phong khach
    button_denpk_flag = 0;
    server.send(200, "text/html", "Tắt");
    break;
  case 1: // bat den phong khach
    button_denpk_flag = 1;
    server.send(200, "text/html", "Bật");
    break;
  case 2: // tat dieu hoa phong khach
    button_dieuhoapk_flag = 0;
    server.send(200, "text/html", "Tắt");
    break;
  case 3:
    button_dieuhoapk_flag = 1;
    server.send(200, "text/html", "Bật");
    break;
  default:
    server.send(200, "text/html", "failed");
    break;
  }
}
*/
void setup() 
{
  //_______________________Setting up Serial port__________________________
  Serial.begin(115200);
  Serial.flush();
  Serial1.begin(115200);
  Serial1.flush();
  Serial2.begin(115200);
  Serial2.flush();
  Serial3.begin(115200);
  Serial3.flush();
  while (!Serial) 
    ; // wait for serial port

  //_________________________Setting up Webserver___________________________
  WiFi.init(Serial3);
  if(WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Comunication with ESP failed.");
    while(1)
      ; // stop
  }
  // connect to WiFi
  WiFi.begin(NAME_OF_SSID, PASSWORD_OF_SSID);
  Serial.println("Waiting for connection to WiFi.");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi network.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  /*
  server.on("/", serverRoot);
  server.on("/CUACONG", serverCong);
  server.on("/PHONGKHACH", serverPK);
  */
  server.begin();
  //Serial.println("HTTP server started.");
  
  //________________________Setting up timer interrupt_____________________
  //setupTimer1();
  timer.setInterval(5000, LCD_PHONG_KHACH);

  //_________________________Setting up devices_____________________________
  // start up 2 servoes for door
  motor1_cuachinh.attach(pinServo1);
  motor2_cuachinh.attach(pinServo2);// cua chinh dung 2 servo
  // start dht sensor
  dht_pk.begin();
  // pin mode init
  pinMode(denpk, OUTPUT);
  pinMode(dieuhoapk, OUTPUT);
  pinMode(button_denpk, INPUT_PULLUP);
  pinMode(button_dieuhoapk, INPUT_PULLUP);
  pinMode(button_dongcua, INPUT_PULLUP);
  pinMode(button_dencong, INPUT_PULLUP);
  pinMode(dencong, OUTPUT);
  pinMode(CBAS, INPUT);
  //pinMode(CBND, INPUT);
  pinMode(CBCD1, INPUT);
  //pinMode(CBCD2, INPUT);

  digitalWrite(denpk, 0); // tat den
  digitalWrite(dieuhoapk, 0); // tat dieu hoa phong khach
  // doors shut at the beginning
  while(pos1 != 90)
  {
    if(pos1 > 90) pos1--;
    else pos1++;
  }
  while(pos2 != 90)
  {
    if(pos2 > 90) pos2--;
    else pos2++;
  }
  // start up lcd in living room
  lcd_pk.begin(16, 2);
  lcd_pk.clear();
  // start up lcd at doors
  lcd_cong.begin(16, 2);
  lcd_cong.clear();
  lcd_cong.print("Smart home 2077!");
  delay(2000);
  lcd_cong.clear();
  lcd_cong.print("Xin moi nhap");
  lcd_cong.setCursor(0, 1);
  lcd_cong.print("mat khau...");
  delay(2000);
  lcd_cong.clear();
  lcd_cong.print("   Mat khau:");
  lcd_cong.setCursor(5, 1);
}

void loop() 
{
  timer.run();
  //_________________________Server scripts_________________________________
  GET_DATA();
  WiFiClient client = server.available();
  if(client)
  {
    // client details
    IPAddress ip = client.remoteIP();
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.print("New client ");
    Serial.println(ip);

    //GET_DATA();

    while(client.connected())
    {
      if(client.available())
      {
        line = client.readStringUntil('\n');
        //Serial.println(line);

        //___________________DIEU KHIEN THIET BI:___________________________
        // CUA CHINH
        if (line.indexOf("CUACHINHON") > 0) 
        {
          state_cuacong = 1;
          cuacong_flag = 1;
          security_lock = 0;
        }
        if (line.indexOf("CUACHINHOFF") > 0) 
        {
          state_cuacong = 0;
          cuacong_flag = 0;
          security_lock = 0;
        }
        // DEN CONG
        if (line.indexOf("DENCONGON") > 0) 
        {
          digitalWrite(dencong, 1);
          state_dencong = 1;
          button_dencong_flag = 1;
        }
        if (line.indexOf("DENCONGOFF") > 0) 
        {
          digitalWrite(dencong, 0);
          state_dencong = 0;
          button_dencong_flag = 0;
        }
        // DEN PHONG KHACH
        if (line.indexOf("DENPKON") > 0) 
        {
          digitalWrite(denpk, 1);
          state_denpk = 1;
        }
        if (line.indexOf("DENPKOFF") > 0) 
        {
          digitalWrite(denpk, 0);
          state_denpk = 0;
        }
        // DIEU HOA PHONG KHACH
        if (line.indexOf("DHPKON") > 0) 
        {
          digitalWrite(dieuhoapk, 1);
          state_dieuhoapk = 1;
        }
        if (line.indexOf("DHPKOFF") > 0) 
        {
          digitalWrite(dieuhoapk, 0);
          state_dieuhoapk = 0;
        }
        // PHONG NGU:
        // DEN PHONG NGU
        if (line.indexOf("DENNGUON") > 0) 
        {
          Serial1.write('1');
          state_denpn = 1;
        }
        if (line.indexOf("DENNGUOFF") > 0) 
        {
          Serial1.write('0');
          state_denpn = 0;
        }
        // REM PHONG NGU
        if (line.indexOf("REMOFF") > 0) 
        { // Tat che do tu dong va dong rem
          Serial1.write('4');
          state_rem = 0;
          state_remauto = 0;
        }
        if (line.indexOf("REMON") > 0)
        { // Tat che do tu dong va mo rem
          Serial1.write('5');
          state_rem = 1;
          state_remauto = 0;
        }
        if (line.indexOf("REMAUTO") > 0)
        { // bat che do tu dong
          Serial1.write('6');
          state_remauto = 1;
        }
        // DIEU HOA PHONG NGU
        if (line.indexOf("DHNGUON") > 0) 
        {
          Serial1.write('3');
          state_dieuhoapn = 1;
        }
        if (line.indexOf("DHNGUOFF") > 0) 
        {
          Serial1.write('2');
          state_dieuhoapn = 0;
        }
        // PHONG BEP:
        // DEN PHONG BEP
        if (line.indexOf("DENBEPON") > 0) 
        {
          Serial2.write('1');
          state_denbep = 1;
        }
        if (line.indexOf("DENBEPOFF") > 0) 
        {
          Serial2.write('0');
          state_denbep = 0;
        }
        // DIEU HOA BEP
        if (line.indexOf("DHBEPON") > 0)
        {
          Serial2.write('3');
          state_dieuhoabep = 1;
        }
        if (line.indexOf("DHBEPOFF") > 0) 
        {
          Serial2.write('2');
          state_dieuhoabep = 0;
        }
        line = "";
        //line.trim();

        //___________________GIAO DIEN WEBSERVER____________________________
        if(line.length() == 0)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          //client.println("Connection: close");
          //client.println("Refresh: 3");
          client.println();      
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<meta http-equiv=\"Refresh\" content=\"3; url=/tt\">");
          client.println("<meta name = \"viewport\" content = \"width=device-width, initial-scale=0.5\"/>");
          client.println("<meta http-equiv=\"Content-Type\" content=\"application/vnd.wap.xhtml+xml; charset=utf-8\" />");
          client.println("<head>");
          client.println("<TITLE>HE THONG DIEU KHIEN QUA MANG LAN</TITLE>");

          client.println(" <style>");
          client.println(" .logo {");
          client.println(" width: 750;");
          client.println(" height: 130;");
          client.println(" margin - top: 10;");
          client.println(" margin - left: 20;");
          client.println("}");
          client.println("</style>");
          client.println("<TITLE>Control Device Via Internet</TITLE>");
          client.println("</head>");

          client.println("<body align \"center\">");
          client.println("<h1><center>HỆ THỐNG GIÁM SÁT VÀ ĐIỀU KHIỂN</center></h1>");

          // BANG THONG SO
          client.println("<table border=\"2\" align \"center\" cellspacing=\"0\" cellpadding=\"4\">");
          client.println("<th width=\"400 px \"  bgcolor=\" violet\">&nbsp;&nbspVỊ TRÍ</th>");
          client.println("<th width=\"350 px \"bgcolor=\"violet\">&nbsp;&nbsp;THÔNG SỐ</th>");
          client.println("<th width=\"300 px \"bgcolor=\"violet\">&nbsp;&nbsp;THIẾT BỊ</th>");
          client.println("<th width=\"300 px \"bgcolor=\"violet\">&nbsp;&nbsp;TRẠNG THÁI</th>");
          client.println("<th width=\"100 px \"bgcolor=\"violet\">&nbsp;&nbsp;THAO TÁC</th>");
          // CUA CONG
          client.println("<tr>");
          client.println("<th rowspan = \"2\">CỬA CỔNG</th>");
          client.println("<th align = \"center\" rowspan = \"2\">NA");
          client.println("</th>");
          client.println("<td align= \"center\"> Cửa chính</td>");
          if (state_cuacong == 0) 
          {
            client.println("<td  align=\"center\"> Đóng");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"CUACHINHON\"><button type=\"button\">Mở</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Mở");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"CUACHINHOFF\"><button type=\"button\">Đóng</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println(" <tr>");
          client.println("<td align= \"center\">Đèn cửa cổng</td>");
          if (state_dencong == 0) 
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENCONGON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENCONGOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          // PHONG KHACH
          client.println("<tr>");
          client.println("<th rowspan = \"3\">PHÒNG KHÁCH</th>");
          client.println("<th align=\"center\" rowspan = \"3\"> Nhiệt độ:      ");
          client.println(nhiet_do_pk);
          client.println(" *C");
          client.println("<br/> ");
          if (nhiet_do_pk >= 60) 
            client.println("NGUY HIỂM !!!");
          else 
            client.println("Bình thường");
          client.println("<br/> ");
          client.println(" <br/>Độ ẩm:    ");
          client.println(do_am_pk);
          client.println(" %");
          client.println("</th>");
          client.println("</tr>");
          client.println(" <tr>");
          client.println("<td align= \"center\">Đèn chiếu sáng</td>");
          if (state_denpk == 0) 
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENPKON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENPKOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("  </td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println(" <td align =\"center\">Điều hòa</td>");
          if (state_dieuhoapk == 0) 
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHPKON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHPKOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          // PHONG NGU
          client.println("<tr>");
          client.println("<th rowspan = \"4\">&nbsp;&nbsp;PHÒNG NGỦ</th>");
          client.println("<th rowspan = \"3\"> &nbsp;&nbsp;Nhiệt độ:     ");
          client.println(nhiet_do_pn);
          client.println(" *C");
          client.println("<br/> ");
          client.println("<br/>  ");
          if (nhiet_do_pn >= 60) 
            client.println("NGUY HIỂM !!!");
          else 
            client.println("Bình thường");
          client.println(" </th>");
          client.println("<td align =\"center\">&nbsp;&nbsp;Đèn chiếu sáng</td>");
          if (state_denpn == 0) 
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENNGUON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENNGUOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td align= \"center\">&nbsp;&nbsp;Rèm phòng ngủ</td>");
          if (state_rem == 0) 
          {
            client.print("<td  align=\"center\"> Đóng - ");
            if (state_remauto == 0) client.print("Bình thường");
            else client.print("Tự động");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"REMON\"><button type=\"button\">Mở</a>");
            if(state_remauto == 0) client.println("<a href = \"REMAUTO\"><button type=\"button\">Auto</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Mở - ");
            if (state_remauto == 0) client.print("Bình thường");
            else client.print("Tự động");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"REMOFF\"><button type=\"button\">Đóng</a>");
            if(state_remauto == 0) client.println("<a href = \"REMAUTO\"><button type=\"button\">Auto</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td align =\"center\">&nbsp;&nbsp;Điều hòa</td>");
          if (state_dieuhoapn == 0) 
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHNGUON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHNGUOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          // KHU BEP
          client.println("<tr>");
          client.println("<tr>");
          client.println("<th rowspan = \"4\">&nbsp;&nbsp;KHU BẾP</th>");
          client.println("<th rowspan = \"2\">&nbsp;&nbsp Nhiệt độ:      ");
          client.println(nhiet_do_bep);
          client.println(" *C");
          client.println("<br/>");
          if (nhiet_do_bep >= 60) 
            client.println("NGUY HIỂM !!!");
          else 
            client.println("Bình thường");
          client.println("</th>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td align =\"center\" rowspan = \"2\">&nbsp;&nbsp;Đèn chiếu sáng</td>");
          if (state_denbep == 0) 
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENBEPON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENBEPOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<th rowspan = \"2\"> Khí gas  ");
          client.println("<br/>");
          if (state_gas == 0) 
            client.println("Bình thường");
          else 
            client.println("NGUY HIỂM !!!");
          client.println("</th>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td align =\"center\" rowspan = \"2\">&nbsp;&nbsp;Điều hòa</td>");
          if (state_dieuhoabep == 0)
          {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHBEPON\"><button type=\"button\">Bật</a>");
          }
          else 
          {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHBEPOFF\"> <button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println("</tr>");
          client.println("</table>");

          client.println("</body");
          client.println("</html>");
          client.stop();
          break;
        }
      }
    }
  }

  //_______________________________________Code cơ cấu chap hành___________________________________________________
  DEN_CUA_CONG();
  DEN_PHONG_KHACH();
  DIEU_HOA_PHONG_KHACH();
  DHT_FUNC();
  CUA_CONG();
  if(state_cuacong == 0 && security_lock == 0)
  {
    NHAP_MAT_KHAU();
    if (error == 5) LOCK_SECURITY();
  }
}

//______________________________________CÁC HÀM CON________________________________________________
unsigned long lastButtonDenCong = millis();
void DEN_CUA_CONG()
{
  int lightSen = analogRead(CBAS);
  int motionSen = digitalRead(CBCD1);
  // dieu khien bang cam bien anh sang va chuyen dong
  if(lightSen > 500 && motionSen == 1)
  {
    dencong_flag = 1;
  }
  else
  {
    dencong_flag = 0;
  }
  // dieu khien bang nut bam
  if(millis() - lastButtonDenCong >= 500 && digitalRead(button_dencong) == 0)
  {
    button_dencong_flag = (button_dencong_flag + 1) % 2;
    lastButtonDenCong = millis();
  }

  if(dencong_flag == 1 || button_dencong_flag == 1) 
  {
    digitalWrite(dencong, 1);
    state_dencong = 1;
  }
  else
  {
    digitalWrite(dencong, 0);
    state_dencong = 0;
  }
}

void CUA_CONG() 
{
  if((digitalRead(button_dongcua) == 0) && state_cuacong == 0)
  {
    cuacong_flag = 1;
    //Serial.println("open");
  }
  if((digitalRead(button_dongcua) == 0) && state_cuacong == 1)
  {
    cuacong_flag = 0;
    //Serial.println("close");
  }

  if (cuacong_flag == 1) // mo cua
  {
    lcd_cong.clear();
    lcd_cong.print("Smart Home 2077!");
    lcd_cong.setCursor(0, 1);
    lcd_cong.print("*** WELCOME ***");
    while (pos1 != 180)
    {
      pos1++;
      pos2--;
      motor1_cuachinh.write(pos1);
      motor2_cuachinh.write(pos2);
      delay(10);
    }
    cuacong_flag = 2;
    state_cuacong = 1;
  }

  if (cuacong_flag == 0) // dong cua
  {
    while (pos1 != 90) 
    {
      pos1--;
      pos2++;
      motor1_cuachinh.write(pos1);
      motor2_cuachinh.write(pos2);
      delay(10);
    }
    cuacong_flag = 2;
    state_cuacong = 0;
    char newpass[] = {'0', '0', '0', '0', '0', '0'};
    correct = 0;
    count = 0;
    clr = 0;
    PASSWORD_REQUEST();
  }
}

void NHAP_MAT_KHAU() 
{
  char key = keypad.getKey();
  if(key != NO_KEY)
  {
    lcd_cong.print(key);
    Serial.print(key);
  }
  
  if (key != NO_KEY && count < 6)
  {
    if (clr == 0)
    {
      lcd_cong.clear();
      clr = 1;
    }
    lcd_cong.setCursor(0, 0);
    lcd_cong.print("   Mat khau:");
    lcd_cong.setCursor(count + 5, 1);
    lcd_cong.print("*");
    newpass[count] = key;
    if (newpass[count] == pass[count]) correct++;
    count++;
  }

  if (correct == 6) 
  {
    lcd_cong.clear();
    lcd_cong.print("*** WELCOME ***");
    // Mo cua chinh
    while (pos1 != 180)
    {
      pos1++;
      pos2--;
      motor1_cuachinh.write(pos1);
      motor2_cuachinh.write(pos2);
      delay(10);
    }
    state_cuacong = 1;
    cuacong_flag = 1;

    if (state_denpk == 0) 
    { // neu den dang tat thi bat den len
      digitalWrite(denpk, 1);
      state_denpk = 1;
    }
    if (state_dieuhoapk == 0) 
    { // neu dieu hoa dang tat thi bat len
      digitalWrite(dieuhoapk, 1);
      state_dieuhoapk = 1;
    }
    correct = 7;
    return;
  }
  else if (correct < 6 && count == 6) 
  {
    lcd_cong.clear();
    lcd_cong.print("Mat khau sai !");
    count = 0;
    correct = 0;
    error++;
    if (error < 5) 
    {
      timer.setTimeout(1000, PASSWORD_REQUEST);
    }
    clr = 0;
    return;
  }
}

void PASSWORD_REQUEST()
{
  lcd_cong.clear();
  lcd_cong.print("   Mat khau:");
  lcd_cong.setCursor(0, 1);
}

unsigned long lastButtonDenpk = millis();
void DEN_PHONG_KHACH() 
{
  if(millis() - lastButtonDenpk >= 500 && digitalRead(button_denpk) == 0)
  {
    state_denpk = (state_denpk + 1) % 2;
    lastButtonDenpk = millis();
  }

  if(state_denpk == 1) 
  {
    digitalWrite(denpk, 1);
  }
  else
  {
    digitalWrite(denpk, 0);
  }
}

unsigned long lastButtonDieuhoapk = millis();
void DIEU_HOA_PHONG_KHACH() 
{
  if(millis() - lastButtonDieuhoapk >= 500 && digitalRead(button_dieuhoapk) == 0)
  {
    state_dieuhoapk = (state_dieuhoapk + 1) % 2;
    lastButtonDieuhoapk = millis();
  }

  if(state_dieuhoapk == 1) 
  {
    digitalWrite(dieuhoapk, 1);
  }
  else
  {
    digitalWrite(dieuhoapk, 0);
  }
}

void DHT_FUNC()
{
  nhiet_do_pk = (int)dht_pk.readTemperature();
  do_am_pk = (int)dht_pk.readHumidity();
}

void GET_DATA()
{
  // PHONG NGU
  if(Serial1.read() == 255)
  {
    //while(Serial1.read() != 255);
    while(Serial1.available() < 2);
    // den phong ngu
    int data = Serial1.read();
    Serial.print("Serial1: byte 1 = ");
    Serial.print(data);
    state_denpn = data / 8;
    // dieu hoa
    data %= 8;
    state_dieuhoapn = data / 4;
    // trang thai dong - mo cua rem
    data %= 4;
    state_rem = data / 2;
    // trang thai tu dong cua rem
    data %= 2;
    state_remauto = data;
    // nhiet do phong bep
    data = Serial1.read();
    Serial.print(" byte 2 = ");
    Serial.println(data);
    nhiet_do_pn = data;
  }
  // PHONG BEP
  if(Serial2.read() == 255)
  {
    //while(Serial2.read() != 255);
    while(Serial2.available() < 2);
    // den phong bep
    int data = Serial2.read();
    Serial.print("Serial2: byte 1 = ");
    Serial.print(data);
    state_denbep = data / 4;
    // dieu hoa
    data %= 4;
    state_dieuhoabep = data / 2;
    // gas
    data %= 2;
    state_gas = data;
    // nhiet do phong bep
    data = Serial2.read();
    Serial.print(" byte 2 = ");
    Serial.println(data);
    nhiet_do_bep = data;
  }
}

void LOCK_SECURITY() 
{
  security_lock = 1;
  lcd_cong.clear();
  lcd_cong.print("Sai 5 lan !");
  lcd_cong.setCursor(0, 1);
  lcd_cong.print("Nhap lai sau 10s.");
  timer.setTimeout(10000, UNLOCK_SECURITY);
}

void UNLOCK_SECURITY()
{
  PASSWORD_REQUEST();
  security_lock = 0;
  error = 0;
  clr = 0;
}

void LCD_PHONG_KHACH()
{
  lcd_pk.clear();
  switch(state_lcdpk)
  {
  case 0: // nhiet do phong khach
    lcd_pk.print("Phong khach:");
    lcd_pk.setCursor(0, 1);
    lcd_pk.print("Nhiet do: ");
    lcd_pk.print(nhiet_do_pk);
    lcd_pk.print("*C");
    break;
  case 1: // do am phong khach
    lcd_pk.print("Phong khach:");
    lcd_pk.setCursor(0, 1);
    lcd_pk.print("Do am: ");
    lcd_pk.print(do_am_pk);
    lcd_pk.print("%");
    break;
  case 2: // nhiet do phong ngu
    lcd_pk.print("Phong ngu:");
    lcd_pk.setCursor(0, 1);
    lcd_pk.print("Nhiet do: ");
    lcd_pk.print(nhiet_do_pn);
    lcd_pk.print("*C");
    break;
  case 3: // nhiet do nha bep
    lcd_pk.print("Nha bep:");
    lcd_pk.setCursor(0, 1);
    lcd_pk.print("Nhiet do: ");
    lcd_pk.print(nhiet_do_bep);
    lcd_pk.print("*C");
    break;
  default:
    break;
  }
  state_lcdpk = (state_lcdpk + 1) % 4;
}
