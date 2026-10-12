void setup() {
  // put your setup code here, to run once:

}

void loop() {
  
constexpr int TFT_CS = -1;       
constexpr int TFT_RST = D2;
constexpr int TFT_SCK = D3;
constexpr int TFT_MOSI = D10;
constexpr int BTN_START = D6;   
constexpr int BTN_MODE = D7;     
constexpr int I2C_SDA = D4;
constexpr int I2C_SCL = D5;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);
Adafruit_LIS3DH lis;
AnimatedGIF gif;
File gifFile;
bool sensorOK = false, fsOK = false, gifIsOpen = false;
bool timerRunning = false, timerDone = false, motionScreen = false;
uint8_t mode = 0;
const uint16_t minutesByMode[] = {25, 5, 50};
const char *modeLabel[] = {"FOCUS 25", "BREAK 5", "DEEP FOCUS 50"};
uint32_t remainingMs = 25UL * 60 * 1000;
uint32_t lastTick = 0, lastDraw = 0, motionUntil = 0, lastMotion = 0;
uint32_t timerStarted = 0;
bool lastStart = HIGH, lastMode = HIGH;
uint32_t startChanged = 0, modeChanged = 0;
float oldX = 0, oldY = 0, oldZ = 9.8;

void centerText(const String &s, int y, uint16_t color, uint8_t size=2) {
  tft.setTextSize(size); tft.setTextColor(color, ST77XX_BLACK);
  int16_t x1,y1; uint16_t w,h;
  tft.getTextBounds(s,0,y,&x1,&y1,&w,&h);
  tft.setCursor((tft.width()-(int)w)/2,y); tft.print(s);
}
void header(const char *s) {
  tft.fillScreen(ST77XX_BLACK);
  centerText(s, 8, ST77XX_RED, 2);
  tft.drawFastHLine(8, 31, tft.width()-16, ST77XX_RED);
}
String timeLeft() {
  uint32_t sec=(remainingMs+999)/1000; char b[12];
  snprintf(b,sizeof(b),"%02lu:%02lu",(unsigned long)(sec/60),(unsigned long)(sec%60));
  return String(b);
}
void drawClock() {
  header("DEATH NOTE");
  struct tm ti;
  if (getLocalTime(&ti, 50)) {
    char tb[20], db[24];
    strftime(tb,sizeof(tb),"%I:%M:%S %p",&ti);
    strftime(db,sizeof(db),"%a, %b %d",&ti);
    centerText(tb,75,ST77XX_WHITE,3); centerText(db,120,ST77XX_CYAN,2);
  } else {
    centerText("CONNECTING WIFI",85,ST77XX_WHITE,1);
    centerText("Clock needs NTP sync",110,ST77XX_CYAN,1);
  }
  centerText("D6: START TIMER",190,ST77XX_WHITE,2);
  centerText("D7: CHANGE MODE",220,ST77XX_YELLOW,1);
}
void drawTimer() {
  header(modeLabel[mode]);
  centerText(timerDone ? "TIME IS UP" : (timerRunning ? "STAY FOCUSED" : "READY?"),
             55, timerDone ? ST77XX_RED : ST77XX_WHITE, 2);
  centerText(timeLeft(),100,ST77XX_CYAN,4);
  centerText(timerRunning ? "D6: PAUSE" : "D6: START",175,ST77XX_WHITE,2);
  centerText("D7: CHANGE MODE",210,ST77XX_YELLOW,1);
  centerText("L IS WATCHING",240,ST77XX_RED,1);
}
void closeGif() {
  if (gifIsOpen) { gif.close(); gifIsOpen=false; }
  if (gifFile) gifFile.close();
}
void *gifOpen(const char *name, int32_t *size) {
  gifFile=LittleFS.open(name,"r");
  if (!gifFile) return nullptr;
  *size=gifFile.size(); return &gifFile;
}
void gifClose(void *handle) {
  File *f=(File*)handle; if (f && *f) f->close();
}
int32_t gifRead(GIFFILE *f, uint8_t *buf, int32_t len) {
  File *file=(File*)f->fHandle; if (!file) return 0;
  int32_t avail=(int32_t)(file->size()-file->position());
  if (len>avail) len=avail; if (len<=0) return 0;
  return file->read(buf,len);
}
int32_t gifSeek(GIFFILE *f, int32_t pos) {
  File *file=(File*)f->fHandle; if (!file) return 0;
  file->seek(pos); return pos;
}
void gifDraw(GIFDRAW *d) {
  
  int y=d->iY+d->y, x=d->iX;
  if (y<0 || y>=tft.height()) return;
  uint8_t *px=d->pPixels; uint16_t *pal=d->pPalette;
  for (int i=0;i<d->iWidth;i++) {
    int xx=x+i; if (xx<0 || xx>=tft.width()) continue;
    if (d->ucHasTransparency && px[i]==d->ucTransparent) continue;
    tft.drawPixel(xx,y,pal[px[i]]);
  }
}
bool openGif(const char *path) {
  if (!fsOK) return false;
  closeGif();
  if (!gif.open(path,gifOpen,gifClose,gifRead,gifSeek,gifDraw)) return false;
  gifIsOpen=true; return true;
}
void startPause() {
  if (timerRunning) {
    uint32_t elapsed=millis()-timerStarted;
    remainingMs = elapsed >= remainingMs ? 0 : remainingMs-elapsed;
    timerRunning=false;
  } else {
    if (timerDone || remainingMs==0) {
      remainingMs=(uint32_t)minutesByMode[mode]*60000UL; timerDone=false;
    }
    timerRunning=true; timerStarted=millis();
    openGif("/gifs/focus.gif");
  }
  drawTimer();
}
void nextMode() {
  mode=(mode+1)%3; timerRunning=false; timerDone=false;
  remainingMs=(uint32_t)minutesByMode[mode]*60000UL;
  closeGif(); drawTimer();
}
void button(int pin, bool &last, uint32_t &changed, void (*action)()) {
  bool now=digitalRead(pin);
  if (now!=last) { last=now; changed=millis(); }
  static bool startStable=HIGH, modeStable=HIGH;
  bool &stable=(pin==BTN_START)?startStable:modeStable;
  if (millis()-changed>30 && now!=stable) {
    stable=now; if (stable==LOW) action();
  }
}
void motionReaction() {
  motionScreen=true; motionUntil=millis()+5000;
  closeGif(); tft.fillScreen(ST77XX_BLACK);
  centerText("L IS WATCHING",60,ST77XX_RED,2);
  centerText("YOU MOVED.",105,ST77XX_WHITE,2);
  centerText("BACK TO WORK",150,ST77XX_CYAN,2);
  openGif("/gifs/motion.gif");
}
void checkMotion() {
  if (!sensorOK || millis()-lastMotion<120 || motionScreen) return;
  lastMotion=millis(); sensors_event_t e; lis.getEvent(&e);
  float delta=fabs(e.acceleration.x-oldX)+fabs(e.acceleration.y-oldY)+fabs(e.acceleration.z-oldZ);
  oldX=e.acceleration.x; oldY=e.acceleration.y; oldZ=e.acceleration.z;
  if (delta>5.0) motionReaction();
}
void setup() {
  Serial.begin(115200);
  pinMode(BTN_START,INPUT_PULLUP); pinMode(BTN_MODE,INPUT_PULLUP);
  SPI.begin(TFT_SCK,-1,TFT_MOSI,TFT_CS);
  tft.init(240,280); tft.setRotation(0); tft.fillScreen(ST77XX_BLACK);
  Wire.begin(I2C_SDA,I2C_SCL);
  sensorOK=lis.begin(0x18); if (!sensorOK) sensorOK=lis.begin(0x19);
  if (sensorOK) lis.setRange(LIS3DH_RANGE_4_G);
  fsOK=LittleFS.begin(true); if (fsOK) gif.begin(BIG_ENDIAN_PIXELS);
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  header("DEATH NOTE"); centerText("BOOTING...",100,ST77XX_WHITE,2);
  uint32_t started=millis();
  while (WiFi.status()!=WL_CONNECTED && millis()-started<12000) delay(100);
  if (WiFi.status()==WL_CONNECTED) configTzTime(TIMEZONE,"pool.ntp.org","time.nist.gov");
  drawClock();
}
void loop() {
  button(BTN_START,lastStart,startChanged,startPause);
  button(BTN_MODE,lastMode,modeChanged,nextMode);
  checkMotion();
  if (timerRunning && millis()-lastTick>=250) {
    uint32_t now=millis(), elapsed=now-timerStarted; timerStarted=now; lastTick=now;
    if (elapsed>=remainingMs) { remainingMs=0; timerRunning=false; timerDone=true; closeGif(); drawTimer(); }
    else remainingMs-=elapsed;
  }
  if (motionScreen && (int32_t)(millis()-motionUntil)>=0) {
    motionScreen=false; closeGif(); if (timerRunning || timerDone) drawTimer(); else drawClock();
  }
  if (gifIsOpen) {
    int delayMs=0;
    if (gif.playFrame(true,&delayMs,nullptr)==0) {
      
      closeGif();
      if (motionScreen) openGif("/gifs/motion.gif");
      else if (timerRunning) openGif("/gifs/focus.gif");
    }
  }
  if (!motionScreen && !gifIsOpen && millis()-lastDraw>=1000) {
    lastDraw=millis(); if (timerRunning || timerDone) drawTimer(); else drawClock();
  }
}


}
