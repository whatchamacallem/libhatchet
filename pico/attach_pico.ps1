# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# To attach a Pico to WSL2 run this as Administrator and keep the window open.
# 2e8a is the vendor ID for Raspberry Pi in general.
# install usbipd with: winget install usbipd

$device = usbipd list | Where-Object { $_ -match '2e8a:' } | Select-Object -First 1

if (-not $device) {
    Write-Error "No Pico found - connect it and re-run"
    usbipd list
    exit 1
}

$hwid = ($device -replace '^.*?(2e8a:\w+).*$', '$1')
$busid = ($device -replace '\s.*')

Write-Host "Found Pico at busid $busid (hardware-id $hwid)"

usbipd bind --busid $busid
usbipd attach --wsl --hardware-id $hwid --auto-attach
