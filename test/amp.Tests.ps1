# Opens a plist file with an invalid XML entity (&amp instead of &amp; the trailing
# ; is missing). Makes sure that plistutil does not process this file.

Describe "FileWithAmpersand" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="amp.plist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAOUT0="$top_builddir/test/data/$TESTFILE.out"

    & $plistutil -i $DATAIN0 -o $DATAOUT0

    It "File should not exist" {
        Test-Path $DATAOUT0 | Should Be $False
    }
}