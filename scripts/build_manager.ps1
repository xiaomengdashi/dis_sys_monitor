param(
  [string]$BuildDir = "build/manager-win",
  [string]$Config = "Release",
  [string]$Generator = "",
  [string]$Architecture = "x64",
  [string]$VcpkgToolchain = ""
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$AbsoluteBuildDir = Join-Path $ProjectRoot $BuildDir

if ([string]::IsNullOrWhiteSpace($VcpkgToolchain) -and $env:VCPKG_ROOT) {
  $VcpkgToolchain = Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"
}

$configureArgs = @(
  "-S", $ProjectRoot,
  "-B", $AbsoluteBuildDir,
  "-DBUILD_MANAGER=ON",
  "-DBUILD_AGENT=OFF"
)

if (-not [string]::IsNullOrWhiteSpace($Generator)) {
  $configureArgs += @("-G", $Generator)
}

if (-not [string]::IsNullOrWhiteSpace($Architecture)) {
  if ([string]::IsNullOrWhiteSpace($Generator) -or $Generator.StartsWith("Visual Studio")) {
    $configureArgs += @("-A", $Architecture)
  }
}

if (-not [string]::IsNullOrWhiteSpace($VcpkgToolchain)) {
  $configureArgs += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain"
}

cmake @configureArgs
cmake --build $AbsoluteBuildDir --target manager --config $Config

Write-Host "manager build done: $AbsoluteBuildDir"
