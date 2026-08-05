"""STL을 X축으로 90도 정렬한 뒤 Z축 중심으로 10.5초 회전시키는 영상."""

from __future__ import annotations

from pathlib import Path

import numpy as np
import trimesh
from manim import *


PROJECT_ROOT = Path(__file__).resolve().parent
STL_PATH = PROJECT_ROOT / "helmet.stl"

config.pixel_width = 1280
config.pixel_height = 720
config.frame_rate = 30
config.background_color = "#07111F"


def load_helmet() -> VGroup:
    """STL을 영상용 저폴리곤 표면으로 읽고 Y축 방향을 위로 맞춘다."""

    mesh = trimesh.load_mesh(STL_PATH, file_type="stl", process=True)
    if not isinstance(mesh, trimesh.Trimesh):
        raise TypeError(f"Expected one STL mesh, got {type(mesh).__name__}")

    # 원본 48,904면을 줄여 영상이 무리 없이 렌더되도록 한다.
    mesh = mesh.simplify_quadric_decimation(face_count=450)
    mesh.remove_unreferenced_vertices()

    vertices = np.asarray(mesh.vertices, dtype=float)
    bounds = np.asarray(mesh.bounds, dtype=float)
    center = (bounds[0] + bounds[1]) / 2
    height = bounds[1][1] - bounds[0][1]
    if height <= 0:
        raise ValueError("STL has no usable Y-axis height")

    # STL의 Y축을 세로축으로 정렬하고, 헬멧의 윗부분이 화면 위를 향하게 한다.
    vertices = (vertices - center) * (4.35 / height)
    polyhedron = Polyhedron(
        vertices.tolist(),
        np.asarray(mesh.faces, dtype=int).tolist(),
        faces_config={
            "fill_color": "#718596",
            "fill_opacity": 1.0,
            "stroke_opacity": 0.0,
            "shade_in_3d": True,
        },
        graph_config={
            "vertex_config": {"radius": 0, "fill_opacity": 0},
            "edge_config": {"stroke_opacity": 0},
        },
    )
    polyhedron.clear_updaters()
    return polyhedron.faces


class HelmetSTLZAxisRotation(ThreeDScene):
    """텍스트 없이 헬멧만 Z축을 기준으로 한 바퀴 회전시킨다."""

    def construct(self) -> None:
        self.set_camera_orientation(phi=68 * DEGREES, theta=-58 * DEGREES)
        self.camera.light_source.move_to(np.array([-5.0, 7.0, 6.0]))

        helmet = load_helmet()
        helmet.move_to(ORIGIN)

        # STL 원본 자세를 먼저 X축 기준 +90도로 돌려 윗부분이 위를 향하게 한다.
        helmet.rotate(90 * DEGREES, axis=RIGHT, about_point=ORIGIN)
        self.add(helmet)

        # 정렬이 끝난 헬멧을 Z_AXIS 중심으로 한 바퀴 회전시킨다.
        rotation_state = [0.0]

        def rotate_around_z_axis(mobject: Mobject, alpha: float) -> None:
            delta = (alpha - rotation_state[0]) * TAU
            mobject.rotate(delta, axis=OUT, about_point=ORIGIN)
            rotation_state[0] = alpha

        # 처음부터 끝까지 헬멧만 일정한 속도로 360도 회전한다.
        self.play(
            UpdateFromAlphaFunc(helmet, rotate_around_z_axis),
            run_time=10.5,
            rate_func=linear,
        )
