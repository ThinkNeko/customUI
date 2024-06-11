
// 8_1_2_referee_0x0202_rcvから移植.
const int DATAOFFSET = 7;  // ジャッジシステム通信フォーマットのData開始Offset

static unsigned int dataNumber_3 = 0;
static uint8_t data_3[60];
static int countRecMiss = 0;
static uint8_t header[4];

union {
  float f;
  unsigned long l;
} wk;
// リアルタイム出力、熱力データ　(0x0202)
typedef struct {
  uint16_t chassisVolt;
  uint16_t chassisCurrent;
  float chassisPower;
  uint16_t chassisPowerBuffer;
  uint16_t shooterHeat0;
  uint16_t shooterHeat1;
} ext_realtime_power_data_t;

ext_realtime_power_data_t realtimePowerData;

void getJudgeSystemData() {
  // Serial.print(" check:");
  // Serial.println(Serial3.available());
  while (Serial3.available() > 0) {
    //    Serial.print(" OK:");
    countRecMiss = 0;
    unsigned int recData = Serial3.read();
    if (recData == 0xA5) {
      dataNumber_3 = 0;
      data_3[dataNumber_3 % 60] = recData;
    } else {
      dataNumber_3++;
      data_3[dataNumber_3 % 60] = recData;
    }

    if ((dataNumber_3 == 0)) {
      // ヘッダ情報---------------------------------------------------------------
      int sof = data_3[0];  // SOF 0xA5固定
      long dataLength =
          (data_3[2] << 8) | (data_3[1]);  // データフレーム内の長さ
      int seq = data_3[3];                 // シーケンス番号
      int crc8Value = data_3[4];           // ヘッダ
      header[0] = sof;
      header[1] = data_3[1];
      header[2] = data_3[2];
      header[3] = data_3[3];
      // Serial.print("CRC8:0x");
      // Serial.println(Get_CRC8_Check_Sum(header,4,0xFF),HEX);

      // コマンドID---------------------------------------------------------------
      long cmdId = (data_3[6] << 8) | (data_3[5]);  // コマンドID

      if (cmdId == 0x0202) {
        // データセグメントの長さ：14byte
        // リアルタイム出力、熱量データ
        // ※50Hz周波数の周期で発信
        realtimePowerData.chassisVolt =
            ((data_3[DATAOFFSET + 1]) << 8) | (data_3[DATAOFFSET + 0]);
        realtimePowerData.chassisCurrent =
            ((data_3[DATAOFFSET + 3]) << 8) | (data_3[DATAOFFSET + 2]);
        wk.l =
            ((data_3[DATAOFFSET + 7] << 24) | (data_3[DATAOFFSET + 6] << 16) |
             (data_3[DATAOFFSET + 5] << 8) | (data_3[DATAOFFSET + 4]));
        realtimePowerData.chassisPower = wk.f;
        realtimePowerData.chassisPowerBuffer =
            ((data_3[DATAOFFSET + 9]) << 8) | (data_3[DATAOFFSET + 8]);
        realtimePowerData.shooterHeat0 =
            ((data_3[DATAOFFSET + 11]) << 8) | (data_3[DATAOFFSET + 10]);
        realtimePowerData.shooterHeat1 =
            ((data_3[DATAOFFSET + 13]) << 8) | (data_3[DATAOFFSET + 12]);
      }

      uint32_t dataNumberCRC16 = (DATAOFFSET + dataLength);
      long crc16Value = (data_3[(dataNumberCRC16 + 1) % 60] << 8) |
                        (data_3[(dataNumberCRC16 + 0) % 60]);
    }
  }
}

// customUI.
#include "ui.h"
#include "ui_default_Ungroup_0.h"
#include "ui_interface.h"
#include "ui_types.h"

#define max_x 1920
#define max_y 1080
typedef struct {
  uint8_t figure_name[3];
  uint32_t operate_tpye : 3;
  uint32_t figure_tpye : 3;
  uint32_t layer : 4;
  uint32_t color : 4;
  uint32_t details_a : 9;
  uint32_t details_b : 9;
  uint32_t width : 10;
  uint32_t start_x : 11;
  uint32_t start_y : 11;
  uint32_t details_c : 10;
  uint32_t details_d : 11;
  uint32_t details_e : 11;
} ui_figure;

typedef struct {
  uint8_t figure_name[3];
  uint32_t operate_tpye : 3;
  uint32_t figure_tpye : 3;
  uint32_t layer : 4;
  uint32_t color : 4;
  uint32_t width : 10;
  uint32_t start_x : 11;
  uint32_t start_y : 11;
  uint32_t details_c : 10;
} ui_circle;

typedef struct {
  uint8_t figure_name[3];
  uint32_t operate_tpye : 3;
  uint32_t figure_tpye : 3;
  uint32_t layer : 4;
  uint32_t color : 4;
  uint32_t font_size : 9;
  uint32_t details_a : 9;
  uint32_t width : 10;
  uint32_t start_x : 11;
  uint32_t start_y : 11;
  int32_t number;
} ui_number;

typedef struct {
  uint8_t figure_name[3];
  uint32_t operate_tpye : 3;
  uint32_t figure_tpye : 3;
  uint32_t layer : 4;
  uint32_t color : 4;
  uint32_t font_size : 9;
  uint32_t str_length : 9;
  uint32_t width : 10;
  uint32_t start_x : 11;
  uint32_t start_y : 11;
  uint32_t details_c : 10;
  uint32_t details_d : 11;
  uint32_t details_e : 11;
  char string[30];
} ui_string;

ui_figure myCircle;
uint8_t send_message_data;
void setup() {
  Serial.begin(115200);
  Serial3.begin(115200, SERIAL_8N1);

  myCircle.figure_name[0] = 0;
  myCircle.figure_name[1] = 1;
  myCircle.figure_name[2] = 2;
  myCircle.operate_tpye = 1;
  myCircle.figure_tpye = 3;
  myCircle.layer = 0;
  myCircle.color = 1;
  myCircle.details_a = 0;
  myCircle.details_b = 0;
  myCircle.width = 10;
  myCircle.start_x = max_x / 2;
  myCircle.start_y = max_y / 2;
  myCircle.details_c = 30;
  myCircle.details_d = 0;
  myCircle.details_e = 0;

  _ui_init_default_Ungroup_0();
  
}

static unsigned long testch[6];

void loop() {
  getJudgeSystemData();
  /*
  Serial.print("chassisVolt:");
  Serial.print(realtimePowerData.chassisVolt / 1000);
  Serial.println(" V ,");
  Serial.print("chassisPower:");
  Serial.print(realtimePowerData.chassisPower);
  Serial.println(" W ,");
  Serial.print("chassisPowerBuffer:");
  Serial.print(realtimePowerData.chassisPowerBuffer);
  Serial.println("");
  Serial.print("shooterHeat0:");
  Serial.print(realtimePowerData.shooterHeat0);
  Serial.println("");
  Serial.print("shooterHeat1:");
  Serial.print(realtimePowerData.shooterHeat1);
  Serial.println("");
  Serial.println("");
  delay(50);
  */

  _ui_update_default_Ungroup_0();
  
  delay(1000);
}
