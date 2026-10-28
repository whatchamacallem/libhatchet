# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Requires wslc.exe from WSL 3. Does not touch WSL2 distributions.

$ErrorActionPreference = 'Stop'

$distributions = [ordered]@{
	ubuntu = @{ Image = 'ubuntu:26.04';             Install = './install_deb.sh' }
	debian = @{ Image = 'debian:13';                Install = './install_deb.sh' }
	rocky  = @{ Image = 'rockylinux/rockylinux:10'; Install = './install_rpm.sh' }
}

# Builds, logs and the list of files copied into the containers.
$work = 'tmp\testmsvc'

# Each test returns with $LASTEXITCODE set to its result.
function Test-Msvc([string]$platform, [string]$configuration) {
	$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
	$visual_studio = & $vswhere -latest -property installationPath
	Import-Module "$visual_studio\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
	Enter-VsDevShell -VsInstallPath $visual_studio -SkipAutomaticLocation

	$name = "$platform-$configuration".ToLower()
	cmake -S . -B $work\$name -A $platform
	if ($LASTEXITCODE -ne 0) { return }
	cmake --build $work\$name --config $configuration --parallel 24
	if ($LASTEXITCODE -ne 0) { return }
	Push-Location $work\$name
	.\hxtest.exe
	Pop-Location
}

function Test-Container([string]$distribution) {
	$image = $distributions[$distribution]
	$name = "libhatchet-test-$distribution"

	$list = wslc list -a --format json --filter "name=$name" | ConvertFrom-Json
	if (@($list | Where-Object { $_.Names -eq $name }).Count -ne 0) {
		wslc remove -f $name | Out-Null
	}
	wslc run -d --name $name --hostname $distribution -w /libhatchet $image.Image `
		sleep infinity | Out-Null
	if ($LASTEXITCODE -ne 0) { return }

	# ThreadSanitizer requires lower ASLR entropy than the wslc session VM default.
	# Containers cannot disable ASLR themselves. This does not persist. stdin is
	# piped because a null stdin handle intermittently fails with ERROR_INVALID_HANDLE.
	'' | wslc system session run sh -c 'echo 28 >/proc/sys/vm/mmap_rnd_bits'
	if ($LASTEXITCODE -ne 0) { return }

	# wslc has no cp command. cmd.exe pipes binary data unmodified. ustar avoids
	# the Windows specific pax headers.
	cmd /c ("$env:SystemRoot\System32\tar.exe -cf - --format ustar --null -T $work\files.txt" +
		" | wslc exec -i $name tar -xf - --no-same-permissions -C /libhatchet")
	if ($LASTEXITCODE -ne 0) { return }
	wslc exec $name bash $image.Install
	if ($LASTEXITCODE -ne 0) { return }

	# Windows tar does not preserve executable bits. Restore them from git.
	wslc exec $name sh -c 'git ls-files -s | grep ^100755 | cut -f 2 | xargs chmod +x'
	if ($LASTEXITCODE -ne 0) { return }
	wslc exec $name ./testall.sh
}

function Main([bool]$verbose, [bool]$debug) {
	if (Test-Path $work) { Remove-Item -Recurse -Force $work }
	if (Test-Path cache) { Remove-Item -Recurse -Force cache }
	New-Item -ItemType Directory -Force $work | Out-Null

	# The working tree and .git minus ignored and deleted files.
	$files = git ls-files --cached --others --exclude-standard |
		Where-Object { Test-Path -LiteralPath $_ }
	[IO.File]::WriteAllText("$PWD\$work\files.txt", (@($files) + '.git') -join "`0")

	$tests = [ordered]@{
		'x64-debug'     = 'Test-Msvc x64 Debug'
		'x64-release'   = 'Test-Msvc x64 Release'
		'win32-debug'   = 'Test-Msvc Win32 Debug'
		'win32-release' = 'Test-Msvc Win32 Release'
	}
	foreach ($distribution in $distributions.Keys) {
		$tests[$distribution] = "Test-Container $distribution"
	}

	$results = @()
	$failed = 0
	foreach ($test in $tests.Keys) {
		Write-Host "=== $test ==="
		$start = Get-Date
		$exit_code = 1
		try {
			# A new process runs each test so that its output can be logged.
			$command = "powershell -NoProfile -ExecutionPolicy Bypass -File testmsvc.ps1 $($tests[$test])"
			if ($verbose) {
				cmd /c $command
			} else {
				cmd /c "$command >$work\$test.log 2>&1"
			}
			$exit_code = $LASTEXITCODE
		} finally {
			# Failed containers are stopped and kept for debugging.
			if ($distributions.Contains($test) -and -not $debug) {
				if ($exit_code -eq 0) {
					wslc remove -f "libhatchet-test-$test" | Out-Null
				} else {
					wslc stop "libhatchet-test-$test" | Out-Null
				}
			}
		}

		$result = "$test passed"
		if ($exit_code -ne 0) {
			$result = "$test failed with exit code $exit_code"
			$failed++
		}
		$result += ' in ' + ('{0:h\:mm\:ss}' -f ((Get-Date) - $start))
		Write-Host $result
		$results += $result

		# Stop at the first MSVC failure.
		if ($exit_code -ne 0 -and -not $distributions.Contains($test)) { break }
	}

	Write-Host '=== results ==='
	$results | ForEach-Object { Write-Host $_ }
	if ($failed -eq 0 -and -not $debug) {
		Remove-Item -Recurse -Force $work, cache
	}
	exit $failed
}

if ($args[0] -like 'Test-*') {
	# A test run by Main.
	$arguments = @($args | Select-Object -Skip 1)
	& $args[0] @arguments
	exit $LASTEXITCODE
}
$verbose = $args -contains '--verbose'
$debug = $args -contains '--debug'
if ($args.Count -ne [int]$verbose + [int]$debug) {
	Write-Host 'usage_error: testmsvc.ps1 [--verbose] [--debug]'
	exit 1
}
Main $verbose $debug
