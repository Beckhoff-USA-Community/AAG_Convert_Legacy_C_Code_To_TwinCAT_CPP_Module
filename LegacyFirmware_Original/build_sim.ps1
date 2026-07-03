# Builds and optionally runs the PC simulation of the legacy heater firmware.
# Needs any Visual Studio with C++ tools. Usage:  .\build_sim.ps1 [-Run]
param([switch]$Run)

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsroot = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsroot) { throw "No Visual Studio installation with C++ build tools found" }
$vsdev = Join-Path $vsroot "Common7\Tools\VsDevCmd.bat"

Push-Location $PSScriptRoot
try {
    cmd /c "`"$vsdev`" -arch=x64 -no_logo && cl /nologo /W4 /O2 /TC /Fe:temp_ctrl_sim.exe src\main.c src\legacy_pid.c src\legacy_temp_seq.c sim\hal_sim.c /I src"
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }
    Remove-Item *.obj -ErrorAction SilentlyContinue
    Write-Host "Built temp_ctrl_sim.exe"
    if ($Run) { .\temp_ctrl_sim.exe }
}
finally { Pop-Location }
