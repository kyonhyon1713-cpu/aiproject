# Bicycle Helmet Rotation Video

`helmet.obj`를 Blender로 불러와 Z축을 기준으로 360도 회전시키고, 1920×1080 H.264 MP4 영상으로 만드는 프로젝트입니다.

완성된 영상은 다음 파일입니다.

```text
helmet_rotation.mp4
```

영상에는 자막, 제목, 설명문, 텍스트 오버레이를 넣지 않았습니다. 헬멧 모델과 조명·카메라만 표시됩니다.

## 프로젝트 구조

```text
bicycle-helmet/
├── helmet.obj                         # Blender로 불러오는 원본 모델
├── helmet.mtl                         # OBJ 재질 정보
├── make_helmet_rotation_blender.py    # Blender 자동 생성 스크립트
├── helmet_rotation.mp4                # 완성된 360도 회전 영상
├── helmet_stl_rotation.mp4            # 기존 STL 기반 영상
└── helmet_web/                        # 브라우저용 GLB 3D 뷰어
    ├── helmet.glb
    ├── index.html
    ├── style.css
    ├── script.js
    └── README.md
```

## 결과 영상 설정

| 항목 | 설정 |
| --- | --- |
| 원본 | `helmet.obj` |
| 프레임 시작 | 1 |
| 프레임 종료 | 300 |
| FPS | 30 |
| 재생 시간 | 10초 |
| 회전축 | Z축 |
| 회전 각도 | 0° → 360° |
| 키프레임 보간 | Linear |
| 해상도 | 1920×1080 |
| 영상 코덱 | H.264 |
| 픽셀 형식 | YUV 4:2:0 (`yuv420p`) |
| 오디오 | 없음 |

## 제작 과정

### 1. OBJ 파일 준비

`helmet.obj`와 같은 폴더에 `helmet.mtl`을 두어 OBJ의 재질 정보를 함께 읽을 수 있도록 합니다.

### 2. Blender에서 모델 가져오기

스크립트가 OBJ를 자동으로 import합니다. OBJ가 여러 mesh 파트로 구성되어 있으면 하나의 `Helmet` 오브젝트로 합칩니다.

### 3. Origin과 변환 적용

다음 Blender 작업을 자동으로 실행합니다.

- `Object > Set Origin > Origin to Geometry`
- `Object > Apply > Rotation and Scale` 또는 `Ctrl + A`
- 모델을 장면 중앙에 배치
- 필요한 경우 Y-up OBJ를 Z-up 방향으로 정렬
- 화면에 맞도록 크기를 정규화

### 4. 카메라와 조명 설정

카메라는 모델 정면인 음의 Y 방향에 배치하고 모델 중심을 바라보도록 설정합니다. 부드러운 제품 촬영 느낌을 위해 Area Light를 세 개 사용합니다.

- `Area Key`: 주 조명
- `Area Fill`: 그림자 완화용 보조 조명
- `Area Rim`: 뒤쪽 윤곽 강조용 조명

바닥면은 그림자 표현을 위해 사용되며, 텍스트나 자막은 추가하지 않습니다.

### 5. 회전 애니메이션

```python
Frame 1   : Z 회전 0도
Frame 300 : Z 회전 360도
```

두 키프레임의 보간을 `LINEAR`로 설정하여 일정한 속도로 회전하도록 했습니다.

### 6. MP4 생성

Blender에서 300장의 임시 PNG 프레임을 렌더링한 뒤 H.264 인코더로 MP4를 생성합니다. 인코딩이 끝나면 임시 PNG 프레임은 자동으로 정리됩니다.

현재 컴퓨터의 Blender 5.2 실행 파일에는 직접 사용할 FFmpeg 실행 파일이 없기 때문에, 프로젝트의 Python 가상환경에 설치된 `imageio-ffmpeg` 인코더를 사용합니다.

## 실행 방법

### 필요한 프로그램

- Blender 5.2 또는 호환 버전
- Python 가상환경의 `imageio-ffmpeg`

현재 작업 환경의 실행 파일 위치는 다음과 같습니다.

```text
C:\Program Files\Blender Foundation\Blender 5.2\blender.exe
C:\manim_community\.venv\Scripts\python.exe
```

### FFmpeg 인코더 설치

이미 설치되어 있다면 건너뜁니다.

```powershell
& "C:\manim_community\.venv\Scripts\python.exe" -m pip install imageio-ffmpeg
```

### Blender 자동 실행

PowerShell에서 다음 명령을 실행합니다.

```powershell
cd "C:\manim_community\bicycle-helmet"
& "C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" `
  --factory-startup `
  --background `
  --python ".\make_helmet_rotation_blender.py"
```

실행이 끝나면 다음 파일이 생성됩니다.

```text
C:\manim_community\bicycle-helmet\helmet_rotation.mp4
```

## 결과 확인

영상 파일의 기본 정보는 다음 명령으로 확인할 수 있습니다.

```powershell
Get-Item "C:\manim_community\bicycle-helmet\helmet_rotation.mp4"
```

브라우저나 동영상 재생 프로그램으로 `helmet_rotation.mp4`를 열어 헬멧이 일정한 속도로 한 바퀴 회전하는지 확인합니다.

## GitHub CLI로 업로드하기

이 프로젝트는 `kyonhyon1713-cpu/aiproject` 저장소의 별도 폴더에 올릴 수 있습니다.

### 1. GitHub 로그인 확인

```powershell
gh auth status
```

로그인이 되어 있지 않다면 다음 명령으로 로그인합니다.

```powershell
gh auth login
```

### 2. 저장소에 파일 복사

저장소의 로컬 복사본이 다음 위치에 있다고 가정합니다.

```text
C:\manim_community\aiproject
```

다음 파일을 저장소의 `bicycle-helmet` 폴더에 넣습니다.

```text
bicycle-helmet/
├── README.md
├── helmet_rotation.mp4
└── make_helmet_rotation_blender.py
```

기존 OBJ, STL, Creo 파일과 `helmet_web` 폴더를 함께 공유하려면 해당 파일들도 유지합니다.

### 3. 커밋하고 push

```powershell
cd "C:\manim_community\aiproject"
gh auth setup-git
git add -- "bicycle-helmet/README.md" `
  "bicycle-helmet/helmet_rotation.mp4" `
  "bicycle-helmet/make_helmet_rotation_blender.py"
git commit -m "Add Blender helmet rotation video guide"
git push origin main
```

### 4. GitHub에서 확인

업로드 후 다음 주소에서 파일을 확인할 수 있습니다.

- [bicycle-helmet 폴더](https://github.com/kyonhyon1713-cpu/aiproject/tree/main/bicycle-helmet)
- [회전 영상](https://github.com/kyonhyon1713-cpu/aiproject/blob/main/bicycle-helmet/helmet_rotation.mp4)
- [Blender 생성 스크립트](https://github.com/kyonhyon1713-cpu/aiproject/blob/main/bicycle-helmet/make_helmet_rotation_blender.py)

## 문제 해결

### `helmet.obj`를 찾지 못하는 경우

스크립트와 OBJ가 같은 폴더에 있는지 확인합니다.

```text
C:\manim_community\bicycle-helmet\helmet.obj
C:\manim_community\bicycle-helmet\make_helmet_rotation_blender.py
```

### FFmpeg를 찾지 못하는 경우

가상환경에 `imageio-ffmpeg`를 설치한 뒤 다시 실행합니다.

```powershell
& "C:\manim_community\.venv\Scripts\python.exe" -m pip install --upgrade imageio-ffmpeg
```

스크립트는 먼저 시스템 PATH의 `ffmpeg`를 찾고, 찾지 못하면 `C:\manim_community\.venv\Lib\site-packages\imageio_ffmpeg\binaries`를 자동으로 검색합니다.

### 영상에 글자가 보이는 경우

스크립트에는 Text 오브젝트, 자막, 제목, 화면 UI를 생성하는 코드가 없습니다. 모델에 포함된 형상처럼 보이는 부분이라면 OBJ 원본 geometry일 수 있으므로 Blender에서 원본 모델을 확인해야 합니다.

## 관련 문서

- [브라우저용 GLB 뷰어 안내](./helmet_web/README.md)
- [Google model-viewer 공식 문서](https://modelviewer.dev/docs/index.html)
