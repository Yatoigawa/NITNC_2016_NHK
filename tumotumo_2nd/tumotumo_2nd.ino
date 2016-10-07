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
#define DEVICE_NO     3   /*	サブボードの識別番号(データ処理部No.)を入力	*/
#define UNIT_MAX	    4   /*	ドライバユニットの個数						*/
#define PARALLEL	    A4	/*	パラレル通信用のピン番号を入力(Cピン)		*/
#define CHECK_LED	    A0  /*	信号確認用のＬＥＤのピン番号を入力(Cピン)	*/
#define VALVE_PIN0	  2   //ピンD2
#define VALVE_PIN1	  3   //ピンD3
#define VALVE_PIN2	  4   //ピンD4
#define VALVE_PIN3	  5   //ピンD5

uint8_t data = 0, deviceNo, valveBool, serial_buffer;

const uint8_t output_pins[] = {VALVE_PIN0, VALVE_PIN1, VALVE_PIN2, VALVE_PIN3};
int i;

void setup() {
  for (i = 0; i < UNIT_MAX; i++) {
    pinMode(output_pins[i], OUTPUT);
  }
  pinMode(PARALLEL, OUTPUT);
  pinMode(CHECK_LED, OUTPUT);

  //電磁弁の出力とパラレル通信するところがHIGH、あとはLOW
  Serial.begin(SERIAL_SPEED);  //ボーレート

  while (Serial.available() == 0);    /*シリアル通信が来るまで待つ*/
  digitalWrite(CHECK_LED, HIGH);       /*通信できてることを返す*/

  // パラレルによる本体への確認通信
  // 仕様変更により不使用
  // digitalWrite(PARALLEL, HIGH);
}

void loop() {
  //シリアル通信受信
  if (Serial.available() > 0) {
    data = Serial.read();
  }

  deviceNo = (data >> 4);
  valveBool = (data & 0b00000001);

  //自分の番号か確認
  if ((deviceNo >= DEVICE_NO) && (deviceNo < (DEVICE_NO + UNIT_MAX))) {
    // 処理開始
    // 電磁弁仕様により以下のプログラムを使用するかどうかを確認
    if (valveBool) {
      digitalWrite(output_pins[deviceNo - DEVICE_NO], HIGH);
    } else {
      digitalWrite(output_pins[deviceNo - DEVICE_NO], LOW);
    }
  }

  else if (deviceNo == 0b1111) {	// 特殊処理
    if (valveBool == 0b1000) {
      // シリアル停止処理
      // ないと多分バッファエラー起こすので回避するため記述
      Serial.end();
      // モータ回転開始用停止
      delay(1000);
      for (i = 0; i < UNIT_MAX; i++) {
        digitalWrite(output_pins[i], HIGH);
      }
      // モータ側と非同期でシリアル再開
      delay(500);
      Serial.begin(SERIAL_SPEED);
    }
  }
  delay(10);
}


