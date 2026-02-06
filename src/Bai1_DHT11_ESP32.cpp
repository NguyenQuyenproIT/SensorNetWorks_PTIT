#include <Arduino.h>

#define DHT_PIN 4   

void delay_us(uint32_t us)
{
    uint32_t start = micros(); // Lưu lại thời điểm bắt đầu - đếm đến (10^6)
    while (micros() - start < us);// lặp liên tục cho đến khi đủ us
}

// start dht11
void DHT11_Start(void)
{
    pinMode(DHT_PIN, OUTPUT); // điều khiển DATA
    digitalWrite(DHT_PIN, LOW); // kéo xuống lOW
    delay(22);                    // ≥18ms

    digitalWrite(DHT_PIN, HIGH); // kéo lên HIGH
    delay_us(34);                 // 20–40us

    pinMode(DHT_PIN, INPUT); // nhả bus - sensor bắt đầu điều khiển wire
}

uint8_t DHT11_Read_Bit(void)
{
    uint32_t t;

    // Chờ DHT kéo LOW kết thúc, không cần biết có đúng 50us hay không, vì không đo LOW
    while (digitalRead(DHT_PIN) == LOW); // chờ LOW kết thúc thì sẽ vào HIGH, đứng yên chờ từ LOW - HIGH

    // Bắt đầu đo thời gian HIGH
    t = micros(); // lưu thời điểm bắt đầu HIGH
                
    while (digitalRead(DHT_PIN) == HIGH);   // CPU đứng chờ HIGH kết thúc

    // >40us → bit 1, micro() sẽ luôn đếm và tính khoảng thời gian từ lúc bắt đầu gán để xác định HIGH
    if ((micros() - t) > 40) // check rằng HIGH đã kéo bao lâu kể từ khi bắt đầu
        return 1; // nếu 
    else
        return 0;
}

uint8_t DHT11_Read_Byte(void) // gửi MSB trước
{
    uint8_t i, byte = 0;

    for (i = 0; i < 8; i++) // sau 8 lần là đủ 1 byte
    {
        byte <<= 1; // vừa tính toán vừa gán lại vào "byte"
        byte |= DHT11_Read_Bit(); // mỗi lần gọi sẽ cộng thêm 1 bit (0 or 1) vào vị trí LSB 
    }
    return byte; // trả về 8 bit - 1 byte
}

// đọc các frame
// Trả về checksum
uint8_t DHT11_Read_Data(
    uint8_t *hum_int,
    uint8_t *hum_dec,
    uint8_t *temp_int,
    uint8_t *temp_dec
)
{
    uint8_t data[5]; // khởi tạo buffer
    uint32_t timeout = 0; // counter time
//Chờ tín hiệu phản hồi của DHT11 và kiểm tra xem có tồn tại hay không.
    /* ---- DHT response LOW 80us ---- */
    while (digitalRead(DHT_PIN) == HIGH) // Chờ DHT kéo chân xuống LOW
    {
        delay_us(1); // tối đa 100us
        if (++timeout > 100) return 0; // không tồn tại
    }

    /* ---- HIGH 80us ---- */
    timeout = 0;
    while (digitalRead(DHT_PIN) == LOW) // chờ LOW kết thúc
    {
        delay_us(1);
        if (++timeout > 100) return 0;
    }

    /* ---- LOW kết thúc response ---- */
    timeout = 0;
    while (digitalRead(DHT_PIN) == HIGH) // đợi LOW 50us đầu tiên
    {
        delay_us(1);
        if (++timeout > 100) return 0;
    }

    /* ---- Read 5 bytes ---- */
    for (int i = 0; i < 5; i++) // 40 bit sẽ từ đây
        data[i] = DHT11_Read_Byte(); // trả về từng byte, mỗi lần gọi hàm DHT11_Read_Byte() là sẽ trả về 8 bit và kết quả của hàm này là 1 byte

    *hum_int  = data[0];
    *hum_dec  = data[1];
    *temp_int = data[2];
    *temp_dec = data[3];

    return data[4]; // checksum
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32 DHT11 RAW DRIVER");
}

void loop()
{
    uint8_t hi, hd, ti, td, cs;

    DHT11_Start();
    cs = DHT11_Read_Data(&hi, &hd, &ti, &td);

    if ((hi + hd + ti + td) == cs) // so sánh checksum với 4 byte hum, temp
    { // thoả mãn
        Serial.print("Do am: ");
        Serial.print(hi);
        Serial.print(".");
        Serial.print(hd);

        Serial.print(" | Nhiet do: ");
        Serial.print(ti);
        Serial.print(".");
        Serial.println(td);
    }
    else
    {
        Serial.println("Loi checksum!");
    }

    delay(2000); // DHT11 nên để >= 2s
}

