Describe "timezone1" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="7.plist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAOUT0="$DATASRC/$TESTFILE.tz0.bin"
    $DATAOUT1="$DATASRC/$TESTFILE.tz1.bin"
    $DATAOUT2="$DATASRC/$TESTFILE.tz2.bin"
    
    $ts = Get-TimeZone

    try
    {
        Set-TimeZone "GMT Standard Time"
        & $plistutil -i $DATAIN0 -o $DATAOUT0

        Set-TimeZone "Pacific Standard Time"
        & $plistutil -i $DATAIN0 -o $DATAOUT2
    }
    finally
    {
        Set-TimeZone $ts
    }

    & $plist_cmp $DATAOUT0 $DATAOUT2
    It "plist_cmp should succeed for timezone 2" {
        $LASTEXITCODE | Should Be 0
    }
}
