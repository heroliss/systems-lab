param([switch]$Publish)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$msbuild=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild/**/Bin/MSBuild.exe' | Select-Object -First 1
if(!$msbuild){throw 'Current Visual Studio C++ tools are required.'}
foreach($name in @('SystemsCore','SystemsTests')) {
 & $msbuild (Join-Path $root "native/$name.vcxproj") /p:Configuration=Release /p:Platform=x64 /m:2 /v:minimal /nologo
 if($LASTEXITCODE){throw "Native build failed: $name"}
}
& (Join-Path $root 'artifacts/native/SystemsTests.exe')
if($LASTEXITCODE){throw 'Native regression failed'}
if($Publish) {
 $out=Join-Path $root 'artifacts/SystemsLab-1.0.0-win-x64'
 dotnet publish (Join-Path $root 'src/SystemsLab.App') -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -p:DebugType=None -o $out
} else { dotnet build (Join-Path $root 'src/SystemsLab.App') -c Release }
if($LASTEXITCODE){throw 'Desktop build failed'}
