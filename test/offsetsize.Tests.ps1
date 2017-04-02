Describe "offsetsize" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILES=@("off1byte.bplist", "off2bytes.bplist", "off3bytes.bplist", "off4bytes.bplist", "off5bytes.bplist", "off6bytes.bplist", "off7bytes.bplist", "off8bytes.bplist")
    $CMPFILE="offxml.plist"
    
    ForEach ($TESTFILE in $TESTFILES)
    {
        & $plist_cmp $DATASRC/$TESTFILE $DATASRC/$CMPFILE
        It "plist_cmp should succeed for $TESTFILE" {
            $LASTEXITCODE | Should Be 0
        }
    }
}
