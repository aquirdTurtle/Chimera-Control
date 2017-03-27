Attribute VB_Name = "PreciseTiming"

Option Base 0

Public Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
Public Declare Function GetInputState Lib "user32" () As Long
Public Declare Function GetTickCount Lib "kernel32" () As Long
Public Sub tic()
    tictime = GetTickCount()
End Sub
Public Sub toc()
    Debug.Print "This took ", GetTickCount() - tictime, "ms"
End Sub

Public Sub qclearacts()

	TTLcounter = 0
	Erase TTLmatrix

	DACcounter = 0    'counts how many DAC commands
	Erase DACmatrix  'list of times, lines, values

	Erase DACdatatoboard  'array of data that gets written to the DAC boards
	Erase TTLdatatoboard
	'Call DACStopTask    move to qgo subroutine

	'Debug.Print "cleared"

End Sub
Public Sub qprep()

    Dim i As Integer
    Dim output0 As Long
    Dim output1 As Long
    Dim output2 As Long
       
    If goflag = False Then Exit Sub
    
    'output starting values. **These are the initial values written to the
    'DACboardmatrix before a routine begins, and are controlled by the panel as well.
    'Hence: "static DAC values" as described where they are declared in TheMainProgram.
    'They are written at t = 0.** AMK, 08/2010
   If writeNewExperiment Then 'so everything is only called on thefirst accumulation
        For i = 0 To numDACs - 1
           qvolts DACarray(i), 0, i
        Next i
    
 '   If writeNewExperiment Then
    
        DACbuffsize = qDACGather  'calls DACboardwrite
    
'ADD SOMETHING HERE TO WAIT UNTIL LOADED?
       
        TTLbuffsize = qTTLGather
        
     
    End If
    
    Call DIOwrite(TTLbuffsize)
    
   'DACcounter = 0
   'TTLcounter = 0
   
     'To update your plot form with new values:  Warning, this is slow (600ms)
    If plotflag = True Then
        Call PlotFrm.PlotqloopTTLs
        Call PlotFrm.PlotTTL
        If TestFrm.chkShowTTLdatatoboard.VALUE = 1 Then Call TestFrm.TestTTLdatatoboard
        Call PlotFrm.PlotDAC
        Call PlotFrm.PlotqloopDACs
        plotflag = False 'necessary unless you want to continue plotting every time
    End If
        'one task per DAC board, 8 channels per board
        'start all tasks to start (precise timing) DAC output
        'note: DAQ tasks must start _before_ dio board start
    output2 = DAQmxStartTask(staticDAC_2)
    If output2 <> 0 And output <> -200077 Then
       Debug.Print "DAQmxStartTask failed:" & output2
       Call DACerrchk(output2)
    End If
        
    output1 = DAQmxStartTask(staticDAC_1)
    If output1 <> 0 And errormsgflag And output1 <> -200077 Then
        Debug.Print "DAQmxStartTask failed:" & output1
        Call DACerrchk(output1)
    End If
        
    output0 = DAQmxStartTask(staticDAC_0)
    If output0 <> 0 And output <> -200077 Then
        Debug.Print "DAQmxStartTask failed:" & output0
        Call DACerrchk(output0)
    End If
End Sub

Public Sub qgo()
	Dim output As Integer
	Dim totaltime As Double
	Dim timeBeforeStopTask, timeAfterStopTask As Double

	If TTLcounter > 25000 Then
		printnow ("TTL array " + CStr(TTLcounter) + "Should not exceed 25000")
	End If
	Debug.Print
	If goflag = False Then Exit Sub

	output = DIO64_Out_Start(0)
	If output <> 0 And errormsgflag Then Debug.Print "Output Start Failed:" & output

	'wait until all qcommands are finished, then stop all DAC tasks
	'	need a bit of extra time (1 ms) to be sure the DIO board is done
	'The /10000 here is to scale clock ticks of 100 ns to ms. This was, 
	'	incorrectly I think, initially /1000. AMK, 08/2010
	totaltime = ((converttounsigned(TTLdatatoboard(0, TTLbuffsize - 1)) + _
			  converttounsigned(TTLdatatoboard(1, TTLbuffsize - 1)) * 65535) / 10000) + 1

	Debug.Print "Calculated total time is: "; totaltime
	wait totaltime / ClockFix

	timeBeforeStopTask = timeGetTime
	Call DACStopTask
	timeAfterStopTask = timeGetTime

	stopTotalTime = timeAfterStopTask - timeBeforeStopTask
	Debug.Print "Measured total time in DACStopTask is: "; stopTotalTime

End Sub

Public Sub qTTL(ByVal time As Double, ByVal line As Long, ByVal VALUE As Long)

	time = CLng(time * 10 ^ 4) 'Convert time from ms to 100ns clock ticks.
	'note:  TTLmatrix holds long (integers)

	TTLmatrix(0, TTLcounter) = time
	TTLmatrix(1, TTLcounter) = line
	TTLmatrix(2, TTLcounter) = VALUE

	TTLcounter = TTLcounter + 1

End Sub

Public Sub qturnoff(ByVal line As Long, ByVal time As Double)
    'syntax used in old program
    Call qTTL(time, line, 0)
End Sub

Public Sub qturnon(ByVal line As Long, ByVal time As Double)
    'syntax used in old program
    Call qTTL(time, line, 1)
End Sub

Private Function qTTLGather() As Integer
	Dim nextuniquetimeindex As Long
	Dim j As Integer
	Dim i As Integer
	Dim k As Integer
	Dim rowindex As Integer
	Dim columnindex As Integer
	Dim uniquetimes As Integer
	Dim stopGatherTime, startGatherTime As Double


	nextuniquetimeindex = 0
	uniquetimes = 0
	Do While TTLcounter > nextuniquetimeindex
		i = nextuniquetimeindex
		Do While (TTLcounter > i) And (TTLmatrix(0, nextuniquetimeindex) = TTLmatrix(0, i))
			i = i + 1
		Loop
		nextuniquetimeindex = i
		uniquetimes = uniquetimes + 1
	Loop

	ReDim TTLdatatoboard(5, uniquetimes) As Integer

	'sort TTLmatrix from low to high times

	startGatherTime = timeGetTime
	Call SortArray(TTLmatrix, TTLcounter)
	stopGatherTime = timeGetTime
	Debug.Print "The TTL sort time is"; stopGatherTime - startGatherTime


	nextuniquetimeindex = 0
	uniquetimes = 0
	Do While TTLcounter > nextuniquetimeindex
		i = nextuniquetimeindex
		Do While (TTLcounter > i) And (TTLmatrix(0, nextuniquetimeindex) = TTLmatrix(0, i))
			rowindex = TTLmatrix(1, i) \ 100
			columnindex = TTLmatrix(1, i) Mod 100
			TTLarray(rowindex, columnindex) = TTLmatrix(2, i)
			i = i + 1
		Loop
		Call TTLarrayConvert
		'THE BELOW COMMENT BY JJZ IS NO LONGER RELEVANT: CLOCK ISSUE FIXED AND = 10 MHz
		'LEAVING THE COMMENT FOR FUTURE REFERENCE, HOWEVER! AMK, 08/2010
		'i.e. CLOCKFIX = 1
	
		'We need to scale the times here to be more precise.  The board thinks the clock is jjz
		'20MHz but it is probably 16MHz. Changing "scan" doesn't fix the problem in a continuous way.
		'ClockFix is a software fix of the problem times are in hundred-nanonseconds to fix rounding problem
		'Correction is only perfect for multiples of 1.2micro sec (ie, 12us give 12us; but 8us gives 8.4us.)
		'To calculate the real time use the following: (time_entered)/1.2)  -> round up to the nearest integer, multiply by 1.2 again
		
		TTLdatatoboard(0, uniquetimes) = converttosigned((TTLmatrix(0, nextuniquetimeindex) * ClockFix) Mod 65535)
		TTLdatatoboard(1, uniquetimes) = converttosigned((TTLmatrix(0, nextuniquetimeindex) * ClockFix) \ 65535)
		TTLdatatoboard(2, uniquetimes) = TTLdata(0)
		TTLdatatoboard(3, uniquetimes) = TTLdata(1)
		TTLdatatoboard(4, uniquetimes) = TTLdata(2)
		TTLdatatoboard(5, uniquetimes) = TTLdata(3)
		nextuniquetimeindex = i
		uniquetimes = uniquetimes + 1
	Loop

	qTTLGather = uniquetimes

End Function

Public Sub qvolts(ByVal VALUE As Double, ByVal time As Double, ByVal line As Long)
	'syntax of old program
	'write time, line, value into new DACmatrix column (data type is long)
  
	time = CLng(time * 10 ^ 4) 'Convert time from ms to 100ns clock tics
	VALUE = CLng(VALUE * 10 ^ 6) 'Convert volts to microvolts

	DACmatrix(0, DACcounter) = time
	DACmatrix(1, DACcounter) = line
	DACmatrix(2, DACcounter) = VALUE

	'increases DACcounter by 1
	DACcounter = DACcounter + 1

	'setup TTL trigger that goes to DAC boards (assume each static DAC boards has its own trigger)
	'later we should fix the code to setup the two DAC arrays separately
	TTLmatrix(0, TTLcounter) = time
	TTLmatrix(1, TTLcounter) = triggerline
	TTLmatrix(2, TTLcounter) = 1

	TTLcounter = TTLcounter + 1

	TTLmatrix(0, TTLcounter) = time
	TTLmatrix(1, TTLcounter) = triggerline2
	TTLmatrix(2, TTLcounter) = 1

	TTLcounter = TTLcounter + 1

	TTLmatrix(0, TTLcounter) = time
	TTLmatrix(1, TTLcounter) = triggerline3
	TTLmatrix(2, TTLcounter) = 1

	TTLcounter = TTLcounter + 1

	TTLmatrix(0, TTLcounter) = time + triggertime
	TTLmatrix(1, TTLcounter) = triggerline
	TTLmatrix(2, TTLcounter) = 0

	TTLcounter = TTLcounter + 1

	TTLmatrix(0, TTLcounter) = time + triggertime
	TTLmatrix(1, TTLcounter) = triggerline2
	TTLmatrix(2, TTLcounter) = 0

	TTLcounter = TTLcounter + 1

	TTLmatrix(0, TTLcounter) = time + triggertime
	TTLmatrix(1, TTLcounter) = triggerline3
	TTLmatrix(2, TTLcounter) = 0

	TTLcounter = TTLcounter + 1

End Sub

'# See bottom
Private Function qDACGather() As Integer

	'This function sets up the data (times, lines, voltages) in a format the DACs can read.
	'It also hands back a number to scale the buffer.
	'Called by qprep

	Dim nextuniquetimeindex As Long
	Dim numberofuniquetimes As Integer
	Dim j As Integer
	Dim i As Integer
	Dim rowindex As Integer
	Dim columnindex As Integer
	Dim finaldata0() As Double
	Dim finaldata1() As Double
	Dim finaldata2() As Double
	Dim output As Long
	Dim numsamples As Long

	'sort DACmatrix from low to high times
	Call SortArray(DACmatrix, DACcounter)

	'find the number of unique times (numberofuniquetimes)
	nextuniquetimeindex = 0
	numberofuniquetimes = 0
	Do While DACcounter > nextuniquetimeindex
		i = nextuniquetimeindex
		Do While (DACcounter > i) And (DACmatrix(0, nextuniquetimeindex) = DACmatrix(0, i))
			i = i + 1
		Loop
		nextuniquetimeindex = i
		numberofuniquetimes = numberofuniquetimes + 1
	Loop

	'redimension empty array DACdatatoboard
	'first row is for times
	'next numDacs rows for each DAC channel (8 channels per board)
	ReDim DACdatatoboard(numDACs, numberofuniquetimes) As Double

	'put data into DACdatatoboard array
	nextuniquetimeindex = 0  'set counters to 0
	numberofuniquetimes = 0
	Do While DACcounter > nextuniquetimeindex
		i = nextuniquetimeindex
		Do While (DACcounter > i) And (DACmatrix(0, nextuniquetimeindex) = DACmatrix(0, i))
		
			rowindex = DACmatrix(1, i)
			'row index is the channel number
		
			If rowindex < 0 Or rowindex > numDACs - 1 Then
				Exit Function
				Debug.Print "qDACGather: invalid DAC channel in qloops"
			End If
		
			DACdatatoboard(0, numberofuniquetimes) = DACmatrix(0, nextuniquetimeindex) 'first row is the times
			DACarray(rowindex) = CDbl(DACmatrix(2, i) / 10 ^ 6)
		
			i = i + 1  'increment the counter for DACmatrix array
		Loop
		For j = 0 To numDACs - 1
			DACdatatoboard(j + 1, numberofuniquetimes) = DACarray(j)
			'j+1 because the first row of DACdatatoboard is for the times
		Next j
		nextuniquetimeindex = i
		numberofuniquetimes = numberofuniquetimes + 1
	Loop

	''We are not entirely sure why we need the following line. However, w/o it we were finding
	''the that the number of unique times is too long and this ends up causing an error
	''in the DACStopTask. This error then causes the next DACStartTask to take an extra 30msec
	numberofuniquetimes = numberofuniquetimes
	Debug.Print numberofuniquetimes
	numsamples = numberofuniquetimes
	'this is puts the data into a vector form that the DAC board likes
	'this vector does not have the times
	ReDim finaldata0((numsamples) * 8 - 1)
	ReDim finaldata1((numsamples) * 8 - 1)
	ReDim finaldata2((numsamples) * 8 - 1)
	For j = 0 To numberofuniquetimes - 1
		For i = 0 To 7
			   finaldata0(j * 8 + i) = DACdatatoboard(i + 1, j)  'board 0
			   finaldata1(j * 8 + i) = DACdatatoboard(i + 9, j) 'board 1
			   finaldata2(j * 8 + i) = DACdatatoboard(i + 17, j) 'board 2
				'Debug.Print "fd1 " & j * 8 + i & " is " & finaldata1(j * 8 + i)
		Next i
	Next j

	'Debug.Print "numsamples " & numsamples
	'If numsamples Mod 2 = 1 Then numsamples = numsamples + 1

	If numsamples > 0 Then
		Call DACboardwrite(2, finaldata2(), numsamples)
		Call DACboardwrite(1, finaldata1(), numsamples)
		Call DACboardwrite(0, finaldata0(), numsamples)
	End If

	qDACGather = numsamples

End Function

