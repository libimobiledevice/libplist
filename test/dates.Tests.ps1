Describe "Dates" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="7.plist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAOUT0="$top_builddir/test/data/$TESTFILE.bplist"
    $DATAOUT1="$top_builddir/test/data/$TESTFILE.xplist"
    

    & $plistutil -i $DATAIN0 -o $DATAOUT0
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plistutil -i $DATAOUT0 -o $DATAOUT1
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plist_cmp $DATAOUT0 $DATAIN0
    It "plist_cmp should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plist_cmp $DATAOUT1 $DATAIN0
    It "plist_cmp should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}
