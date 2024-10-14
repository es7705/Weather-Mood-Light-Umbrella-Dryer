int Raindrops_pin = A0;   // 1번 빗방울 센서 핀을 A0으로 설정
int Raindrops_pin1 = A1;  // 2번 빗방울 센서 핀을 A1으로 설정
int Raindrops_pin2 = A2;  // 3번 빗방울 센서 핀을 A2으로 설정
#define RELAY 4            // 릴레이 핀 정의

void setup() {
    Serial.begin(9600);  
    pinMode(A0, INPUT);   // 1번 빗방울 감지 핀을 아날로그 입력으로 설정
    pinMode(A1, INPUT);   // 2번 빗방울 감지 핀을 아날로그 입력으로 설정
    pinMode(A2, INPUT);   // 3번 빗방울 감지 핀을 아날로그 입력으로 설정 (새로 추가된 센서)
    pinMode(RELAY, OUTPUT); // 릴레이 핀을 출력으로 설정
}

void loop() {
    Serial.println(analogRead(A0));  // 1번 센서값을 시리얼 모니터로 전송
    Serial.println(analogRead(A1));  // 2번 센서값을 시리얼 모니터로 전송
    Serial.println(analogRead(A2));  // 3번 센서값을 시리얼 모니터로 전송
    delay(100);

    // 세 개의 센서 중 하나라도 수분이 감지되면
    if (analogRead(A0) < 700 || analogRead(A1) < 700 || analogRead(A2) < 700) {
        digitalWrite(RELAY, LOW); // 팬 작동
    } else { // 수분이 감지되지 않으면 팬 작동 끄기
        digitalWrite(RELAY, HIGH);
    }
    delay(100);
}
