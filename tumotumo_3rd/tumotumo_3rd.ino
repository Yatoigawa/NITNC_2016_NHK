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

#define F_CPU 16000000UL
#define DEVICE_NO	7/*	サブボードの識別番号(データ処理部No.)を入力	*/
#define UNIT_MAX	2	/*	ドライバユニットの個数						*/
#define PARALLEL	A4	/*	パラレル通信用のピン番号を入力(Cピン)		*/
#define CHECK_LED	A0  /*	信号確認用のＬＥＤのピン番号を入力(Cピン)	*/
#define VALVE_PIN0	2  //ピンD2
#define VALVE_PIN1	3  //ピンD3
#define VALVE_PIN2	4  //ピンD4
#define VALVE_PIN3	5  //ピンD5

uint8_t data = 0, deviceNo, valveBool, serial_buffer;

const uint8_t output_pins[] = {2, 3, 4, 5};

void setup() {
  /*
    DDRD = 0x0F;
    DDRC = 0x20;
    DDRB = 0x00;
  */
  pinMode(VALVE_PIN0, OUTPUT);
  pinMode(VALVE_PIN1, OUTPUT);
  pinMode(VALVE_PIN2, OUTPUT);
  pinMode(VALVE_PIN3, OUTPUT);
  pinMode(PARALLEL, OUTPUT);
  pinMode(CHECK_LED, OUTPUT);


  //電磁弁の出力とパラレル通信するところがHIGH、あとはLOW
  //sei();
  Serial.begin(9600);  //ボーレート

  while (Serial.available() == 0);	/*シリアル通信が来るまで待つ*/
  digitalWrite(CHECK_LED, HIGH);	/*通信できてることを返す*/
  digitalWrite(PARALLEL, HIGH);
}

void loop() {
  //シリアル通信受信
  if (Serial.available() != 0) {
    data = Serial.read();
  }

  deviceNo = (data >> 4);
  valveBool = (data & 0b00000001);

  //自分の番号か確認
  if ((deviceNo >= DEVICE_NO) && (deviceNo < (DEVICE_NO + UNIT_MAX))) {
    //処理開始
    if (valveBool) {
      digitalWrite(output_pins[deviceNo - DEVICE_NO], HIGH);
    } else {
      digitalWrite(output_pins[deviceNo - DEVICE_NO], LOW);
    }
  }
  delay(10);
}


