# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Tests with MSVC and wslc.exe Linux containers in parallel. Caches pre-built
# containers with the required packages installed so iterative development
# doesn't hit the network. --clear-wslc removes them first.
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

# The memory needed by each parallel compile.
$memory_per_job_gb = 0.5

# Parallel compiles per core when there is enough memory.
$jobs_per_core = 1.5

$distributions = [ordered]@{
	debian = @{ Image = 'debian:13';                 Install = 'install_deb.sh' }
	mint   = @{ Image = 'linuxmintd/mint22.3-amd64'; Install = 'install_deb.sh' }
	rocky  = @{ Image = 'rockylinux/rockylinux:10';  Install = 'install_rpm.sh' }
	# The emsdk and Pico SDK downloads are only in the Ubuntu image.
	ubuntu = @{ Image = 'ubuntu:26.04';              Install = 'install_deb.sh'; Options = '--sdks' }
}

$build_dir = 'build'

function Get-MsvcTest([string]$platform, [string]$configuration, [bool]$verbose) {
	$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
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
		"cmake -S . -B $build_dir\$name -A $platform"
		# Single threaded because the containers take longer.
		"cmake --build $build_dir\$name --config $configuration --parallel 1"
		"cd $build_dir\$name"
		$run
	) -join ' && '
}

# Get-ContainerBuild - The image is built from the base image by its install
# script. Returns nothing if the image is current.
function Get-ContainerBuild([string]$distribution) {
	$image = $distributions[$distribution]
	$name = "libhatchet-test-$distribution"
	$tag = "libhatchet-test/$($image.Image)"
	$install = "$($image.Install) $($image.Options)".Trim()

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

		# A kept container prevents its image from being removed.
		"(wslc remove -f $name >nul 2>&1 & wslc rmi $tag >nul 2>&1" +
			" & wslc build --no-cache --progress plain -t $tag $context)"
	}
}

# Get-ContainerTest - Failed containers are kept, restarted and debugged by
# running "test_all.sh --verbose" in them.
function Get-ContainerTest([string]$distribution, [int]$jobs) {
	$image = $distributions[$distribution]
	$name = "libhatchet-test-$distribution"
	$tag = "libhatchet-test/$($image.Image)"
	$install = "$($image.Install) $($image.Options)".Trim()

	$required = @(
		# The init process exits on SIGTERM, stopping the container.
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

		# Confirms that the image has every package and nothing is installed.
		"wslc exec $name sh -c `"bash $install | tee /dev/stderr" +
			" | grep -q packages_already_installed`""

		# Executable bits are not tested.
		"wslc exec $name sh -c `"chmod +x *.sh`""

		# /libhatchet is a tmpfs so HX_RAMDISK=1 builds in it instead of in /tmp.
		# stdout and stderr are merged because wslc.
		"wslc exec -e HX_RAMDISK=1 -e HX_MAX_CORES=$jobs $name sh -c `"./test_all.sh 2>&1`""
	)
	"wslc remove -f $name >nul 2>&1 & " + ($required -join ' && ')
}

function Main([bool]$verbose, [bool]$clear_wslc) {
	$running = @(wslc ps --quiet)
	if ($running) {
		wslc ps
		Write-Host "error: wslc containers already running. Use: wslc stop $running"
		exit 1
	}

	if ($clear_wslc) {
		wslc ps --all --format json | ConvertFrom-Json |
			Where-Object Names -like 'libhatchet-test-*' |
			ForEach-Object { wslc remove -f $_.Names | Out-Null }
		wslc images --format json | ConvertFrom-Json |
			Where-Object Repository -like 'libhatchet-test/*' |
			ForEach-Object { wslc rmi "$($_.Repository):$($_.Tag)" | Out-Null }
	}

	if (Test-Path $build_dir) { Remove-Item -Recurse -Force $build_dir }
	if (Test-Path cache) { Remove-Item -Recurse -Force cache }
	New-Item -ItemType Directory -Force $build_dir | Out-Null

	# The container input bundle. ustar avoids the Windows specific pax headers.
	& "$env:SystemRoot\System32\tar.exe" -cf $build_dir\libhatchet.tar --format ustar `
		--exclude .git --exclude $build_dir --exclude-from .gitignore .

	# The jobs for the cores in the wslc session that the available memory allows
	# are split between the containers.
	$session = @('' | wslc system session run sh -c 'nproc; grep MemAvailable /proc/meminfo')
	$cores = [int]$session[0]
	$available_gb = [int64]($session[1] -replace '\D') / 1MB
	$container_jobs = [Math]::Max(1, [Math]::Floor([Math]::Min($cores * $jobs_per_core,
		$available_gb / $memory_per_job_gb) / $distributions.Count))
	Write-Host ('# test_msvc.ps1: {0} cores, {1:N1} GB available, using {2} jobs/container.' -f
		$cores, $available_gb, $container_jobs)

	# The tests are in the order they start. wslc fails with E_UNEXPECTED when
	# builds run concurrently, so the images are built one at a time first.
	$tests = [ordered]@{}
	$builds = @(foreach ($distribution in $distributions.Keys) { Get-ContainerBuild $distribution })
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
				# A new process runs each test so that its output can be logged.
				$command = "/c ($($tests[$test]))"
				if (-not $verbose) { $command += " >$build_dir\$test.log 2>&1" }
				$info = New-Object System.Diagnostics.ProcessStartInfo cmd, $command
				$info.UseShellExecute = $false
				# The containers watch stdin for this script exiting.
				$info.RedirectStandardInput = $distributions.Contains($test)
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

			# Stop at the first image build or MSVC failure.
			if ($process.ExitCode -ne 0 -and -not $distributions.Contains($test)) { break }
		}
	} finally {
		# Failed containers are stopped and kept for debugging.
		foreach ($distribution in $distributions.Keys) {
			if (-not $processes.Contains($distribution)) { continue }
			if ($processes[$distribution].HasExited -and $processes[$distribution].ExitCode -eq 0) {
				wslc remove -f "libhatchet-test-$distribution" | Out-Null
			} else {
				wslc stop "libhatchet-test-$distribution" | Out-Null
			}
		}
	}

	Write-Host '## results'
	$results | ForEach-Object { Write-Host $_ }
	exit $failed
}

$verbose = $args -contains '--verbose'
$clear_wslc = $args -contains '--clear-wslc'
if ($args.Count -ne [int]$verbose + [int]$clear_wslc) {
	Write-Host 'error: usage: test_msvc.ps1 [--verbose] [--clear-wslc]'
	exit 1
}
Main $verbose $clear_wslc
Write-Host '🪓🪓🪓'
