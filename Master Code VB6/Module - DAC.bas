Attribute VB_Name = "DAC"

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Under "Project/References", make sure NI-DAQmx C API appears and is checked
'
'  Go to "View/Object Browser" to view library of DAQ commands from National Intruments DAQmx.
'  Go to "Start/programs/National Instruments/Ni-DAQ/Ni-DAQmx Help or Ni-DACmx C Reference Help"
'     for more documentation
'  'Note: Sometimes null parameter must be passed as "ByVal 0&" (no quotation marks)
'
'   The two static DAC boards are in PXI1Slot3 and PXISlot4
'   The both use the same TTL line for a clock trigger.
'      This TTL goes into the PFI0 input on each of the DAC boards.
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'11/9/2010: AMK modified code to be general with respect to number of DAQ boards, of which we presently have three. This will be done by having a global that specifies number of boards and ports.
'AMK: If there is a pound sign above a routine, this means that it is not general and specific information must be added to accomodate a new DAC board.

'Notes to self: 1. In main program: changed Dim of DACarray[15] -> DACarray[numDACs-1], added declaration of static_dac2,
Option Base 0
'#
Public Sub DACStopTask()

    Dim output As Long

    output = DAQmxStopTask(staticDAC_0)
    If output <> 0 And errormsgflag And output <> -200077 Then
                 Debug.Print "DAQmxStopTask failed:" & output
                 Call DACerrchk(output)
    End If
    output = DAQmxStopTask(staticDAC_1)
    If output <> 0 And errormsgflag And output <> -200077 Then
                 Debug.Print "DAQmxStopTask failed:" & output
                 Call DACerrchk(output)
    End If
    output = DAQmxStopTask(staticDAC_2)
    If output <> 0 And errormsgflag And output <> -200077 Then
                 Debug.Print "DAQmxStopTask failed:" & output
                 Call DACerrchk(output)
    End If
    
End Sub
'#
'Added numDACs assignment here.
Public Sub DACreset()
    Dim output As Long

    output = DAQmxResetDevice("PXI1Slot3")
    output = DAQmxResetDevice("PXI1Slot4")
    output = DAQmxResetDevice("PXI1Slot5")
End Sub
'#
Public Sub DACinitialize()
	 
	Dim output As Long
	Dim sampsPerChanWritten As Long

	'Create a task for each board
	'assume 3 boards, 8 channels per board. AMK 11/2010, modified for three from 2
	'task names are defined as public variables of type Long in TheMainProgram Declarations

	'This creates the task to output from DAC 2
	output = DAQmxCreateTask("", staticDAC_2)
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Task 1 Failed:" & output
			Call DACerrchk(output)
		End If

	'This creates the task to output from DAC 1
	output = DAQmxCreateTask("", staticDAC_1)
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Task 1 Failed:" & output
			Call DACerrchk(output)
		End If

	'This creates the task to output from DAC 0
	output = DAQmxCreateTask("", staticDAC_0)
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Task 0 Failed:" & output
			Call DACerrchk(output)
		End If
		
		
	'''INPUTS
	'This creates a task to read in a digital input from DAC 0 on port 0 line 0
	output = DAQmxCreateTask("", digitalDAC_0_00)
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Task 2 Failed:" & output
			Call DACerrchk(output)
		End If
		
	'This creates a task to read in a digital input from DAC 0 on port 1 line 1 (currently unused 11/08)
	output = DAQmxCreateTask("", digitalDAC_0_01)

		If output <> 0 And errormsgflag Then
			Debug.Print "Create Task 2 Failed:" & output
			Call DACerrchk(output)
		End If

	'Configure the output
	'Function DAQmxCreateAOVoltageChan(taskHandle As Long, physicalChannel As String, nameToAssignToChannel As String, _
	   minVal As Double, maxVal As Double, units As DAQmxVoltageUnits2, customScaleName As String) As Long
	'physicalChannel can be a range

	output = DAQmxCreateAOVoltageChan(staticDAC_2, "PXI1Slot5/ao0:7", "StaticDAC_2", _
	   -10#, 10#, DAQmx_Val_VoltageUnits2_Volts, "")
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Voltage Channel 1 Failed:" & output
			Call DACerrchk(output)
		End If

	'Not sure why Tara and Debbie chose to switch the labels (for staticDac_0 -> StaticDac_1) here, but I'll stick with it to be consistent everywhere else in the program. AMK, 11/2010

	output = DAQmxCreateAOVoltageChan(staticDAC_0, "PXI1Slot3/ao0:7", "StaticDAC_1", _
	   -10#, 10#, DAQmx_Val_VoltageUnits2_Volts, "")
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Voltage Channel 1 Failed:" & output
			Call DACerrchk(output)
		End If
		
	output = DAQmxCreateAOVoltageChan(staticDAC_1, "PXI1Slot4/ao0:7", "StaticDAC_0", _
	   -10#, 10#, DAQmx_Val_VoltageUnits2_Volts, "")
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Voltage Channel 0 Failed:" & output
			Call DACerrchk(output)
		End If
		
		'''''''''INPUTS
		
	output = DAQmxCreateDIChan(digitalDAC_0_00, "PXI1Slot3/port0/line0", "DIDAC_0", DAQmx_Val_ChanPerLin)
		If output <> 0 And errormsgflag Then
			Debug.Print "Create Digital DAC Failed:" & output
			Call DACerrchk(output)
		End If
		
	'currently unused 11/08
	output = DAQmxCreateDIChan(digitalDAC_0_01, "PXI1Slot3/port0/line1", "DIDAC_0", DAQmx_Val_ChanPerLin)
	If output <> 0 And errormsgflag Then
		Debug.Print "Create Digital DAC Failed:" & output
		Call DACerrchk(output)
	End If
			
End Sub
'#
Public Sub DACboardwrite(ByVal boardindex As Integer, ByRef voltage() As Double, numsamples As Long)

	Dim taskhand As Long
	Dim sampsPerChanWritten As Long
	Dim output As Long

	'choose the right task for a certain DAC board
	'AMK: Here's where the pound applies - unfortunately there isn't a general way to do this since it involves 
	' 	the staticDac names, though I guess we could store them in an array. This may get too complicated.

	'If boardindex = 0 Then
	  '  taskhand = staticDAC_0
	'Else
	 '   If boardindex = 1 Then
	 '       taskhand = staticDAC_1
	  '  Else
	  '      taskhand = staticDAC_2
	  '  End If
	'End If

	Select Case boardindex
			 Case 0: taskhand = staticDAC_0
			 Case 1: taskhand = staticDAC_1
			 Case Else: taskhand = staticDAC_2 'this is always the last case
	End Select

	'Function DAQmxCfgSampClkTiming(taskHandle As Long, source As String, rate As Double, activeEdge As DAQmxEdge, _
	'	sampleMode As DAQmxAcquisitionType, sampsPerChan As Long) As Long
	
	'rate is samples per sec, 1 MHz is maximum expected rate of external clock from DIO64 board
	'sampsPerChan is set to numsamples (cannot be larger or the voltages start to repeat)
	'numsamples = 10
	'**The maximum sample rate is 1 MS/s - see PXI6733 specs. Meaning 1 MHz, as set here, is the appropriate maximum. AMK 08-2010
	If (1) Then
		Select Case boardindex
		Case 0:
			output = DAQmxCfgSampClkTiming(taskhand, "/PXI1Slot3/PFI0", 1000000, DAQmx_Val_Rising, _
			   DAQmx_Val_AcquisitionType_FiniteSamps, numsamples)
			If output <> 0 And errormsgflag Then
			   Debug.Print "DACboardwrite: Clock Configure Failed:" & output
			   'Call DACerrchk(output)
			End If
		Case 1:
			output = DAQmxCfgSampClkTiming(taskhand, "/PXI1Slot4/PFI0", 1000000, DAQmx_Val_Rising, _
			   DAQmx_Val_AcquisitionType_FiniteSamps, numsamples)
			If output <> 0 And errormsgflag Then
				Debug.Print "DACboardwrite: Clock Configure Failed:" & output
				Call DACerrchk(output)
			End If
		Case Else: 'this is always the last case
			output = DAQmxCfgSampClkTiming(taskhand, "/PXI1Slot5/PFI0", 1000000, DAQmx_Val_Rising, _
			DAQmx_Val_AcquisitionType_FiniteSamps, numsamples)
			If output <> 0 And errormsgflag Then
				Debug.Print "DACboardwrite: Clock Configure Failed:" & output
				Call DACerrchk(output)
			End If
		End Select
	End If
	'Function DAQmxWriteAnalogF64(taskHandle As Long, numSampsPerChan As Long, autoStart As Boolean, timeout As Double, _
	'  dataLayout As DAQmxFillMode, writeArray As Double, sampsPerChanWritten As Long, reserved As Any) As Long
	
	'chg Cone, 1
	output = DAQmxWriteAnalogF64(taskhand, numsamples, False, 0.0001, _
		DAQmx_Val_GroupByScanNumber, voltage(0), sampsPerChanWritten, ByVal 0&)
	If output <> 0 And errormsgflag And output <> -200077 Then
		Debug.Print "DACboardwrite: Write " & CStr(boardindex) & " Failed:" & output
		Call DACerrchk(output)
	Else
		'Debug.Print "write worked " & sampsPerChanWritten
	End If
	'chg Cone, 0

	'"-200077" is the code for "the DAC board did not update during your qloop." This occurs when a qloop includes only DIO commands and is no cause for alarm (I think).
		
End Sub

Public Sub DACforce2(ByVal voltage As Double, ByVal DACline As Long)
    Dim i As Integer
    
    If DACline < 0 Or DACline > numDACs - 1 Then
        Debug.Print "DACforce2: DACline not valid"
        Exit Sub
    End If    

    DACarray(DACline) = voltage

    qclearacts
        For i = 0 To numDACs - 1
            qvolts DACarray(i), 10, i
        Next i
    qprep
    qgo
    
    Call PlotFrm.PlotDAC
        
End Sub
'# Added in volts2 array, also pretty sure this code isn't used so didn't actually continue with this section. Yes - it is not used.
'wtf is j in that loop? Should be 1 for volts1, 2 for volts 2 etc....
Public Sub DACforce(ByVal voltage As Double, ByVal DACline As Long)
'Force the static DAC output

    Dim output As Long
    Dim sampsPerChanWritten As Long
    Dim taskname As Long
    Dim i As Integer
    Dim volts0(7) As Double
    Dim volts1(7) As Double
    Dim volts2(7) As Double
    If DACline < 0 Or DACline > numDACs - 1 Then
        Debug.Print "DACforce: DACline not valid"
        Exit Sub
    End If
    
    DACarray(DACline) = voltage
    For i = 0 To 7
        volts0(i) = DACarray(i)
        volts1(i) = DACarray(j * 8 + i)
    Next i
    
    'Write all DAC values and output them
    
    'Function DAQmxCfgSampClkTiming(taskHandle As Long, source As String, rate As Double, activeEdge As DAQmxEdge, _
    ' sampleMode As DAQmxAcquisitionType, sampsPerChan As Long) As Long

    'rate is samples per sec, 1 MHz is maximum expected rate of external clock from DIO64 board
    'sampsPerChan is set to numsamples (cannot be larger or the voltages start to repeat)

   ' output = DAQmxCfgSampClkTiming(staticDAC_0, "/PXI1Slot3/PFI0", 1000000, DAQmx_Val_Rising, _
   '     DAQmx_Val_AcquisitionType_FiniteSamps, numsamples)
        'if output <> 0 and errormsgflag Then
         '   Debug.Print "DACforce: Clock Configure Failed:" & output
        '    Call DACerrchk(output)
       ' End If
    
    'Function DAQmxWriteAnalogF64(taskHandle As Long, numSampsPerChan As Long, autoStart As Boolean, timeout As Double, _
    ' dataLayout As DAQmxFillMode, writeArray As Double, sampsPerChanWritten As Long, reserved As Any) As Long

    output = DAQmxWriteAnalogF64(staticDAC_0, 1, True, 10#, _
        DAQmx_Val_GroupByScanNumber, volts0(0), sampsPerChanWritten, ByVal 0&)
        If output <> 0 And errormsgflag Then
            Debug.Print "DACforce: Write Failed:" & output
            Call DACerrchk(output)
        Else
        'Debug.Print "write worked " & sampsPerChanWritten
    End If
    
    Call PlotFrm.PlotDAC
    
    'May need to add a "clear task" command
    
End Sub

Public Sub DACerrchk(errorCode As Long)
	'Utility function to convert output errors to meaningful messages

	Dim errorString As String
	Dim bufferSize As Long
	Dim status As Long
		
	'Find out the error message length.
	bufferSize = DAQmxGetErrorString(errorCode, 0, 0)
	'Allocate enough space in the string.
	errorString = String$(bufferSize, 0)
	'Get the actual error message.
	status = DAQmxGetErrorString(errorCode, errorString, bufferSize)
	'Trim it to the actual length, and display the message
	errorString = Left(errorString, InStr(errorString, Chr$(0)))
	Debug.Print errorString

End Sub
