# Bicycle Helmet 3D Viewer

브라우저에서 자전거 헬멧 3D 모델을 확인하는 작은 웹 프로젝트입니다. 헬멧을 마우스로 회전하고 확대·축소할 수 있으며, 자동 회전도 사용할 수 있습니다.

이 프로젝트는 별도의 빌드 과정 없이 **Live Server** 또는 간단한 **Python 웹 서버**로 실행할 수 있습니다.

## 완성된 화면에서 할 수 있는 일

- 마우스를 드래그해서 헬멧을 여러 방향으로 회전
- 마우스 휠로 확대·축소
- 터치 기기에서 손가락으로 회전 및 확대·축소
- 자동 회전 켜기·끄기
- `Reset view` 버튼으로 기본 시점 복원
- 흰색 전체 화면에서 반응형으로 확인

## 프로젝트 위치

```text
C:\manim_community\bicycle-helmet\helmet_web
```

## 프로젝트 구조

```text
helmet_web/
├── index.html     # 웹 페이지 구조와 model-viewer 설정
├── style.css      # 전체 화면, 색상, 반응형 디자인
├── script.js      # 로딩 표시, 자동 회전, 초기화 버튼 동작
├── helmet.glb     # 브라우저에서 표시할 3D 헬멧 모델
└── README.md      # 실행 방법과 제작 과정
```

원본 OBJ 파일은 다음 위치에 있습니다.

```text
C:\manim_community\bicycle-helmet\helmet.obj
```

브라우저의 `<model-viewer>`는 OBJ보다 웹에 적합한 glTF Binary 형식인 GLB를 사용하므로, 웹 프로젝트에는 `helmet.glb`를 넣었습니다.

## 1. 가장 쉬운 실행 방법: VS Code Live Server

### Live Server 설치

1. VS Code를 실행합니다.
2. `Ctrl + Shift + X`를 눌러 확장 기능 화면을 엽니다.
3. `Live Server`를 검색합니다.
4. 게시자가 **Ritwick Dey**인 확장 기능을 설치합니다.
5. VS Code를 다시 시작합니다.

### 웹 페이지 열기

1. VS Code에서 다음 폴더를 엽니다.

   ```text
   C:\manim_community\bicycle-helmet\helmet_web
   ```

2. 왼쪽 파일 목록에서 `index.html`을 마우스 오른쪽 버튼으로 클릭합니다.
3. **Open with Live Server**를 선택합니다.
4. 브라우저에서 다음과 같은 주소가 열리면 정상입니다.

   ```text
   http://127.0.0.1:5500/
   ```

확장 기능을 설치했는데 메뉴가 보이지 않으면, 파일이 아니라 `helmet_web` 폴더 전체를 VS Code로 연 뒤 다시 시도하세요.

## 2. Live Server가 없을 때: Python으로 실행

Windows PowerShell을 열고 다음 명령을 순서대로 실행합니다.

```powershell
cd "C:\manim_community\bicycle-helmet\helmet_web"
python -m http.server 8000
```

Python 명령을 찾을 수 없다고 나오면 설치된 Python 경로를 직접 사용합니다.

```powershell
cd "C:\manim_community\bicycle-helmet\helmet_web"
& "C:\Python310\python.exe" -m http.server 8000
```

PowerShell 창을 닫지 않은 상태에서 브라우저로 다음 주소에 접속합니다.

```text
http://localhost:8000/index.html
```

서버를 종료하려면 실행 중인 PowerShell 창에서 `Ctrl + C`를 누릅니다.

> `index.html`을 파일 탐색기에서 바로 더블클릭하는 `file://` 방식은 GLB 파일이나 외부 라이브러리 로딩이 브라우저 보안 정책으로 차단될 수 있습니다. 반드시 웹 서버를 통해 실행하는 것을 권장합니다.

## 3. 화면 조작 방법

| 조작 | 동작 |
| --- | --- |
| 마우스 왼쪽 드래그 | 모델 회전 |
| 마우스 휠 | 확대·축소 |
| 터치 드래그 | 모델 회전 |
| 두 손가락 벌리기·모으기 | 확대·축소 |
| `Reset view` | 기본 카메라 위치로 복원 |
| `Auto-rotate on/off` | 자동 회전 켜기·끄기 |

## 4. 제작 과정

### 4.1 3D 원본 준비

1. `helmet.obj` 파일을 준비합니다.
2. OBJ를 Blender에서 불러옵니다.
3. 헬멧의 윗부분이 위를 향하도록 방향을 확인합니다.
4. Blender에서 `File > Export > glTF 2.0`을 선택합니다.
5. 파일 형식을 **GLB**로 지정하고 다음 위치에 저장합니다.

   ```text
   C:\manim_community\bicycle-helmet\helmet_web\helmet.glb
   ```

모델을 교체할 때도 파일 이름을 `helmet.glb`로 유지하면 HTML을 수정할 필요가 없습니다.

### 4.2 HTML 작성

`index.html`에서 Google Hosted Libraries의 `model-viewer`를 불러옵니다.

```html
<script type="module"
  src="https://ajax.googleapis.com/ajax/libs/model-viewer/4.3.1/model-viewer.min.js">
</script>
```

그 다음 GLB 파일을 지정합니다.

```html
<model-viewer
  src="helmet.glb"
  alt="3D bicycle helmet"
  camera-controls
  auto-rotate>
</model-viewer>
```

여기서 `src="helmet.glb"`는 `index.html`과 같은 폴더에 있는 파일을 의미합니다.

### 4.3 CSS 작성

`style.css`에서 뷰어가 브라우저 화면 전체를 사용하도록 설정했습니다.

```css
model-viewer {
  width: 100vw;
  height: 100vh;
  background-color: #fff;
}
```

추가로 화면 중앙 배치, 흰색 배경, 작은 화면 대응, 상태 표시와 버튼 디자인을 적용했습니다.

### 4.4 JavaScript 작성

`script.js`는 다음 기능을 담당합니다.

- GLB 로딩 진행률 표시
- 모델 로딩 완료·실패 상태 표시
- 자동 회전 켜기·끄기
- 카메라 초기화
- 모델 로딩 오류 안내

## 5. GitHub Pages에서 공유하기

1. GitHub 저장소에 `helmet_web` 폴더 전체를 업로드합니다.
2. GitHub 저장소의 `Settings > Pages`로 이동합니다.
3. 배포할 브랜치와 폴더를 선택합니다.
4. 저장 후 Pages 주소가 생성될 때까지 잠시 기다립니다.

저장소 이름이 `aiproject`이고 사용자 이름이 `kyonhyon1713-cpu`라면, 저장소 루트에서 배포할 경우 다음 주소 형식으로 접근할 수 있습니다.

```text
https://kyonhyon1713-cpu.github.io/aiproject/helmet_web/
```

`helmet.glb`를 포함한 모든 파일의 대·소문자와 상대 경로를 정확히 유지해야 합니다. 이 프로젝트는 `src="helmet.glb"`와 `href="style.css"`처럼 상대 경로를 사용하므로 로컬 서버와 GitHub Pages에서 같은 구조로 동작합니다.

## 6. 문제가 생겼을 때

### 화면은 나오지만 모델이 보이지 않는 경우

- `helmet.glb`가 `index.html`과 같은 `helmet_web` 폴더에 있는지 확인합니다.
- 주소창에서 다음 경로가 열리는지 확인합니다.

  ```text
  http://localhost:8000/helmet.glb
  ```

- PowerShell 서버를 `helmet_web` 폴더에서 실행했는지 확인합니다.

### `helmet.glb` 404 오류가 나는 경우

HTML의 파일 이름과 실제 파일 이름이 정확히 같은지 확인합니다.

```text
helmet.glb       정상
Helmet.glb       다른 파일 이름
helmet.GLB       다른 파일 이름
```

GitHub Pages는 파일 이름의 대·소문자를 구분할 수 있습니다.

### 자동 회전이 보이지 않는 경우

마우스로 모델을 조작하는 동안에는 자동 회전이 잠시 멈출 수 있습니다. 화면 아래의 자동 회전 버튼을 눌러 상태를 확인하고, 마우스를 모델에서 이동한 뒤 잠시 기다립니다.

### 인터넷 연결이 없는 경우

`model-viewer` 라이브러리를 Google CDN에서 불러오므로 처음 실행할 때 인터넷 연결이 필요합니다.

## 7. 최종 확인 목록

- [ ] `index.html`, `style.css`, `script.js`, `helmet.glb`가 같은 폴더에 있습니다.
- [ ] `index.html`을 웹 서버를 통해 실행했습니다.
- [ ] 모델이 흰색 화면 중앙에 표시됩니다.
- [ ] 마우스 드래그로 모델을 회전할 수 있습니다.
- [ ] 휠로 확대·축소할 수 있습니다.
- [ ] 자동 회전 버튼이 동작합니다.
- [ ] GitHub Pages 주소에서 `helmet.glb`가 정상적으로 로딩됩니다.

## 참고 자료

- [Google model-viewer 공식 문서](https://modelviewer.dev/docs/index.html)
- [model-viewer 카메라·자동 회전 예제](https://modelviewer.dev/examples/stagingandcameras/)
- [model-viewer 공식 FAQ](https://modelviewer.dev/docs/faq.html)
