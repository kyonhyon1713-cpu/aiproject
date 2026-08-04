# 오늘 배운 내용 영상

`make_today_video.py`는 다음 실습 흐름을 짧게 요약합니다.

- XIAO ESP32-S3와 BH1750/GY-302 조도 센서의 I²C 연결
- `timestamp,lux` 형식의 1초 간격 CSV 수집
- 브라우저 수집기에서 CSV를 ZIP 데이터셋으로 저장
- 카메라 사진 `battery` 154장 + `fan` 150장으로 이미지 분류
- Edge Impulse 테스트 리포트의 float32 / int8 결과

## 렌더링

프로젝트 폴더에서 실행합니다.

```powershell
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
.\render_today_video.ps1
```

기본 결과는 `media/videos/make_today_video/720p30/TodayIoTAI.mp4`입니다.
`index.html`을 브라우저에서 열면 재생 화면이 표시됩니다.

참고한 폴더:

- `C:\arduinoTest\sensorTest`
- `C:\arduinoTest\camera_dataset`
- `C:\arduinoTest\dataset`
- `C:\arduinoTest2\Webppt`
