Attribute VB_Name = "DIO64"

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  The file "dio64_32.dll" must be in the systems32 folder.
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Base 0

Public Const scan As Double = 10000000#
'THE BELOW WAS A FIX FOR A PROBLEM SEEN IN EARLIER INSTANTIATIONS OF THIS CODE; IT IS NO LONGER AN ISSUE,
'BUT WE KEEP IT HERE FOR THE POSSIBILITY OF FUTURE REFERENCE - AMK 08/2010
'Public Const ClockFix As Double = 1 / 1.2000413  'this is a factor to fix the clock's problem
                                                 'correction is only perfect for multiples of 1.2micro sec
Public Const ClockFix As Double = 1

'TTL Channel Constants (mnemonics for the user and default values)
Public Const Azero As Integer = 0, Azero0 As Boolean = False
Public Const Aone As Integer = 1, Aone0 As Boolean = False
Public Const Atwo As Integer = 2, Atwo0 As Boolean = False
Public Const Athree As Integer = 3, Athree0 As Boolean = False
Public Const Afour As Integer = 4, Afour0 As Boolean = False
Public Const Afive As Integer = 5, Afive0 As Boolean = False
Public Const Asix As Integer = 6, Asix0 As Boolean = False
Public Const Aseven As Integer = 7, Aseven0 As Boolean = False
Public Const Aeight As Integer = 8, Aeight0 As Boolean = False
Public Const Anine As Integer = 9, Anine0 As Boolean = False
Public Const Aten As Integer = 10, Aten0 As Boolean = False
Public Const Aeleven As Integer = 11, Aeleven0 As Boolean = False
Public Const Atwelve As Integer = 12, Atwelve0 As Boolean = False
Public Const Athirteen As Integer = 13, Athirteen0 As Boolean = False
Public Const Afourteen As Integer = 14, Afourteen0 As Boolean = False
Public Const Afifteen As Integer = 15, Afifteen0 As Boolean = False

Public Const Bzero As Integer = 100, Bzero0 As Boolean = False
Public Const Bone As Integer = 101, Bone0 As Boolean = False
Public Const Btwo As Integer = 102, Btwo0 As Boolean = False
Public Const Bthree As Integer = 103, Bthree0 As Boolean = False
Public Const Bfour As Integer = 104, Bfour0 As Boolean = False
Public Const Bfive As Integer = 105, Bfive0 As Boolean = False
Public Const Bsix As Integer = 106, Bsix0 As Boolean = False
Public Const Bseven As Integer = 107, Bseven0 As Boolean = False
Public Const Beight As Integer = 108, Beight0 As Boolean = False
Public Const Bnine As Integer = 109, Bnine0 As Boolean = False
Public Const Bten As Integer = 110, Bten0 As Boolean = False
Public Const Beleven As Integer = 111, Beleven0 As Boolean = False
Public Const Btwelve As Integer = 112, Btwelve0 As Boolean = False
Public Const Bthirteen As Integer = 113, Bthirteen0 As Boolean = False
Public Const Bfourteen As Integer = 114, Bfourteen0 As Boolean = False
Public Const Bfifteen As Integer = 115, Bfifteen0 As Boolean = False

Public Const Czero As Integer = 200, Czero0 As Boolean = False
Public Const Cone As Integer = 201, Cone0 As Boolean = False
Public Const Ctwo As Integer = 202, Ctwo0 As Boolean = False
Public Const Cthree As Integer = 203, Cthree0 As Boolean = False
Public Const Cfour As Integer = 204, Cfour0 As Boolean = False
Public Const Cfive As Integer = 205, Cfive0 As Boolean = False
Public Const Csix As Integer = 206, Csix0 As Boolean = False
Public Const Cseven As Integer = 207, Cseven0 As Boolean = False
Public Const Ceight As Integer = 208, Ceight0 As Boolean = False
Public Const Cnine As Integer = 209, Cnine0 As Boolean = False
Public Const Cten As Integer = 210, Cten0 As Boolean = False
Public Const Celeven As Integer = 211, Celeven0 As Boolean = False
Public Const Ctwelve As Integer = 212, Ctwelve0 As Boolean = False
Public Const Cthirteen As Integer = 213, Cthirteen0 As Boolean = False
Public Const Cfourteen As Integer = 214, Cfourteen0 As Boolean = False
Public Const Cfifteen As Integer = 215, Cfifteen0 As Boolean = False

Public Const Dzero As Integer = 300, Dzero0 As Boolean = False
Public Const Done As Integer = 301, Done0 As Boolean = False
Public Const Dtwo As Integer = 302, Dtwo0 As Boolean = False
Public Const Dthree As Integer = 303, Dthree0 As Boolean = False
Public Const Dfour As Integer = 304, Dfour0 As Boolean = False
Public Const Dfive As Integer = 305, Dfive0 As Boolean = False
Public Const Dsix As Integer = 306, Dsix0 As Boolean = False
Public Const Dseven As Integer = 307, Dseven0 As Boolean = False
Public Const Deight As Integer = 308, Deight0 As Boolean = False
Public Const Dnine As Integer = 309, Dnine0 As Boolean = False
Public Const Dten As Integer = 310, Dten0 As Boolean = False
Public Const Deleven As Integer = 311, Deleven0 As Boolean = False
Public Const Dtwelve As Integer = 312, Dtwelve0 As Boolean = False
Public Const Dthirteen As Integer = 313, Dthirteen0 As Boolean = False
Public Const Dfourteen As Integer = 314, Dfourteen0 As Boolean = False
Public Const Dfifteen As Integer = 315, Dfifteen0 As Boolean = False

Public Const aazero As Integer = 8
Public Const aaone As Integer = 9
Public Const aatwo As Integer = 10
Public Const aathree As Integer = 11
Public Const aafour As Integer = 12
Public Const aafive As Integer = 13
Public Const aasix As Integer = 14
Public Const aaseven As Integer = 15
Public Const bbzero As Integer = 108
Public Const bbone As Integer = 109
Public Const bbtwo As Integer = 110
Public Const bbthree As Integer = 111
Public Const bbfour As Integer = 112
Public Const bbfive As Integer = 113
Public Const bbsix As Integer = 114
Public Const bbseven As Integer = 115
Public Const cczero As Integer = 208
Public Const ccone As Integer = 209
Public Const cctwo As Integer = 210
Public Const ccthree As Integer = 211
Public Const ccfour As Integer = 212
Public Const ccfive As Integer = 213
Public Const ccsix As Integer = 214
Public Const ccseven As Integer = 215

'Constants
Const DIO64_ATTR_INPUTMODE                                                                      As Integer = 0
Const DIO64_ATTR_OUTPUTMODE                                                                     As Integer = 1
Const DIO64_ATTR_INPUTBUFFERSIZE                                                As Integer = 2
Const DIO64_ATTR_OUTPUTBUFFERSIZE                                               As Integer = 3
Const DIO64_ATTR_MAJORCLOCKSOURCE                                               As Integer = 4
Const DIO64_ATTR_INPUTTHRESHOLD                                                 As Integer = 5
Const DIO64_ATTR_OUTPUTTHRESHOLD                                                As Integer = 6
Const DIO64_ATTR_INPUTTIMEOUT                                                           As Integer = 7
Const DIO64_ATTR_RTSIGLOBALENABLE                                               As Integer = 8
Const DIO64_ATTR_RTSICLKSOURCE                                                  As Integer = 9
Const DIO64_ATTR_RTSICLKTRIG7ENABLE                                             As Integer = 10
Const DIO64_ATTR_EXTERNALCLKENABLE                                              As Integer = 11
Const DIO64_ATTR_PXICLKENABLE                                                           As Integer = 12
Const DIO64_ATTR_RTSISCANCLKTRIG0ENABLE                                         As Integer = 13
Const DIO64_ATTR_RTSISTARTTRIG2ENABLE                                           As Integer = 14
Const DIO64_ATTR_RTSISTOPTRIG3ENABLE                                            As Integer = 15
Const DIO64_ATTR_RTSIMODSCANCLKTRIG4ENABLE                                      As Integer = 16
Const DIO64_ATTR_PXISTARENABLE                                                  As Integer = 17
Const DIO64_ATTR_PORTROUTING                                                    As Integer = 18
Const DIO64_ATTR_STATICOUTPUTMASK                                               As Integer = 19
Const DIO64_ATTR_SERIALNUMBER                                                   As Integer = 20
Const DIO64_ATTR_ARMREENABLE                                                    As Integer = 21
Const DIO64_ATTR_SCLKENABLE                                                     As Integer = 22
Const DIO64_ATTR_FPGAINFO                                                       As Integer = 23

Const DIO64_ERR_ILLEGALBOARD                                                    As Integer = -8
Const DIO64_ERR_BOARDNOTOPENED                                                  As Integer = -9
Const DIO64_ERR_STATUSOVERRUNUNDERRUN                                           As Integer = -10
Const DIO64_ERR_INVALIDPARAMETER                                                As Integer = -12
Const DIO64_ERR_NODRIVERINTERFACE                                               As Integer = -13
Const DIO64_ERR_OCXOOPTIONNA                                                    As Integer = -14
Const DIO64_ERR_PXIONLYSIGNALS                                                  As Integer = -15
Const DIO64_ERR_STOPTRIGSRCINVALID                                              As Integer = -16
Const DIO64_ERR_PORTNUMBERCONFLICTS                                             As Integer = -17
Const DIO64_ERR_MISSINGDIO64CATFILE                                             As Integer = -18
Const DIO64_ERR_NOTENOUGHRESOURCES                                              As Integer = -19
Const DIO64_ERR_INVALIDSIGNITUREDIO64CAT                                        As Integer = -20
Const DIO64_ERR_REQUIREDIMAGENOTFOUND                                           As Integer = -21
Const DIO64_ERR_ERRORprogFPGA                                                   As Integer = -22
Const DIO64_ERR_FILENOTFOUND                                                    As Integer = -23
Const DIO64_ERR_BOARDERROR                                                      As Integer = -24
Const DIO64_ERR_FUNCTIONCALLINVALID                                             As Integer = -25
Const DIO64_ERR_NOTENOUGHTRANS                                                  As Integer = -26

Type DIO64STAT
   pktsize As Integer
   portCount As Integer
   writePtr As Integer
   readPtr As Integer
   time As Long
   fifoSize As Long
   fifo0 As Integer
   ticks As Long
   flags As Integer
   clkControl As Integer
   startControl As Integer
   stopControl As Integer
   AIControl As Long
   AICurrent As Integer
   starttime As Long
   stopTime As Long
   user0 As Integer
   user1 As Integer
   user2 As Integer
   user3 As Integer
End Type

'dll Library Functions

Declare Function DIO64_Open Lib "dio64_32.dll" (ByVal board As Integer, ByVal baseio As Integer) As Integer
Declare Function DIO64_Load Lib "dio64_32.dll" (ByVal board As Integer, ByVal bnmFile As String, ByVal InputHint As Integer, ByVal OutputHint As Integer) As Integer
Declare Function DIO64_Close Lib "dio64_32.dll" (ByVal board As Integer) As Integer
Declare Function DIO64_In_Start Lib "dio64_32.dll" (ByVal board As Integer, ByVal ticks As Long, ByRef mask As Integer, ByVal maskLength As Integer, ByVal flags As Integer, ByVal clkControl As Integer, ByVal starttype As Integer, ByVal startsource As Integer, ByVal stopType As Integer, ByVal stopsource As Integer, ByVal AIControl As Long, ByRef scanrate As Double) As Integer
Declare Function DIO64_In_Status Lib "dio64_32.dll" (ByVal board As Integer, ByRef scansavail As Long, ByRef stat As DIO64STAT) As Integer
Declare Function DIO64_In_Read Lib "dio64_32.dll" (ByVal board As Integer, ByRef buffer As Any, ByVal scansToRead As Long, ByRef stat As DIO64STAT) As Integer
Declare Function DIO64_In_Stop Lib "dio64_32.dll" (ByVal board As Integer) As Integer
Declare Function DIO64_Out_ForceOutput Lib "dio64_32.dll" (ByVal board As Integer, ByRef buffer As Integer, ByVal mask As Long) As Integer
Declare Function DIO64_Out_GetInput Lib "dio64_32.dll" (ByVal board As Integer, ByRef buffer As Integer) As Integer
Declare Function DIO64_Out_Config Lib "dio64_32.dll" (ByVal board As Integer, ByVal ticks As Long, ByRef mask As Integer, ByVal maskLength As Integer, ByVal flags As Integer, ByVal clkControl As Integer, ByVal starttype As Integer, ByVal startsource As Integer, ByVal stopType As Integer, ByVal stopsource As Integer, ByVal AIControl As Long, ByVal reps As Integer, ByVal ntrans As Integer, ByRef scanrate As Double) As Integer
Declare Function DIO64_Out_Start Lib "dio64_32.dll" (ByVal board As Integer) As Integer
Declare Function DIO64_Out_Status Lib "dio64_32.dll" (ByVal board As Integer, ByRef scansavail As Long, ByRef stat As DIO64STAT) As Integer
Declare Function DIO64_Out_Write Lib "dio64_32.dll" (ByVal board As Integer, ByRef buffer As Integer, ByVal bufsize As Long, ByRef stat As DIO64STAT) As Integer
Declare Function DIO64_Out_Stop Lib "dio64_32.dll" (ByVal board As Integer) As Integer

Declare Function DIO64_SetAttr Lib "dio64_32.dll" (ByVal board As Integer, ByVal attrID As Integer, ByVal VALUE As Integer) As Integer
Declare Function DIO64_GetAttr Lib "dio64_32.dll" (ByVal board As Integer, ByRef attrID As Integer, ByRef VALUE As Integer) As Integer

Public Sub DIOopen()
'Open and Load DIO64

Dim output As Integer

'Dim outputModeTemp, outputMode, outputMode2, outputModetemp2
'outputModeTemp = 1
'outputModetemp2 = 20

'outputMode = outputModeTemp
'outputMode2 = outputModetemp2

Dim filename As String
Dim maskvalue(3) As Integer
Dim i As Integer
Dim attr_val As Integer

filename = CStr("")
maskvalue(0) = -1
maskvalue(1) = -1
maskvalue(2) = -1
maskvalue(3) = -1

output = DIO64_Open(0, 0)
If output <> 0 And errormsgflag Then Debug.Print "Open Failed:" & output

output = DIO64_Load(0, filename, 0, 4)
If output <> 0 And errormsgflag Then Debug.Print "Load Failed:" & output

output = DIO64_Out_Config(0, 0, maskvalue(0), 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, scan)
If output <> 0 And errormsgflag Then Debug.Print "Output Configure Failed:" & output

'output = DIO64_GetAttr(0, outputMode, outputMode2)
'printnow ("Mode:" + CStr(outputMode2))
'pause
'output = DIO64_SetAttr(0, 4, 2) 'for PXI jjz
'if output <> 0 and errormsgflag Then Debug.Print "Set Attribute Failed:" & output


End Sub

Public Sub DIOwrite(ByVal buffsize As Integer)

	'Write to DIO board
	'Called by qprep

	Dim output As Integer
	Dim availablescans As Long
	Dim status As DIO64STAT

	Dim maskvalue(3) As Integer

	maskvalue(0) = -1
	maskvalue(1) = -1
	maskvalue(2) = -1
	maskvalue(3) = -1

	output = DIO64_Out_Stop(0)
	If output <> 0 And errormsgflag Then Debug.Print "Output Stop (in DIOwrite) Failed:" & output

	'Public Declare Function DIO64_Out_Config Lib "dio64_32.dll"  Alias "DIO64_Out_Config" _
	'   (ByVal board As Integer, ByVal ticks As Long, mask As Integer, ByVal maskLength As Integer, _
	'   ByVal flags As Integer, ByVal clkControl As Integer, ByVal starttype As Integer, _
	'   ByVal startsource As Integer, ByVal stopType As Integer, ByVal stopsource As Integer, _
	'   ByVal AIControl As Long, ByVal reps As Integer, ByVal ntrans As Integer, scanrate As Double) As Integer

	output = DIO64_Out_Config(0, 0, maskvalue(0), 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, scan)
	If output <> 0 And errormsgflag Then Debug.Print "Output Configure Failed2:" & output

	output = DIO64_Out_Status(0, availablescans, status)
	If output <> 0 And errormsgflag Then Debug.Print "Output Status Failed:" & output

	output = DIO64_Out_Write(0, TTLdatatoboard(0, 0), buffsize, status)
	If output <> 0 And errormsgflag Then Debug.Print "Output Write Failed:" & output

End Sub

Public Sub DIOforceout()
'Force DIO64 Output

	Dim output As Integer
	Dim i As Integer

	output = DIO64_Out_ForceOutput(0, TTLdata(0), 15)
	If output <> 0 And errormsgflag Then Debug.Print "Force Output Failed:" & output

	Call PlotFrm.PlotTTL

End Sub

Public Sub DIOstop()
'Force DIO64 Output

	Dim output As Integer

	output = DIO64_Out_Stop(0)
	If output <> 0 And errormsgflag Then Debug.Print "Output Stop Failed:" & output

End Sub

Public Function DIOclockstatus() As Double
'Read the value of the DIO clock

	Dim availablescans As Long
	Dim stat As DIO64STAT
	Dim dummy As Integer
	Dim output As Integer

	availablescans = 0

	output = DIO64_Out_Status(0, availablescans, stat)
	If output <> 0 And errormsgflag Then
		DIOclockstatus = Now * 24 * 60 * 60 * 1000
		Debug.Print "using now"
	Else
		DIOclockstatus = stat.time / 10000
		'assume the DIO runs at 10 MHz, return time in units of ms
	End If

End Function

Public Sub TTLzero()
'Set every element of TTL array to zero and force all DIO lines low

	Dim j As Integer
	Dim i As Integer

	For j = 0 To 3
		For i = 0 To 15
			TTLarray(j, i) = 0
		Next i
		TTLdata(j) = 0
	Next j

	Call DIOforceout

End Sub

Public Sub TTLdefault()
	'Set every element of TTL array to its default value and call DIO force out

	TTLarray(0, 0) = Azero0
	TTLarray(0, 1) = Aone0
	TTLarray(0, 2) = Atwo0
	TTLarray(0, 3) = Athree0
	TTLarray(0, 4) = Afour0
	TTLarray(0, 5) = Afive0
	TTLarray(0, 6) = Asix0
	TTLarray(0, 7) = Aseven0
	TTLarray(0, 8) = Aeight0
	TTLarray(0, 9) = Anine0
	TTLarray(0, 10) = Aten0
	TTLarray(0, 11) = Aeleven0
	TTLarray(0, 12) = Atwelve0
	TTLarray(0, 13) = Athirteen0
	TTLarray(0, 14) = Afourteen0
	TTLarray(0, 15) = Afifteen0

	TTLarray(1, 0) = Bzero0
	TTLarray(1, 1) = Bone0
	TTLarray(1, 2) = Btwo0
	TTLarray(1, 3) = Bthree0
	TTLarray(1, 4) = Bfour0
	TTLarray(1, 5) = Bfive0
	TTLarray(1, 6) = Bsix0
	TTLarray(1, 7) = Bseven0
	TTLarray(1, 8) = Beight0
	TTLarray(1, 9) = Bnine0
	TTLarray(1, 10) = Bten0
	TTLarray(1, 11) = Beleven0
	TTLarray(1, 12) = Btwelve0
	TTLarray(1, 13) = Bthirteen0
	TTLarray(1, 14) = Bfourteen0
	TTLarray(1, 15) = Bfifteen0

	TTLarray(2, 0) = Czero0
	TTLarray(2, 1) = Cone0
	TTLarray(2, 2) = Ctwo0
	TTLarray(2, 3) = Cthree0
	TTLarray(2, 4) = Cfour0
	TTLarray(2, 5) = Cfive0
	TTLarray(2, 6) = Csix0
	TTLarray(2, 7) = Cseven0
	TTLarray(2, 8) = Ceight0
	TTLarray(2, 9) = Cnine0
	TTLarray(2, 10) = Cten0
	TTLarray(2, 11) = Celeven0
	TTLarray(2, 12) = Ctwelve0
	TTLarray(2, 13) = Cthirteen0
	TTLarray(2, 14) = Cfourteen0
	TTLarray(2, 15) = Cfifteen0

	TTLarray(3, 0) = Dzero0
	TTLarray(3, 1) = Done0
	TTLarray(3, 2) = Dtwo0
	TTLarray(3, 3) = Dthree0
	TTLarray(3, 4) = Dfour0
	TTLarray(3, 5) = Dfive0
	TTLarray(3, 6) = Dsix0
	TTLarray(3, 7) = Dseven0
	TTLarray(3, 8) = Deight0
	TTLarray(3, 9) = Dnine0
	TTLarray(3, 10) = Dten0
	TTLarray(3, 11) = Deleven0
	TTLarray(3, 12) = Dtwelve0
	TTLarray(3, 13) = Dthirteen0
	TTLarray(3, 14) = Dfourteen0
	TTLarray(3, 15) = Dfifteen0

	Call TTLarrayConvert
	Call DIOforceout

End Sub

Public Sub TTLarrayConvert()
'Converts TTLArray into TTLdata (array of 4 integers)
'uses hexidecimal numbers as an intermediate step

	Dim j As Integer
	Dim i As Integer
	Dim linevalue(3) As String
	Dim rowvalue(3) As String
	
	For j = 0 To 3
		For i = 0 To 3
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "0"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "1"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "2"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "3"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "4"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "5"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "6"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = False Then linevalue(i) = "7"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "8"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "9"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "A"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = False And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "B"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "C"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = False And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "D"
			If TTLarray(j, 4 * i) = False And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "E"
			If TTLarray(j, 4 * i) = True And TTLarray(j, 4 * i + 1) = True And TTLarray(j, 4 * i + 2) = True And TTLarray(j, 4 * i + 3) = True Then linevalue(i) = "F"
		Next i
		rowvalue(j) = linevalue(3) & linevalue(2) & linevalue(1) & linevalue(0)
		TTLdata(j) = CInt("&H" & rowvalue(j))
	Next j

End Sub


Public Sub TTLhigh(ByVal lineindex As Integer)
	'Force a particular TTL line high

	Dim dummy1 As String
	Dim dummy2 As String
	Dim rowindex As Integer
	Dim columnindex As Integer

	rowindex = lineindex \ 100    'this is the TTL port  (0 to 3)
	columnindex = lineindex Mod 100    'this is the TTL line (0 to 15)

	TTLarray(rowindex, columnindex) = 1

	Call TTLarrayConvert
	Call DIOforceout

End Sub

'Force a particular TTL line low
Public Sub TTLlow(ByVal lineindex)

	Dim dummy1 As String
	Dim dummy2 As String
	Dim rowindex As Integer
	Dim columnindex As Integer

	rowindex = lineindex \ 100       'this is the TTL port  (0 to 3)
	columnindex = lineindex Mod 100  'this is the TTL line (0 to 15)

	TTLarray(rowindex, columnindex) = 0

	Call TTLarrayConvert
	Call DIOforceout

End Sub

Public Function converttosigned(ByVal tobeconverted As Long) As Integer
	'converts a 16 bit long integer (0 to 65436) to a signed 16 bit integer (-32767 to 32767)
	'necessary because the DIO64 board wants to see a signed 16 bit integer

	If tobeconverted > 65535 Then Exit Function
	If tobeconverted < 0 Then Exit Function

	If tobeconverted <= 32767 Then converttosigned = CInt(tobeconverted)
	If tobeconverted > 32767 Then converttosigned = CInt(tobeconverted - 65536)

End Function

Public Function converttounsigned(ByVal tobeconverted As Integer) As Long
	'converts a signed 16 bit integer (-32767 to 32767) to a 16 bit long integer (0 to 65436)
	'necessary because the DIO64 board wants to see a signed 16 bit integer

	If tobeconverted >= 0 Then converttounsigned = CLng(tobeconverted)
	If tobeconverted < 0 Then converttounsigned = CLng(tobeconverted + 65536)

End Function

Public Sub chg(ByVal channel As Integer, ByVal VALUE As Integer)
    'syntax used in old program
    
    If VALUE = 1 Then
        Call TTLhigh(channel)
    ElseIf VALUE = 0 Then Call TTLlow(channel)
    Else: Debug.Print "chg: invalid TTL value"
    End If
    
End Sub
