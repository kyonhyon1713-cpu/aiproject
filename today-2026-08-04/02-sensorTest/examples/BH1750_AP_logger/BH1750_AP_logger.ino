/*
 * BH1750 / GY-302 AP data collector for XIAO ESP32-S3.
 *
 * The board serves a browser page over its own Wi-Fi hotspot. The browser
 * displays live lux values, groups 30 one-second readings into one time-series
 * sample, and creates an Edge Impulse-ready ZIP file when requested.
 *
 * AP: bkh / 11112222
 * URL: http://192.168.4.1
 * BH1750/GY-302 address: 0x23
 * SDA: GPIO5 (D4), SCL: GPIO6 (D5)
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

constexpr uint8_t SDA_PIN = 5;
constexpr uint8_t SCL_PIN = 6;
constexpr uint8_t BH1750_ADDRESS = 0x23;

WebServer server(80);
uint8_t sensorAddress = BH1750_ADDRESS;
bool sensorReady = false;

bool probeSensor() {
  Wire.beginTransmission(sensorAddress);
  return Wire.endTransmission() == 0;
}

bool sendCommand(uint8_t command) {
  Wire.beginTransmission(sensorAddress);
  Wire.write(command);
  return Wire.endTransmission() == 0;
}

bool beginBH1750() {
  if (!probeSensor()) return false;
  if (!sendCommand(0x01)) return false;
  delay(10);
  if (!sendCommand(0x07)) return false;
  delay(10);
  if (!sendCommand(0x10)) return false;
  delay(180);
  return true;
}

bool readLux(float &lux, uint16_t &raw) {
  const uint8_t received = Wire.requestFrom(sensorAddress, (uint8_t)2);
  if (received != 2 || Wire.available() < 2) return false;

  raw = ((uint16_t)Wire.read() << 8) | Wire.read();
  lux = raw / 1.2f;
  return true;
}

static const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="ko"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BH1750 데이터 수집기</title>
<style>
body{font-family:system-ui,sans-serif;max-width:720px;margin:auto;padding:18px;background:#f5f7fb;color:#172033}
main{background:white;border-radius:14px;padding:18px;box-shadow:0 2px 12px #0001}
h1{font-size:24px;margin-top:0}.value{font-size:42px;font-weight:700;color:#1677d2;margin:14px 0}
.row{display:flex;gap:10px;flex-wrap:wrap;margin:10px 0;align-items:center}
label{font-weight:600}input{font-size:16px;padding:8px;border:1px solid #bbc5d4;border-radius:7px}
button{font-size:16px;padding:10px 14px;border:0;border-radius:8px;background:#1677d2;color:white;cursor:pointer}
button.secondary{background:#607086}button:disabled{opacity:.45;cursor:not-allowed}
.card{background:#eef4fb;border-radius:9px;padding:12px;margin:12px 0}.muted{color:#627086;font-size:14px}
#status{min-height:24px;margin-top:8px}.ok{color:#117a3a}.warn{color:#a45b00}
</style></head><body><main>
<h1>BH1750 / GY-302 조도 수집기</h1>
<div id="sensorStatus" class="muted">센서 확인 중...</div>
<div id="lux" class="value">-- lux</div>
<div class="card">
  <div>완성 샘플: <b id="sampleCount">0</b> / <span id="targetView">100</span></div>
  <div>현재 샘플 값: <b id="readingCount">0</b> / <span id="perSampleView">30</span></div>
  <div>수집한 전체 값: <b id="valueCount">0</b></div>
</div>
<div class="row"><label for="label">라벨</label><input id="label" value="indoor" maxlength="32">
<label for="target">라벨당 샘플</label><input id="target" type="number" min="1" max="1000" value="100" style="width:80px">
<label for="perSample">샘플당 값</label><input id="perSample" type="number" min="5" max="300" value="30" style="width:70px"></div>
<div class="row"><button id="start">수집 시작</button><button id="stop" class="secondary" disabled>중지</button><button id="zip" class="secondary">ZIP 다운로드</button></div>
<div id="status" class="muted">라벨을 입력하고 수집을 시작하세요.</div>
<p class="muted">1초마다 값을 읽습니다. CSV는 <code>timestamp,lux</code> 형식이며 ZIP 안에 샘플별 CSV로 저장됩니다. ZIP은 브라우저에서 원하는 위치에 저장하세요.</p>
</main><script>
const $=id=>document.getElementById(id);
let samples=[],current=[],recording=false,requestBusy=false;
function target(){return Math.max(1,Number($('target').value)||100)}
function perSample(){return Math.max(5,Number($('perSample').value)||30)}
function currentLabel(){return ($('label').value.trim()||'unlabelled').replace(/[^A-Za-z0-9_-]/g,'_')}
function labelSamples(){return samples.filter(s=>s.label===currentLabel()).length}
function render(){
  $('sampleCount').textContent=samples.length;
  $('targetView').textContent=target(); $('perSampleView').textContent=perSample();
  $('readingCount').textContent=current.length;
  $('valueCount').textContent=samples.reduce((n,s)=>n+s.rows.length,0)+current.length;
}
function setStatus(message,kind='muted'){$('status').textContent=message;$('status').className=kind}
function finishSample(){
  samples.push({label:currentLabel(),rows:current}); current=[]; render();
  const done=labelSamples();
  if(done>=target()){
    recording=false;$('start').disabled=false;$('stop').disabled=true;$('label').disabled=false;
    setStatus(currentLabel()+' 라벨의 목표 샘플을 완료했습니다.','ok')
  }else setStatus(currentLabel()+' 샘플 '+done+'/'+target()+' 완료','ok');
}
function startRecording(){
  if(recording)return;
  current=[];recording=true;$('start').disabled=true;$('stop').disabled=false;$('label').disabled=true;
  setStatus(currentLabel()+' 라벨 수집 중... 30초마다 샘플이 하나씩 완성됩니다.','ok');render();
}
function stopRecording(){
  recording=false;current=[];$('start').disabled=false;$('stop').disabled=true;$('label').disabled=false;
  setStatus('수집을 중지했습니다. 완성된 샘플만 보관됩니다.','warn');render()
}
async function poll(){
  if(requestBusy)return;requestBusy=true;
  try{
    const response=await fetch('/data?t='+Date.now());const d=await response.json();
    if(!d.ok)throw new Error(d.error||'sensor');
    $('lux').textContent=Number(d.lux).toFixed(2)+' lux';
    $('sensorStatus').textContent='BH1750/GY-302 연결됨 · I2C 0x'+Number(d.address).toString(16).toUpperCase();
    if(recording){
      current.push({timestamp:current.length*1000,lux:Number(d.lux)});
      if(current.length>=perSample())finishSample();render()
    }
  }catch(e){$('sensorStatus').textContent='센서 응답 없음 · 배선과 주소 0x23 확인';$('lux').textContent='-- lux'}
  requestBusy=false;
}
function csvFor(s){return 'timestamp,lux\n'+s.rows.map(r=>r.timestamp+','+r.lux.toFixed(3)).join('\n')+'\n'}
function bytesJoin(parts){let n=0;for(const p of parts)n+=p.length;const out=new Uint8Array(n);let o=0;for(const p of parts){out.set(p,o);o+=p.length}return out}
let crcTable;
function crc32(data){
  if(!crcTable){crcTable=[];for(let n=0;n<256;n++){let c=n;for(let k=0;k<8;k++)c=(c&1)?(0xEDB88320^(c>>>1)):(c>>>1);crcTable[n]=c>>>0}}
  let c=0xFFFFFFFF;for(const b of data)c=crcTable[(c^b)&255]^(c>>>8);return (c^0xFFFFFFFF)>>>0
}
function u16(v){const a=new Uint8Array(2);new DataView(a.buffer).setUint16(0,v,true);return a}
function u32(v){const a=new Uint8Array(4);new DataView(a.buffer).setUint32(0,v>>>0,true);return a}
function zipStore(files){
  const enc=new TextEncoder(),local=[],central=[];let offset=0;
  for(const file of files){
    const name=enc.encode(file.name),data=enc.encode(file.data),crc=crc32(data);
    const h=new Uint8Array(30),dv=new DataView(h.buffer);
    dv.setUint32(0,0x04034b50,true);dv.setUint16(4,20,true);dv.setUint16(8,0,true);
    dv.setUint32(14,crc,true);dv.setUint32(18,data.length,true);dv.setUint32(22,data.length,true);
    dv.setUint16(26,name.length,true);local.push(bytesJoin([h,name,data]));
    const c=new Uint8Array(46),cd=new DataView(c.buffer);
    cd.setUint32(0,0x02014b50,true);cd.setUint16(4,20,true);cd.setUint16(6,20,true);
    cd.setUint32(16,crc,true);cd.setUint32(20,data.length,true);cd.setUint32(24,data.length,true);
    cd.setUint16(28,name.length,true);cd.setUint32(42,offset,true);central.push(bytesJoin([c,name]));
    offset+=30+name.length+data.length
  }
  const body=bytesJoin(local),dir=bytesJoin(central),end=new Uint8Array(22),ed=new DataView(end.buffer);
  ed.setUint32(0,0x06054b50,true);ed.setUint16(8,files.length,true);ed.setUint16(10,files.length,true);
  ed.setUint32(12,dir.length,true);ed.setUint32(16,body.length,true);return bytesJoin([body,dir,end])
}
function downloadZip(){
  if(!samples.length){setStatus('먼저 완성된 샘플을 수집하세요.','warn');return}
  const files=samples.map((s,i)=>({name:s.label+'.'+String(i+1).padStart(4,'0')+'.csv',data:csvFor(s)}));
  files.push({name:'README.txt',data:'BH1750/GY-302 time-series dataset\naddress=0x23\nsample_rate_hz=1\n'});
  const blob=new Blob([zipStore(files)],{type:'application/zip'}),url=URL.createObjectURL(blob),a=document.createElement('a');
  a.href=url;a.download='bh1750_dataset.zip';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);setStatus('ZIP 다운로드를 시작했습니다.','ok')
}
$('start').onclick=startRecording;$('stop').onclick=stopRecording;$('zip').onclick=downloadZip;
setInterval(poll,1000);poll();render();
</script></body></html>
)HTML";

void handleRoot() { server.send_P(200, "text/html", PAGE); }

void handleData() {
  float lux = 0.0f;
  uint16_t raw = 0;
  if (!sensorReady || !readLux(lux, raw)) {
    server.send(503, "application/json", "{\"ok\":false,\"error\":\"BH1750 read failed\"}");
    return;
  }

  String json = "{\"ok\":true,\"address\":23,\"lux\":" +
                String(lux, 3) + ",\"raw\":" + String(raw) +
                ",\"ms\":" + String(millis()) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);
  sensorReady = beginBH1750();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("bkh", "11112222", 1);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.print("BH1750 AP logger: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() { server.handleClient(); }
