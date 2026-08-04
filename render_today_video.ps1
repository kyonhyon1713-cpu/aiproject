param(
    [ValidateSet("ql", "qm", "qh")]
    [string]$Quality = "qm"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location -LiteralPath $projectRoot

$manim = Join-Path $projectRoot ".venv\Scripts\manim.exe"
if (-not (Test-Path -LiteralPath $manim)) {
    throw "Manim venv not found. Run: python -m venv .venv; .\.venv\Scripts\python.exe -m pip install -r requirements.txt"
}

& $manim "-$Quality" "make_today_video.py" "TodayIoTAI"
if ($LASTEXITCODE -ne 0) {
    throw "Manim render failed with exit code $LASTEXITCODE"
}

$video = Join-Path $projectRoot "media\videos\make_today_video\720p30\TodayIoTAI.mp4"
if (-not (Test-Path -LiteralPath $video)) {
    $video = Get-ChildItem -LiteralPath (Join-Path $projectRoot "media\videos\make_today_video") -Recurse -Filter "TodayIoTAI.mp4" | Select-Object -First 1 -ExpandProperty FullName
}
if (-not $video) {
    throw "Rendered MP4 was not found."
}

Write-Host "Browser video: $video"
