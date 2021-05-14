#include <M5Stack.h>

//Microphone input
#define Input_M 35
#define SAMPLING_FREQUENCY 16000
#define SAMPLES 512
int buff[SAMPLES];
int cnt=0;
unsigned int sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));

//EMG input
#define Input_E 36 

// 保存するファイル名
//const char* fname = "/btnevent_log.csv";
char filename[13] = "/data000.csv";
byte filenum_1 = 0;
byte filenum_10 = 0;
byte filenum_100 = 0;
File file;

void SD_init(){
  while(1){
    if(SD.exists(filename)){
      M5.Lcd.println("Exist");
      delay(1000);
      filenum_1++;
      if(filenum_1 > 9){
        filenum_10++;
        filenum_1 = 0;
        if(filenum_10 > 9){
          filenum_100++;
          filenum_10 = 0;
        }
      }
      filename[5] = filenum_100+48;
      filename[6] = filenum_10+48;
      filename[7] = filenum_1+48;
    }else{
      M5.Lcd.println("Not exist");
      delay(1000);
      break;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  // ノイズ対策
  M5.Speaker.begin(); // これが無いとmuteしても無意味です。
  M5.Speaker.mute();
  
  pinMode(Input_M, INPUT);
  pinMode(Input_E, INPUT);
  M5.Lcd.setTextSize(2);

  //LCDの輝度を下げています。このPWMの周波数が10kHzで、
  //デフォルトの輝度だとこの信号をADCが拾ってしまい、次のように無音時でも10kHzの信号が測定されてしまいました。
  //LCDの輝度を下げることでPWMの影響を低減させています。
  M5.Lcd.setBrightness(20); 

  unsigned int sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  
  M5.Lcd.fillScreen(BLACK);  // 画面をクリア
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Waiting");
  //file = SD.open(fname, FILE_WRITE);
  //file.close();
  delay(1000);
  //Serial.begin(115200); 
  M5.Lcd.fillScreen(BLACK);  // 画面をクリア
  
  SD_init();
  M5.Lcd.println(filename);
  delay(1000);
}

void record(){
  file = SD.open(filename, FILE_APPEND);
  M5.Lcd.fillScreen(BLACK);  // 画面をクリア
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Start");
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.print(cnt);
  //M5.Lcd.fillRect(0, 120, 320, 100, BLACK); 
  
  for(int i=0; i<SAMPLES; i++){
    unsigned long t = micros();

    // put your main code here, to run repeatedly:
    // M5StackのAD変換は12ビットの分解能で、0〜3.3vの入力に対して0〜4095の値が返されます。
    buff[i] = analogRead(Input_M);
    int eE = analogRead(Input_E);
  
    //int ampM = eM >> 2 - 2048;

    // vccを5Vに推奨．0.5*vccがバイアスになる．
    // 3.6Vの入力に対してバイアスが2.5Vは均等ではない．
    // 1.4V未満は0にする． 3.6:4095 = 1.4:x -> x=1592.5
    //if(buff[i]<1593) buff[i]=0;

    // 3.6:4095 = 2.5:x -> x=4095*2.5/3.6=2843.75
    // 2843 を引くことで平均0の信号に
    //buff[i] = buff[i] - 2844;

    
    float VoutE = eE / 4095.0 * 3.3 + 0.1132;
    
    Serial.println(buff[i]);
    file.printf("%d,%.2f\n", buff[i], VoutE);
    //file.printf("%.2f\n", VoutE);

    M5.Lcd.setCursor(10, 50);
    M5.Lcd.print(i);
    
    if (i==0) continue;
    Serial.println(buff[i]);
    // 数値をある範囲から別の範囲に変換
    int y0 = map((int)(buff[i-1]), 0, 4096, 0, 480);
    int y1 = map((int)(buff[i]),   0, 4096, 0, 480);

    M5.Lcd.drawLine(i - 1 + 5, y0, i + 5, y1, GREEN);
    

    //delay(10);
    while ((micros() - t) < sampling_period_us) ;

  }
  
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Finish");
  file.close();

  //for (int i = 0; i < SAMPLES; i++) {
  //  Serial.println(buff[i]);
  //}
}

void loop() {
  //M5.update();
  
  if (cnt<50) {
    //M5.Lcd.fillScreen(BLACK);  // 画面をクリア
    record();
    cnt++; 
  }
}
