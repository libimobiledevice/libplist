Describe "Empty" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="1.plist"
    
    & $plist_test $DATASRC/$TESTFILE $DATAOUT/$TESTFILE.out
    It "plist_test should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plist_cmp $DATASRC/$TESTFILE $DATAOUT/$TESTFILE.out
    It "plist_cmp should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}
