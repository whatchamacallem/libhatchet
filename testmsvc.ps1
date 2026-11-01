# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Uses wslc.exe containers from WSL 3. Does not touch earlier WSL containers.
# Caches pre-built containers with the required packages installed so
# iterative development doesn't hit the network.

$ErrorActionPreference = 'Stop'

$distributions = [ordered]@{
	debian = @{ Image = 'debian:13';                 Install = 'install_deb.sh' }
	mint   = @{ Image = 'linuxmintd/mint22.3-amd64'; Install = 'install_deb.sh' }
	rocky  = @{ Image = 'rockylinux/rockylinux:10';  Install = 'install_rpm.sh' }
	ubuntu = @{ Image = 'ubuntu:26.04';              Install = 'install_deb.sh' }
}

$build_dir = 'build'

function Get-MsvcTest([string]$platform, [string]$configuration) {
	$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
	$visual_studio = & $vswhere -latest -property installationPath
	$name = "$platform-$configuration".ToLower()
	@(
		"call `"$visual_studio\Common7\Tools\VsDevCmd.bat`" -no_logo"
		"cmake -S . -B $build_dir\$name -A $platform"
		"cmake --build $build_dir\$name --config $configuration --parallel 24"
		"cd $build_dir\$name"
		'.\hxtest.exe'
	) -join ' && '
}

# Failed containers are kept, restarted and debugged by running "testall.sh
# --verbose" in them.
function Get-ContainerTest([string]$distribution) {
	$image = $distributions[$distribution]
	$name = "libhatchet-test-$distribution"

	# The image name includes the base image so that changing it forces a build.
	$tag = "libhatchet-test/$($image.Image)"
	$ignored = @("wslc remove -f $name >nul 2>&1")
	$required = @()

	# The image is built from the base image by its install script and is rebuilt
	# when it is older than that script.
	$listed = wslc images --format json --filter "reference=$tag" | ConvertFrom-Json
	if (-not $listed -or [DateTimeOffset]::Parse($listed.CreatedAt -replace ' \D+$') -lt
			(Get-Item $image.Install).LastWriteTime) {
		$context = "$build_dir\$distribution"
		New-Item -ItemType Directory $context | Out-Null
		Copy-Item $image.Install $context
		Set-Content "$context\Dockerfile" -NoNewline -Encoding ascii ("FROM $($image.Image)`n" +
			"RUN --mount=type=bind,source=$($image.Install),target=/$($image.Install)" +
			" bash /$($image.Install)`n")
		$ignored += "wslc rmi $tag >nul 2>&1"
		$required += "wslc build --no-cache --progress plain -t $tag $context"
	}

	$required += @(
		"wslc run -d --name $name --hostname $distribution --tmpfs /libhatchet:exec" +
			" -w /libhatchet $tag sleep infinity >nul"

		# ThreadSanitizer requires lower ASLR entropy than the wslc session VM default.
		# Containers cannot disable ASLR themselves. This does not persist. stdin is
		# piped because a null stdin handle intermittently fails with ERROR_INVALID_HANDLE.
		'echo.|wslc system session run sh -c "echo 28 >/proc/sys/vm/mmap_rnd_bits"'

		# wslc has no cp command. cmd.exe redirects binary data unmodified.
		"wslc exec -i $name tar -xf - --no-same-permissions -C /libhatchet <$build_dir\libhatchet.tar"

		# Confirms that the image has every package and nothing is installed.
		"wslc exec $name sh -c `"bash $($image.Install) | tee /dev/stderr" +
			" | grep -q packages_already_installed`""

		# testall.sh requires a git repository. Executable bits are not tested.
		"wslc exec $name sh -c `"chmod +x *.sh && git init -q && git add -A`""

		# /libhatchet is a tmpfs so HX_RAMDISK=1 builds in it instead of in /tmp.
		"wslc exec -e HX_RAMDISK=1 $name ./testall.sh"
	)
	($ignored -join ' & ') + ' & ' + ($required -join ' && ')
}

function Main([bool]$verbose, [bool]$debug) {
	if (Test-Path $build_dir) { Remove-Item -Recurse -Force $build_dir }
	if (Test-Path cache) { Remove-Item -Recurse -Force cache }
	New-Item -ItemType Directory -Force $build_dir | Out-Null

	# The container input bundle. ustar avoids the Windows specific pax headers.
	& "$env:SystemRoot\System32\tar.exe" -cf $build_dir\libhatchet.tar --format ustar `
		--exclude .git --exclude tmp --exclude $build_dir .

	$tests = [ordered]@{
		'x64-debug'     = Get-MsvcTest x64 Debug
		'x64-release'   = Get-MsvcTest x64 Release
		'win32-debug'   = Get-MsvcTest Win32 Debug
		'win32-release' = Get-MsvcTest Win32 Release
	}

	foreach ($distribution in $distributions.Keys) {
		$tests[$distribution] = Get-ContainerTest $distribution
	}

	$results = @()
	$failed = 0
	$processes = [ordered]@{}
	try {
		# The containers start first and run in parallel with the MSVC tests,
		# which run one at a time. The containers are waited for last.
		foreach ($test in @($distributions.Keys) + @($tests.Keys)) {
			if (-not $processes.Contains($test)) {
				Write-Host "=== $test ==="
				# A new process runs each test so that its output can be logged.
				$command = "/c ($($tests[$test]))"
				if (-not $verbose) { $command += " >$build_dir\$test.log 2>&1" }
				$processes[$test] = Start-Process cmd $command -NoNewWindow -PassThru
				# ExitCode is only available if Handle is read before the process exits.
				$processes[$test].Handle | Out-Null
				if ($distributions.Contains($test)) { continue }
			}

			$process = $processes[$test]
			$process.WaitForExit()
			$result = "$test passed"
			if ($process.ExitCode -ne 0) {
				$result = "$test failed with exit code $($process.ExitCode)"
				$failed++
			}
			$result += ' in ' + ('{0:h\:mm\:ss}' -f ($process.ExitTime - $process.StartTime))
			Write-Host $result
			$results += $result

			# Stop at the first MSVC failure.
			if ($process.ExitCode -ne 0 -and -not $distributions.Contains($test)) { break }
		}
	} finally {
		# Failed containers are stopped and kept for debugging.
		foreach ($distribution in $distributions.Keys) {
			if ($debug -or -not $processes.Contains($distribution)) { continue }
			if ($processes[$distribution].HasExited -and $processes[$distribution].ExitCode -eq 0) {
				wslc remove -f "libhatchet-test-$distribution" | Out-Null
			} else {
				wslc stop "libhatchet-test-$distribution" | Out-Null
			}
		}
	}

	Write-Host '=== results ==='
	$results | ForEach-Object { Write-Host $_ }
	if ($failed -eq 0 -and -not $debug) {
		Remove-Item -Recurse -Force $build_dir, cache
	}
	exit $failed
}

$verbose = $args -contains '--verbose'
$debug   = $args -contains '--debug'
if ($args.Count -ne [int]$verbose + [int]$debug) {
	Write-Host 'usage_error: testmsvc.ps1 [--verbose] [--debug]'
	exit 1
}
Main $verbose $debug
