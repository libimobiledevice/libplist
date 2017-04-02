Describe "malformed_dict" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="malformed_dict.bplist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAOUT0="$top_builddir/test/data/$TESTFILE.out"
    
    & $plistutil -i $DATAIN0 -o $DATAOUT0
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}
