Describe "refsize" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILES=@("dictref1byte.bplist", "dictref2bytes.bplist", "dictref3bytes.bplist", "dictref4bytes.bplist", "dictref5bytes.bplist", "dictref6bytes.bplist", "dictref7bytes.bplist", "dictref8bytes.bplist")
    $CMPFILE="offxml.plist"
    
    ForEach ($TESTFILE in $TESTFILES)
    {
        & $plist_cmp $DATASRC/$TESTFILE $DATASRC/$CMPFILE
        It "plist_cmp should succeed for $TESTFILE" {
            $LASTEXITCODE | Should Be 0
        }
    }
}
