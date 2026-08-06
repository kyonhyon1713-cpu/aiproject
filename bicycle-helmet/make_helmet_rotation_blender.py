import math
import os
import glob
import shutil
import subprocess

import bpy
from mathutils import Vector


BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OBJ_PATH = os.path.join(BASE_DIR, "helmet.obj")
OUTPUT_PATH = os.path.join(BASE_DIR, "helmet_rotation.mp4")
FRAME_PREFIX = os.path.join(BASE_DIR, "helmet_rotation_frame_")

FRAME_START = 1
FRAME_END = 300
FPS = 30


def clear_scene():
    """Remove the default scene so the setup is repeatable."""
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    for datablocks in (
        bpy.data.cameras,
        bpy.data.lights,
        bpy.data.meshes,
        bpy.data.curves,
        bpy.data.materials,
    ):
        for datablock in list(datablocks):
            if datablock.users == 0:
                datablocks.remove(datablock)


def import_obj(filepath):
    """Import an OBJ file with a Blender-version-compatible operator."""
    if not os.path.isfile(filepath):
        raise FileNotFoundError(f"OBJ file not found: {filepath}")

    # Blender 4+ / 5.x uses bpy.ops.wm.obj_import.
    try:
        bpy.ops.wm.obj_import(
            filepath=filepath,
            forward_axis="NEGATIVE_Z",
            up_axis="Y",
            use_split_objects=True,
            use_split_groups=False,
        )
    except (AttributeError, TypeError):
        # Compatibility fallback for older Blender versions.
        bpy.ops.import_scene.obj(
            filepath=filepath,
            axis_forward="-Z",
            axis_up="Y",
            use_split_objects=True,
            use_split_groups=False,
        )

    mesh_objects = [obj for obj in bpy.context.selected_objects if obj.type == "MESH"]
    if not mesh_objects:
        raise RuntimeError("The OBJ file did not produce a mesh object.")

    # Join OBJ parts so one origin and one Z-axis animation control the whole helmet.
    bpy.context.view_layer.objects.active = mesh_objects[0]
    for obj in mesh_objects:
        obj.select_set(True)
    if len(mesh_objects) > 1:
        bpy.ops.object.join()

    model = bpy.context.object
    model.name = "Helmet"
    return model


def apply_origin_rotation_and_scale(model):
    """Set origin to geometry and apply Rotation + Scale, like Ctrl+A."""
    bpy.context.view_layer.objects.active = model
    model.select_set(True)

    # Object > Set Origin > Origin to Geometry.
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY", center="BOUNDS")

    # Object > Apply > Rotation and Scale (Ctrl+A).
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    model.location = (0.0, 0.0, 0.0)

    # The source OBJ is Y-up in its source coordinates. If the importer kept that
    # orientation, rotate it so the helmet stands upright on Blender's Z axis.
    dimensions = model.dimensions.copy()
    if dimensions.y < dimensions.z:
        model.rotation_euler[0] = math.radians(90.0)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)

    # Normalize the scene scale for practical camera and light distances, then
    # apply that scale as another explicit Ctrl+A operation.
    height = max(model.dimensions.z, 1e-6)
    model.scale = (2.8 / height, 2.8 / height, 2.8 / height)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

    # Smooth shading improves the appearance of the imported helmet mesh.
    for polygon in model.data.polygons:
        polygon.use_smooth = True

    return model


def point_at(obj, target=(0.0, 0.0, 0.0)):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def add_area_light(name, location, energy, size, color):
    light_data = bpy.data.lights.new(name=name, type="AREA")
    light_data.energy = energy
    light_data.shape = "DISK"
    light_data.size = size
    light_data.color = color

    light_object = bpy.data.objects.new(name, light_data)
    bpy.context.collection.objects.link(light_object)
    light_object.location = location
    point_at(light_object)
    return light_object


def make_material(name, base_color, roughness=0.45, metallic=0.0):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    principled = material.node_tree.nodes.get("Principled BSDF")
    if principled:
        principled.inputs["Base Color"].default_value = (*base_color, 1.0)
        principled.inputs["Roughness"].default_value = roughness
        principled.inputs["Metallic"].default_value = metallic
    return material


def prepare_materials(model):
    # Keep the colors imported from helmet.mtl and make the surfaces render softly.
    if not model.data.materials:
        model.data.materials.append(
            make_material("Helmet Material", (0.32, 0.48, 0.70), roughness=0.38)
        )

    for material in model.data.materials:
        if not material:
            continue
        material.use_nodes = True
        principled = material.node_tree.nodes.get("Principled BSDF")
        if principled:
            principled.inputs["Roughness"].default_value = 0.38


def add_ground(model):
    ground_material = make_material("Ground", (0.035, 0.045, 0.06), roughness=0.7)
    bpy.ops.mesh.primitive_plane_add(size=30.0, location=(0.0, 0.0, -model.dimensions.z / 2.0 - 0.04))
    ground = bpy.context.object
    ground.name = "Ground"
    ground.data.materials.append(ground_material)


def add_camera(model):
    camera_data = bpy.data.cameras.new("Camera")
    camera = bpy.data.objects.new("Camera", camera_data)
    bpy.context.collection.objects.link(camera)

    # Front view: camera looks from negative Y toward the centered model.
    view_size = max(model.dimensions.x, model.dimensions.z)
    camera.location = (0.0, -view_size * 2.15, view_size * 0.05)
    camera_data.lens = 55.0
    camera_data.sensor_width = 36.0
    point_at(camera)

    bpy.context.scene.camera = camera
    return camera


def animate_rotation(model):
    model.rotation_mode = "XYZ"
    model.animation_data_clear()

    model.rotation_euler = (0.0, 0.0, 0.0)
    model.keyframe_insert(data_path="rotation_euler", index=2, frame=FRAME_START)

    model.rotation_euler[2] = math.radians(360.0)
    model.keyframe_insert(data_path="rotation_euler", index=2, frame=FRAME_END)

    action = model.animation_data.action if model.animation_data else None
    if action:
        # Blender 5.x stores F-curves in layered Action channel bags, while
        # older versions expose them directly as action.fcurves.
        if hasattr(action, "fcurves"):
            fcurves = list(action.fcurves)
        else:
            fcurves = []
            for layer in action.layers:
                for strip in layer.strips:
                    for channelbag in strip.channelbags:
                        fcurves.extend(channelbag.fcurves)

        for fcurve in fcurves:
            for keyframe in fcurve.keyframe_points:
                keyframe.interpolation = "LINEAR"


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
    scene.render.fps = FPS
    scene.render.fps_base = 1.0

    scene.render.resolution_x = 1920
    scene.render.resolution_y = 1080
    scene.render.resolution_percentage = 100

    try:
        scene.render.engine = "BLENDER_EEVEE_NEXT"
    except (TypeError, ValueError):
        scene.render.engine = "BLENDER_EEVEE"

    # Blender 5.x selects video output through the FFmpeg settings below;
    # FFMPEG is no longer an image_settings.file_format enum value.
    scene.render.ffmpeg.format = "MPEG4"
    scene.render.ffmpeg.codec = "H264"
    scene.render.ffmpeg.constant_rate_factor = "MEDIUM"
    scene.render.ffmpeg.ffmpeg_preset = "GOOD"
    # Blender 5.2 installations without built-in FFmpeg support render a PNG
    # sequence first. The script then encodes that sequence to H.264 MP4.
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = FRAME_PREFIX
    scene.render.film_transparent = False

    scene.world.color = (0.012, 0.018, 0.028)
    try:
        scene.view_settings.look = "AgX - Medium High Contrast"
    except (TypeError, ValueError):
        pass


def main():
    clear_scene()
    model = import_obj(OBJ_PATH)
    apply_origin_rotation_and_scale(model)
    prepare_materials(model)
    configure_scene()
    add_ground(model)
    add_camera(model)

    # Key, fill, and rim Area Lights create a soft product-visualization look.
    add_area_light("Area Key", (-4.5, -5.5, 5.5), 900.0, 4.5, (1.0, 0.90, 0.80))
    add_area_light("Area Fill", (4.0, -3.0, 2.5), 500.0, 4.0, (0.72, 0.84, 1.0))
    add_area_light("Area Rim", (0.0, 4.0, 4.5), 1100.0, 3.5, (0.80, 0.88, 1.0))

    animate_rotation(model)
    bpy.context.scene.frame_set(FRAME_START)

    print(f"OBJ imported: {OBJ_PATH}")
    print(f"Output: {OUTPUT_PATH}")
    print(f"Frames: {FRAME_START}-{FRAME_END} at {FPS} FPS")
    print("Rotation axis: Z; interpolation: LINEAR")

    bpy.ops.render.render(animation=True)
    encode_mp4()


def find_ffmpeg():
    candidates = []
    path_ffmpeg = shutil.which("ffmpeg")
    if path_ffmpeg:
        candidates.append(path_ffmpeg)

    # The workspace virtual environment contains imageio-ffmpeg on this PC.
    local_ffmpeg_dir = os.path.abspath(
        os.path.join(BASE_DIR, os.pardir, ".venv", "Lib", "site-packages", "imageio_ffmpeg", "binaries")
    )
    candidates.extend(sorted(glob.glob(os.path.join(local_ffmpeg_dir, "ffmpeg*.exe"))))

    for candidate in candidates:
        if os.path.isfile(candidate):
            return candidate
    raise RuntimeError("FFmpeg executable was not found. Install FFmpeg or imageio-ffmpeg.")


def encode_mp4():
    """Encode the Blender PNG sequence as the requested H.264 MP4."""
    ffmpeg_path = find_ffmpeg()
    input_pattern = f"{FRAME_PREFIX}%04d.png"
    frame_count = FRAME_END - FRAME_START + 1
    command = [
        ffmpeg_path,
        "-y",
        "-hide_banner",
        "-loglevel",
        "warning",
        "-framerate",
        str(FPS),
        "-start_number",
        str(FRAME_START),
        "-i",
        input_pattern,
        "-frames:v",
        str(frame_count),
        "-c:v",
        "libx264",
        "-preset",
        "medium",
        "-crf",
        "18",
        "-pix_fmt",
        "yuv420p",
        "-movflags",
        "+faststart",
        "-an",
        OUTPUT_PATH,
    ]
    subprocess.run(command, check=True)

    # Remove only the exact temporary frames produced by this render.
    for frame_path in glob.glob(f"{FRAME_PREFIX}*.png"):
        os.remove(frame_path)
    print(f"H.264 MP4 created: {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
