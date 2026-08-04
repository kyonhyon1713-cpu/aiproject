# 2026-08-04 오늘 배운 내용

ESP32-S3에서 센서 데이터를 읽고, 브라우저에서 데이터셋으로 저장한 뒤,
카메라 이미지 분류 모델로 연결하는 실습을 한 날짜별 기록이다.

## 핵심 흐름

1. **조도 센서 읽기**: XIAO ESP32-S3와 BH1750을 I²C로 연결한다. SDA는 D4/GPIO5,
   SCL은 D5/GPIO6, 센서 주소는 `0x23`이다.
2. **브라우저 데이터 수집**: `timestamp,lux` 형식으로 약 1초 간격의 조도 값을 기록하고,
   CSV를 ZIP 데이터셋으로 저장한다.
3. **카메라 이미지 분류**: `battery` 154장과 `fan` 150장을 사용해 두 클래스를 학습한다.
   입력 크기는 96×96이고 transfer learning 흐름을 사용했다.
4. **추론 결과 확인**: 학습 리포트의 float32와 int8 결과를 비교해 실제 배포 시
   정확도와 메모리의 균형을 확인한다.

## 폴더 안내

| 폴더 | 내용 |
| --- | --- |
| `01-manim-video` | 오늘 배운 내용을 시각화한 Manim 소스, MP4, 실행 문서 |
| `02-sensorTest` | BH1750 조도 센서 스케치, 예제, CSV 샘플 |
| `03-camera-ai` | 카메라 분류 학습 리포트와 Arduino 모델 라이브러리 ZIP |
| `04-image-dataset` | battery/fan 원본 이미지 데이터셋 |
| `05-webppt` | 수업 WebPPT, 화면 이미지, 원본 실습 영상 |
| `06-existing-repo-notes` | 기존 저장소에 먼저 올렸던 ESP32/BLE 문서와 스케치 |

## 영상 보기

- 이 폴더의 [Manim 영상 페이지](01-manim-video/)
- 저장소 루트의 [브라우저 영상 페이지](../index.html)
- [Netlify 공개 페이지](https://kyonhyon1713-aiproject.netlify.app/)

## Manim 영상 다시 만들기

`01-manim-video`에서 실행한다.

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
.\render_today_video.ps1
```

스크립트는 이제 이 날짜 폴더 안의 CSV, 이미지, 학습 리포트, WebPPT 화면을 사용하므로
원래의 `C:\arduinoTest` 경로가 없어도 자료 폴더를 복사한 상태에서 재현할 수 있다.

## 추가 실습 기록

원격에서 먼저 정리된 자료도 보존했다.

- `docs/today-2026-08-04.md`: 이미지 추론·센서·BH1750 실습 상세 기록
- `03-camera-ai/xiao_webcam_ap_infer.ino`: XIAO ESP32-S3 Sense AP 카메라 추론 스케치
- `02-sensorTest/`: 비밀번호를 비워 둔 공개용 센서 예제와 CSV 샘플

카메라·센서 스케치의 AP 비밀번호는 공개 저장소에 넣지 않고
`YOUR_AP_PASSWORD` 자리표시자로 관리한다. 실제 보드에 업로드하기 전에 로컬 값으로 바꾼다.
