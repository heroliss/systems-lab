$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
& (Join-Path $PSScriptRoot 'build.ps1') -Publish
if($LASTEXITCODE){throw 'Build failed'}
$out=Join-Path $root 'artifacts/SystemsLab-1.0.0-win-x64'
$report=Join-Path $root 'artifacts/interop.json'
$p=Start-Process -FilePath (Join-Path $out 'Liss.SystemsLab.exe') -ArgumentList '--verify',('"'+$report+'"') -WindowStyle Hidden -PassThru -Wait
if($p.ExitCode -ne 0 -or !(Test-Path -LiteralPath $report)){throw 'Final package interop checks failed'}
Copy-Item -LiteralPath (Join-Path $root 'README.md') -Destination $out
$notices=Join-Path $out 'licenses'
New-Item -ItemType Directory -Force -Path $notices | Out-Null
foreach($file in (Get-ChildItem -LiteralPath (Join-Path $root 'licenses') -File)){Copy-Item -LiteralPath $file.FullName -Destination $notices}
$records=Get-ChildItem -LiteralPath $out -File -Recurse | Where-Object Name -ne 'manifest.json' | ForEach-Object { [pscustomobject]@{path=[IO.Path]::GetRelativePath($out,$_.FullName).Replace('\','/');bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()} }
$records | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $out 'manifest.json') -Encoding utf8
$zip=Join-Path $root 'artifacts/SystemsLab-1.0.0-win-x64.zip'
Compress-Archive -Path (Join-Path $out '*') -DestinationPath $zip -Force
$hash=(Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
"$hash  $([IO.Path]::GetFileName($zip))" | Set-Content -LiteralPath "$zip.sha256" -Encoding ascii
[pscustomobject]@{zip=$zip;bytes=(Get-Item -LiteralPath $zip).Length;sha256=$hash;interop=(Get-Content -LiteralPath $report -Raw | ConvertFrom-Json)} | ConvertTo-Json -Depth 5
