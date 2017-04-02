Describe "order" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="order.bplist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAIN1="$DATASRC/order.plist"
    $DATAOUT0="$top_builddir/test/data/$TESTFILE.out"
    
    & $plistutil -i $DATAIN0 -o $DATAOUT0
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plist_cmp $DATAIN1 $DATAOUT0
    It "plist_cmp should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}