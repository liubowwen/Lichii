# SPDX-License-Identifier: GPL-3.0-or-later

# Installs the LLVM release that the Windows CI jobs compile with (clang-cl, lld-link, llvm-lib)
# and exports its location as LLVM_ROOT for later steps. The installer is pinned by checksum so a
# runner image update never changes the compiler.

$ErrorActionPreference = 'Stop'

$version = '22.1.8'
$sha256 = '16e5709785fef73c854646241c4a92c5cd574318d1b33c63330dd7721903e55c'

$installer = Join-Path $env:RUNNER_TEMP 'llvm.exe'
$url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-$version/LLVM-$version-win64.exe"

# GitHub's release CDN answers 504 for large assets often enough that a single attempt regularly
# failed the whole job. curl retries inside one request and again across attempts; the checksum
# below still pins the bytes, so a partial download can never slip through.
for ($attempt = 1; $attempt -le 5; $attempt++) {
    & curl.exe -fsSL --retry 5 --retry-all-errors --retry-delay 3 --connect-timeout 30 -o $installer $url
    if ($LASTEXITCODE -eq 0 -and (Test-Path $installer)) {
        break
    }
    Write-Host "LLVM download attempt $attempt failed (curl exit $LASTEXITCODE)"
    Remove-Item $installer -ErrorAction SilentlyContinue
    Start-Sleep -Seconds (5 * $attempt)
}
if (-not (Test-Path $installer)) {
    throw 'LLVM installer download failed after 5 attempts'
}
if ((Get-FileHash -Algorithm SHA256 $installer).Hash -ne $sha256) {
    throw 'LLVM installer checksum mismatch'
}
$root = Join-Path $env:RUNNER_TEMP 'llvm'
Start-Process -Wait -FilePath $installer -ArgumentList '/S', "/D=$root"
$reported = & "$root/bin/clang-cl.exe" --version
$reported
if (-not ($reported -match "clang version $([regex]::Escape($version))")) {
    throw "Unexpected clang-cl: $reported"
}
"LLVM_ROOT=$($root.Replace('\', '/'))" | Out-File -Append -Encoding utf8 $env:GITHUB_ENV
