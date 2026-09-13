# Rebuild Jaymod 3.1.0 modules and pack jaymod-3.1.0-64bit-lua.zip
# Usage: powershell -File tools/pack-release.ps1

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'build'
$dist = Join-Path $root 'dist'
$stage = Join-Path $dist 'jaymod-3.1.0-server\jaymod'
$srcPk3 = Join-Path $root 'tools\pk3work\jaymod-2.3.0.pk3'
$outPk3 = Join-Path $dist 'jaymod-3.1.0.pk3'
$finalZip = Join-Path $root 'jaymod-3.1.0-64bit-lua.zip'
$x86Dir = Join-Path $dist 'client-x86'

function Add-ZipFile($archive, $entryName, $filePath) {
    $name = $entryName.Replace('\', '/')
    $entry = $archive.CreateEntry($name, [System.IO.Compression.CompressionLevel]::Optimal)
    $dest = $entry.Open()
    $src = [System.IO.File]::OpenRead($filePath)
    try { $src.CopyTo($dest) } finally { $src.Dispose(); $dest.Dispose() }
}

function Add-ZipBytes($archive, $entryName, [byte[]]$bytes) {
    $name = $entryName.Replace('\', '/')
    $entry = $archive.CreateEntry($name, [System.IO.Compression.CompressionLevel]::Optimal)
    $dest = $entry.Open()
    try { $dest.Write($bytes, 0, $bytes.Length) } finally { $dest.Dispose() }
}

Write-Host '== build 64-bit modules =='
$mingw = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
$env:PATH = "C:\Program Files\CMake\bin;$mingw;" + $env:PATH
if (-not (Test-Path (Join-Path $build 'CMakeCache.txt'))) {
    cmake -S $root -B $build -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release
}
cmake --build $build --target qagame --target cgame --target ui -j 8
if ($LASTEXITCODE -ne 0) { throw "cmake --build failed: $LASTEXITCODE" }

Write-Host '== build 32-bit client modules =='
$mingw32 = Join-Path $root 'tools\mingw32\bin'
$buildX86 = Join-Path $root 'build-x86'
if (-not (Test-Path (Join-Path $mingw32 'g++.exe'))) {
    throw "missing 32-bit MinGW at $mingw32 (need i686 g++ for cgame_mp_x86.dll)"
}
New-Item -ItemType Directory -Force -Path $x86Dir | Out-Null
$savedPath = $env:PATH
$env:PATH = "C:\Program Files\CMake\bin;$mingw32;" + $env:PATH
if (-not (Test-Path (Join-Path $buildX86 'CMakeCache.txt'))) {
    cmake -S $root -B $buildX86 -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { $env:PATH = $savedPath; throw "32-bit cmake configure failed" }
}
cmake --build $buildX86 --target cgame --target ui -j 8
$x86BuildOk = ($LASTEXITCODE -eq 0)
$env:PATH = $savedPath
if (-not $x86BuildOk) { throw "32-bit cmake --build failed" }
Copy-Item (Join-Path $buildX86 'cgame_mp_x86.dll') (Join-Path $x86Dir 'cgame_mp_x86.dll') -Force
Copy-Item (Join-Path $buildX86 'ui_mp_x86.dll') (Join-Path $x86Dir 'ui_mp_x86.dll') -Force

$need = @(
    (Join-Path $build 'qagame_mp_x64.dll'),
    (Join-Path $build 'cgame_mp_x64.dll'),
    (Join-Path $build 'ui_mp_x64.dll'),
    (Join-Path $x86Dir 'cgame_mp_x86.dll'),
    (Join-Path $x86Dir 'ui_mp_x86.dll'),
    $srcPk3,
    (Join-Path $dist 'SERVER.txt')
)
foreach ($f in $need) {
    if (-not (Test-Path $f)) { throw "missing $f" }
}

function Stamp-X86Version([string]$path) {
    $b = [IO.File]::ReadAllBytes($path)
    $pairs = @(
        @{ From = [Text.Encoding]::ASCII.GetBytes('Jaymod 2.3.2'); To = [Text.Encoding]::ASCII.GetBytes('Jaymod 3.1.0') },
        @{ From = [Text.Encoding]::ASCII.GetBytes('^f2.3.2'); To = [Text.Encoding]::ASCII.GetBytes('^f3.1.0') }
    )
    $changed = 0
    foreach ($p in $pairs) {
        $from = $p.From; $to = $p.To
        if ($from.Length -ne $to.Length) { throw 'stamp length mismatch' }
        for ($i = 0; $i -le $b.Length - $from.Length; $i++) {
            $ok = $true
            for ($j = 0; $j -lt $from.Length; $j++) {
                if ($b[$i + $j] -ne $from[$j]) { $ok = $false; break }
            }
            if ($ok) {
                for ($j = 0; $j -lt $to.Length; $j++) { $b[$i + $j] = $to[$j] }
                $changed++
            }
        }
    }
    if ($changed -gt 0) {
        [IO.File]::WriteAllBytes($path, $b)
        Write-Host ("  stamped {0} ({1} replacements)" -f (Split-Path $path -Leaf), $changed)
    }
}

Write-Host '== stamp 32-bit client version to 3.1.0 =='
Stamp-X86Version (Join-Path $x86Dir 'cgame_mp_x86.dll')
Stamp-X86Version (Join-Path $x86Dir 'ui_mp_x86.dll')

Write-Host '== pack pk3 (forward-slash entries) =='
if (Test-Path $outPk3) { Remove-Item -LiteralPath $outPk3 -Force }

$src = [System.IO.Compression.ZipFile]::OpenRead($srcPk3)
$dst = [System.IO.Compression.ZipFile]::Open($outPk3, [System.IO.Compression.ZipArchiveMode]::Create)
$skip = @{
    'jaymod-2.3.0.dat' = $true
    'cgame_mp_x86.dll' = $true
    'ui_mp_x86.dll' = $true
}
try {
    foreach ($e in $src.Entries) {
        $name = $e.FullName.Replace('\', '/')
        $leaf = $name.Split('/')[-1]
        if ($skip.ContainsKey($leaf) -or $name -eq 'jaymod-2.3.0.dat') { continue }
        $ne = $dst.CreateEntry($name, [System.IO.Compression.CompressionLevel]::Optimal)
        $in = $e.Open()
        $out = $ne.Open()
        try { $in.CopyTo($out) } finally { $in.Dispose(); $out.Dispose() }
    }
    Add-ZipBytes $dst 'jaymod-3.1.0.dat' ([byte[]]@())
    Add-ZipFile $dst 'cgame_mp_x86.dll' (Join-Path $x86Dir 'cgame_mp_x86.dll')
    Add-ZipFile $dst 'ui_mp_x86.dll' (Join-Path $x86Dir 'ui_mp_x86.dll')
    Add-ZipFile $dst 'cgame_mp_x64.dll' (Join-Path $build 'cgame_mp_x64.dll')
    Add-ZipFile $dst 'ui_mp_x64.dll' (Join-Path $build 'ui_mp_x64.dll')
    Add-ZipFile $dst 'cgame_mp_x86_64.dll' (Join-Path $build 'cgame_mp_x64.dll')
    Add-ZipFile $dst 'ui_mp_x86_64.dll' (Join-Path $build 'ui_mp_x64.dll')
}
finally {
    $dst.Dispose()
    $src.Dispose()
}

Write-Host '== stage server folder =='
if (Test-Path $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stage | Out-Null
Copy-Item (Join-Path $build 'qagame_mp_x64.dll') (Join-Path $stage 'qagame_mp_x64.dll') -Force
Copy-Item $outPk3 (Join-Path $stage 'jaymod-3.1.0.pk3') -Force
Copy-Item (Join-Path $dist 'SERVER.txt') (Join-Path $stage 'SERVER.txt') -Force
$enh = Join-Path $dist 'enhmod'
foreach ($f in @(
    'ModEnhConfig.xml',
    'enhmod_commands.db',
    'enhmod_level.db',
    'enhmod_antirush.db',
    'forcecvarfile.cfg',
    'commands_flags.txt'
)) {
    $p = Join-Path $enh $f
    if (-not (Test-Path $p)) { throw "missing $p" }
    Copy-Item $p (Join-Path $stage $f) -Force
}

Write-Host '== zip =='
if (Test-Path $finalZip) { Remove-Item -LiteralPath $finalZip -Force }
$zip = [System.IO.Compression.ZipFile]::Open($finalZip, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    Get-ChildItem -LiteralPath $stage -Recurse -File | ForEach-Object {
        $rel = $_.FullName.Substring($stage.Length).TrimStart('\')
        Add-ZipFile $zip ("jaymod/" + $rel) $_.FullName
    }
}
finally { $zip.Dispose() }

Write-Host '== verify =='
$pk3 = [System.IO.Compression.ZipFile]::OpenRead($outPk3)
$backslash = @($pk3.Entries | Where-Object { $_.FullName.Contains('\') }).Count
$dlls = $pk3.Entries | Where-Object { $_.Name -match '\.dll$' } | ForEach-Object { $_.FullName }
$dats = $pk3.Entries | Where-Object { $_.Name -match '\.dat$' } | ForEach-Object { $_.FullName }
$pk3.Dispose()
Write-Host ("pk3={0} bytes backslash={1}" -f (Get-Item $outPk3).Length, $backslash)
$dlls | ForEach-Object { Write-Host "  pk3 $_" }
$dats | ForEach-Object { Write-Host "  pk3 $_" }
if ($backslash -ne 0) { throw 'pk3 has backslash entries' }
$x86Cgame = [IO.File]::ReadAllBytes((Join-Path $x86Dir 'cgame_mp_x86.dll'))
$x86Text = [Text.Encoding]::ASCII.GetString($x86Cgame)
if ($x86Text -notlike '*Jaymod 3.1.0*') { throw 'cgame_mp_x86.dll is not stamped Jaymod 3.1.0' }
if ($x86Text -notlike '*jay_fixedAspect*') { throw 'cgame_mp_x86.dll is still the old 32-bit HUD (missing jay_fixedAspect)' }

$z = [System.IO.Compression.ZipFile]::OpenRead($finalZip)
$zipNames = @($z.Entries | ForEach-Object { $_.FullName })
$zipNames | ForEach-Object { Write-Host ("  zip {0}" -f $_) }
$z.Dispose()
if ($zipNames | Where-Object { $_ -match 'report\.lua' }) {
    throw 'release zip still contains report.lua'
}
if ($zipNames | Where-Object { $_ -match 'qagame_mp_x86_64\.dll' }) {
    throw 'release zip still contains qagame_mp_x86_64.dll'
}
Write-Host ("zip={0} {1} bytes" -f $finalZip, (Get-Item $finalZip).Length)
Write-Host 'done'
