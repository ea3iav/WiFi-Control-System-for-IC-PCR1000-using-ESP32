#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <HardwareSerial.h>

// WIFI AP
const char* ssid = "prc1000";
const char* password = "12345678";

WebServer server(80);
WebSocketsServer webSocket(81);

// PCR1000
HardwareSerial PCR(2);

// ===== ESTADO =====
float frequency = 95.800;
int currentMode = 6;
int currentFilter = 4;

bool NB=false, AGC=false, ATT=false, ANF=false;

// STEP
float stepList[] = {0.001,0.005,0.0125,0.02,0.05,0.1};
int stepIndex = 5;
float step = 0.1;

// MEMORIAS
struct Memory {
  float freq;
  int mode;
  int filter;
};

Memory mem[5];
int memIndex = 0;

// ================= SETUP =================
void setup(){

  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  PCR.begin(9600, SERIAL_8N1, 16, 17);
  delay(2000);

  initPCR();

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

// ================= LOOP =================
void loop(){

  server.handleClient();
  webSocket.loop();

  static unsigned long t=0;
  if(millis()-t>2000){
    PCR.println("H800");
    t=millis();
  }

  static unsigned long smT=0;
  if(millis()-smT>400){
    readSMeter();
    smT=millis();
  }
}

// ================= INIT =================
void initPCR(){

  PCR.println("H101"); delay(800);
  PCR.println("G301"); delay(800);
  PCR.println("H800"); delay(500);

  PCR.println("J4026"); delay(500);
  PCR.println("J4100"); delay(500);

  PCR.println("J4501"); delay(300);
  PCR.println("J4600"); delay(300);
  PCR.println("J4700"); delay(300);

  sendFrequency();
}

// ================= FRECUENCIA =================
void sendFrequency(){

  long hz=(long)(frequency*1000000);
  int mhz=hz/1000000;
  int hz_part=hz%1000000;

  char cmd[32];
  sprintf(cmd,"K0%04d%06d%02d%02d00",mhz,hz_part,currentMode,currentFilter);

  PCR.print(cmd); PCR.print("\r\n");

  webSocket.broadcastTXT("FREQ:"+String(frequency,3));
}

// ================= S-METER =================
void readSMeter(){

  PCR.print("I1?\r\n");
  delay(10);

  String data="";

  while(PCR.available()){
    data+=(char)PCR.read();
  }

  int pos=data.indexOf("I1");

  if(pos!=-1 && data.length()>=pos+4){

    String hex=data.substring(pos+2,pos+4);
    int val=strtol(hex.c_str(),NULL,16);

    int sm=map(val,0x60,0x90,0,9);
    sm=constrain(sm,0,9);

    webSocket.broadcastTXT("SM:"+String(sm));
  }
}

// ================= MEM =================
void saveMemory(){
  mem[memIndex].freq = frequency;
  mem[memIndex].mode = currentMode;
  mem[memIndex].filter = currentFilter;

  memIndex++;
  if(memIndex>=5) memIndex=0;
}

void loadMemory(int i){
  frequency = mem[i].freq;
  currentMode = mem[i].mode;
  currentFilter = mem[i].filter;
  sendFrequency();
}

// ================= OPCIONES =================
void sendOptions(){

  PCR.println(AGC ? "J4501" : "J4500");
  PCR.println(NB  ? "J4601" : "J4600");
  PCR.println(ATT ? "J4701" : "J4700");
}

// ================= WS =================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){

  if(type == WStype_TEXT){

    String msg = (char*)payload;

    if(msg=="UP"){ frequency+=step; sendFrequency(); }
    if(msg=="DOWN"){ frequency-=step; sendFrequency(); }

    if(msg.startsWith("STEP:")){
      stepIndex = msg.substring(5).toInt();
      step = stepList[stepIndex];
    }

    if(msg.startsWith("MODE:")){
      currentMode = msg.substring(5).toInt();
      sendFrequency();
    }

    if(msg.startsWith("VOL:")){
      char cmd[10];
      sprintf(cmd,"J40%02X",msg.substring(4).toInt());
      PCR.println(cmd);
    }

    if(msg.startsWith("SQL:")){
      char cmd[10];
      sprintf(cmd,"J41%02X",msg.substring(4).toInt());
      PCR.println(cmd);
    }

    if(msg.startsWith("FILTER:")){
      currentFilter = msg.substring(7).toInt();
      sendFrequency();
    }

    if(msg.startsWith("TOGGLE:")){
      String t=msg.substring(7);
      if(t=="NB") NB=!NB;
      if(t=="AGC") AGC=!AGC;
      if(t=="ATT") ATT=!ATT;
      if(t=="ANF") ANF=!ANF;
      sendOptions();
    }

    if(msg.startsWith("FREQ:")){
      frequency = msg.substring(5).toFloat();
      sendFrequency();
    }

    if(msg=="SAVE"){ saveMemory(); }
    if(msg.startsWith("LOAD:")) loadMemory(msg.substring(5).toInt());
  }
}

// ================= WEB =================
void handleRoot(){

server.send(200,"text/html",R"rawliteral(

<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">

<style>

body{background:#0b0b0b;color:#00ff88;font-family:Arial;margin:0;text-align:center;touch-action:manipulation;}

.freq{font-size:48px;padding:15px 0;font-weight:bold;color:white;}

.smeter{width:70%;margin:10px auto;height:14px;border-radius:8px;background:black;}
.fill{height:100%;width:0%;border-radius:8px;background:linear-gradient(to right,#0f0,yellow,red);}

.row{display:flex;justify-content:space-between;padding:5px;}

.btn{flex:1;margin:4px;padding:14px;border-radius:10px;background:#1c1c1c;color:#ccc;border:1px solid #333;}
.active{color:red;border-color:red;}

.memfull{color:red;}
.memsel{border:2px solid red;}
.mpressed{background:red;color:black;}

.tune{
  display:flex;
  justify-content:center;
  gap:10px;
}

.fbtn{
  width:100px;
  height:100px;
  border-radius:20px;
  background:#d0d0d0;
  border:2px solid white;
  color:black;
  font-weight:bold;
  font-size:28px;   /* 👈 más grande */
  box-shadow:0 3px 6px rgba(0,0,0,0.5);
}

.step{
  width:65px;     /* antes 110px */
  height:65px;
  border-radius:50%;
  background:#d0d0d0;
  border:2px solid white;
  color:black;
  font-weight:bold;
  box-shadow:0 3px 6px rgba(0,0,0,0.5);
  font-size:12px;
}

.slider{
  width:90%;
  -webkit-appearance:none;
  background:transparent;
}

.slider::-webkit-slider-runnable-track{
  height:8px;
  background:red;
  border-radius:5px;
}

.slider::-webkit-slider-thumb{
  -webkit-appearance:none;
  width:18px;height:18px;
  border-radius:50%;
  background:white;
  margin-top:-5px;
}

#keypad{
  position:fixed;top:0;left:0;width:100%;height:100%;
  background:black;display:none;
}

.keygrid{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:10px;
  padding:10px;
}

.key{height:80px;font-size:24px;border-radius:12px;background:#222;color:white;}
.ok{background:white;color:black;}
.clear{background:#aa3333;}

.watermark{margin-top:25px;font-size:10px;color:#555;}

</style>
</head>

<body>

<div class="freq" onclick="openKeypad()" id="freq">95.800</div>
<div style="color:white;font-size:12px;margin-top:-10px;">IC-PCR1000</div>

<div class="smeter"><div id="sm" class="fill"></div></div>

<div class="tune">
<button class="fbtn" onclick="freqDown()">F-</button>
<button class="step" onclick="changeStep()" id="stepBtn">100k</button>
<button class="fbtn" onclick="freqUp()">F+</button>
</div>

<div class="row">
<button class="btn" id="Mbtn" ontouchstart="mPress()" ontouchend="mRelease()" onclick="save()">M</button>
<button class="btn" id="M0" onclick="loadMem(0)">M1</button>
<button class="btn" id="M1" onclick="loadMem(1)">M2</button>
<button class="btn" id="M2" onclick="loadMem(2)">M3</button>
<button class="btn" id="M3" onclick="loadMem(3)">M4</button>
<button class="btn" id="M4" onclick="loadMem(4)">M5</button>
</div>

<div class="row">
<button id="mode2" class="btn" onclick="mode(2)">AM</button>
<button id="mode5" class="btn" onclick="mode(5)">NFM</button>
<button id="mode6" class="btn active" onclick="mode(6)">WFM</button>
<button id="mode1" class="btn" onclick="mode(1)">USB</button>
<button id="mode0" class="btn" onclick="mode(0)">LSB</button>
<button id="mode3" class="btn" onclick="mode(3)">CW</button>
</div>

<div class="row">
<button id="NB" class="btn" onclick="tog('NB')">NB</button>
<button id="AGC" class="btn" onclick="tog('AGC')">AGC</button>
<button id="ATT" class="btn" onclick="tog('ATT')">ATT</button>
<button id="ANF" class="btn" onclick="tog('ANF')">ANF</button>
</div>

<div class="row">
<button id="F0" class="btn" onclick="filter(0)">2.8k</button>
<button id="F1" class="btn" onclick="filter(1)">6k</button>
<button id="F2" class="btn" onclick="filter(2)">15k</button>
<button id="F3" class="btn" onclick="filter(3)">50k</button>
<button id="F4" class="btn active" onclick="filter(4)">230k</button>
</div>

<div style="color:white;">VOL<br><input type="range" min="0" max="255" value="38" class="slider" oninput="vol(this.value)"></div>
<div style="color:white;">SQL<br><input type="range" min="0" max="255" value="0" class="slider" oninput="sql(this.value)"></div>

<div id="keypad">
<h1 id="display" style="color:white;">0</h1>

<div class="keygrid">
<button class="key" onclick="add('1')">1</button>
<button class="key" onclick="add('2')">2</button>
<button class="key" onclick="add('3')">3</button>
<button class="key" onclick="add('4')">4</button>
<button class="key" onclick="add('5')">5</button>
<button class="key" onclick="add('6')">6</button>
<button class="key" onclick="add('7')">7</button>
<button class="key" onclick="add('8')">8</button>
<button class="key" onclick="add('9')">9</button>
<button class="key" onclick="add('.')">.</button>
<button class="key" onclick="add('0')">0</button>
<button class="key clear" onclick="clearInput()">C</button>
</div>

<div class="row">
<button class="btn ok" onclick="sendFreq()">OK</button>
<button class="btn clear" onclick="closeKeypad()">EXIT</button>
</div>
</div>

<div class="watermark">Developed by EA3IAV</div>

<script>

let ws = new WebSocket("ws://" + location.hostname + ":81");

ws.onmessage = function(e){
  if(e.data.startsWith("FREQ:")){
    document.getElementById("freq").innerText = e.data.substring(5);
  }
  if(e.data.startsWith("SM:")){
    let v=parseInt(e.data.substring(3));
    document.getElementById("sm").style.width = v<=0 ? "0%" : (v*11)+"%";
  }
};

function freqUp(){ clearMemSel(); ws.send("UP"); }
function freqDown(){ clearMemSel(); ws.send("DOWN"); }

function vol(v){ ws.send("VOL:"+v); }
function sql(v){ ws.send("SQL:"+v); }

function filter(f){
  ws.send("FILTER:"+f);
  for(let i=0;i<5;i++) document.getElementById("F"+i).classList.remove("active");
  document.getElementById("F"+f).classList.add("active");
}

function tog(t){
  ws.send("TOGGLE:"+t);
  document.getElementById(t).classList.toggle("active");
}

function mode(m){
  ws.send("MODE:"+m);
  document.querySelectorAll("[id^='mode']").forEach(b=>b.classList.remove("active"));
  document.getElementById("mode"+m).classList.add("active");
}

let memPtr=0;

function mPress(){ document.getElementById("Mbtn").classList.add("mpressed"); }
function mRelease(){ document.getElementById("Mbtn").classList.remove("mpressed"); }

function save(){
  ws.send("SAVE");
  document.getElementById("M"+memPtr).classList.add("memfull");
  clearMemSel();
  document.getElementById("M"+memPtr).classList.add("memsel");
  memPtr++;
  if(memPtr>4) memPtr=0;
}

function loadMem(i){
  ws.send("LOAD:"+i);
  clearMemSel();
  document.getElementById("M"+i).classList.add("memsel");
}

function clearMemSel(){
  for(let i=0;i<5;i++) document.getElementById("M"+i).classList.remove("memsel");
}

let input="";

function openKeypad(){ document.getElementById("keypad").style.display="block"; }
function closeKeypad(){ document.getElementById("keypad").style.display="none"; input=""; }

function add(v){
  input+=v;
  document.getElementById("display").innerText=input;
}

function clearInput(){
  input="";
  document.getElementById("display").innerText="0";
}

function sendFreq(){
  ws.send("FREQ:"+input);
  closeKeypad();
  clearMemSel();
}

let stepIndex=5;
let stepLabels=["1k","5k","12.5k","20k","50k","100k"];

function changeStep(){
  stepIndex++;
  if(stepIndex>5) stepIndex=0;

  ws.send("STEP:"+stepIndex);

  document.getElementById("stepBtn").innerText = stepLabels[stepIndex];
}

</script>

</body>
</html>

)rawliteral");
}