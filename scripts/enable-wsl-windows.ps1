#Requires -RunAsAdministrator
<#
.SYNOPSIS
  Enable Windows optional features commonly required for WSL2 + VM platform.

.RUN
  Right-click PowerShell -> Run as administrator, then from repo root:

    powershell.exe -ExecutionPolicy Bypass -File ".\scripts\enable-wsl-windows.ps1"

.NOTES
  - Reboot after this completes, then run: wsl --update
  - If commands still fail with HCS / hypervisor errors, enable CPU virtualization (SVM / VT-x) in BIOS/UEFI and ensure no conflicting hypervisors block Hyper-V/WSL.

#>

$features = @(
    "VirtualMachinePlatform",
    "Microsoft-Windows-Subsystem-Linux"
)

foreach ($name in $features) {
    $state = Get-WindowsOptionalFeature -Online -FeatureName $name | Select-Object -ExpandProperty State
    Write-Host "[*] Feature $name : $state"
    if ($state -ne "Enabled") {
        Enable-WindowsOptionalFeature -Online -FeatureName $name -NoRestart -All | Out-Host
        Write-Host "[+] Enabled $name"
    }
}

Write-Host ""
Write-Host "Reboot recommended, then:"
Write-Host "  wsl --update"
Write-Host "  wsl -l -v"
Write-Host "If WSL cannot start afterwards, verify Virtualization is ON in BIOS/UEFI and that no incompatible hypervisor is blocking."
