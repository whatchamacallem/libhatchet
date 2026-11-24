# SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
# SPDX-License-Identifier: MIT
# This file is licensed under the terms of the LICENSE.md file.
#
# Tests with MSVC and wslc.exe Linux containers in parallel. Caches pre-built
# libhatchet-cache-* images with the required packages installed so iterative
# development doesn't hit the network. The libhatchet-test-* containers that
# run the tests are removed at the start and end of every run. --clear-wslc
# removes both and exits without testing. --clone-sdks creates a container with
# the given name from the --sdks image, building the image first if it is not
# ready. The clone has the test containers' settings, can attach USB devices,
# and runs bash as t. --run runs only the MSVC tests or only the tests in one
# distribution's container.
#
# The images run as user t, with any SDKs in /home/t, and can be used directly:
#   wslc run -it --cap-add SYS_ADMIN libhatchet-cache-ubuntu:26.04
#
# USB devices are shared from Windows by usbipd-win. Share each device once
# from an Administrator prompt with "usbipd bind --busid <busid>". In the clone,
# or the --sdks image run as above with --cap-add SYS_MODULE, t can attach it:
#   usbip list -r windows
#   usbip attach -r windows -b <busid>
# "usbip port" lists the attached devices and "usbip detach -p <port>" detaches
# one. A device that re-enumerates, such as a Pico rebooting into BOOTSEL, has
# to be attached again. "windows" is the WSL adapter's address when the clone
# was created, which -e HX_WINDOWS_IP=<address> sets with wslc run.
#
# usage: test_msvc_wslc.ps1 [--verbose] [--clear-wslc | --clone-sdks <container-name> |
#   --run <msvc|debian|mint|rocky|ubuntu>]
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

	$context = "$build_dir\$distribution"
	New-Item -ItemType Directory $context | Out-Null

	# target\wslc_entry.sh is the entrypoint. It prepares the container for
	# ThreadSanitizer and USB/IP and then runs the command as t.
	Copy-Item $image.Install, target\wslc_entry.sh $context

	# The tests run as t. --sdks installs to ~, so the install runs with t's
	# HOME and then t is given ownership.
	Set-Content "$context\Dockerfile" -NoNewline -Encoding ascii ((@(
		"FROM $($image.Image)"
		# Rocky Linux might not include these.
		'RUN command -v setpriv && command -v useradd || dnf install -y util-linux shadow-utils'
		'RUN useradd -m -s /bin/bash t'
		"RUN --mount=type=bind,source=$($image.Install),target=/$($image.Install)" +
			" HOME=/home/t bash /$install && chown -R t:t /home/t"
		'COPY wslc_entry.sh /usr/local/bin/libhatchet-entrypoint.sh'
		'WORKDIR /home/t'
		'ENTRYPOINT ["sh", "/usr/local/bin/libhatchet-entrypoint.sh"]'
		'CMD ["bash"]'
	) -join "`n") + "`n")

	# The image is rebuilt when the hash of its build context changes.
	$hashes = (Get-ChildItem $context -File | Sort-Object Name | Get-FileHash).Hash -join ''
	$hash = (Get-FileHash -InputStream ([IO.MemoryStream]::new(
		[Text.Encoding]::ASCII.GetBytes($hashes)))).Hash.ToLower()

	if ($verbose) { Write-Host "+ wslc images --format json --filter reference=$tag" }
	if (wslc images --format json --filter "reference=$tag" | ConvertFrom-Json) {
		if ($verbose) { Write-Host "+ wslc image inspect --format json $tag" }
		$labels = (wslc image inspect --format json $tag | ConvertFrom-Json).Config.Labels
		if ($labels.'libhatchet.hash' -eq $hash) { return }
	}

	"(wslc rmi $tag >nul 2>&1 & wslc build --pull --no-cache --progress plain" +
		" -l libhatchet.hash=$hash -t $tag $context)"
}

# The container settings shared by the test containers and --clone-sdks.
function Get-ContainerOptions([string]$distribution) {
	# Windows ~/Downloads is mounted at its WSL path, e.g. /mnt/c/Users/name/Downloads.
	$downloads = "$env:USERPROFILE\Downloads" -replace '\\', '/'
	$downloads_mount = '/mnt/' + $downloads.Substring(0, 1).ToLower() + $downloads.Substring(2)

	# SYS_PTRACE is required by LeakSanitizer (ASan). PERFMON is for perf.
	# SYS_ADMIN is for the image's entrypoint, which drops it.
	"--hostname $distribution -v `"${downloads}:$downloads_mount`"" +
		" --cap-add PERFMON --cap-add SYS_ADMIN --cap-add SYS_PTRACE"
}

function Get-ContainerTest([string]$distribution, [int]$jobs) {
	$image = $distributions[$distribution]
	$name = "libhatchet-test-$distribution"
	$tag = "libhatchet-cache-$($image.Image)"
	$install = "$($image.Install) $($image.Options)".Trim()

	$source = '/home/t/libhatchet'

	@(
		"wslc run -d --name $name $(Get-ContainerOptions $distribution) --tmpfs ${source}:exec" +
			" -w $source $tag sh -c `"trap exit TERM; sleep infinity & wait`" >nul"

		# Stops the container when this script exits and closes its stdin.
		"start /b wslc exec -i $name sh -c `"cat >/dev/null; kill 1`" >nul 2>&1"

		# The tarball skips .git and the ignored files, which wslc cp would copy.
		# cmd.exe redirects binary data unmodified.
		"wslc exec -i $name tar -xf - --no-same-permissions -C $source <$build_dir\libhatchet.tar"

		# The tests run as t, not root.
		"wslc exec $name chown -R t:t $source"

		"wslc exec -u t $name sh -c `"bash $install | tee /dev/stderr" +
			" | grep -q packages_already_installed`""

		# Executable bits are not tested.
		"wslc exec -u t $name sh -c `"chmod +x *.sh`""

		# The source is in a tmpfs so HX_RAMDISK=1 builds in it instead of in /tmp.
		# stdout and stderr are merged because wslc.
		"wslc exec -u t -e HX_RAMDISK=1 -e HX_MAX_JOBS=$jobs $name sh -c `"./test_all.sh 2>&1`""
	) -join ' && '
}

# Runs a cmd.exe command line in the current directory.
function Start-Cmd([string]$command, [bool]$redirect_input, [bool]$verbose) {
	$info = New-Object System.Diagnostics.ProcessStartInfo cmd, $command
	$info.UseShellExecute = $false
	# Set-Location does not change the directory processes start in.
	$info.WorkingDirectory = $PWD.ProviderPath
	$info.RedirectStandardInput = $redirect_input
	if ($verbose) { Write-Host "+ cmd $command" }
	[System.Diagnostics.Process]::Start($info)
}

# Creates a container from the --sdks image, building the image first if it is
# not ready. SYS_MODULE is for the image's entrypoint, which sets up USB/IP and
# drops it. The containers reach Windows at the WSL adapter's address, which the
# entrypoint names windows.
function New-SdksClone([string]$clone, [bool]$verbose) {
	$distribution = @($distributions.Keys | Where-Object { $distributions[$_].Options -eq '--sdks' })[0]
	$tag = "libhatchet-cache-$($distributions[$distribution].Image)"
	$build = @(Get-ContainerBuild $distribution $verbose)

	if ($verbose) { Write-Host "+ Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias 'vEthernet (WSL*'" }
	$windows = @(Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias 'vEthernet (WSL*' `
		-ErrorAction SilentlyContinue)[0].IPAddress
	$options = '--cap-add SYS_MODULE'
	if ($windows) {
		$options += " -e HX_WINDOWS_IP=$windows"
	} else {
		Write-Host 'warning: The WSL adapter was not found. usbip will need the address of Windows.'
	}

	$commands = @($build +
		"wslc create -it --name $clone $(Get-ContainerOptions $distribution) $options $tag") -join ' && '

	$process = Start-Cmd "/c ($commands)" $false $verbose
	$process.WaitForExit()
	$process.ExitCode
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

# Interactive containers using the libhatchet-cache-* images are left running.
function Stop-TestJobs([bool]$verbose) {
	Get-CimInstance Win32_Process | Where-Object {
		$_.Name -in 'cmd.exe', 'hxtest.exe', 'wslc.exe' -and
			$_.CommandLine -match 'hxtest\.exe|libhatchet-test-|wslc\s+build\s.*libhatchet-cache-'
	} | ForEach-Object {
		if ($verbose) { Write-Host "+ taskkill /f /t /pid $($_.ProcessId) # $($_.Name)" }
		cmd /c "taskkill /f /t /pid $($_.ProcessId) >nul 2>&1"
	}
}

function Test-MSVC([bool]$verbose, [bool]$clear_wslc, [string]$clone, [string]$run) {
	Stop-TestJobs $verbose
	Remove-TestContainers $verbose

	if ($clear_wslc) {
		$failed = 0
		if ($verbose) { Write-Host '+ wslc images --format json' }
		wslc images --format json | ConvertFrom-Json |
			Where-Object Repository -like 'libhatchet-cache-*' |
			ForEach-Object {
				if ($verbose) { Write-Host "+ wslc rmi $($_.Repository):$($_.Tag)" }
				wslc rmi "$($_.Repository):$($_.Tag)" | Out-Null
				if ($LASTEXITCODE -ne 0) { $failed++ }
			}
		return $failed
	}

	if (Test-Path $build_dir) { Remove-Item -Recurse -Force $build_dir }
	if (Test-Path cache) { Remove-Item -Recurse -Force cache }
	New-Item -ItemType Directory -Force $build_dir | Out-Null

	if ($clone) { return New-SdksClone $clone $verbose }

	# --run selects either the MSVC tests or one container. Otherwise all run.
	$containers = @($distributions.Keys | Where-Object { -not $run -or $_ -eq $run })
	$msvc = -not $run -or $run -eq 'msvc'

	$tests = [ordered]@{}
	if ($containers) {
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
			$available_gb / $memory_per_job_gb) / $containers.Count))
		Write-Host ('# test_msvc_wslc.ps1: {0} cores, {1:N1} GB available, using {2} jobs/container.' -f
			$cores, $available_gb, $container_jobs)

		# wslc fails with E_UNEXPECTED when builds run concurrently, so the
		# images are built one at a time first.
		$builds = @(foreach ($distribution in $containers) { Get-ContainerBuild $distribution $verbose })
		if ($builds) { $tests['images'] = $builds -join ' && ' }
		foreach ($distribution in $containers) {
			$tests[$distribution] = Get-ContainerTest $distribution $container_jobs
		}
	}
	if ($msvc) {
		$tests['x64-debug']     = Get-MsvcTest x64 Debug $verbose
		$tests['x64-release']   = Get-MsvcTest x64 Release $verbose
		$tests['win32-debug']   = Get-MsvcTest Win32 Debug $verbose
		$tests['win32-release'] = Get-MsvcTest Win32 Release $verbose
	}

	$results = @()
	$failed = 0
	$processes = [ordered]@{}
	try {
		# The containers run in parallel with the MSVC tests, which run one at
		# a time. The containers are waited for last.
		foreach ($test in @($tests.Keys) + $containers) {
			if (-not $processes.Contains($test)) {
				Write-Host "## $test"
				$command = "/c ($($tests[$test]))"
				if (-not $verbose) { $command += " >$build_dir\$test.log 2>&1" }
				# The containers watch stdin for this script exiting.
				$processes[$test] = Start-Cmd $command $distributions.Contains($test) $verbose
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
	$failed
}

function Main {
	$runs = @('msvc') + @($distributions.Keys)
	$usage = 'error: usage: test_msvc_wslc.ps1 [--verbose] [--clear-wslc |' +
		" --clone-sdks <container-name> | --run <$($runs -join '|')>]"

	$options = @{}
	for ($i = 0; $i -lt $args.Count; $i++) {
		$option = "$($args[$i])"
		if ($option -cnotin '--verbose', '--clear-wslc', '--clone-sdks', '--run' -or
				$options.Contains($option)) {
			Write-Host $usage
			exit 1
		}
		$options[$option] = $true
		if ($option -in '--clone-sdks', '--run') {
			$options[$option] = "$($args[++$i])"
			if ($options[$option] -match '^(--|$)') {
				Write-Host $usage
				exit 1
			}
		}
	}
	$verbose = [bool]$options['--verbose']
	$clear_wslc = [bool]$options['--clear-wslc']
	$clone = "$($options['--clone-sdks'])"
	$run = "$($options['--run'])"
	if ([int]$clear_wslc + [int][bool]$clone + [int][bool]$run -gt 1 -or
			($run -and $run -cnotin $runs)) {
		Write-Host $usage
		exit 1
	}

	$mutex = [System.Threading.Mutex]::new($false, 'libhatchet-test-msvc')
	$owned = $false
	try {
		try { $owned = $mutex.WaitOne(0) } catch [System.Threading.AbandonedMutexException] { $owned = $true }
		if (-not $owned) {
			Write-Host 'error: test_msvc_wslc.ps1 is already running.'
			exit 1
		}
		$exit_code = Test-MSVC $verbose $clear_wslc $clone $run
	} finally {
		if ($owned) { $mutex.ReleaseMutex() }
		$mutex.Dispose()
	}

	if ($exit_code -ne 0) { exit $exit_code }
	Write-Host '🪓🪓🪓'
}

Main @args
