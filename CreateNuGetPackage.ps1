Param(
  [string]$build
)

Write-Host Changing build number to $build

# Update the build number
(gc .\libplist.autoconfig).replace('{build}', $build)|sc .\libplist.out.autoconfig

# Create the NuGet package
Import-Module "C:\Program Files (x86)\Outercurve Foundation\modules\CoApp"
Write-NuGetPackage .\libplist.out.autoconfig
