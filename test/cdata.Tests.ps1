Describe "CDATA" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="cdata.plist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAOUT0="$DATASRC/$TESTFILE.bin"
    
    & $plistutil -i $DATAIN0 -o $DATAOUT0
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plist_cmp $DATAIN0 $DATAOUT0
    It "plist_cmp should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}
