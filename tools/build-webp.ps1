param([string]$QtPrefix = "C:/Qt/6.8.3/msvc2022_64")
$ErrorActionPreference = "Stop"
# Run from an x64 Visual Studio developer shell with CMake/Ninja/Git on PATH.
$repoRoot = Split-Path $PSScriptRoot -Parent
$sourcePath = Join-Path $repoRoot "build/qtimageformats-source"
$buildPath = Join-Path $repoRoot "build/qtimageformats"
$revision = "3b6b1fb96041f1bc85a8f42593bef2ee7bcdca6c" # Qt ImageFormats v6.8.3, including bundled libwebp.
if (!(Test-Path -LiteralPath $sourcePath)) {
    git clone --depth 1 --branch v6.8.3 https://code.qt.io/qt/qtimageformats.git $sourcePath
    if ($LASTEXITCODE) { throw "Qt ImageFormats download failed" }
}
$actualRevision = git -C $sourcePath rev-parse HEAD
if ($LASTEXITCODE -or $actualRevision -ne $revision) { throw "Unexpected Qt ImageFormats source revision" }
git -C $sourcePath diff HEAD --quiet
if ($LASTEXITCODE) { throw "Qt ImageFormats source has local changes" }
cmake -S $sourcePath -B $buildPath -G "Ninja Multi-Config" "-DCMAKE_PREFIX_PATH=$QtPrefix" -DQT_BUILD_TESTS=OFF -DQT_BUILD_EXAMPLES=OFF -DFEATURE_webp=ON -DFEATURE_tiff=OFF -DFEATURE_mng=OFF -DFEATURE_jasper=OFF
if ($LASTEXITCODE) { throw "Qt ImageFormats configuration failed" }
foreach ($configuration in @("RelWithDebInfo", "Debug")) {
    cmake --build $buildPath --config $configuration --target QWebpPlugin
    if ($LASTEXITCODE) { throw "WebP plugin build failed: $configuration" }
}
