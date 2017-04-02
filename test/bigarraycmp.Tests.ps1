Describe "BigArray" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="6.plist"

    & $plist_cmp "$DATASRC/$TESTFILE" "$DATAOUT/$TESTFILE.out"
    It "plist_cmp should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}
