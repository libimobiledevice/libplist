Describe "BigArray" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="6.plist"
    & $plist_test $DATASRC/$TESTFILE $DATAOUT/$TESTFILE.out
    
    It "plist_test should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}