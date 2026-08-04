# sensorTest

XIAO ESP32-S3에 연결한 I2C 센서를 테스트하는 폴더입니다.

현재 `sensorTest.ino`는 다음을 수행합니다.

- XIAO ESP32-S3의 I2C 버스 스캔
- BH1750 주소 자동 확인: `0x23` 또는 `0x5C`
- BNO055 주소 후보 표시: `0x28` 또는 `0x29`
- BME280 주소 후보 표시: `0x76`
- 1초마다 조도 값을 lux로 출력
- 같은 I2C 버스에 연결된 다른 센서 주소 표시

## BH1750 연결

| BH1750 | XIAO ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | D4 / GPIO5 |
| SCL | D5 / GPIO6 |
| ADDR | GND → 현재 모듈 주소 `0x23` |

센서 모듈의 전원과 I2C 신호는 3.3V 기준으로 연결하세요. 여러 센서를 한 버스에 연결할 때는 I2C 주소가 겹치지 않아야 합니다.

## 컴파일 및 업로드

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 C:\arduinoTest\sensorTest
arduino-cli upload -p COM11 --fqbn esp32:esp32:XIAO_ESP32S3 C:\arduinoTest\sensorTest
```

업로드 후 115200 baud로 시리얼을 열면 주소 스캔 결과와 조도 값이 출력됩니다.

## BH1750 / GY-302 통합 예제

BH1750 칩과 GY-302 모듈은 같은 I2C 센서이므로 하나의 예제로 통합했습니다.
예제 파일은 `examples/BH1750/BH1750.ino`입니다. 외부 라이브러리 없이 주소를 자동 확인하고 1초마다 lux 값을 출력합니다.

GY-302 모듈의 핀 표기를 확인하여 다음처럼 연결합니다.

| GY-302 | XIAO ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | D5 / GPIO6 |
| SDA | D4 / GPIO5 |
| ADDR | GND=`0x23`, 3V3=`0x5C` |

모듈마다 실제 핀 순서가 다를 수 있으므로 기판에 인쇄된 `VCC/GND/SCL/SDA/ADDR` 이름을 기준으로 연결하세요.
현재 사용 중인 모듈은 ADDR이 GND에 연결되어 주소 `0x23`으로 설정되어 있습니다. 따라서 통합 예제도 `0x23`만 확인합니다.
`BH1750.ino`의 `RUN_FULL_I2C_SCAN`을 `true`로 바꾸면 같은 버스에 연결된 다른 센서의 주소도 출력할 수 있습니다.

기존의 별도 `GY302_BH1750` 예제는 이 통합 예제로 대체했습니다.

## BH1750 AP 브라우저 수집기

브라우저에서 실시간 lux를 보고 Edge Impulse용 ZIP을 만들려면 `examples/BH1750_AP_logger/BH1750_AP_logger.ino`를 사용합니다.

- AP: `bkh`
- 비밀번호: `11112222`
- 주소: `http://192.168.4.1`
- 측정 주기: 1초
- 기본 샘플: 30개 값 = 30초
- 권장 수량: 라벨 1개당 100개 샘플 = 3,000개 값
- 초기 확인용 최소 수량: 라벨 1개당 20개 샘플 = 600개 값

스마트폰을 `bkh`에 연결한 후 페이지에서 라벨을 입력하고 `수집 시작`을 누릅니다. 라벨을 바꿀 때는 먼저 수집을 중지하고 새 라벨로 다시 시작합니다. 완성된 샘플은 `ZIP 다운로드` 버튼으로 브라우저에서 저장합니다. ZIP 안에는 `label.0001.csv` 형식의 샘플별 CSV와 `README.txt`가 들어갑니다.

CSV는 `timestamp,lux` 형식이며 1초 간격의 30개 값을 포함합니다. 브라우저에서 받은 ZIP은 `C:\arduinoTest\sensorTest\bh1750_dataset`에 저장해 두었다가, 압축을 풀고 Edge Impulse Data acquisition의 CSV 업로드/CSV Wizard에서 가져오면 됩니다.

## BNO055 8핀 전용 예제

BNO055 전용 예제는 `examples/BNO055_0x29/BNO055_0x29.ino`입니다. 외부 라이브러리 없이 BNO055의 CHIP_ID를 확인하고, 1초마다 다음 값을 출력합니다.

- Euler heading / roll / pitch
- 3축 가속도
- 3축 자이로
- 온도
- 시스템/자이로/가속도/자기 센서 보정 상태

| BNO055 모듈 표기 | XIAO ESP32-S3 |
|---|---|
| VCC 또는 VIN | 3V3 (모듈 표기 우선) |
| GND | GND |
| SDA | D4 / GPIO5 |
| SCL | D5 / GPIO6 |
| ADR/COM3 | `0x29`가 되도록 HIGH인 모듈 설정 |

8핀 모듈의 실제 핀 순서는 제품마다 다를 수 있으므로 기판의 표기를 따라 연결하세요. PS0/PS1은 I2C 모드 설정이어야 하며, RST/INT는 이 예제에서 사용하지 않습니다. BNO055의 주소는 모듈 설정에 따라 `0x28` 또는 `0x29`가 될 수 있고, 예제는 우선 `0x29`를 확인한 뒤 `0x28`도 진단용으로 확인합니다.

## BME280 4핀 전용 예제

BME280 전용 예제는 `examples/BME280_0x76/BME280_0x76.ino`입니다. 주소 `0x76`과 `CHIP_ID=0x60`을 확인한 뒤 온도, 기압, 습도를 1초마다 출력합니다. Bosch 보정 데이터를 직접 읽어 계산하므로 외부 BME280 라이브러리가 필요하지 않습니다.

| BME280 | XIAO ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | D4 / GPIO5 |
| SCL | D5 / GPIO6 |

BME280 모듈에서 SDO/ADR을 사용하는 경우 LOW가 일반적으로 `0x76`, HIGH가 `0x77`입니다. 현재 센서는 `0x76`으로 설정되어 있으므로 예제는 `0x76`만 확인합니다.
