Describe "signedunsigned1" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE0="signed.plist"
    $TESTFILE1="unsigned.plist"
    $DATAIN0="$DATASRC/$TESTFILE0"
    $DATAIN1="$DATASRC/$TESTFILE1"

    $CMPFILE0="signed.bplist"
    $CMPFILE1="unsigned.bplist"
    $DATACMP0="$DATASRC/$CMPFILE0"
    $DATACMP1="$DATASRC/$CMPFILE1"

    $DATAOUT0="$DATASRC/$TESTFILE0.bin"
    $DATAOUT1="$DATASRC/$TESTFILE1.bin"
    
    & $plistutil -i $DATAIN0 -o $DATAOUT0
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }

    & $plistutil -i $DATAIN1 -o $DATAOUT1
    It "plistutil should succeed" {
        $LASTEXITCODE | Should Be 0
    }
}
