/*
  NHK2016 本番用メインプログラム
  SBDBT 5V使用
  since_2016/8/2/Wed.
  programmed by M.Seki
  changed by Y.Utsumi

  ver ツモツモ

  パターン
  タイヤ用モータ回転速度
	0000	慣性による停止
	0001	強制停止
	0010	回転速度25%		A
	0011	回転速度25%		B
	0100	回転速度50%		A
	0101	回転速度50%		B
	0110	回転速度100%	A
	0111	回転速度100%	B

  台車固定シーケンス用命令
	1000	シーケンス開始
			->	シーケンス開始から数sは命令通らなくなる
	ex)	Serial.write(0b11111000);

	参考ページ:
	http://easylabo.com/2015/04/arduino/8365/
*/

//*** 使用ピン設定 ***
#define LED_PIN_1 7
#define LED_PIN_2 8

//*** スティック設定 ***
#define R_STICK_EFFECTIVE 1 //0:無効 1:有効
#define L_STICK_EFFECTIVE 1

//** 一度に送ることのできる最大byte数 ****
#define MAX_SEND_BYTE_NUM 15

//*** アナログスティックの閾値 ***
#define THRESHOLD_100	60
#define THRESHOLD_50	40
#define THRESHOLD_25	20

//*** ボーレート ***
#define	BAUDRATE 115200

int requestSendDataNum = 0; //現時点での送信データ数
byte getData[8];
byte sendData[MAX_SEND_BYTE_NUM];
int brakeCheck		= 0;	// (1,1)で停止させるフラグ
int pushCheck		= 0;	// 押されているか、離されてるか
int stateCheck		= 0;	// フラグの状態
// int upDown_state	= 0;	// エアシリンダの上下フラグ

void setup() {
  //*** 出力設定 ***
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);

  //*** シリアル通信ボーレート ***
  Serial.begin(BAUDRATE);
  Serial1.begin(BAUDRATE);
  Serial2.begin(BAUDRATE);
  Serial3.begin(BAUDRATE);

  //*** 電源LED ON ***
  digitalWrite(LED_PIN_1, HIGH);
}

void loop() {
  requestSendDataNum = 0; //送信データ数リセット

  //*** 受信データの初期化 ***
  for (int n = 0; n < 8; n++) {
    getData[n] = 0;
  }

  //*** 送信データの初期化 ***
  for (int n = 0; n < MAX_SEND_BYTE_NUM; n++) {
    sendData[n] = 0;
  }

  //*** データ受信 ***
  if (Serial.available() > 0) {
    //*** データ受信LED ON ***
    digitalWrite(LED_PIN_2, HIGH);
    getData[0] = Serial.read();

    //*** データが0xFFのゴミだったら無視して読み込み直す ***
    if (getData[0] != 0xFF) {

      //*** 先頭識別子判断 ***
      if (getData[0] == 0x80) {

        //*** データ格納 ***
        for (int n = 1; n < 8; n++) {
          getData[n] = Serial.read();

          //*** データが0xFFのゴミだったら読み込み直すためにiを１もどす ***
          if (getData[n] == 0XFF) {
            n --;
          }
        }

        //	//*** データ確認（確認用） ***
        //	for (int n = 0; n < 8; n++) {
        //		Serial.write(getData[n]);
        //	}
        //	Serial.write(0x0D);

        //*** ボタン処理 ***
        if (getData[1] != 0x00 || getData[2] != 0x00) {
          //*** 条件分岐（if文の嵐） ***

          // 前後シリンダ系
          // 前シリンダ上昇
          if (getData[2] == 0x10 && getData[2] == 0x01) { //triangle & up
            requestValve(B0111, B0001);
          }
          // 前シリンダ下降
          if (getData[2] == 0x10 && getData[2] == 0x02) { //triangle & down
            requestValve(B0111, B0000);
          }
          // 後シリンダ上昇
          if (getData[2] == 0x20 && getData[2] == 0x01) {	//cross & up
            requestValve(B1000, B0001);
          }
          // 後シリンダ下降
          if (getData[2] == 0x20 && getData[2] == 0x02) { //cross & down
            requestValve(B1000, B0000);
          }

          if (getData[2] == 0x20) { //cross
            requestMotor(B0010, B0010);
          }
          if (getData[1] == 0x02 && getData[2] == 0x01) { //L1&square
            requestValve(B0011, B0001);
            requestValve(B0100, B0001);
            requestValve(B0101, B0001);
            requestValve(B0110, B0001);
          }
          if (getData[1] == 0x04 && getData[2] == 0x01) { //R1&square
            requestValve(B0011, B0000);
            requestValve(B0100, B0000);
            requestValve(B0101, B0000);
            requestValve(B0110, B0000);
          }
          // 特殊処理
          if (getData[1] == 0x02 && getData[2] == 0x40) { //L1&circle
            requestMotor(B1111, B1000);
          }
        }
        //*** スティック処理 ***
        else if ((getData[3] != 0x40 || getData[4] != 0x40) || (getData[5] != 0x40 || getData[6] != 0x40)) {
          brakeCheck = 1;
          double Lx, Ly, Rx, Ry;
          double L_radius, R_radius;
          double L_theta, R_theta;
          double L_angle, R_angle;
          double L_deg, R_deg;

          //*** 値代入 ***
          Lx = getData[3] - 0x3F;
          Ly = getData[4] - 0x3F;
          Rx = getData[5] - 0x3F;
          Ry = getData[6] - 0x3F;

          //*** 半径計算 ***
          L_radius = sqrt(( Lx * Lx ) + ( Ly * Ly ));
          R_radius = sqrt(( Rx * Rx ) + ( Ry * Ry ));

          //*** theta計算 ***
          L_theta = acos(Lx / L_radius);
          if (Ly > 0)
            L_theta *= (-1);

          R_theta = acos(Rx / R_radius);
          if (Ry > 0)
            R_theta *= (-1);

          //*** degree計算 ***
          L_deg = 57 * L_theta; //比より 57.295779513082320876798154814105
          if (L_theta < 0)
            L_deg += 360; //比より

          R_deg = 57 * R_theta; //比より 57.295779513082320876798154814105
          if (R_theta < 0)
            R_deg += 360; //比より


          //*** 左スティック 半径が閾値以上なら ***
          // to 5th board
          // 現在うまくいかない模様
          if (L_radius > THRESHOLD_100 && L_STICK_EFFECTIVE == 1) {
            if ( 45 <= L_deg && L_deg < 135) {
              requestMotor(B1100, B0110);
              requestMotor(B1101, B0110);
            }
            else if ( 225 <= L_deg && L_deg < 315 ) {
              requestMotor(B1100, B0111);
              requestMotor(B1101, B0111);
            }
          }
          else if (L_radius > THRESHOLD_50 && L_STICK_EFFECTIVE == 1) {
            if ( 45 <= L_deg && L_deg < 135) {
              requestMotor(B1100, B0100);
              requestMotor(B1101, B0100);
            }
            else if ( 225 <= L_deg && L_deg < 315 ) {
              requestMotor(B1100, B0101);
              requestMotor(B1101, B0101);
            }
          }
          else if (L_radius > THRESHOLD_25 && L_STICK_EFFECTIVE == 1) {
            if ( 45 <= L_deg && L_deg < 135) {
              requestMotor(B1100, B0010);
              requestMotor(B1101, B0010);
            }
            else if ( 225 <= L_deg && L_deg < 315 ) {
              requestMotor(B1100, B0011);
              requestMotor(B1101, B0011);
            }
          }
          else {
            requestMotor(B1100, B0000);
            requestMotor(B1101, B0000);
          }


          //*** 右スティック 半径が閾値以上なら ***
          // to 4th board
          // 動作確認済み
          if (R_radius > THRESHOLD_100 && R_STICK_EFFECTIVE == 1) {
            if ( 45 <= R_deg && R_deg < 135) {
              requestMotor(B1010, B0110);
              requestMotor(B1011, B0110);
            }
            else if ( 225 <= R_deg && R_deg < 315 ) {
              requestMotor(B1010, B0111);
              requestMotor(B1011, B0111);
            }
          }
          else if (R_radius > THRESHOLD_50 && R_STICK_EFFECTIVE == 1) {
            if ( 45 <= R_deg && R_deg < 135) {
              requestMotor(B1010, B0100);
              requestMotor(B1011, B0100);
            }
            else if ( 225 <= R_deg && R_deg < 315 ) {
              requestMotor(B1010, B0101);
              requestMotor(B1011, B0101);
            }
          }
          else if (R_radius > THRESHOLD_25 && R_STICK_EFFECTIVE == 1) {
            if ( 45 <= R_deg && R_deg < 135) {
              requestMotor(B1010, B0010);
              requestMotor(B1011, B0010);
            }
            else if ( 225 <= R_deg && R_deg < 315 ) {
              requestMotor(B1010, B0011);
              requestMotor(B1011, B0011);
            }
          }
          else {
            requestMotor(B1010, B0000);
            requestMotor(B1011, B0000);
          }

        }
        //*** 何も押してないとき ***
        else {
          if (brakeCheck) {
            brakeCheck = 0;
            requestMotor(B1010, B0000);
            requestMotor(B1011, B0000);
            requestMotor(B1100, B0000);
            requestMotor(B1101, B0000);
          }
          else{
          	requestMotor(B0000,B0000);
          	requestMotor(B0001,B0000);
          	requestMotor(B0010,B0000);
          	requestMotor(B1001,B0000);
          	requestMotor(B1110,B0000);
          }
        }

        //*** 出力 ***
        if (requestSendDataNum > 0) {
          for (int n = 0; n < requestSendDataNum; n++) {
            Serial3.write(sendData[n]);
          }
        }

        //		//*** データ確認（確認用） ***
        //		for (int n = 0; n < 8; n++) {
        //			Serial.write(getData[n]);
        //		}
        //		Serial.write(0x0D);

      }
    }
  }
  else {
    //***【データ受信していないとき】***
    digitalWrite(LED_PIN_2, LOW);
    requestMotor(B0000, B0000);
    requestMotor(B0001, B0000);
    requestMotor(B0010, B0000);
    requestValve(B0011, B0000);
    requestValve(B0100, B0000);
    requestValve(B0101, B0000);
    requestValve(B0110, B0000);
    requestValve(B0111, B0000);
    requestValve(B1000, B0000);
    requestMotor(B1001, B0000);
    requestMotor(B1010, B0000);
    requestMotor(B1011, B0000);
    requestMotor(B1100, B0000);
    requestMotor(B1101, B0000); //オムニ
    requestMotor(B1110, B0000); //昇降機構
  }
  delay(10);
}

//*** センサーデータ要求＆受け取り ***
int requestSensorData(byte sensorNo) {
  byte sensorData;
  Serial1.write(sensorNo);
  while (Serial1.available() == 0);
  sensorData = Serial1.read();
  return sensorData;
}

//**** LED要求 ***
void requestLED(byte deviceNo, byte colorData) {
  byte data;
  data += deviceNo;
  data = data << 4;
  data += colorData;
  Serial2.write(data);
}

//*** モータ要求 ***
void requestMotor(byte deviceNo, byte motorMotion) {
  sendData[requestSendDataNum] += deviceNo;
  sendData[requestSendDataNum] = sendData[requestSendDataNum] << 4;
  sendData[requestSendDataNum] += motorMotion;
  requestSendDataNum++;
}

//*** 電磁弁要求 ***
void requestValve(byte deviceNo, byte state) {
  sendData[requestSendDataNum] += deviceNo;
  sendData[requestSendDataNum] = sendData[requestSendDataNum] << 4;
  sendData[requestSendDataNum] += state;
  requestSendDataNum++;
}
