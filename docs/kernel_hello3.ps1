# Marker test v3: wait GBL -> carrier -> watch USB enumeration + xnuloader -> hello.
$ErrorActionPreference = "Continue"
$log = "C:\temp\kernel-hello-run3.log"
Remove-Item $log -ErrorAction SilentlyContinue
function Log($m) { $line = "[{0}] {1}" -f (Get-Date -Format "HH:mm:ss"), $m; Add-Content -Path $log -Value $line; Write-Output $line }
function FB([string[]]$fbArgs, [int]$timeoutSec) {
    $job = Start-Job -ScriptBlock { param($a) & fastboot @a 2>&1 | Out-String } -ArgumentList (,$fbArgs)
    if (Wait-Job $job -Timeout $timeoutSec) { $out = Receive-Job $job 2>&1 | Out-String } else { $out = "TIMEOUT" }
    Stop-Job $job -ErrorAction SilentlyContinue
    Remove-Job $job -Force -ErrorAction SilentlyContinue
    return $out
}
function USBlist() {
    Get-PnpDevice -PresentOnly -ErrorAction SilentlyContinue |
      Where-Object { $_.InstanceId -match '^USB' } |
      ForEach-Object { "{0} | {1}" -f $_.FriendlyName, $_.InstanceId }
}

$carrier = "C:\temp\carrier-final\gbl-verify2\BootstrapVerify2.efi"
$payload = "C:\temp\kernel-hello-payload.bin"
Log "carrier=$((Get-Item $carrier).Length) payload=$((Get-Item $payload).Length)"

Log "waiting for GBL (565844a2 only, max 15 min)..."
$ready = $false
for ($i = 0; $i -lt 150; $i++) {
    $d = FB @("devices") 6
    if ($d -match "565844a2") { $ready = $true; Log "GBL up"; break }
    Start-Sleep -Seconds 4
}
if (-not $ready) { Log "NO GBL - abort"; exit 0 }

$baseUSB = USBlist
Log "stage carrier: $(FB @('stage', $carrier) 120)"
Log "boot-efi: $(FB @('oem','boot-efi') 30)"

Log "watching USB + xnuloader (max 240s)..."
$loaderUp = $false
$seenUSB = @{}
for ($i = 0; $i -lt 240; $i++) {
    $p = FB @("getvar","product") 4
    $t = $p.Trim()
    if ($t -match "xnuloader") { $loaderUp = $true; Log "probe ${i}: xnuloader UP"; break }
    if ($t -ne "TIMEOUT" -and $t -ne "") { Log "probe ${i}: $t" }
    if ($i % 5 -eq 0) {
        foreach ($u in (USBlist)) {
            if ($baseUSB -notcontains $u -and -not $seenUSB.ContainsKey($u)) {
                $seenUSB[$u] = $i
                Log "USB +${i}s: $u"
            }
        }
    }
    Start-Sleep -Seconds 1
}
if (-not $loaderUp) {
    Log "LOADER NEVER CAME UP after ${i}s"
    Log "adb: $(& adb devices 2>$null | Out-String)"
    Log "fastboot: $(FB @('devices') 6)"
    exit 0
}

$ll = "C:\temp\hello-loader-log3.txt"
Remove-Item $ll -ErrorAction SilentlyContinue
Log "loader log fetch: $(FB @('get_staged', $ll) 30)"
if (Test-Path $ll) { Log ([IO.File]::ReadAllText($ll) -replace "`0", "") }

Log "stage hello payload...: $(FB @('stage', $payload) 300)"
Log "oem run: $(FB @('oem','run') 20)"
Log "polling result (max 240s)..."
$done = $false
for ($i = 0; $i -lt 48; $i++) {
    $r = FB @("getvar","result") 8
    if ($r -match "returned") { Log "RESULT: $($r.Trim())"; $done = $true; break }
    if ($i % 6 -eq 5) { Log "still: $($r.Trim())" }
    Start-Sleep -Seconds 5
}
$st = "C:\temp\kernel-hello-log3.txt"
Remove-Item $st -ErrorAction SilentlyContinue
$got = $false
for ($try = 0; $try -lt 3 -and -not $got; $try++) {
    Log "get_staged attempt $($try+1): $(FB @('get_staged', $st) 90)"
    if (Test-Path $st) { if ((Get-Item $st).Length -gt 0) { $got = $true } }
    if (-not $got) { Start-Sleep -Seconds 3 }
}
if ($got) {
    Log "kernel log size: $((Get-Item $st).Length)"
    $txt = [IO.File]::ReadAllText($st) -replace "`0", ""
    Log "===== KERNEL LOG ====="
    Log $txt
    Log "===== END ====="
} else { Log "NO KERNEL LOG" }
Log "done"
