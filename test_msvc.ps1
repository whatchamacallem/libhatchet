# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Tests with MSVC and wslc.exe Linux containers in parallel. Caches pre-built
# libhatchet-cache-* images with the required packages installed so iterative
# development doesn't hit the network. The libhatchet-test-* containers that
# run the tests are removed at the start and end of every run. --clear-wslc
# removes both and exits without testing.
#
# usage: test_msvc.ps1 [--verbose] [--clear-wslc]
#
# The wslc session VM is limited to half of physical memory by default. 3/4 is
# recommended. For example, add this to %LOCALAPPDATA%\wslc\settings.yaml
# and then run "wslc system session terminate":
#
# session:
#   memorySize: 24GB

$ErrorActionPreference = 'Stop'

$memory_per_job_gb = 0.5
$jobs_per_core = 1.5

$distributions = [ordered]@{
	debian = @{ Image = 'debian:13';                 Install = 'install_deb.sh' }
	mint   = @{ Image = 'linuxmintd/mint22.3-amd64'; Install = 'install_deb.sh' }
	rocky  = @{ Image = 'rockylinux/rockylinux:10';  Install = 'install_rpm.sh' }
	ubuntu = @{ Image = 'ubuntu:26.04';              Install = 'install_deb.sh'; Options = '--sdks' }
}

$build_dir = 'build'

function Get-MsvcTest([string]$platform, [string]$configuration, [bool]$verbose) {
	$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
	if ($verbose) { Write-Host "+ `"$vswhere`" -latest -property installationPath" }
	$visual_studio = & $vswhere -latest -property installationPath
	$name = "$platform-$configuration".ToLower()

	$run = '(.\hxtest.exe >console_output.txt 2>&1 && findstr /l /c:"[  PASSED  ]"' +
		' console_output.txt || (type console_output.txt & exit 1))'
	if ($verbose) { $run = '.\hxtest.exe' }
	@(
		# VsDevCmd.bat runs vswhere.exe from its directory, which is not searched
		# when NoDefaultCurrentDirectoryInExePath is set.
		"set `"PATH=$(Split-Path $vswhere);%PATH%`""
		"call `"$visual_studio\Common7\Tools\VsDevCmd.bat`" -no_logo"
		# Idle MSBuild nodes can lock the build directory.
		'set "MSBUILDDISABLENODEREUSE=1"'
		"cmake -S . -B $build_dir\$name -A $platform"
		# Single threaded because the containers take longer.
		"cmake --build $build_dir\$name --config $configuration --parallel 1"
		"cd $build_dir\$name"
		$run
	) -join ' && '
}

function Get-ContainerBuild([string]$distribution, [bool]$verbose) {
	$image = $distributions[$distribution]
	$tag = "libhatchet-cache-$($image.Image)"
	$install = "$($image.Install) $($image.Options)".Trim()

	if ($verbose) { Write-Host "+ wslc images --format json --filter reference=$tag" }
	$listed = wslc images --format json --filter "reference=$tag" |
		ConvertFrom-Json
	if (-not $listed -or [DateTimeOffset]::Parse($listed.CreatedAt -replace ' \D+$') -lt
			(Get-Item $image.Install).LastWriteTime) {
		$context = "$build_dir\$distribution"
		New-Item -ItemType Directory $context | Out-Null
		Copy-Item $image.Install $context
		Set-Content "$context\Dockerfile" -NoNewline -Encoding ascii ("FROM $($image.Image)`n" +
			"RUN --mount=type=bind,source=$($image.Install),target=/$($image.Install)" +
			" bash /$install`n")

		"(wslc rmi $tag >nul 2>&1 & wslc build --no-cache --progress plain -t $tag $context)"
	}
}

function Get-ContainerTest([string]$distribution, [int]$jobs) {
	$image = $distributions[$distribution]
	$name = "libhatchet-test-$distribution"
	$tag = "libhatchet-cache-$($image.Image)"
	$install = "$($image.Install) $($image.Options)".Trim()

	@(
		"wslc run -d --name $name --hostname $distribution --tmpfs /libhatchet:exec" +
			" -w /libhatchet $tag sh -c `"trap exit TERM; sleep infinity & wait`" >nul"

		# Stops the container when this script exits and closes its stdin.
		"start /b wslc exec -i $name sh -c `"cat >/dev/null; kill 1`" >nul 2>&1"

		# ThreadSanitizer requires lower ASLR entropy than the wslc session VM default.
		# Containers cannot disable ASLR themselves. This does not persist. stdin is
		# piped because a null stdin handle intermittently fails with ERROR_INVALID_HANDLE.
		'echo.|wslc system session run sh -c "echo 28 >/proc/sys/vm/mmap_rnd_bits"'

		# wslc has no cp command. cmd.exe redirects binary data unmodified.
		"wslc exec -i $name tar -xf - --no-same-permissions -C /libhatchet <$build_dir\libhatchet.tar"

		"wslc exec $name sh -c `"bash $install | tee /dev/stderr" +
			" | grep -q packages_already_installed`""

		# Executable bits are not tested.
		"wslc exec $name sh -c `"chmod +x *.sh`""

		# /libhatchet is a tmpfs so HX_RAMDISK=1 builds in it instead of in /tmp.
		# stdout and stderr are merged because wslc.
		"wslc exec -e HX_RAMDISK=1 -e HX_MAX_JOBS=$jobs $name sh -c `"./test_all.sh 2>&1`""
	) -join ' && '
}

# The test containers belong to this script. Other containers are ignored.
function Remove-TestContainers([bool]$verbose) {
	if ($verbose) { Write-Host '+ wslc ps --all --format json' }
	wslc ps --all --format json | ConvertFrom-Json |
		Where-Object Names -like 'libhatchet-test-*' |
		ForEach-Object {
			if ($verbose) { Write-Host "+ wslc remove -f $($_.Names)" }
			wslc remove -f $_.Names | Out-Null
		}
}

function Stop-TestJobs([bool]$verbose) {
	Get-CimInstance Win32_Process | Where-Object {
		$_.Name -in 'cmd.exe', 'hxtest.exe', 'wslc.exe' -and
			$_.CommandLine -match 'hxtest\.exe|libhatchet-(cache|test)-'
	} | ForEach-Object {
		if ($verbose) { Write-Host "+ taskkill /f /t /pid $($_.ProcessId) # $($_.Name)" }
		cmd /c "taskkill /f /t /pid $($_.ProcessId) >nul 2>&1"
	}
}

function Test-MSVC([bool]$verbose, [bool]$clear_wslc) {
	Stop-TestJobs $verbose
	Remove-TestContainers $verbose

	if ($clear_wslc) {
		if ($verbose) { Write-Host '+ wslc images --format json' }
		wslc images --format json | ConvertFrom-Json |
			Where-Object Repository -like 'libhatchet-cache-*' |
			ForEach-Object {
				if ($verbose) { Write-Host "+ wslc rmi $($_.Repository):$($_.Tag)" }
				wslc rmi "$($_.Repository):$($_.Tag)" | Out-Null
			}
		exit 0
	}

	if (Test-Path $build_dir) { Remove-Item -Recurse -Force $build_dir }
	if (Test-Path cache) { Remove-Item -Recurse -Force cache }
	New-Item -ItemType Directory -Force $build_dir | Out-Null

	# ustar avoids the Windows specific pax headers.
	if ($verbose) {
		Write-Host ("+ `"$env:SystemRoot\System32\tar.exe`" -cf $build_dir\libhatchet.tar" +
			" --format ustar --exclude .git --exclude $build_dir --exclude-from .gitignore .")
	}
	& "$env:SystemRoot\System32\tar.exe" -cf $build_dir\libhatchet.tar --format ustar `
		--exclude .git --exclude $build_dir --exclude-from .gitignore .

	if ($verbose) { Write-Host "+ wslc system session run sh -c 'nproc; grep MemAvailable /proc/meminfo'" }
	$session = @('' | wslc system session run sh -c 'nproc; grep MemAvailable /proc/meminfo')
	$cores = [int]$session[0]
	$available_gb = [int64]($session[1] -replace '\D') / 1MB
	$container_jobs = [Math]::Max(1, [Math]::Floor([Math]::Min($cores * $jobs_per_core,
		$available_gb / $memory_per_job_gb) / $distributions.Count))
	Write-Host ('# test_msvc.ps1: {0} cores, {1:N1} GB available, using {2} jobs/container.' -f
		$cores, $available_gb, $container_jobs)

	# wslc fails with E_UNEXPECTED when builds run concurrently, so the images
	# are built one at a time first.
	$tests = [ordered]@{}
	$builds = @(foreach ($distribution in $distributions.Keys) { Get-ContainerBuild $distribution $verbose })
	if ($builds) { $tests['images'] = $builds -join ' && ' }
	foreach ($distribution in $distributions.Keys) {
		$tests[$distribution] = Get-ContainerTest $distribution $container_jobs
	}
	$tests['x64-debug']     = Get-MsvcTest x64 Debug $verbose
	$tests['x64-release']   = Get-MsvcTest x64 Release $verbose
	$tests['win32-debug']   = Get-MsvcTest Win32 Debug $verbose
	$tests['win32-release'] = Get-MsvcTest Win32 Release $verbose

	$results = @()
	$failed = 0
	$processes = [ordered]@{}
	try {
		# The containers run in parallel with the MSVC tests, which run one at
		# a time. The containers are waited for last.
		foreach ($test in @($tests.Keys) + @($distributions.Keys)) {
			if (-not $processes.Contains($test)) {
				Write-Host "## $test"
				$command = "/c ($($tests[$test]))"
				if (-not $verbose) { $command += " >$build_dir\$test.log 2>&1" }
				$info = New-Object System.Diagnostics.ProcessStartInfo cmd, $command
				$info.UseShellExecute = $false
				# Set-Location does not change the directory processes start in.
				$info.WorkingDirectory = $PWD.ProviderPath
				# The containers watch stdin for this script exiting.
				$info.RedirectStandardInput = $distributions.Contains($test)
				if ($verbose) { Write-Host "+ cmd $command" }
				$processes[$test] = [System.Diagnostics.Process]::Start($info)
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

			if ($process.ExitCode -ne 0 -and -not $distributions.Contains($test)) { break }
		}
	} finally {
		Remove-TestContainers $verbose
	}

	Write-Host '## results'
	$results | ForEach-Object { Write-Host $_ }
	exit $failed
}

function Main {
	$verbose = $args -contains '--verbose'
	$clear_wslc = $args -contains '--clear-wslc'
	if ($args.Count -ne [int]$verbose + [int]$clear_wslc) {
		Write-Host 'error: usage: test_msvc.ps1 [--verbose] [--clear-wslc]'
		exit 1
	}

	$mutex = [System.Threading.Mutex]::new($false, 'libhatchet-test-msvc')
	$owned = $false
	try {
		try { $owned = $mutex.WaitOne(0) } catch [System.Threading.AbandonedMutexException] { $owned = $true }
		if (-not $owned) {
			Write-Host 'error: test_msvc.ps1 is already running.'
			exit 1
		}
		Test-MSVC $verbose $clear_wslc
	} finally {
		if ($owned) { $mutex.ReleaseMutex() }
		$mutex.Dispose()
	}
	Write-Host '🪓🪓🪓'
}

Main @args
