#define SERIAL_SPEED	115200	// シリアル通信のスピード
#define MOTOR_NUM_MIN	0		// 最初のユニットのナンバー
#define UNIT_MAX		3		// ドライバユニットの個数
#define LED_PIN			14		// 確認用LED

// #define BOARD_NUM	1


/*
 * 	モータ/電磁弁番号
 * 	ボード1	モータ
 * 		1:	
 * 		2:	
 * 		3:
 * 	ボード2	電磁弁
 * 		4:
 * 		5:
 * 		6:
 * 		7:
 * 	ボード3	電磁弁
 * 		5:
 * 		6:
 * 		7:
 * 		8:
 * 	ボード4	モータ
 * 		9:	船固定用モータ
 * 		10:	右後車輪
 * 		11:	右前車輪
 * 	ボード5	モータ
 * 		12:	左後車輪
 * 		13:	左前車輪
 * 		14:	船固定用モータ
 * 	特殊データ
 * 		16:	台車固定シーケンスへの移行
 * 		
 * 	モータ回転速度
 * 		0000	慣性による停止
 * 		0001	強制停止
 * 		0010	回転速度25%		A
 * 		0011	回転速度25%		B
 * 		0100	回転速度50%		A
 * 		0101	回転速度50%		B
 * 		0110	回転速度100%	A
 * 		0111	回転速度100%	B
 */

const uint8_t output_pins[] = {5, 6, 9, 10, 11, 3};		// 出力PIN

uint8_t subBoard_data, motor_num, motor_vel;
int i;				// ループ用

void setup() {
	Serial.begin(SERIAL_SPEED);
	for(i = 0;i < 6;i++){
		pinMode(output_pins[i]	,	OUTPUT);
	}
	pinMode(LED_PIN , OUTPUT);
	digitalWrite(LED_PIN , HIGH);
}

void loop() {
	if(Serial.available() > 0){
		subBoard_data = Serial.begin();

		motor_num = (subBoard_data >> 4);
		motor_vel = (subBoard_data & 0b00001111);

		if((motor_num >= MOTOR_NUM_MIN) && (motor_num < (MOTOR_NUM_MIN + UNIT_MAX))){
			switch(motor_vel){
				case 0b0000:	// 慣性による停止
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	0	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	0	);
					break;
				
				case 0b0001:	// 強制停止
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	250	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	250	);
					break;

				case 0b0010:	// 25%A
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	63	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	0	);
					break;

				case 0b0011:	// 25%B
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	0	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	63	);
					break;

				case 0b0100:	// 50%A
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	127	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	0	);
					break;

				case 0b0101:	// 50%B
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	0	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	127	);
					break;

				case 0b0110:	// 100%A
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	255	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	0	);
					break;

				case 0b0111:	// 100%B
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2]		,	0	);
					analogWrite(output_pins[(motor_num - MOTOR_NUM_MIN) * 2 + 1]	,	255	);
					break;
				
				default :
					for(i = 0;i < 6;i++){
						analogWrite(output_pins[i]	, 0	);
					}
					break;
			}
		}
	}
}
