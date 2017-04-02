Describe "timezone1" {
    & "$PSScriptRoot\tests.ps1"

    $TESTFILE="7.plist"
    $DATAIN0="$DATASRC/$TESTFILE"
    $DATAOUT0="$DATASRC/$TESTFILE.bin"
    $DATAOUT1="$DATASRC/$TESTFILE.tz0.xml"
    $DATAOUT2="$DATASRC/$TESTFILE.tz1.xml"
    $DATAOUT3="$DATASRC/$TESTFILE.tz2.xml"
    
    $ts = Get-TimeZone

    try
    {
        Set-TimeZone "GMT Standard Time"
        & $plistutil -i $DATAIN0 -o $DATAOUT0
        & $plistutil -i $DATAOUT0 -o $DATAOUT1

        Set-TimeZone "Pacific Standard Time"
        & $plistutil -i $DATAOUT0 -o $DATAOUT3
    }
    finally
    {
        Set-TimeZone $ts
    }

    & $plist_cmp $DATAIN0 $DATAOUT1
    It "plist_cmp should succeed for timezone 1" {
        $LASTEXITCODE | Should Be 0
    }

    & $plist_cmp $DATAIN0 $DATAOUT3
    It "plist_cmp should succeed for timezone 2" {
        $LASTEXITCODE | Should Be 0
    }
}
