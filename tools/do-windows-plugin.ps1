# do-windows-plugin.ps1 - builds an AllAlike VST3 ON WINDOWS, with llvm-mingw. The native twin of
# tools/do-windows-plugin (which cross-builds on the Mac), and the same result:
#
#   powershell -ExecutionPolicy Bypass -File tools\do-windows-plugin.ps1 Pulse2Alike          this machine's arch
#   powershell -ExecutionPolicy Bypass -File tools\do-windows-plugin.ps1 all -Arch x64,arm64
#
# Result: <Product>\build\windows\<Product>.vst3, the VST3 BUNDLE FOLDER (Contents\<arch>-win\<Product>.vst3),
# as the zsh script makes it. THE SOURCE LIST AND THE DEFINES ARE THE PRODUCT'S do-plugin's, read from it.
# The shared build functions are G2Alike's tools\windows-common.ps1, as the zsh script uses G2Alike's
# platform\windows; FreeType is built once per architecture into SynthLib\build\windows-<arch>.
#
# Environment: LLVM_MINGW, VST3_SDK, G2ALIKE_DIR, ALIKE_WIN_VERSION (the version string).

param([Parameter(Mandatory = $true)][string[]]$Products,
      [string[]]$Arch = @($(if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64') { 'arm64' } else { 'x64' })))

$AllAlike = Split-Path -Parent $PSScriptRoot
$G2 = if ($env:G2ALIKE_DIR) { $env:G2ALIKE_DIR } else { Join-Path (Split-Path -Parent $AllAlike) 'G2Alike' }
. "$G2\tools\windows-common.ps1"    # sets $SynthLib and $Third from G2Alike's place, beside this checkout

$Win = "$G2\platform\windows"
$Sdk = if ($env:VST3_SDK) { $env:VST3_SDK } else { "$env:USERPROFILE\Documents\vst3sdk" }
if (-not (Test-Path "$Sdk\pluginterfaces")) { throw "VST3 SDK not found at $Sdk - set VST3_SDK" }
if ($Products -contains 'all') { $Products = @('Pulse2Alike', 'TiAlike', 'Sn2Alike', 'Nl3Alike') }

# SynthLib from source, as the Mac library takes it but without the Mac's files
$slSources = @(Get-ChildItem "$SynthLib\src\*.c", "$SynthLib\audio\*.c", "$SynthLib\plugin\*.c" |
               Where-Object { $_.Name -notin 'synthlibMidi.c', 'synthlibMidiPorts.c', 'synthlibPluginAu.c', 'device.c' } |
               ForEach-Object FullName) + @("$Win\synthlibMidiWin.c", "$Win\synthlibMidiPortsWin.c")
$slCxx = @(Get-ChildItem "$SynthLib\src\*.cpp" | ForEach-Object FullName) +
         @("$SynthLib\plugin\synthlibPluginVst3.cpp", "$SynthLib\plugin\synthlibPluginVst3ViewWin.cpp") +
         @('pluginterfaces\base\funknown.cpp', 'pluginterfaces\base\coreiids.cpp', 'public.sdk\source\vst\vstinitiids.cpp',
           'public.sdk\source\common\commoniids.cpp', 'pluginterfaces\base\ustring.cpp',
           'pluginterfaces\base\conststringtable.cpp' | ForEach-Object { "$Sdk\$_" })

function Build-Product([string]$product, $t, [string]$archName) {
    $dir = Join-Path $AllAlike $product
    $script = Get-Content "$dir\do-plugin" -Raw

    $name = [regex]::Match($script, '(?m)^PLUGIN_NAME="([^"]+)"').Groups[1].Value
    $verMatch = [regex]::Match($script, '(?m)^([A-Z0-9]+_VERSION)="\$\{\1:-([^}]*)\}"')
    $verVar = $verMatch.Groups[1].Value
    # the default is AllAlike's own version, read from its VERSION file
    $version = if ($env:ALIKE_WIN_VERSION) { $env:ALIKE_WIN_VERSION } elseif ($verMatch.Groups[2].Value -like '*VERSION*') { (Get-Content (Join-Path $AllAlike 'VERSION') -Raw).Trim() } else { $verMatch.Groups[2].Value }
    $core = ($version -split '-')[0] -split '\.'
    $auVersion = ([int]$core[0] -shl 16) -bor ([int]$core[1] -shl 8) -bor [int]$core[2]

    # DEFS, with the version filled in and zsh's quoting taken off: -DX="\"v\"" is the argument -DX="v"
    $defsLine = [regex]::Match($script, '(?m)^DEFS=\((.*)\)\s*$').Groups[1].Value
    $defs = foreach ($tok in [regex]::Matches($defsLine, '(?:"(?:\\.|[^"])*"|\S)+')) {
        $d = $tok.Value.Replace("`$$verVar", $version).Replace('$AU_VERSION', "$auVersion")
        $d = $d -replace '="\\"(.*?)\\""', '="$1"'
        Quote $d
    }

    $sources = @(Read-ZshArray "$dir\do-plugin" 'SOURCES' ([ordered]@{ HERE = $dir; SL = $SynthLib }) |
                 Where-Object { $_ -notlike '*Hardware.c' })
    $sources += @(Get-ChildItem "$dir\plugin\*HardwareWin.c" | ForEach-Object FullName)
    $sources += "$AllAlike\tools\windows\alikeWin.c"
    foreach ($s in $sources + $slSources + $slCxx) { if (-not (Test-Path $s)) { throw "missing source: $s" } }

    $incs = @("$dir\src", "$dir\plugin", "$SynthLib\src", "$SynthLib\audio", "$SynthLib\plugin",
              "$Third\glfw\include", "$Third\freetype\include") | ForEach-Object { Quote "-I$_" }
    $incs += '-isystem', (Quote $Sdk)
    $flags = @('-O2', '-include', (Quote "$Win\winCompat.h"), '-include', (Quote "$AllAlike\tools\windows\alikeCompat.h"),
               '-D_USE_MATH_DEFINES', '-D_POSIX_THREAD_SAFE_FUNCTIONS=200112L', '-DSYNTHLIB_PLUGIN_BUILD=1') + $defs + $incs

    $out = "$dir\build\windows-$archName"
    $obj = "$out\obj"
    if (Test-Path $obj) { Remove-Item -Recurse -Force $obj }
    New-Item -ItemType Directory -Force $obj | Out-Null

    Write-Host "==> $name $version VST3 for $($t.Triple) ($($sources.Count) + $($slSources.Count) C, $($slCxx.Count) C++)"
    $own = foreach ($s in $sources) {
        New-CompileJob $t.CC $s (Join-Path $obj (([IO.Path]::GetFileNameWithoutExtension($s)) + '.o')) (@('-std=gnu17') + $flags)
    }
    $lib = @(foreach ($s in $slSources) {
        New-CompileJob $t.CC $s (Join-Path $obj ('sl_' + [IO.Path]::GetFileNameWithoutExtension($s) + '.o')) (@('-std=gnu17') + $flags)
    }) + @(foreach ($s in $slCxx) {
        $quiet = @(if ($s.StartsWith($Sdk)) { '-w' })    # Steinberg's own files, theirs to warn about
        New-CompileJob $t.CXX $s (Join-Path $obj (([IO.Path]::GetFileNameWithoutExtension($s)) + '.oxx')) `
            (@('-std=gnu++17') + $flags + $quiet + @('-Wno-pragma-pack'))
    })
    Invoke-Compile (@($own) + @($lib))

    # SynthLib as an archive, as on the Mac, so the files this plug-in never calls (the GLFW window) drop out
    $archive = "$obj\libSynthLib.a"
    & $t.AR rcs $archive @($lib | ForEach-Object Obj)
    if ($LASTEXITCODE) { throw 'llvm-ar failed' }

    $dll = "$out\$name.vst3"
    Remove-Item $dll -ErrorAction SilentlyContinue
    & $t.CXX -shared -static -o $dll '-Wl,-u,GetPluginFactory' @($own | ForEach-Object Obj) $archive $ftLib `
        -lopengl32 -lgdi32 -luser32 -lole32 -lshell32 -lcomdlg32 -lwinmm -lpthread '-Wl,--exclude-all-symbols'
    if ($LASTEXITCODE) { throw "link failed for $name" }

    $bundleArch = if ($archName -eq 'arm64') { 'arm64-win' } else { 'x86_64-win' }    # Steinberg's folder names
    $contents = "$dir\build\windows\$name.vst3\Contents\$bundleArch"
    New-Item -ItemType Directory -Force $contents | Out-Null
    Copy-Item $dll $contents -Force
    $info = Show-Binary $t $dll
    Write-Host "  -> $contents"
}

$failed = @()
foreach ($product in $Products) {
    Remove-Item -Recurse -Force (Join-Path $AllAlike "$product\build\windows") -ErrorAction SilentlyContinue
    foreach ($a in $Arch) {
        $t = Get-Toolchain $a
        $t.Out = "$SynthLib\build\windows-$a"    # FreeType beside SynthLib's own builds, as the zsh script keeps it
        New-Item -ItemType Directory -Force "$($t.Out)\deps" | Out-Null
        $ftLib = Build-FreeType $t
        try { Build-Product $product $t $a } catch { Write-Host "error: $_"; $failed += $product; break }
    }
}
if ($failed) { throw "FAILED: $($failed -join ', ')" }
