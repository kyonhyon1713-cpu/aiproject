# ESP32-S3 BLE 양방향 통신 실습

Seeed Studio XIAO ESP32-S3와 nRF Connect를 이용해 터치 센서 값을 스마트폰으로
전송하고, 스마트폰에서 내장 LED를 제어하는 실습 기록이다.

## 오늘 배운 내용

### ESP32-S3 BLE 구조

- ESP32-S3는 Bluetooth Classic이 아니라 BLE를 사용한다.
- ESP32-S3는 BLE 서버로 동작하고, 스마트폰의 nRF Connect는 BLE 클라이언트로
  동작한다.
- GATT는 서비스(Service)와 특성(Characteristic)으로 구성된다.
- 센서처럼 보드에서 계속 보내는 값은 `Notify`를 사용한다.
- 스마트폰에서 보드로 명령을 보내려면 `Write`와 `onWrite` 콜백을 사용한다.
- Notify를 받으려면 nRF Connect에서 해당 특성의 Notify를 먼저 활성화해야 한다.
- 연결이 끊어지면 `onDisconnect()`에서 다시 광고를 시작하면 재연결할 수 있다.

### 현재 BLE 예제의 하드웨어 매핑

- 보드: `esp32:esp32:XIAO_ESP32S3`
- 장치 이름: `bkh-touch-LED`
- 터치 센서: D0 / GPIO1 / TOUCH1
- 내장 LED: GPIO21, active-low (`LOW`가 켜짐)
- 터치 Notify 주기: 100ms

## BLE 프로토콜

| 항목 | UUID | 역할 |
| --- | --- | --- |
| Service | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` | BLE 서비스 |
| Touch characteristic | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | `touchRead()` 값을 십진수 문자열로 Notify |
| LED characteristic | `9c1f0002-7f2a-4a2d-9b8e-2e6c1d5a0002` | LED 상태 Write/Notify |

LED 특성에 한 바이트를 Write한다.

- `0x01` 또는 ASCII `1`: LED ON
- `0x00` 또는 ASCII `0`: LED OFF
- 그 외 값: 무시

## nRF Connect 사용 순서

1. Scanner에서 `bkh-touch-LED`를 검색하고 연결한다.
2. 서비스 UUID를 펼친다.
3. Touch characteristic에서 Notify를 활성화한다.
4. D0에 손가락을 대며 터치 값이 변하는지 확인한다.
5. LED characteristic의 Write에서 HEX `01`을 보내 LED를 켠다.
6. HEX `00`을 보내 LED를 끈다.
7. 연결을 끊고 다시 검색해 자동 재광고되는지 확인한다.

## 실제 하드웨어 테스트 결과

ESP32-S3에 예제를 실제 업로드하고 nRF Connect로 테스트했다.

- 터치 센서 값 Notify 정상
- `01` 수신 시 LED ON 정상
- `00` 수신 시 LED OFF 정상
- 연결 해제 후 재광고 정상

따라서 위 네 가지 항목은 단순 컴파일 검사가 아니라 실제 보드와 스마트폰으로
검증된 동작이다.

## Arduino CLI

예제 위치:

`.agents/skills/esp32-s3-ble-bidirectional/examples/esp32_s3_ble_bidirectional`

컴파일:

```powershell
$sketch = (Resolve-Path ".agents\skills\esp32-s3-ble-bidirectional\examples\esp32_s3_ble_bidirectional").Path
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 $sketch
```

COM9로 업로드:

```powershell
arduino-cli upload -p COM9 --fqbn esp32:esp32:XIAO_ESP32S3 $sketch
```

업로드 전 `arduino-cli board list`에서 실제 포트가 COM9인지 확인한다.

## Skill 파일

- [BLE 양방향 통신 Skill](.agents/skills/esp32-s3-ble-bidirectional/SKILL.md)
- [컴파일 가능한 Arduino 예제](.agents/skills/esp32-s3-ble-bidirectional/examples/esp32_s3_ble_bidirectional/esp32_s3_ble_bidirectional.ino)
- [BLE 설명서](BLE.md)

## 이전 실습과의 관계

- AP 모드는 노트북에서 테스트하면 인터넷 연결이 끊길 수 있으므로 스마트폰으로
  직접 로컬 접속해 검증하는 방식으로 정리했다.
- ProjectBee MQTT 연결은 별도 실습이며, 현재 BLE 예제는 Wi-Fi/AP/MQTT 없이
  독립적으로 동작한다.
- 최종 BLE 예제에는 deep sleep을 사용하지 않는다. 터치 값 Notify와 BLE 연결을
  계속 유지해야 하기 때문이다.

## 보안 메모

Wi-Fi 비밀번호나 MQTT 인증 정보는 공개 저장소의 README와 소스 코드에 기록하지
않는다. 필요한 값은 로컬 설정이나 별도 비공개 파일로 관리한다.
