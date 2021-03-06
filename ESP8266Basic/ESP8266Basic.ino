//ESP8266 Basic Interperter
//HTTP://ESP8266BASIC.COM
//
//The MIT License (MIT)
//
//Copyright (c) 2015 Michael Molinari
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

//Onewire tempture sensor code conntributed by Rotohammer.


#include <ArduinoJson.h>
#include "spiffs/spiffs.h"
#include <FS.h>
#include <ESP8266mDNS.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
//#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <ESP8266httpUpdate.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Base64.h"

//#include <WiFiClientSecure.h>
#include "ESP8266httpUpdate.h"
#include <time.h>
//#include <HttpClient.h>


//LCD Stuff
#include <LiquidCrystal_SR.h>
#include <I2CIO.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_SR3W.h>
#include <LCD.h>
#include <LiquidCrystal_SR2W.h>
#include <FastIO.h>
#include <LiquidCrystal_I2C.h>

//PS2 Key Board
//#include <PS2Keyboard.h>


#include <Time.h>

#include <Adafruit_NeoPixel.h>


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(255, 15, NEO_GRB + NEO_KHZ800);;

//ThingSpeak Stuff


const String BasicVersion = "ESP Basic 1.76";









OneWire oneWire(2);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address

//PS2Keyboard keyboard;

//WiFiClient EmailClient;
String EmailServer;
int Emailport;
String EmailSMTPuser;
String EmailSMTPpassword;



WiFiClient client;
ESP8266WebServer server(80);

//Web Server Variables
String HTMLout;
const String InputFormText = R"=====( <input type="text" id="myid" name="input"><input type="submit" value="Submit" name="inputButton"><hr>)=====";
const String TextBox = R"=====( <input type="text" id="myid" name="variablenumber" value="variablevalue">)=====";
const String passwordbox = R"=====( <input type="password" id="myid" name="variablenumber" value="variablevalue">)=====";
const String Slider = R"=====( <input type="range" id="myid" name="variablenumber" min="minval" max="maxval" value=variablevalue>)=====";
const String GOTObutton =  R"=====(<input type="submit" id="myid" value="gotonotext" name="gotonobranch">)=====";
const String GOTOimagebutton =  R"=====(<input type="image" id="myid" src="/file?file=gotonotext" value="gotonotext" name="gotonobranch">)=====";
const String normalImage =  R"=====(<img src="/file?file=name">)=====";
const String javascript =  R"=====(<script src="/file?file=name"></script>)=====";
const String CSSscript =  R"=====(<link rel="stylesheet" type="text/css" href="/file?file=name">)=====";
const String DropDownList =  R"=====(<select name="variablenumber" id="myid" size="theSize">options</select>
<script>document.getElementsByName("variablenumber")[0].value = "VARS|variablenumber";</script>)=====";
const String DropDownListOpptions =  R"=====(<option>item</option>)=====";


String LastElimentIdTag;



byte WaitForTheInterpertersResponse = 1;

const String AdminBarHTML = R"=====(
<a href="./vars">[ VARS ]</a> 
<a href="./edit">[ EDIT ]</a>
<a href="./run">[ RUN ]</a>
<a href="./settings">[ SETTINGS ]</a>
<a href="./filemng">[ FILE MANAGER ]</a>
<hr>)=====";



const String UploadPage = R"=====(
<form method='POST' action='/filemng' enctype='multipart/form-data'>
<input type='file' name='Upload'>
<input type='submit' value='Upload'>
</form>
<form id="filelist">
<input type="submit" value="Delete" name="Delete">
<input type="submit" value="View" name="View">
</form>

<select name="fileName" size="25" form="filelist">*table*</select>
)=====";



//<a href="http://www.esp8266basic.com/help"  target="_blank">[ HELP ]</a>

const String EditorPageHTML =  R"=====(
<script src="editor.js"></script>
<form action='edit' id="usrform">
<input type="text" name="name" value="*program name*">
<input type="submit" value="Open" name="open">
</form><button onclick="SaveTheCode()">Save</button>
<br>
<textarea rows="30" style="width:100%" name="code" id="code">*program txt*</textarea><br>
<input type="text" id="Status" value="">
)=====";


const String editCodeJavaScript =  R"=====(
function SaveTheCode() {
  var textArea = document.getElementById("code");
  var arrayOfLines = textArea.value.split("\n");
  httpGet("/codein?SaveTheCode=start");
  httpGet("/codein?SaveTheCode=yes");
for (i = 0; i <= arrayOfLines.length - 1; i++) 
{ 
  var x = i + 1;
  if (arrayOfLines[i] != "undefined")
  {
    arrayOfLines[i] = replaceAll(arrayOfLines[i],"+", "%2B");
    arrayOfLines[i] = replaceAll(arrayOfLines[i],"&", "%26");
    var WhatToSend = "/codein?line=" + x + "&code=" + encodeURI(arrayOfLines[i]);
    httpGet(WhatToSend);
    document.getElementById("Status").value = i.toString();
  }
}
document.getElementById("Status").value = "Saved";
alert("Saved");
}
function httpGet(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, false ); // false for synchronous request
    xmlHttp.send( null );
    return xmlHttp.responseText;
}

function replaceAll(str, find, replace) {
  for (x = 0; x <= 10; x++) 
{
  str = str.replace(find, replace);
}
return str;
}
)=====";


const String SettingsPageHTML =  R"=====(
<form action='settings' id="usrform"><table>
*BasicVersion*
<tr><th>
Station Mode (Connect to your router):</th></tr>
<tr><th><p align="right">Name:</p></th><th><input type="text" name="staName" value="*sta name*"></th></tr>
<tr><th><p align="right">Pass:</p></th><th><input type="password" name="staPass" value="*sta pass*"></th></tr>
<tr><th>
<br><br>Ap mode (ESP brocast out its own ap):</th></tr>
<tr><th><p align="right">Name:</p></th><th><input type="text" name="apName" value="*ap name*"></th></tr>
<tr><th><p align="right">Pass:<br>Must be at least 8 characters</p></th><th><input type="password" name="apPass" value="*ap pass*"></th></tr>
<tr><th>
<br><br>Log In Key (For Security):</th></tr>
<tr><th><p align="right">Log In Key:</p></th><th><input type="password" name="LoginKey" value="*LoginKey*"></th></tr>
<tr><th><p align="right">Display menu bar on index page:</p></th><th><input type="checkbox" name="showMenueBar" value="off" **checked**> Disable<br></th></tr>
<tr><th><p align="right">OTA URL. Leave blank for default:</p></th><th><input type="text" name="otaurl" value="*otaurl*"></th></tr>
<tr><th>
<input type="submit" value="Save" name="save">
<input type="submit" value="Format" name="format">
<input type="submit" value="Update" name="update">
<input type="submit" value="Restart" name="restart">
</th></tr>
</table></form>
<br>
)=====";







const String LogInPage =  R"=====(
<form action='settings' id="usrform">
Log In Key
<input type="password" name="key" value="">
<input type="submit" value="login" name="login">
</form>
)=====";


//Graphics HTML CODE

const String GraphicsStartCode =  R"=====(<svg width="*wid*" height="*hei*">)=====";

const String GraphicsLineCode =  R"=====(<line x1="*x1*" y1="*y1*" x2="*x2*" y2="*y2*" stroke="*collor*"/>)=====";

const String GraphicsCircleCode =  R"=====(<circle cx="*x1*" cy="*y1*" r="*x2*" fill="*collor*"/>)=====";

const String GraphicsEllipseCode =  R"=====(<ellipse cx="*x1*" cy="*y1*" rx="*x2*" ry="*y2*" fill="*collor*"/>)=====";

const String GraphicsRectangleCode =  R"=====(<rect x="*x1*" y="*y1*" width="*x2*" height="*y2*" style="fill:*collor*"/>)=====";



String WebArgumentsReceived;
String WebArgumentsReceivedInput;
byte numberButtonInUse = 0;
String ButtonsInUse[11];


String   msgbranch;
String   MsgBranchRetrnData;


// Buffer to store incoming commands from serial port
String inData;

int TotalNumberOfLines = 255;
//String BasicProgram[255];                                //Array of strings to hold basic program


String AllMyVaribles[50][2];
int LastVarNumberLookedUp;                                 //Array to hold all of the basic variables
bool VariableLocated;


bool RunningProgram = 1;                                //Will be set to 1 if the program is currently running
byte RunningProgramCurrentLine = 0;                     //Keeps track of the currently running line of code
byte NumberOfReturns;
bool BasicDebuggingOn;
byte ReturnLocations[254];

int TimerWaitTime;
int timerLastActiveTime;
String TimerBranch;

String refreshBranch;

int GraphicsEliments[100][7];

File fsUploadFile;

int noOfLinesForEdit;
String ProgramName;

bool fileOpenFail;


bool inputPromptActive = 0;

int LoggedIn = 0;

int SerialTimeOut;



byte ForNextReturnLocations[255];



Servo Servo0;
Servo Servo1;
Servo Servo2;
Servo Servo3;
Servo Servo4;
Servo Servo5;

Servo Servo12;
Servo Servo13;
Servo Servo14;
Servo Servo15;
Servo Servo16;

String  PinListOfStatus[16];
int  PinListOfStatusValues[16];


//time Stff
int timezone = 3;
int dst = 0;



FSInfo fs_info;

void setup() {
  pixels.begin();
  SPIFFS.begin();
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  WiFi.mode(WIFI_AP_STA);
  PrintAndWebOut(BasicVersion);

  //CheckWaitForRunningCode();

  server.on("/", []()
  {
    String WebOut;
    if (LoadDataFromFile("ShowMenueBar") != "off") WebOut =    AdminBarHTML;
    WebOut += RunningProgramGui();
    server.send(200, "text/html", WebOut);
  });


  server.on("/settings", []()
  {

    if ( server.arg("key") == LoadDataFromFile("LoginKey"))
    {
      LoggedIn = millis();
    }


    WaitForTheInterpertersResponse = 1;
    String WebOut = AdminBarHTML;
    WebOut += SettingsPageHTML;
    String staName;
    String staPass;
    String apName;
    String apPass;
    String LoginKey;
    String ShowMenueBar;
    String otaUrl;
    //Serial.print("Loading Settings Files");

    staName      = LoadDataFromFile("WIFIname");
    staPass      = LoadDataFromFile("WIFIpass");
    apName       = LoadDataFromFile("APname");
    apPass       = LoadDataFromFile("APpass");
    LoginKey     = LoadDataFromFile("LoginKey");
    ShowMenueBar = LoadDataFromFile("ShowMenueBar");
    otaUrl       = LoadDataFromFile("otaUrl");

    if (millis() > LoggedIn + 600000 || LoggedIn == 0 )
    {
      WebOut = LogInPage;
    }
    else
    {

      if ( server.arg("restart") == "Restart" ) ESP.restart();


      if ( server.arg("update") == "Update" )
      {

        //        Serial.println(BasicOTAupgrade());
        if (LoadDataFromFile("otaUrl") == "")
        {
          t_httpUpdate_return  ret = ESPhttpUpdate.update("esp8266basic.smbisoft.com", 80, "/4M/ESP8266Basic.cpp.bin");
          if (ret == HTTP_UPDATE_FAILED ) Serial.println("Update failed");
        }
        else
        {
          String URLtoGet = LoadDataFromFile("otaUrl");
          String ServerToConnectTo;
          String PageToGet;
          ServerToConnectTo = URLtoGet.substring(0, URLtoGet.indexOf("/"));
          PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));
          t_httpUpdate_return  ret = ESPhttpUpdate.update(ServerToConnectTo, 80, PageToGet);
          if (ret == HTTP_UPDATE_FAILED ) Serial.println("Update failed");
        }
        //t_httpUpdate_return  ret = ESPhttpUpdate.update("cdn.rawgit.com", 80, "/esp8266/Basic/master/Flasher/Build/4M/ESP8266Basic.cpp.bin");

      }


      if ( server.arg("save") == "Save" )
      {
        staName = GetRidOfurlCharacters(server.arg("staName"));
        staPass = GetRidOfurlCharacters(server.arg("staPass"));
        apName  = GetRidOfurlCharacters(server.arg("apName"));
        apPass  = GetRidOfurlCharacters(server.arg("apPass"));
        LoginKey = GetRidOfurlCharacters(server.arg("LoginKey"));
        ShowMenueBar = GetRidOfurlCharacters(server.arg("showMenueBar"));
        otaUrl       = GetRidOfurlCharacters(server.arg("otaurl"));

        SaveDataToFile("WIFIname" , staName);
        SaveDataToFile("WIFIpass" , staPass);
        SaveDataToFile("APname" , apName);
        SaveDataToFile("APpass" , apPass);
        SaveDataToFile("LoginKey" , LoginKey);
        SaveDataToFile("ShowMenueBar" , ShowMenueBar);
        SaveDataToFile("otaUrl" , otaUrl);
      }

      if ( server.arg("format") == "Format" )
      {
        Serial.println("Formating ");
        Serial.print(SPIFFS.format());
      }

      WebOut.replace("*sta name*", staName);
      WebOut.replace("*sta pass*", staPass);
      WebOut.replace("*ap name*",  apName);
      WebOut.replace("*ap pass*",  apPass);
      WebOut.replace("*LoginKey*", LoginKey);
      WebOut.replace("*BasicVersion*", BasicVersion);
      WebOut.replace("*otaurl*", otaUrl);

      if ( ShowMenueBar == "off")
      {
        WebOut.replace("**checked**", "checked");
      }
      else
      {
        WebOut.replace("**checked**", "");
      }
    }

    server.send(200, "text/html", WebOut);
  });



  server.on("/vars", []()
  {
    String WebOut = AdminBarHTML;
    if ( CheckIfLoggedIn() )
    {
      WebOut = LogInPage;
    }
    else
    {
      WebOut += "<div style='float: left;'>Variable Dump:";
      for (byte i = 0; i <= 50; i++)
      {
        if (AllMyVaribles[i][1] != "" ) WebOut += String("<hr>" + AllMyVaribles[i][1] + " = " + AllMyVaribles[i][2]);
      }


      WebOut += "<hr></div><div style='float: right;'>Pin Stats";
      for (byte i = 0; i <= 15; i++)
      {
        if ( i < 6 | i > 11) WebOut += String("<hr>" + String(i) + " = " + PinListOfStatus[i] + "  , " + String(PinListOfStatusValues[i]));
      }
      WebOut += "</div>";
    }

    server.send(200, "text/html", WebOut);
  });


  server.on("/run", []()
  {
    String WebOut;
    RunningProgram = 1;
    RunningProgramCurrentLine = 0;
    WaitForTheInterpertersResponse = 0 ;
    numberButtonInUse = 0;
    HTMLout = "";
    TimerWaitTime = 0;
    GraphicsEliments[0][0] = 0;
    WebOut = R"=====(  <meta http-equiv="refresh" content="0; url=./input?" />)=====";

    server.send(200, "text/html", WebOut);
  });



  server.onFileUpload(handleFileUpdate);

  server.on("/filemng", []()
  {
    DoSomeFileManagerCode();
  });


  server.on("/edit", []()
  {
    String WebOut;
    if (CheckIfLoggedIn())
    {
      WebOut = LogInPage;
    }
    else
    {

      WaitForTheInterpertersResponse = 1;

      String TextboxProgramBeingEdited;
      //String ProgramName;
      //WebOut = String("<form action='input'>" + HTMLout + "</form>");
      ProgramName = server.arg("name");


      if ( server.arg("open") == "Open" )
      {
        TextboxProgramBeingEdited = "";
        for (int i = 0; i <= TotalNumberOfLines; i++)
        {
          delay(0);
          String yada;
          yada = BasicProgram(i);
          yada.trim();
          if (yada != "")  TextboxProgramBeingEdited = String( TextboxProgramBeingEdited + String('\n') + BasicProgram(i) );
          if (fileOpenFail == 1 & i > 1) break;
        }
      }


      WebOut += EditorPageHTML;

      WebOut.replace("*program txt*", TextboxProgramBeingEdited);
      WebOut.replace("*program name*", ProgramName);

      //TextboxProgramBeingEdited
    }
    server.send(200, "text/html", String(AdminBarHTML + WebOut ));
  });



  server.on("/editor.js", []() {
    server.send(200, "text/html", editCodeJavaScript);
  });


  server.on("/codein", []() {

    if (ProgramName == "")
    {
      ProgramName = "default";
    }

    if (server.arg("SaveTheCode") == "start")
    {
      inData = "end";
      ExicuteTheCurrentLine();
    }

    if (server.arg("SaveTheCode") != "yes" & server.arg("SaveTheCode") != "start")
    {
      String LineNoForWebEditorIn;
      LineNoForWebEditorIn = server.arg("line");
      int y = LineNoForWebEditorIn.toInt();
      delay(0);
      //Serial.println(server.arg("code"));
      BasicProgramWriteLine(y, GetRidOfurlCharacters(server.arg("code")));
      delay(0);
      noOfLinesForEdit = y;

    }

    if (server.arg("SaveTheCode") == "yes")
    {

      String directoryToDeleteFilesFrom;
      directoryToDeleteFilesFrom = String(" /data/" + ProgramName );
      Dir dir1 = SPIFFS.openDir(directoryToDeleteFilesFrom);

      while (dir1.next())
      {
        delay(0);
        File f = dir1.openFile("r");
        if (dir1.fileName().substring(0, directoryToDeleteFilesFrom.length()) == directoryToDeleteFilesFrom) SPIFFS.remove(dir1.fileName());
      }
    }
    server.send(200, "text/html", "good");
  });





  server.on("/msg", []() {

    MsgBranchRetrnData = "No MSG Branch Defined";

    if (msgbranch != "")
    {
      inData = String(" goto " + msgbranch + " ");
      WaitForTheInterpertersResponse = 0;
      ExicuteTheCurrentLine();
      runTillWaitPart2();
    }


    server.send(200, "text/html", MsgBranchRetrnData);
  });





  server.on("/input", []() {
    server.send(200, "text/html", RunningProgramGui());
  });

  server.onNotFound ( []() {
    String fileNameToServeUp;
    fileNameToServeUp = GetRidOfurlCharacters(server.arg("file"));
    File mySuperFile = SPIFFS.open(String("uploads/" + fileNameToServeUp), "r");
    if (mySuperFile)
    {
      server.streamFile(mySuperFile, getContentType(fileNameToServeUp));
      //server.send(200, getContentType(fileNameToServeUp), mySuperFile.readString());

    }
    else
    {
      server.send(200, "text/html", RunningProgramGui());
    }
    mySuperFile.close();
  });

  //LoadBasicProgramFromFlash("");


  if (  ConnectToTheWIFI(LoadDataFromFile("WIFIname"), LoadDataFromFile("WIFIpass"), "", "", "") == 0)
  {
    if (LoadDataFromFile("APname") == "")
    {
      CreateAP("", "");
    }
    else
    {
      CreateAP(LoadDataFromFile("APname"), LoadDataFromFile("APpass"));
    }
  }


  Wire.begin(0, 2);

  //  keyboard.begin(14, 12); //For PS2 keyboard input

  StartUp_OLED();
  lcd.begin(16, 2); // initialize the lcd for 16 chars 2 lines and turn on backlight
  sensors.begin();

  server.begin();
  RunningProgram = 0;
  WaitForTheInterpertersResponse = 1;
  StartUpProgramTimer();

}


String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void StartUpProgramTimer()
{
  while  (millis() < 60000)
  {
    delay(0);
    //Serial.println(millis());
    server.handleClient();
    if (WaitForTheInterpertersResponse == 0) return;
  }
  Serial.println("Starting Default Program");
  RunningProgram = 1;
  RunningProgramCurrentLine = 0;
  WaitForTheInterpertersResponse = 0 ;
  numberButtonInUse = 0;
  HTMLout = "";
  return;
}



void DoSomeFileManagerCode()
{
  String WholeUploadPage = UploadPage;
  String FileListForPage ;

  if (CheckIfLoggedIn())
  {
    WholeUploadPage = LogInPage;
  }
  else
  {
    if (server.arg("Delete") != "")
    {
      String FIleNameForDelete = server.arg("fileName");
      FIleNameForDelete = GetRidOfurlCharacters(FIleNameForDelete);
      Serial.println(FIleNameForDelete);
      SPIFFS.remove(FIleNameForDelete);
      //Serial.println(SPIFFS.remove("uploads/settings.png"));
    }

    Dir dir = SPIFFS.openDir(String("uploads" ));
    while (dir.next()) {
      FileListForPage += String("<option>" + dir.fileName() + "</option>");
      delay(0);
    }

    WholeUploadPage.replace("*table*", FileListForPage);

    if (server.arg("View") != "")
    {
      String FileNameToView = server.arg("fileName");
      FileNameToView = GetRidOfurlCharacters(FileNameToView);
      FileNameToView.replace("uploads/", "");
      WholeUploadPage = R"=====(  <meta http-equiv="refresh" content="0; url=./file?file=item" />)=====";
      WholeUploadPage.replace("item", FileNameToView);
    }

  }
  server.send(200, "text/html",  String( AdminBarHTML + WholeUploadPage ));
}



void handleFileUpdate()
{
  //if (server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    //DBG_OUTPUT_PORT.print("Upload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(String("uploads/" + filename), "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("Upload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    //DBG_OUTPUT_PORT.print("Upload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}








String  getSerialInput()
{
  unsigned long  serialTimeOutStart = millis();
  bool donereceivinginfo = 0;
  Serial.println(">");

  String someInput;
  while (donereceivinginfo == 0)
  {
    if (serialTimeOutStart + SerialTimeOut < millis() & SerialTimeOut != 0) return someInput;
    delay(0);
    while (Serial.available() > 0)
    {
      char recieved = Serial.read();
      // Process message when new line character is recieved
      if (recieved == '\n')
      {
        Serial.println(someInput);
        donereceivinginfo = 1;
        return someInput;
      }
      someInput += recieved;
    }
  }
}






//String GetPS2input()
//{
//  String PS2inputString;
//
//
//  bool donereceivinginfo = 0;
//
//  while (donereceivinginfo == 0)
//  {
//    delay(0);
//    while (keyboard.available())
//    {
//      delay(0);
//      // read the next key
//      char c = keyboard.read();
//
//      // check for some of the special keys
//      if (c == PS2_ENTER) {
//        return PS2inputString;
//      } else if (c == PS2_TAB) {
//        //Serial.print("[Tab]");
//      } else if (c == PS2_ESC) {
//        return "";
//      } else if (c == PS2_PAGEDOWN) {
//        //Serial.print("[PgDn]");
//      } else if (c == PS2_PAGEUP) {
//        //Serial.print("[PgUp]");
//      } else if (c == PS2_LEFTARROW) {
//        //Serial.print("[Left]");
//      } else if (c == PS2_RIGHTARROW) {
//        //Serial.print("[Right]");
//      } else if (c == PS2_UPARROW) {
//        //Serial.print("[Up]");
//      } else if (c == PS2_DOWNARROW) {
//        //Serial.print("[Down]");
//      } else if (c == PS2_DELETE) {
//        //Serial.print("[Del]");
//      } else {
//        if (BasicDebuggingOn == 1) Serial.print(c);
//        PS2inputString += c;
//      }
//    }
//  }
//}






bool CheckIfLoggedIn()
{
  if (LoadDataFromFile("LoginKey") != "")
  {
    if ( millis() > LoggedIn + 600000 || LoggedIn == 0 )     return 1;
  }
  return 0;
}










void loop()
{

  RunBasicTillWait();

  server.handleClient();
}



void RunBasicTillWait()
{
  runTillWaitPart2();
  if (RunningProgramCurrentLine > TotalNumberOfLines)
  {
    RunningProgram = 0 ;
    TimerWaitTime = 0;
    return;
  }

  if (TimerWaitTime + timerLastActiveTime <= millis() &  TimerWaitTime != 0)
  {
    inData = String(" goto " + TimerBranch + " ");
    WaitForTheInterpertersResponse = 0;
    timerLastActiveTime = millis() ;
    ExicuteTheCurrentLine();
    runTillWaitPart2();
  }
  delay(0);
  for (int pinnn = 0; pinnn <= 15 ; pinnn++)
  {
    delay(0);
    //Serial.println(pinnn);
    if ((PinListOfStatus[pinnn] != "po") & ( PinListOfStatus[pinnn] != "pi") & (PinListOfStatus[pinnn] != "pwi") & (PinListOfStatus[pinnn] != "pwo") & (PinListOfStatus[pinnn] != "servo") & ( PinListOfStatus[pinnn] != ""))
    {
      //Serial.println("Foud interupt pin");
      if ( PinListOfStatusValues[pinnn] != UniversalPinIO("pi", String(pinnn), 0))
      {
        inData = String(" goto " + PinListOfStatus[pinnn] + " ");
        WaitForTheInterpertersResponse = 0;
        //Serial.println(PinListOfStatus[pinnn]);
        ExicuteTheCurrentLine();
        runTillWaitPart2();
      }
    }
  }
}


void runTillWaitPart2()
{
  while (RunningProgram == 1 && RunningProgramCurrentLine < TotalNumberOfLines && WaitForTheInterpertersResponse == 0 )
  {
    delay(0);
    RunningProgramCurrentLine++;
    inData = BasicProgram(RunningProgramCurrentLine);
    if (fileOpenFail == 1) inData  = "end";
    ExicuteTheCurrentLine();
    delay(0);
  }
}





String getValueforPrograming(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}




String getValue(String data, char separator, int index)
{
  data = String(data + "           ");
  int maxIndex = data.length() - 1;
  int j = 0;
  byte WaitingForQuote;
  String chunkVal = "";
  String ChunkReturnVal;
  for (int i = 0; i <= maxIndex && j <= index; i++)
  {
    if (data[i] == '\"' )
    {
      i++;
      while (i <= maxIndex && data[i] != '\"' ) {
        chunkVal.concat(data[i]);
        i++;
        delay(0);
      }
    }
    else if (data[i] == '|' )
    {
      i++;
      while (i <= maxIndex && data[i] != '|' ) {
        chunkVal.concat(data[i]);
        i++;
        delay(0);
      }
    }
    else
    {
      if (data[i] != separator) chunkVal.concat(data[i]);
    }

    if (data[i] == separator & data[i - 1] != separator)
    {
      j++;
      if (j > index)
      {
        //chunkVal.trim();
        if (chunkVal != String(separator))
        {
          ChunkReturnVal = chunkVal;
          break;
        }
      }
      chunkVal = "";
    }
  }
  //    Serial.println("index");
  //    Serial.println(index);
  //    Serial.println(j);

  if (j == index + 1)
  {

    return ChunkReturnVal;
  }
}












String DoMathForMe(String cc, String f, String dd )
{
  double e;
  String ee = cc;

  double c = cc.toFloat();
  double d = dd.toFloat();

  f.trim();

  if (f == "-") {
    e = c - d;
    ee = String(e);
  }
  if (f == "+") {
    e = c + d;
    ee = String(e);
  }
  if (f == "*") {
    e = c * d;
    ee = String(e);
  }
  if (f == "/") {
    e = c / d;
    ee = String(e);
  }
  if (f == "^") {
    e = pow(c , d);
    ee = String(e);
  }



  if (f == "&") {
    ee = String(cc + dd);
  }


  if (f ==  ">") {
    ee = String((c > d));
  }
  if (f ==  "<") {
    ee = String((c < d));
  }


  if (f ==  ">=") {
    ee = String((c >= d));
  }
  if (f ==  "<=") {
    ee = String((c <= d));
  }

  if (f == "<>" || f == " != ")
  {
    if (cc != dd)
    {
      ee = "1";
    }
    else
    {
      if (c != d)
      {
        ee = "1";
      }
    }
  }


  if (f == "==")
  {
    ee = String((cc == dd));
  }

  if (f == "=")
  {
    ee = String((c == d));
  }
  return ee;
}












String FetchWebUrl(String URLtoGet)
{
  String str = "             ";
  String ServerToConnectTo = URLtoGet.substring(0, URLtoGet.indexOf("/"));
  String PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));
  // ServerToConnectTo ;
  //PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));

  Serial.println(ServerToConnectTo);
  Serial.println(PageToGet);


  if (client.connect(ServerToConnectTo.c_str() , 80))
  {
    client.print(String("GET " + PageToGet + " HTTP/1.1\r\nHost: " +  ServerToConnectTo + "\r\n\r\n"));
    delay(300);
    while (client.available())
    {
      delay(0);
      if (str.endsWith(String("\r\n\r\n")))  str = "";

      str.concat( (const char)client.read());
      delay(0);
    }



    client.stop();
    return str.substring(0, str.indexOf(String(String(char(10)) + "0" )  ));
  }
  client.stop();
  return "";
}



void serialFlush()
{

  while (Serial.available() > 0)
  {
    delay(0);
    char t = Serial.read();
  }
}
