"""오늘 배운 ESP32 센서·브라우저 데이터·TinyML 흐름을 요약하는 Manim 영상.

실행 예:
    .\\.venv\\Scripts\\manim.exe -qm make_today_video.py TodayIoTAI

영상은 제공된 실습 폴더의 실제 CSV, 이미지, 학습 리포트 값을 참고한다.
외부 Python 패키지는 Manim Community만 사용하고, 데이터 읽기는 표준 라이브러리로 처리한다.
"""

from __future__ import annotations

import csv
import json
from pathlib import Path

from manim import *


# 16:9 영상. 브라우저에서 바로 재생하기 좋은 1280×720 MP4로 렌더링한다.
config.pixel_width = 1280
config.pixel_height = 720
config.frame_rate = 30
config.background_color = "#07111F"


FONT_KO = "Malgun Gothic"
FONT_MONO = "Consolas"
BG = "#07111F"
PANEL = "#101F33"
PANEL_2 = "#162B43"
TEXT = "#F4F8FF"
MUTED = "#A8BCD4"
CYAN = "#42D9FF"
BLUE = "#2388E8"
GREEN = "#46E98B"
YELLOW = "#FFD166"
ORANGE = "#FF9F5B"
RED = "#FF6B7A"
PURPLE = "#B89CFF"


SENSOR_CSV = Path(r"C:\arduinoTest\sensorTest\bh1750_dataset\bh1750_data\indoor.0001.csv")
TRAINING_REPORT = Path(r"C:\arduinoTest\camera_dataset\training_report.json")
BATTERY_IMAGE = Path(r"C:\arduinoTest\dataset\battery\battery.1.jpg")
FAN_IMAGE = Path(r"C:\arduinoTest\dataset\fan\battery.1.jpg")
INFERENCE_SCREEN = Path(r"C:\arduinoTest2\Webppt\tlftmq1.png")


def make_text(
    value: str,
    size: float = 24,
    color: str = TEXT,
    font: str = FONT_KO,
    weight: str = "NORMAL",
) -> Text:
    """한글이 포함된 Text를 일관된 폰트와 색으로 만든다."""

    return Text(value, font=font, font_size=size, color=color, weight=weight)


def load_lux_values() -> list[float]:
    """실제 indoor.0001.csv를 읽고, 파일이 없으면 녹화에 사용한 값으로 대체한다."""

    fallback = [602.5, 614.167, 578.333, 590.0, 583.333, 586.667, 583.333]
    if not SENSOR_CSV.exists():
        return fallback
    try:
        with SENSOR_CSV.open("r", encoding="utf-8", newline="") as handle:
            values = [float(row["lux"]) for row in csv.DictReader(handle) if row.get("lux")]
        return values or fallback
    except (OSError, KeyError, ValueError):
        return fallback


def load_report() -> dict:
    """학습 리포트의 실제 수치를 읽되, 리포트가 없어도 영상이 렌더링되도록 한다."""

    fallback = {
        "labels": {"battery": 154, "fan": 150},
        "dataset": {"total": 304, "training": 241, "testing": 63},
        "impulse": {"imageWidth": 96, "imageHeight": 96},
        "training": {"cycles": 20, "finalValidationAccuracy": 1.0},
        "modelTesting": {
            "variants": {
                "float32": {"good": 63, "accuracy": 1.0},
                "int8": {"good": 50, "accuracy": 0.7936507936507937},
            }
        },
    }
    if not TRAINING_REPORT.exists():
        return fallback
    try:
        with TRAINING_REPORT.open("r", encoding="utf-8") as handle:
            return json.load(handle)
    except (OSError, json.JSONDecodeError):
        return fallback


def pill(label: str, color: str, font_size: float = 20, width: float | None = None) -> VGroup:
    """짧은 상태 라벨용 둥근 배지."""

    caption = make_text(label, font_size, TEXT, weight="BOLD")
    box_width = width or max(1.0, caption.width + 0.45)
    box = RoundedRectangle(
        width=box_width,
        height=0.46,
        corner_radius=0.12,
        fill_color=color,
        fill_opacity=1,
        stroke_width=0,
    )
    caption.move_to(box)
    return VGroup(box, caption)


def board_icon() -> VGroup:
    """XIAO ESP32-S3를 단순화한 벡터 아이콘."""

    board = RoundedRectangle(
        width=3.05,
        height=1.85,
        corner_radius=0.18,
        fill_color="#153A5B",
        fill_opacity=1,
        stroke_color=CYAN,
        stroke_width=2.5,
    )
    usb = RoundedRectangle(
        width=0.42,
        height=0.25,
        corner_radius=0.05,
        fill_color="#B7C7D8",
        fill_opacity=1,
        stroke_width=0,
    ).next_to(board, LEFT, buff=-0.04)
    title = make_text("XIAO ESP32-S3", 22, TEXT, weight="BOLD").move_to(board.get_center() + UP * 0.45)
    sub = make_text("마이크로컨트롤러", 16, MUTED).move_to(board.get_center() + DOWN * 0.1)
    led = Circle(radius=0.09, fill_color=GREEN, fill_opacity=1, stroke_width=0).move_to(
        board.get_center() + DOWN * 0.52 + RIGHT * 0.95
    )
    led_text = make_text("LED", 13, GREEN, font=FONT_MONO).next_to(led, LEFT, buff=0.08)

    pin_lines = VGroup()
    pin_labels = VGroup()
    for y, label in ((0.40, "D4 / GPIO5"), (-0.40, "D5 / GPIO6")):
        start = board.get_right() + UP * y
        pin_lines.add(Line(start, start + RIGHT * 0.28, stroke_color=YELLOW, stroke_width=4))
        pin_labels.add(make_text(label, 15, YELLOW, font=FONT_MONO).next_to(start + RIGHT * 0.33, RIGHT, buff=0.02))

    return VGroup(board, usb, title, sub, led, led_text, pin_lines, pin_labels)


def sensor_icon() -> VGroup:
    """BH1750/GY-302 조도 센서 아이콘."""

    module = RoundedRectangle(
        width=2.25,
        height=1.65,
        corner_radius=0.16,
        fill_color="#244435",
        fill_opacity=1,
        stroke_color=GREEN,
        stroke_width=2.5,
    )
    chip = RoundedRectangle(
        width=0.62,
        height=0.62,
        corner_radius=0.06,
        fill_color="#0D1B1B",
        fill_opacity=1,
        stroke_color=GREEN,
        stroke_width=1.5,
    ).move_to(module.get_center() + LEFT * 0.58)
    rays = VGroup(
        Line(chip.get_right() + UP * 0.22, chip.get_right() + RIGHT * 0.33 + UP * 0.22, stroke_color=YELLOW, stroke_width=2),
        Line(chip.get_right() + RIGHT * 0.06, chip.get_right() + RIGHT * 0.42, stroke_color=YELLOW, stroke_width=2),
        Line(chip.get_right() + DOWN * 0.22, chip.get_right() + RIGHT * 0.33 + DOWN * 0.22, stroke_color=YELLOW, stroke_width=2),
    )
    title = make_text("BH1750", 24, TEXT, font=FONT_MONO, weight="BOLD").move_to(module.get_center() + RIGHT * 0.45 + UP * 0.36)
    sub = make_text("조도 센서", 18, MUTED).move_to(module.get_center() + RIGHT * 0.45 + DOWN * 0.05)
    addr = make_text("I²C 0x23", 17, CYAN, font=FONT_MONO).move_to(module.get_center() + RIGHT * 0.45 + DOWN * 0.43)
    return VGroup(module, chip, rays, title, sub, addr)


def browser_collector() -> VGroup:
    """Webppt의 조도수집기 화면을 참고해 만든 벡터 브라우저 UI."""

    window = RoundedRectangle(
        width=11.75,
        height=5.55,
        corner_radius=0.18,
        fill_color="#F7FAFE",
        fill_opacity=1,
        stroke_color="#D7E2F0",
        stroke_width=2,
    )
    bar = Rectangle(width=11.72, height=0.44, fill_color="#E9F0F8", fill_opacity=1, stroke_width=0)
    bar.move_to(window.get_top() + DOWN * 0.22)
    dots = VGroup(
        Circle(radius=0.055, fill_color="#FF6B6B", fill_opacity=1, stroke_width=0),
        Circle(radius=0.055, fill_color="#FFD166", fill_opacity=1, stroke_width=0),
        Circle(radius=0.055, fill_color="#46E98B", fill_opacity=1, stroke_width=0),
    ).arrange(RIGHT, buff=0.12).move_to(bar.get_left() + RIGHT * 0.45)
    page_title = make_text("BH1750 / GY-302 조도 수집기", 26, "#12233C", weight="BOLD")
    page_title.move_to(window.get_center() + UP * 2.13 + LEFT * 2.9)
    connected = make_text("BH1750/GY-302 연결됨 · I²C 0x23", 16, "#5C7695")
    connected.move_to(window.get_center() + UP * 1.67 + LEFT * 3.07)
    lux = make_text("713.33 lux", 44, BLUE, font=FONT_MONO, weight="BOLD")
    lux.move_to(window.get_center() + UP * 1.02 + LEFT * 3.1)

    stat_box = RoundedRectangle(
        width=5.55,
        height=1.32,
        corner_radius=0.12,
        fill_color="#EAF2FB",
        fill_opacity=1,
        stroke_width=0,
    ).move_to(window.get_center() + LEFT * 2.45 + DOWN * 0.16)
    stat_lines = VGroup(
        make_text("완성 샘플: 2 / 20", 19, "#172B45"),
        make_text("현재 샘플 값: 1 / 30", 19, "#172B45"),
        make_text("수집한 전체 값: 61", 19, "#172B45", weight="BOLD"),
    ).arrange(DOWN, aligned_edge=LEFT, buff=0.06)
    stat_lines.move_to(stat_box)

    labels = VGroup(
        make_text("라벨", 18, "#172B45", weight="BOLD"),
        RoundedRectangle(width=2.45, height=0.48, corner_radius=0.08, fill_color=WHITE, fill_opacity=1, stroke_color="#B7C7DA", stroke_width=1),
        make_text("indoor", 18, "#172B45", font=FONT_MONO),
        make_text("라벨당 샘플", 18, "#172B45", weight="BOLD"),
        RoundedRectangle(width=0.94, height=0.48, corner_radius=0.08, fill_color=WHITE, fill_opacity=1, stroke_color="#B7C7DA", stroke_width=1),
        make_text("20", 18, "#172B45", font=FONT_MONO),
        make_text("샘플당 값", 18, "#172B45", weight="BOLD"),
        RoundedRectangle(width=0.94, height=0.48, corner_radius=0.08, fill_color=WHITE, fill_opacity=1, stroke_color="#B7C7DA", stroke_width=1),
        make_text("30", 18, "#172B45", font=FONT_MONO),
    ).arrange(RIGHT, buff=0.10)
    labels.move_to(window.get_center() + DOWN * 1.18)

    buttons = VGroup(
        pill("수집 시작", BLUE, 17, 1.35),
        pill("중지", "#607086", 17, 0.8),
        pill("ZIP 다운로드", "#607086", 17, 1.65),
    ).arrange(RIGHT, buff=0.15)
    buttons.move_to(window.get_center() + DOWN * 1.78 + LEFT * 3.55)
    status = make_text("1초마다 값을 읽습니다. CSV → ZIP", 16, "#5C7695")
    status.move_to(window.get_center() + DOWN * 2.22 + LEFT * 3.05)

    return VGroup(window, bar, dots, page_title, connected, lux, stat_box, stat_lines, labels, buttons, status)


def photo_card(path: Path, label: str, count: int, accent: str) -> VGroup:
    """실제 데이터셋 사진을 카드에 배치한다."""

    card = RoundedRectangle(
        width=4.45,
        height=4.65,
        corner_radius=0.18,
        fill_color=PANEL,
        fill_opacity=1,
        stroke_color=accent,
        stroke_width=2.5,
    )
    if path.exists():
        image = ImageMobject(str(path)).scale_to_fit_width(3.55)
    else:
        image = RoundedRectangle(width=3.55, height=3.0, corner_radius=0.1, fill_color=PANEL_2, fill_opacity=1, stroke_width=0)
    image.move_to(card.get_center() + UP * 0.32)
    image_border = SurroundingRectangle(image, buff=0.06, color=accent, stroke_width=2)
    label_text = make_text(label, 24, accent, font=FONT_MONO, weight="BOLD")
    label_text.move_to(card.get_center() + DOWN * 1.78)
    count_text = make_text(f"{count}장", 20, TEXT, weight="BOLD")
    count_text.move_to(card.get_center() + DOWN * 2.12)
    # ImageMobject는 일반 Mobject이므로 VGroup 대신 범용 Group을 반환한다.
    return Group(card, image, image_border, label_text, count_text)


def flow_card(title: str, body: str, accent: str, width: float = 2.5, height: float = 1.55) -> VGroup:
    card = RoundedRectangle(
        width=width,
        height=height,
        corner_radius=0.16,
        fill_color=PANEL,
        fill_opacity=1,
        stroke_color=accent,
        stroke_width=2,
    )
    head = make_text(title, 20, accent, weight="BOLD").move_to(card.get_center() + UP * 0.34)
    content = make_text(body, 17, TEXT).move_to(card.get_center() + DOWN * 0.25)
    return VGroup(card, head, content)


class TodayIoTAI(Scene):
    """오늘 실습의 데이터 흐름을 1분 안팎으로 요약하는 장면."""

    def clear_section(self) -> None:
        if self.mobjects:
            # ImageMobject처럼 일반 Mobject인 객체도 포함될 수 있으므로
            # VMobject 전용 VGroup 대신 범용 Group을 사용한다.
            self.play(FadeOut(Group(*self.mobjects)), run_time=0.35)

    def construct(self) -> None:
        lux_values = load_lux_values()
        report = load_report()
        labels = report.get("labels", {})
        dataset = report.get("dataset", {})
        impulse = report.get("impulse", {})
        training = report.get("training", {})
        testing = report.get("modelTesting", {}).get("variants", {})

        battery_count = int(labels.get("battery", 154))
        fan_count = int(labels.get("fan", 150))
        total_count = int(dataset.get("total", battery_count + fan_count))
        float_good = int(testing.get("float32", {}).get("good", 63))
        float_accuracy = float(testing.get("float32", {}).get("accuracy", 1.0))
        int8_good = int(testing.get("int8", {}).get("good", 50))
        int8_accuracy = float(testing.get("int8", {}).get("accuracy", 0.7936507936507937))
        mean_lux = sum(lux_values) / len(lux_values)

        # 1. 시작
        title = make_text("오늘 배운 IoT + AI 데이터 흐름", 44, TEXT, weight="BOLD")
        subtitle = make_text("센서 측정 → 브라우저 수집 → 데이터셋 → TinyML 추론", 26, CYAN)
        accent_line = Line(LEFT * 4.2, RIGHT * 4.2, stroke_color=BLUE, stroke_width=5)
        tag = pill("XIAO ESP32-S3  ·  BH1750  ·  Edge Impulse", PURPLE, 18, 5.5)
        start_group = VGroup(title, subtitle, accent_line, tag).arrange(DOWN, buff=0.34)
        self.play(FadeIn(start_group, shift=UP * 0.15), run_time=0.9)
        self.wait(1.3)
        self.clear_section()

        # 2. I²C 센서 연결
        header = make_text("1. 센서와 보드 연결", 35, TEXT, weight="BOLD").to_edge(UP, buff=0.38)
        header_line = Line(LEFT * 5.8, RIGHT * 5.8, stroke_color="#1D3958", stroke_width=2).next_to(header, DOWN, buff=0.22)
        board = board_icon().move_to(LEFT * 3.3 + UP * 0.2)
        sensor = sensor_icon().move_to(RIGHT * 3.1 + UP * 0.2)
        bus = Arrow(board.get_right() + RIGHT * 0.12, sensor.get_left() + LEFT * 0.12, buff=0.12, color=CYAN, stroke_width=4)
        bus_label = make_text("I²C 버스", 22, CYAN, weight="BOLD").next_to(bus, UP, buff=0.13)
        address = pill("BH1750  ·  주소 0x23", GREEN, 18, 2.9).next_to(sensor, DOWN, buff=0.55)
        read_note = make_text("1초마다 조도 값을 읽어 lux로 출력", 25, TEXT, weight="BOLD").to_edge(DOWN, buff=0.43)
        wiring = make_text("SDA = D4 / GPIO5     SCL = D5 / GPIO6", 18, YELLOW, font=FONT_MONO).move_to(DOWN * 1.18)
        self.play(FadeIn(header), Create(header_line), run_time=0.5)
        self.play(FadeIn(board, shift=LEFT * 0.2), FadeIn(sensor, shift=RIGHT * 0.2), run_time=0.8)
        self.play(GrowArrow(bus), FadeIn(bus_label), FadeIn(wiring), run_time=0.7)
        self.play(FadeIn(address), FadeIn(read_note, shift=UP * 0.1), run_time=0.55)
        self.wait(1.3)
        self.clear_section()

        # 3. 실제 CSV를 그래프로 보여주기
        header = make_text("2. 조도 값을 시계열 데이터로 저장", 35, TEXT, weight="BOLD").to_edge(UP, buff=0.38)
        header_line = Line(LEFT * 5.8, RIGHT * 5.8, stroke_color="#1D3958", stroke_width=2).next_to(header, DOWN, buff=0.22)
        axes = Axes(
            x_range=[0, max(30, len(lux_values)), 5],
            y_range=[0, 800, 200],
            x_length=7.2,
            y_length=4.05,
            tips=False,
            axis_config={"color": MUTED, "stroke_width": 2},
        ).move_to(LEFT * 2.15 + DOWN * 0.15)
        x_caption = make_text("시간 (초)", 17, MUTED).next_to(axes, DOWN, buff=0.12)
        y_caption = make_text("lux", 17, MUTED, font=FONT_MONO).next_to(axes, LEFT, buff=0.15)
        graph = axes.plot_line_graph(
            list(range(len(lux_values))),
            lux_values,
            line_color=CYAN,
            add_vertex_dots=True,
            vertex_dot_radius=0.035,
        )
        data_box = RoundedRectangle(
            width=4.25,
            height=3.85,
            corner_radius=0.18,
            fill_color=PANEL,
            fill_opacity=1,
            stroke_color=BLUE,
            stroke_width=2,
        ).move_to(RIGHT * 3.95 + DOWN * 0.15)
        file_name = make_text("indoor.0001.csv", 22, CYAN, font=FONT_MONO, weight="BOLD").move_to(data_box.get_center() + UP * 1.35)
        format_text = make_text("timestamp,lux", 20, TEXT, font=FONT_MONO).move_to(data_box.get_center() + UP * 0.8)
        values_text = make_text(f"{len(lux_values)}개 값  ·  30초", 20, TEXT).move_to(data_box.get_center() + UP * 0.2)
        mean_text = make_text(f"평균  {mean_lux:.2f} lux", 22, GREEN, font=FONT_KO, weight="BOLD").move_to(data_box.get_center() + DOWN * 0.55)
        interval_text = make_text("1초 간격 측정", 18, MUTED).move_to(data_box.get_center() + DOWN * 1.18)
        current_box = RoundedRectangle(width=2.4, height=0.65, corner_radius=0.12, fill_color="#11324C", fill_opacity=1, stroke_color=CYAN, stroke_width=1.5)
        current_value = make_text(f"{lux_values[0]:.2f} lux", 25, CYAN, font=FONT_MONO, weight="BOLD").move_to(current_box)
        current_box_group = VGroup(current_box, current_value).move_to(LEFT * 2.15 + UP * 2.55)
        self.play(FadeIn(header), Create(header_line), run_time=0.5)
        self.play(Create(axes), FadeIn(x_caption), FadeIn(y_caption), FadeIn(data_box), run_time=0.8)
        self.play(Create(graph), FadeIn(file_name), FadeIn(format_text), run_time=1.1)
        self.play(FadeIn(current_box_group), FadeIn(values_text), FadeIn(mean_text), FadeIn(interval_text), run_time=0.6)
        for index in (5, 10, 15, 20, 25):
            next_value = make_text(f"{lux_values[index]:.2f} lux", 25, CYAN, font=FONT_MONO, weight="BOLD").move_to(current_box)
            self.play(Transform(current_value, next_value), run_time=0.22)
        self.wait(1.1)
        self.clear_section()

        # 4. 브라우저에서 수집하고 ZIP 만들기
        header = make_text("3. 브라우저에서 데이터셋 만들기", 35, TEXT, weight="BOLD").to_edge(UP, buff=0.38)
        header_line = Line(LEFT * 5.8, RIGHT * 5.8, stroke_color="#1D3958", stroke_width=2).next_to(header, DOWN, buff=0.22)
        collector = browser_collector().move_to(DOWN * 0.18)
        bottom_note = make_text("수집 시작  →  CSV 샘플 완성  →  ZIP 다운로드", 22, GREEN, weight="BOLD").to_edge(DOWN, buff=0.28)
        self.play(FadeIn(header), Create(header_line), run_time=0.5)
        self.play(FadeIn(collector, shift=UP * 0.1), run_time=1.2)
        self.play(FadeIn(bottom_note, shift=UP * 0.1), run_time=0.6)
        self.wait(1.4)
        self.clear_section()

        # 5. 카메라 데이터셋
        header = make_text("4. 카메라 사진으로 분류 데이터셋 수집", 35, TEXT, weight="BOLD").to_edge(UP, buff=0.38)
        header_line = Line(LEFT * 5.8, RIGHT * 5.8, stroke_color="#1D3958", stroke_width=2).next_to(header, DOWN, buff=0.22)
        battery = photo_card(BATTERY_IMAGE, "battery", battery_count, GREEN).move_to(LEFT * 2.65 + DOWN * 0.05)
        fan = photo_card(FAN_IMAGE, "fan", fan_count, YELLOW).move_to(RIGHT * 2.65 + DOWN * 0.05)
        dataset_tag = pill(f"전체 {total_count}장  ·  battery + fan", PURPLE, 18, 4.4).to_edge(DOWN, buff=0.25)
        zip_note = make_text("브라우저에서 촬영 → ZIP → 학습 데이터로 업로드", 19, MUTED).next_to(dataset_tag, UP, buff=0.18)
        self.play(FadeIn(header), Create(header_line), run_time=0.5)
        self.play(FadeIn(battery, shift=LEFT * 0.2), FadeIn(fan, shift=RIGHT * 0.2), run_time=1.0)
        self.play(FadeIn(zip_note), FadeIn(dataset_tag), run_time=0.65)
        self.wait(1.4)
        self.clear_section()

        # 6. Edge Impulse 이미지 분류
        header = make_text("5. TinyML 학습과 현장 추론", 35, TEXT, weight="BOLD").to_edge(UP, buff=0.38)
        header_line = Line(LEFT * 5.8, RIGHT * 5.8, stroke_color="#1D3958", stroke_width=2).next_to(header, DOWN, buff=0.22)
        if INFERENCE_SCREEN.exists():
            inference = ImageMobject(str(INFERENCE_SCREEN)).scale_to_fit_height(3.2).move_to(LEFT * 3.7 + DOWN * 0.18)
        else:
            inference = RoundedRectangle(width=3.85, height=3.2, corner_radius=0.12, fill_color=PANEL, fill_opacity=1, stroke_color=GREEN, stroke_width=2)
        inference_border = SurroundingRectangle(inference, buff=0.08, color=GREEN, stroke_width=2)
        inference_caption = make_text("Webppt 참고 화면", 17, MUTED).next_to(inference, DOWN, buff=0.12)
        prep = flow_card("입력", f"{impulse.get('imageWidth', 96)} × {impulse.get('imageHeight', 96)}", CYAN, 1.9, 1.35).move_to(RIGHT * 0.0 + UP * 0.85)
        train = flow_card("학습", f"전이학습\n{training.get('cycles', 20)} cycles", PURPLE, 2.25, 1.35).move_to(RIGHT * 2.45 + UP * 0.85)
        result = flow_card("추론", "battery / fan", GREEN, 2.25, 1.35).move_to(RIGHT * 4.95 + UP * 0.85)
        arrow_1 = Arrow(prep.get_right(), train.get_left(), buff=0.12, color=CYAN, stroke_width=3)
        arrow_2 = Arrow(train.get_right(), result.get_left(), buff=0.12, color=CYAN, stroke_width=3)
        float_pct = f"float32  {float_good}/63  =  {float_accuracy * 100:.0f}%"
        int8_pct = f"int8     {int8_good}/63  =  {int8_accuracy * 100:.1f}%"
        metric_float = pill(float_pct, GREEN, 18, 3.7).move_to(RIGHT * 2.5 + DOWN * 0.75)
        metric_int8 = pill(int8_pct, ORANGE, 18, 3.7).move_to(RIGHT * 2.5 + DOWN * 1.38)
        caution = make_text("기존 테스트셋 결과 · 새 이미지로 다시 검증하기", 17, YELLOW).move_to(RIGHT * 2.5 + DOWN * 2.12)
        self.play(FadeIn(header), Create(header_line), run_time=0.5)
        self.play(FadeIn(inference), Create(inference_border), FadeIn(inference_caption), run_time=0.8)
        self.play(FadeIn(prep), GrowArrow(arrow_1), FadeIn(train), GrowArrow(arrow_2), FadeIn(result), run_time=1.0)
        self.play(FadeIn(metric_float), FadeIn(metric_int8), FadeIn(caution), run_time=0.75)
        self.wait(1.5)
        self.clear_section()

        # 7. 마무리
        header = make_text("오늘의 핵심 한 줄", 38, TEXT, weight="BOLD").to_edge(UP, buff=0.48)
        header_line = Line(LEFT * 5.8, RIGHT * 5.8, stroke_color="#1D3958", stroke_width=2).next_to(header, DOWN, buff=0.25)
        flow_titles = [
            ("측정", "BH1750\nlux", CYAN),
            ("수집", "브라우저\nCSV / ZIP", BLUE),
            ("학습", "사진\nbattery + fan", PURPLE),
            ("검증", "테스트셋\n정확도", ORANGE),
            ("추론", "새 이미지\n현장에서 확인", GREEN),
        ]
        cards = VGroup(*[flow_card(title, body, color, 2.05, 1.55) for title, body, color in flow_titles])
        cards.arrange(RIGHT, buff=0.42).move_to(DOWN * 0.05)
        arrows = VGroup(*[
            Arrow(cards[i].get_right(), cards[i + 1].get_left(), buff=0.08, color=MUTED, stroke_width=2.5)
            for i in range(len(cards) - 1)
        ])
        final_line = make_text("측정 → 저장 → 학습 → 검증 → 현장 적용", 28, CYAN, weight="BOLD").move_to(DOWN * 1.55)
        final_note = make_text("정확도 숫자보다 중요한 것은 새로운 환경에서도 다시 확인하는 것", 19, MUTED).move_to(DOWN * 2.18)
        self.play(FadeIn(header), Create(header_line), run_time=0.5)
        for card in cards:
            self.play(FadeIn(card, shift=UP * 0.12), run_time=0.22)
        self.play(*[GrowArrow(arrow) for arrow in arrows], run_time=0.75)
        self.play(FadeIn(final_line, shift=UP * 0.1), FadeIn(final_note, shift=UP * 0.1), run_time=0.75)
        self.wait(2.2)
