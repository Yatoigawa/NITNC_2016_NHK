/*
	NEW電ドラのデータ構造
	0	  1	  2 	3 	4	  5 	6 	7
	デバイスNo　    電磁弁No		        ON：1　 使用
        (デバイスNo.)                 OFF:0   未使用
*/

/*
  シリアル通信受信Ｒｘ  PD0
  クロック              PC3
  パラレル通信          PC4
  ＯＵＴ1               PB1
  ＯＵＴ2
  ＯＵＴ3
  ＯＵＴ4
  信号確認用ＬＥＤ  　  PC0
*/

#define SERIAL_SPEED  115200
#define DEVICE_NO     1   /*	サブボードの識別番号(データ処理部No.)を入力	*/
#define UNIT_MAX	    4   /*	ドライバユニットの個数						*/
#define PARALLEL	    A4	/*	パラレル通信用のピン番号を入力(Cピン)		*/
#define CHECK_LED	    A0  /*	信号確認用のＬＥＤのピン番号を入力(Cピン)	*/

uint8_t data = 0, deviceNo, valveBool, serial_buffer;

const uint8_t output_pins[] = {2, 3, 4, 5};
int i;

void setup() {
  Serial.begin(SERIAL_SPEED);  //ボーレート
  for (i = 0; i < 4; i++) {
    pinMode(output_pins[i], OUTPUT);
  }
  pinMode(CHECK_LED, OUTPUT);
  digitalWrite(CHECK_LED, HIGH);       /*通信できてることを返す*/
}

void loop() {
  if (Serial.available() > 0) {
    data = Serial.read();
  }
  deviceNo = (data >> 4);
  valveBool = (data & 0b00000001);

  //自分の番号か確認
  if ((deviceNo >= DEVICE_NO) && (deviceNo < (DEVICE_NO + UNIT_MAX))) {
    // 処理開始
    // 電磁弁仕様により以下のプログラムを使用するかどうかを確認
    if (valveBool == B0001) {
      digitalWrite(output_pins[deviceNo - DEVICE_NO], HIGH);
    } else {
      digitalWrite(output_pins[deviceNo - DEVICE_NO], LOW);
    }
  }

  delay(10);
}


