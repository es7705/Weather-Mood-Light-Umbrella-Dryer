// 와이파이 관련 헤더
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;

// LED 헤더
#include <Adafruit_NeoPixel.h>

// 핀 지정
#define PIN D7 // 네오픽셀 핀
#define PIR D6 // PIR센서  핀
// LED 객체 생성
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800); // LED 개수, 아두이노 핀번호, flag

// 글로벌 변수 지정
String payload;           // HTTP 응답 데이터 저장 변수
bool sw;                  // 스위치 상태를 위한 변수
unsigned long timer1;     // 시간 측정을 위한 변수
unsigned long timer2 = 0; // 마지막 HTTP 요청 시간 저장 변수
String wt_twfEn;          // 날씨 상태 저장 변수
String sky_info;          // 하늘 정보 저장 변수
int bright;               // LED 밝기 저장 변수
uint32_t color;           // LED 색상 저장 변수

// 사용자 설정 WIFI 정보
const char *ssid = "abcd";                                                     // WIFI SSID
const char *password = "12349876";                                             // WIFI 비밀번호
const String URL = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=5113037000"; // 날씨 API URL

void setup()
{
    // WIFI 셋팅
    pinMode(PIR, INPUT);  // PIR 센서 핀을 입력으로 설정
    Serial.begin(115200); // 시리얼 통신 시작, 통신 속도 115200
    Serial.println();
    Serial.println();
    Serial.println();

    // 초기 대기 메시지 출력
    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t); // 대기 시간 출력
        Serial.flush();                           // 시리얼 버퍼 비우기
        delay(1000);
    }

    // WIFI 모드 설정
    WiFi.mode(WIFI_STA);             // WIFI 스테이션 모드로 설정
    WiFiMulti.addAP(ssid, password); // WIFI의 SSID, 패스워드 정보 추가

    // 스위치 셋팅
    pinMode(D7, OUTPUT); // D7(LED핀)을 출력으로 설정
    strip.begin();       // 네오픽셀을 초기화하기 위해 모든LED를 off시킨다
    strip.show();
}

void loop()
{
    int val = digitalRead(PIR); // PIR 센서 값 읽기
    Serial.print("PIR : ");
    Serial.println(val); // PIR 센서 값 출력
    delay(100);
    timer1 = millis(); // 현재 시간 저장

    // WIFI 연결 확인 및 HTTP 요청 주기 설정
    if ((WiFiMulti.run() == WL_CONNECTED) && (timer1 - timer2 >= 5000) || (timer2 == 0))
    {
        WiFiClient client; // WIFI 클라이언트 객체 생성
        HTTPClient http;   // HTTP 클라이언트 객체 생성

        Serial.print("[HTTP] begin...\n"); // HTTP 요청 시작 메시지 출력
        if (http.begin(client, URL))       // HTTP 요청 시작
        {
            Serial.print("[HTTP] GET...\n");
            int httpCode = http.GET();

            // HTTP 응답 처리
            if (httpCode > 0) // 요청 성공 시
            {
                Serial.printf("[HTTP] GET... code: %d\n", httpCode); // 응답 코드 출력

                // 서버에서 파일을 찾은 경우
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    payload = http.getString(); // 응답 데이터 저장
                    Serial.println("");
                    parsing();                              // 응답 데이터 파싱
                    bright = 220 - (sky_info.toInt() * 75); // 밝기 계산
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); // 오류 메시지
            }

            http.end(); // HTTP 연결 종료
        }

        timer2 = timer1; // 마지막 요청 시간을 현재 시간으로 갱신
    }

    // LED 루프
    // 각 상황별 LED 패턴
    if (val == 1) // PIR 센서가 감지된 경신
    {
        if (wt_twfEn == "Clear") // 맑은 날씨
        {
            weather_clear();
        }
        if (wt_twfEn == "Cloudy" || wt_twfEn == "Mostly Cloudy") // 흐림 또는 구름 많음
        {
            weather_cloudy();
        }

        if (wt_twfEn == "Rain" || wt_twfEn == "Shower") // 비 또는 소나기
        {
            weather_rain();
        }
    }
    else if (val == 0) // PIR 센서가 감지되지 않은 경우
    {
        weather_n(); // 모든 LED 끄기
    }
}

// HTTP 응답 데이터 파싱 함수
void parsing()
{
    String line = 싱payload; // 응답 데이터 복사

    // 시간 정보
    int temp11 = line.indexOf("</hour>"); // 시간 태그 종료 인뎃스 찾기
    if (temp11 > 0)
    {
        String tmp_str = "<hour>";
        String wt_hour = line.substring(line.indexOf(tmp_str) + tmp_str.length(), temp11); // 시간 정보 추출
        Serial.print("현재 시간 : ");
        Serial.println(wt_hour); // 현재 시간 출력
    }

    // 온도 정보
    int temp = line.indexOf("</temp>"); // 온도 태그 종료 인덱스 찾기
    if (temp > 0)
    {
        String tmp_str = "<temp>"기 String wt_temp = line.substring(line.indexOf(tmp_str) + tmp_str.length(), temp); // 온도 정보 추출
        Serial.print("온도 : ");
        Serial.println(wt_temp); // 온도 출력
    }

    // 날씨 정보
    int wfEn = line.indexOf("</wfEn>"); // 날씨 태그 종료 인덱스 찾기
    if (wfEn > 0)
    {
        String tmp_str = "<wfEn>";
        wt_twfEn = line.substring(line.indexOf(tmp_str) + tmp_str.length(), wfEn); // 날씨 상태 추출
        Serial.print("날씨 : ");
        Serial.println(wt_twfEn); // 날씨 상태 출력
    }

    // 하늘 정보
    int sky = line.indexOf("</sky>"); // 하늘 태그 종료 인덱스 찾기
    if (wfEn > 0)
    {
        String tmp_str = "<sky>";
        String wt_sky = line.substring(line.indexOf(tmp_str) + tmp_str.length(), sky); // 하늘 정보 추출
        sky_info = wt_sky;
        Serial.print("하늘 : ");
        Serial.println(wt_sky); // 하늘 정보 출력
    }

    // 습도
    int reh = line.indexOf("</reh>"); // 습도 태그 종료 인덱스 찾기
    if (reh > 0)
    {
        String tmp_str = "<reh>";
        String wt_reh = line.substring(line.indexOf(tmp_str) + tmp_str.length(), reh); // 습도 정보 추출
        Serial.print("습도 : ");
        Serial.println(wt_reh); // 습도 정보 출력
    }
}

// 비 오는 날씨 LED 함수
void weather_rain()
{
    for (uint16_t i = 0; i < strip.numPixels(); i++) // 모든 픽셀에 대해 반복
    {
        strip.setPixelColor(i, strip.Color(0, 0, 127)); // 비 : 파란색
    }
    strip.setBrightness(bright); // LED 밝기 설정
    strip.show();                // LED 상태 업데이트
    delay(3000);
}

// 맑은 날씨 LED 함수
void weather_clear()
{
    for (uint16_t i = 0; i < strip.numPixels(); i++) // 모든 픽셀에 대해 반복
    {
        strip.setPixelColor(i, strip.Color(127, 50, 50)); // 맑음 : 분홍색
    }
    strip.setBrightness(bright); // LED 밝기 설정
    strip.show();                // LED 상태 업데이트
    delay(3000);
}

// 흐림 또는 구름 많음 LED 함수
void weather_cloudy()
{
    for (uint16_t i = 0; i < strip.numPixels(); i++) // 모든 픽셀에 대해 반복
    {
        strip.setPixelColor(i, strip.Color(0, 255, 0)); // 구름 : 초록색
    }
    strip.setBrightness(bright); // LED 밝기 설정
    strip.show();                // LED 상태 업데이트
    delay(3000);
}

// 모든 LED 끄기
void weather_n()
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)    // 모든 픽셀에 대해 반복 
    {
        strip.setPixelColor(i, strip.Color(0, 0, 0));   // 모든 LED를 끔 
    }
    strip.setBrightness(bright); // LED 밝기 설정
    strip.show();                // LED 상태 업데이트
    delay(3000);
}

// NeoPixel에 달린 LED를 각각 주어진 인자값 색으로 채워나가는 함수
void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)    //  모든 픽셀에 대해
    {
        strip.setPixelColor(i, c);      // 주어진 색으로 설정 
        strip.show();                   // LED 상태 업데이트 
        delay(wait);                    
    }
}

// 모든 LED를 출력가능한 모든색으로 한번씩 보여주는 동작을 한번하는 함수
void rainbow(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256; j++)                   // 256가지 색상 반복 
    {
        for (i = 0; i < strip.numPixels(); i++) // 모든 픽셀에 대해 반복    
        {
            strip.setPixelColor(i, Wheel((i + j) & 255)); 
        }
        strip.show();   // LED 상태 업데이트 
        delay(wait);   
    }
}

// NeoPixel에 달린 LED를 각각 다른색으로 시작하여 다양한색으로 5번 반복한다
void rainbowCycle(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++)
    {
        for (i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// 입력한 색으로 LED를 깜빡거리며 표현한다
void theaterChase(uint32_t c, uint8_t wait)
{
    for (int j = 0; j < 10; j++)
    { // do 10 cycles of chasing
        for (int q = 0; q < 3; q++)
        {
            for (int i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, c); // turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (int i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, 0); // turn every third pixel off
            }
        }
    }
}
// 255가지의 색을 나타내는 함수
uint32_t Wheel(byte WheelPos)
{
    if (WheelPos < 85)
    {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
    else if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}
