# 2026-08-04 실습 정리

이 폴더는 2026년 8월 4일에 진행한 ESP32-S3 카메라·센서 실습만 별도로 보관합니다. 이전 BLE 프로젝트 파일과 합치지 않았습니다.

## 포함 내용

| 경로 | 내용 |
| --- | --- |
| `docs/today-2026-08-04.md` | 이미지 추론, 센서, BH1750 실습 기록 |
| `camera/xiao_webcam_ap_infer.ino` | XIAO ESP32-S3 Sense AP 카메라 추론 스케치 |
| `camera/training_report.json` | Edge Impulse 이미지 분류 학습·테스트 결과 |
| `camera/model/battery-fan-classifier-arduino.zip` | ESP32-S3용 Edge Impulse Arduino 배포 라이브러리 |
| `sensors/` | BH1750, BME280, BNO055 테스트 코드와 BH1750 AP 수집기 |
| `sensors/bh1750_dataset/bh1750_data/` | BH1750 CSV 샘플 20개 |

## 카메라 모델 실습

배터리 154장과 선풍기 150장을 수집해 96×96 이미지 분류 모델을 학습했습니다. 모델 테스트 결과는 정수 양자화 모델 정확도 약 79.4%, float32 모델 정확도 100%였습니다. 배포 ZIP은 `camera/model/`에 보관했습니다.

카메라 스케치를 사용하기 전에 공개 저장소에 비밀번호가 노출되지 않도록 AP 비밀번호를 로컬에서 설정하세요.

```cpp
WiFi.softAP("bkh", "YOUR_AP_PASSWORD", 1);
```

실습 당시 사용한 비밀번호는 이 저장소에 기록하지 않았습니다. 스마트폰을 AP에 연결한 뒤 `http://192.168.4.1`로 접속합니다.

## 센서 실습

XIAO ESP32-S3의 I2C 핀은 SDA=`D4/GPIO5`, SCL=`D5/GPIO6`입니다.

| 센서 | 역할 | 실습 주소 |
| --- | --- | --- |
| BH1750 / GY-302 | 조도(lux) 측정 | `0x23` |
| BNO055 | 자세·방향 및 가속도/자이로/자기장 측정 | `0x29` |
| BME280 | 온도·기압·습도 측정 | `0x76` |

BH1750 AP 수집기는 1초마다 값을 읽어 브라우저에 표시하고, 샘플별 30개 값을 CSV/ZIP으로 저장합니다. `sensors/README.md`와 각 `examples/` 스케치에 연결 및 실행 방법을 정리했습니다.

## 제외한 파일

재현에 필요하지 않은 빌드 디렉터리, 캐시, 설치된 Skill 복사본, Edge Impulse 압축 해제 중복 폴더, 촬영 원본 사진·사진 ZIP, `.env`와 API 키는 커밋에서 제외했습니다. AP 비밀번호도 공개 저장소에 넣지 않았습니다.

원본 작업 디렉터리의 파일은 삭제하거나 이동하지 않고, 필요한 파일만 이 폴더에 복사했습니다.
