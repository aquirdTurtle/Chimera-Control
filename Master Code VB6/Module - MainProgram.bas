Attribute VB_Name = "TheMainProgram"
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'COMPUTERCONTROL CODE  (authors: Travis Nicholson, Debbie Jin)
'   Summer 2007
'   Modifications, Tara Drake, Fall 2008
'
'NOTES:
'    1.  All array indices start at 0.
'    2.  qvolt, qturnon, qturnoff commands at 0 time will not work.
'    3.  old program:
'         azero-aseven remain the same
'         aazero-aaseven become aeight-afifteen
'         bzero-bseven remain the same
'         bbzero-bbseven become beight-bfifteen
'         cczero-ccseven becom ceight-cfifteen
'    4.  Dont redefine your publics within your subs unless you want to reset them
'
'RECENT CHANGES:
'    replace "global" with "public"
'    move relevant public variables to this module
'    added a sub called SortArray
'    changed TTLarray to a boolean array rather than an integer array
'    add an indicator for the state of the TTLs
'
'IDEAS for future changes:
'
'    wait doesnt work
'    make a new version of the code with no code in the forms
'    figure out how long to wait for qloop
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Base 0  'array indices start from 0
Option Explicit  'must declare all variables

Public Declare Function timeGetTime Lib "winmm.dll" () As Long


Public routine As String
Public goflag As Boolean  'used when trying to abort current experiment
Public tictime As Double 'used for timing the code
Public errormsgflag As Boolean  'use for turning on or off the DAC and DIO error messages

Public qfield, agcoil As Integer  'quantization field, E/W Stern-Gerlach coil
Public magtime, rampTime, holdTime, losstime, refdelay As Double
Public biastime As Single

Public takepicflag As Integer
Public shot, moltime, droptime, buildtime As Double
Public ztvolts, sqvolts, Bx, By, Bz, pdel, quadlevel, mwPulseLength, ramanTime, LatticeHoldTime, OPTime, lastrf, expandtime, flourFrequency As Double
Public fileflag, PGCFlag, blowAway, detuneflag, imageLower, latticeFlag, popLower, specflag, rotflag, cmotflag, evapflag As Integer
Public rfevapflag, ud, du, valflag, wave, optrapflag As Integer
Public RSFreq, RSPow, FreeValue As Double
Public plotflag As Boolean
Public imDetuneProbe, ProbeDetuning, MolassesDetuning, MOTdetuning, OffResDetuning As Double

Public expRep, accumulations, automateFlag As Integer
Public t As Double
Public indDataPoints(1000) As Double

'**************************
'This is where I am adding all of the public variables I need to program cooling freqs,
'       powers, trap spacings, etc. -- there may be a lot of them, but will gather all
'       possible variables to one place, so if we can keep them straight, it should be better!
Public imagingLength, imagingLengthProbe, tunnelTime, tunnelBias, rampToTunnelBiasTime, tunnelDACdepth, leftWellTunnelFreq, leftWellImageFreq, AOfreqRampTime, intRampDownTime, intRampUpTime, rightWellFreq, AOholdTime, topRSBcooling, botRSBcooling, axialRSBcooling, exchangeARPrange, exchangeARPtime, exchangeTime, gradField, gradientTime, rabiPulseLength, ARPinitBias, ARPfinalBias, timeOfARP, holdAfterARP As Double
Public commFromNIAWG As String
Public flagFromNIAWG, varFlagFromNIAWG, sendAccumsFlag, turnOnTweezerAFGs As Boolean

'**************************

Public checksum As Integer
Public Voltarray(16000) As Integer

Type usernum                        'used for experiment parameters.
        label As String * 12          'that can be changed by keyboard entry
        letter As String * 1
        VALUE As Single
        inc As Single
        biginc As Single
        max As Single
        min As Single
End Type
Public plist(26) As usernum         'experiment parameter list
Public noparams As Integer             'actual number of parameters (<26)
Public key As Integer           'for user keyboard input
Public inputvariable As Double
Public inputflag As Integer

'Define number of DACS
Public Const numDACs As Long = 24

'Store initial TTL data
Public TTLarray(3, 15) As Boolean  'stores the current state of all TTLs
Public TTLdata(3) As Integer       'same information as TTLarray, but stored as 4 integers
'                    each 16 bit integer corresponds to one output port of the DIO64 board
Public DACarray(numDACs - 1) As Double 'array for holding current values of the static DACs

Public writeNewExperiment As Integer

Public TTLdatatoboard() As Integer   'array to write to DIO board for precise timing
Public TTLbuffsize As Integer     'size of TTLdatatoboard array (number of unique times)
Public TTLmatrix(2, 32000) As Long  'array of TTL times, lines, and values
Public TTLcounter As Integer      'counter for number of TTL commands, set to 0 in qclearacts, incremented by 1 in qttl
Public plotTTLdata() As Double
Public plotTTLbuffsize As Integer

Public DACdatatoboard() As Double  'array for holding times and values for precise timing
Public DACbuffsize As Integer   'y size of DACdatatoboard, returned by qDACgather, (=numsamples)
Public DACmatrix(2, 4000) As Long  'array of DAC times, lines, values
Public DACcounter As Integer      'counter for number of DAC commands, set to zero in qclearacts, incremented by 1 in qvolts
Public plotDACdata() As Double   'data for the DAC plot
Public plotbuffsize As Integer

Public triggerline As Long   'TTL lines for triggering the static DAC boards
Public triggerline2 As Long
Public triggerline3 As Long
Public triggertime As Long   'time that this trigger TTL is held high (in 100s of ns)

Public staticDAC_0 As Long  'task for DACboard0 (tasks are a national instruments DAQmx thing)
Public staticDAC_1 As Long  'task for DACboard1
Public staticDAC_2 As Long 'task for DACboard2
Public digitalDAC_0_00 As Long  'another task for DACboard0 (reads in a digital line)
Public digitalDAC_0_01 As Long  'another task for DACboard0 (reads in a digital line)

'GPIB Device Constants
Public hpone As Integer
Public Agilenttwo As Integer
Public srsone As Integer  'I could not find this in the old code
Public micro As Integer 'I could not find this in the old code
Public srstwo As Integer
Public srsthree As Integer
Public pulsegen As Integer
Public pulseg2 As Integer
Public microhp As Integer
Public powerhp As Integer
Public Agilent As Integer

Public Sub reset()
    Dim output As Integer
    Dim i As Integer
    Dim bdindx, pad, sad, tmo, eot, eos As Integer

    writeNewExperiment = 1
    ProbeDetuning = 5.062
    MolassesDetuning = 3.71
    MOTdetuning = 3.168
    OffResDetuning = 0.86
    
    plotflag = False
    errormsgflag = True 'should be true if you want to see DAC and DIO error messages
    
    InputFrm.Show
    PlotFrm.Show  'show the plots
    'TestFrm.Show  'show the testing form
    InputFrm.SetFocus

    triggerline = Dfourteen  'trigger for static DACs
    triggerline2 = Dfifteen
    triggerline3 = Dthirteen
    triggertime = 5 'In 100 ns, i.e. 10  = 1 microsecond. Since this sets the trigger time for the DACs, and the DACs can sample at max 1 MHz, .5 microsecond trigger time is appropriate. AMK - 08/2010
    
    goflag = True   'flag used to abort a run
    
    Call DIOopen    'DIO board stuff
    output = DIO64_Out_Start(0)
    Call TTLdefault   'set TTLs to their default values and force output of DIO
    
    Call DACreset
    Call DACinitialize  'DAC board stuff
    
    InputFrm.BackColor = &H8000000F
    
    qfield = 0      'quantization field - default E/W
    agcoil = 0      'Stern-Gerlach coil - default off
    
    key = 0          'keyboard input stuff
    
    Call InputFrm.paramfix("init")    'set values of plist (experiment parameter list)
    Call InputFrm.paramfix("write")   'write from plist into the relevant public variables
    
    For i = 0 To numDACs - 1: DACarray(i) = 0: Next i
    
        InputFrm.Label1.Caption = "F1 0 MOT" + vbCrLf + "F3 H " + vbCrLf + "F4 T Single Atoms cw" + vbCrLf + "F5 G magload" + vbCrLf + "F6 K " + vbCrLf + "F7 C Camera" + vbCrLf + "F8 P MOTfill" + vbCrLf + "F9 A " + vbCrLf + "F10 L " + vbCrLf + "F11 B Test" + vbCrLf + "F12 D " + vbCrLf + "G g " + vbCrLf + "X x " + vbCrLf + "Q End"
        InputFrm.Label1.Caption = InputFrm.Label1.Caption + vbCrLf + vbCrLf + "PARAMS"
        For i = 1 To noparams
        InputFrm.Label1.Caption = InputFrm.Label1.Caption + vbCrLf + plist(i).letter + "   " + plist(i).label
    Next i
    
    '************************* Initialize HP synthesizer ****************
     
     bdindx = 0       'board address
     pad = 17 'was 7        'device address of wavetek
     sad = 0          ' see gpib dos manual for the rest
     tmo = 13
     eot = 1
     eos = 0
    
    hpone = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    Call ibwrt(hpone, "DCL")
    Call ibwrt(hpone, "R2")         'resets the hp
     
    pad = 12
    Agilenttwo = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    
    pad = 6
    srstwo = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    
    pad = 19
    srsthree = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    
    pad = 7
    pulsegen = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    
    pad = 4
    pulseg2 = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    
    pad = 10
    microhp = ildev(bdindx, pad, sad, tmo, eot, eos)  'gets the device descriptor ud
    'CALL IBWRT(microhp, "DCL")
    'CALL IBWRT(microhp, "R2")         'resets the hp
    
    pad = 5
    powerhp = ildev(bdindx, pad, sad, tmo, eot, eos) 'gets the device descriptor ud
    
    pad = 11
    Agilent = ildev(bdindx, pad, sad, tmo, eot, eos) 'gets the device descriptor ud

    
    clearscreen (0)
    routine = "O" 'MOT routine
    printnow "mot routine requested"
    
    Call ibwrt(hpone, "FR 260 MZ")
    Call ibwrt(hpone, "AP -1 DM")
    Call ibwrt(hpone, "R3")

    Call ibwrt(microhp, "FM:SOUR EXT1")
    Call ibwrt(microhp, "FM:EXT:COUP DC")
    Call ibwrt(microhp, "FM:DEV .2 MHZ")
    Call ibwrt(microhp, "OUTP:MOD OFF")
    
    Call runroutine
    
End Sub

Public Sub runroutine()
     Select Case routine
         Case "g": Call antigrav
         Case "x": Call laserJumpTest
         Case "G": Call Magload          'go magnetic trap
         Case "O": Call MOT                    'MOT
         Case "H": Call gusCode               'ImageMOT
         Case "C": Call cameraTest
         Case "R": Call singleatoms
         Case "P": Call MOTfill
         Case "L": Call hitF10
         Case "A": Call testAndorSolis
         Case "B": Call Test
         Case "D": Call Test_DODAC
         Case "M": Call Test_DAC_Timing
         Case "Q": End
        End Select
End Sub
Public Sub exitprog()
    Call DACStopTask
    Call DAQmxClearTask(staticDAC_0)
    Call DAQmxClearTask(staticDAC_1)
    End
End Sub
Public Sub abortclick()
    If InputFrm.chkAbort.VALUE = 0 Then
        goflag = True
        qclearacts
        qprep
        qgo
    End If

    If InputFrm.chkAbort.VALUE = 1 Then
        goflag = False
        Call DIOstop
    End If
End Sub
Public Sub InitializeDevices()
    'This sub-routine initializes devices (such as mag trap power supply)
    'to set up for the experiment
    
    Call ibwrt(powerhp, "VOLT 19.25") '19.2
    Call ibwrt(powerhp, "CURR 200")  '200
    
    'setup frequency synthesizers (HP ESG for evaporation and HP 8657A for AO)
    'unsure about gpib routines
    add(1) = 17   'hpone
    add(2) = NOADDR
    Call EnableRemote(0, add()) 'make sure freq synth is in remote mode
    add(1) = 7   'microhp
    add(2) = NOADDR
    Call EnableRemote(0, add()) 'make sure freq synth is in remote mode
    qwait 100
    Call ibwrt(hpone, "FR 178 MZ;AP 2 DM")  'set freq synth for push
    Call ibwrt(microhp, "FREQ 1190 MHZ;POW -90 DBM")  'set freq. for microwaves
    Call ibwrt(micro%, "POW -90 DBM") 'AP -90 DM freq 50 MHz
End Sub
Sub antigrav()

If agcoil = 0 Then
        agcoil = 1
        printnow "anti-grav coil on"
Else
        agcoil = 0
        printnow "anti-grav coil off"
End If

routine = "O"

End Sub
Sub attenon(ByRef level As Double)
    'for compatibility with old program
    Call DACforce2(level, 1)
End Sub
Sub qdirection()

If qfield = 0 Then
        qfield = 1
        printnow "q field U/D"
Else
        qfield = 0
        printnow "qfield E/W"
End If

routine = "O"

End Sub
Public Sub MOT()

	InputFrm.txtMain.BackColor = vbWhite
	clearscreen (0)
	qTTL 1, Bzero, 0 ' not exactly sure what this is doing, but leaving it for now...
	printnow ("now in MOT routine")
	qclearacts
	
    'switch on MOT light, set power, set detuning
	qTTL 1, Azero, 1 ' MOT AO RF
	qTTL 1, Dzero, 1 ' MOT slave shutter
	qTTL 1, Bone, 1 'RP shutter open
	qTTL 1, Cthirteen, 1 'F=2 shutter open (blocks side MOT beam)
	'trap light detuning
	qvolts -0.1, 1, 20  '0 for MOT detuning (0 is 10 MHz detuning, -.3 is 17 MHz detuning)
	'trap light power
	qvolts 0.2, 1, 9 'full power
	'trap light AO detuning
	qvolts 0, 1, 8
   
    'switch on repump light
	qTTL 1, Aone, 1
	'Switch on side beam
	qTTL 1, Afive, 1
	qvolts 0.07, 1, 22
	
	'Raman shutter Closed initially
	qTTL 1, Cfour, 0
	
	
	qvolts 0.32, 1, 11 'setting power (was 0.285)
	qvolts 0, 1, 21 'setting frequency via offset lock
	
   'quad coils on
	qvolts 7, 1, 6  'CR
	
	 qvolts 0.59, 1, 0 'Right, 0.117 from the perspective of looking east to west across the table '0.64
	 qvolts -0.59, 1, 1 'Left, -0.64
	 qvolts -1.07, 1, 2 'Front, -1.272
	 qvolts 1.07, 1, 3 'Back, 1.272
	 qvolts -1.3, 1, 4  'Bottom, -1.24
	 qvolts 1.3, 1, 5 'Top, 1.24	
	
	qprep
	qgo
	
   routine = "justwaiting"
	While routine = "justwaiting"
	   wait (200)
	   DoEvents
	'   If DIOclockstatus < 1000 Then printnow "restarted DIOclock"
	Wend

	Call runroutine
	  
End Sub
Public Sub MOTfill()

qclearacts

Dim i As Double

'for MOT fill on labview program, assumes running MOT program first
t = 1

    qTTL t, Azero, 1   ' Trap light on
    qTTL t, Aone, 1  ' Repump light on
    qvolts 0, t, 6 'MOT coils off
    t = t + 200
    qvolts 7, t, 6 'MOT coils back on
    
For i = 0 To 20
    
    qTTL t, Bzero, 1      'Camera shutter line high
    qTTL t + 0.1, Bzero, 0  'Camera shutter line low
    
    t = t + 500 'wait until next exposure

 Next i


'for toggling gradient

'For i = 0 To 30

 '   qvolts 0, t, 6 'MOT coils off
 '   t = t + 200
  '  qvolts 7, t, 6 'MOT coils back on
  '  t = t + 3000

'Next i

qprep
qgo

routine = "O"
Call runroutine

End Sub
Public Sub ImageMot()

qclearacts

Dim i As Double
Dim PGCtime As Double

printnow "Imaging MOT"

PGCtime = 4

'qvolts -1.42, 1, 4  'just for now for imaging
'qvolts 1.42, 1, 5
i = 0
'For i = 0 To 1 'This is 5 shots
    qvolts 0, 100 + i * 3000, 6 'MOT coils off
    'qTTL 100 + i * 3000, Azero, 0 'Trap light off
    'qTTL 100 + i * 3000, Aone, 0 ' Repump light off
    If PGCFlag = 1 Then
        qvolts -3, 100 + i * 2000, 20
        qTTL 100 + i * 2000, Azero, 1
       ' qvolts 6, 100 + 3 + i * 2000, 6
       ' qvolts 0, 100 + 5 + i * 2000, 6
        PGCtime = 4.5
        printnow "PG cooled"
    End If
    'qvolts 0.387, 100 + 0.5 + i * 3000, 20 'Trap light ramped to resonance
    qTTL 100 + PGCtime + 0.5 + droptime + i * 3000, Azero, 1 ' Trap light on
    qTTL 100 + PGCtime + 0.5 + droptime + i * 3000, Aone, 1  ' Repump light on
    qTTL 100 + PGCtime + 0.5 + droptime + i * 3000, Bzero, 1    'Camera shutter line high
    qTTL 100.2 + PGCtime + 0.5 + droptime + i * 3000, Bzero, 0  'Camera shutter line low
    qTTL 99 + PGCtime + 0.5 + droptime + i * 3000, Bone, 1   'Camera shutter line high
    qTTL 102 + PGCtime + 0.5 + droptime + i * 3000, Bone, 0  'Camera shutter line low
    qvolts 7, 500 + 0.5 + i * 3000, 6  'MOT coils back on
    qvolts 0, 500 + 0.5 + i * 3000, 20  'Trap light back to 10 MHz red
'Next i

qprep
qgo

printnow "MOT imaged"

routine = "O"
Call runroutine

End Sub
Public Sub laserJumpTest()
Dim i As Integer

t = 100

For i = 0 To 100
    qTTL t, Atwo, 1 'laser unlock hi
    qvolts -1, t, 20
    t = t + 1
    qTTL t, Atwo, 0 'laser unlock low
    t = t + 100
    qTTL t, Atwo, 1 'laser unlock hi
    qvolts 0, t, 20
    t = t + 1
    qTTL t, Atwo, 0 'laser unlock low
    t = t + 100
Next i

qprep
qgo


printnow "camera tested"

routine = "O"
Call runroutine
End Sub
Public Sub testAndorSolis()
    qclearacts
    
        t = 20
        qvolts 0, t - 5, 6 'coils off
        qTTL t - 5, Azero, 0  'MOT and repump light off
        qTTL t - 5, Aone, 0
        qvolts -1, t - 5, 2
        qvolts 1, t - 5, 3

        'setting imaging light detuning
        qvolts 0.42, t - 5, 20 'Trap light ramped to optimal imaging, see P. 59 nb 3
        
        'atoms
        qTTL t - 1, Ctwo, 1 'camTrig
        qTTL t - 4, Dtwo, 1 ' camShutter
        qTTL t - 0.05, Afive, 1 'prb

        qTTL t, Ctwo, 0 'camTrig
        qTTL t, Dtwo, 0 ' camShutter
        qTTL t, Afive, 0  'prb

        'light
        t = t + 50 '

        qTTL t - 1, Ctwo, 1 'camTrig
        qTTL t - 4, Dtwo, 1 ' camShutter
        qTTL t - 0.05, Afive, 1 'prb

        qTTL t, Ctwo, 0 'camTrig
        qTTL t, Dtwo, 0 ' camShutter
        qTTL t, Afive, 0  'prb

        'dark
        t = t + 400

        qTTL t - 1, Ctwo, 1 'camTrig
        qTTL t - 4, Dtwo, 1 ' camShutter

        qTTL t, Ctwo, 0 'camTrig
        qTTL t, Dtwo, 0 ' camShutter

        qvolts 0, t, 2
        qvolts 0, t, 3
        
        qvolts 0, t, 16
        t = t + 400
        qvolts 1, t, 16
        t = t + 400
        qvolts 0, t, 16
        t = t + 400
        qvolts 1, t, 16
        t = t + 400
        qvolts 0, t, 16
        t = t + 400
        qvolts 1, t, 16
        t = t + 400
        qvolts 0, t, 16
        
    qprep
    qgo


routine = "O"
Call runroutine

End Sub
Public Sub cameraTest()
    
    t = 100
    
    printnow "Flourescence imaging"
  '  qTTL t - 2, Dzero, 1 'trap shutter on
  '  qTTL t - 2, Done, 1 'Repump shutter open
  '  qTTL t, Azero, 0   ' Trap light on
 '   qTTL t, Aone, 0  ' Repump light on
 '   qTTL t, Afive, 1 'probe
    qTTL t, Bzero, 1      'Camera shutter line high
    
    t = t + 0.3
    
 '   qTTL t, Afive, 0 'probe
    qTTL t, Bzero, 0 'Camera shutter line low
'    qTTL t, Azero, 0 'Trap light off,  no shutter turn off here since we don't need to worry about anything afterward.
 '   qTTL t, Aone, 0  'Repump light off
    
    qprep
    qgo
    
printnow "camera tested"
routine = "O"
Call runroutine
End Sub


Public Sub dacRamp(ByVal t As Double, ByVal DAC As Integer, ByVal rampTime As Double, ByVal rampStart As Double, ByVal rampStop As Double, ByVal rampInc As Double)

	Dim timeInc, rampRange As Double
	Dim i As Double
	Dim stopFlag, upRamp, dummy As Integer

	stopFlag = 0



	'ramp parameters

	If rampStart < rampStop Then
		rampRange = rampStop - rampStart
	Else
		rampRange = rampStart - rampStop
	End If

	timeInc = rampTime / (rampRange / rampInc)
	If rampStop > rampStart Then
		upRamp = 1
	Else
		upRamp = 0
	End If
	'printnow (CStr(timeInc))
	i = 0

	Do While stopFlag = 0
		If upRamp = 1 Then
				If (rampStart + rampInc * i) < rampStop Then
					qvolts rampStart + rampInc * i, t + timeInc * i, DAC
				Else
					qvolts rampStop, t + rampTime, DAC
					stopFlag = 1
				End If
		 Else
			   If (rampStart - rampInc * i) > rampStop Then
				  qvolts rampStart - rampInc * i, t + timeInc * i, DAC
			   Else
				  qvolts rampStop, t + rampTime, DAC
				  stopFlag = 1
			   End If
		  End If
		i = i + 1
	Loop


End Sub
Public Sub tanhRamp(ByVal t As Double, ByVal DAC As Integer, ByVal totaltime As Double, ByVal rampStart As Double, ByVal rampStop As Double, ByVal res As Double)
    Dim timeCons, presVal, presT, fVal, starttime, argF As Double
    
    Dim stopFlag As Integer
    
    timeCons = totaltime / 8
    stopFlag = 0
    presVal = 0
    starttime = t
    presT = starttime
    
    Do While stopFlag = 0
        argF = -((presT - (starttime + totaltime / 2)) / timeCons)
        fVal = (Exp(argF) - Exp(-argF)) / (Exp(argF) + Exp(-argF))
        presVal = ((rampStart - rampStop) / 2) * (fVal + 1) + rampStop
        
        If presVal < 10 & presVal > -10 Then
           ' printnow "Set value"
           'printnow CStr(presVal)
           qvolts presVal, presT, DAC
        End If
        presT = presT + res
        If presT > (starttime + totaltime) Then
            stopFlag = 1
        End If
    Loop
    
End Sub
Public Sub linRamp(ByVal t As Double, ByVal DAC As Integer, ByVal totaltime As Double, ByVal rampStart As Double, ByVal rampStop As Double, ByVal res As Double)
    Dim timeCons, presVal, presT, fVal, starttime, argF As Double
    
    Dim stopFlag As Integer
    
    
    stopFlag = 0
    presVal = 0
    starttime = t
    presT = Math.round(starttime, 5)
    
    Do While stopFlag = 0
        presVal = Math.round((-(rampStart - rampStop) * (presT - starttime) / (totaltime)) + rampStart, 3)
        If presVal < 10 & presVal > -10 Then
           ' printnow "Set value"
          ' printnow CStr(presT)
          ' printnow CStr(presVal)
           qvolts presVal, presT, DAC
        End If
        If presT > (starttime + totaltime) Then
            stopFlag = 1
        End If
        presT = Math.round(presT + res, 5)
    Loop
    
End Sub

Public Sub ToggleMOT()
qclearacts
Dim i As Integer
t = 100

For i = 0 To 100
    qvolts 0, t, 8 'MOT field off
    t = t + 1
    qvolts 7, t, 8 'MOT field on
    t = t + 1
Next i

qprep
qgo

End Sub
Sub Test_DAC_Timing()

printnow "Hello world"
qwait 200

qclearacts

    qturnon ccone, 10
    qturnoff ccone, 10.5
    qturnon ccone, 12
    qturnoff ccone, 12.5

    qvolts 1, 10, 2
    qvolts 0, 11, 2
    qvolts 1, 12, 2
    qvolts 0, 13, 2
    
qprep
qgo

routine = "O"
Call runroutine       'return to looping MOT routine

End Sub
Public Sub singleatoms()
   
InputFrm.txtMain.BackColor = vbWhite
    clearscreen (0)
    qTTL 1, Bzero, 0
    printnow ("now looking at single atoms in molasses")
    qclearacts
        
   'quad coils off
    qvolts 0, 1, 6  'CR
    
    'PGC FIELDS
'    qvolts -0.5422, 1, 2 'Front
'    qvolts 0.5422, 1, 3 'Back
'    qvolts 0.406 - 0.026, 1, 0 'Right, from the perspective of looking east to west across the table
'    qvolts -0.406 + 0.026, 1, 1 'Left
'    qvolts -1.972, 1, 4 'Bottom -1.9
'    qvolts 1.972, 1, 5 'Top 1.9
      qvolts -0.7037 - 0.022, 1, 2 'Front
    qvolts 0.3719 + 0.022, 1, 3 'Back
    qvolts 0.8819 - 0.13, 1, 0 'Right,+ (-0.04)
    qvolts 0.0281 + 0.13, 1, 1 'Left
    qvolts -2.0304 - 0.07, 1, 4 'Bottom -1.9
    qvolts 2.0104 + 0.07, 1, 5 'Top 1.9
    
    ' start PGC dither
    'qTTL 1, Atwelve, 1
    ''''
    
    'dipole trap settings
    qvolts 0.4, 1, 19 'default with servo: 2.25 (without is 0.107)
    qTTL 1, 6, 1
    
    
    
   'switch on MOT light, set power, set detuning
    qTTL 1, Azero, 1
    qTTL 1, Dzero, 1
    'trap light detuning
    qvolts 0.1, 1, 20 '0 for MOT detuning (0 is 10 MHz detuning, -.3 is 17 MHz detuning) -> 0.15 in magload
    'trap light power
    qvolts 0.1, 1, 9 '0 for full power, 0.2 for half power, 0.2, -> 0.19 for imaging in magload...
    'trap light AO detuning
    qvolts 0, 1, 8
   
   'switch on repump light
    qTTL 1, Aone, 1
    qTTL 1, Done, 1
    qvolts 0.11, 1, 11  'setting power
   'qvolts 0, 0, 21 setting frequency via offset lock
       
    qprep
    qgo
    
     routine = "justwaiting"
While routine = "justwaiting"
   wait (200)
   DoEvents
'   If DIOclockstatus < 1000 Then printnow "restarted DIOclock"
Wend

Call runroutine

End Sub



Sub hitF8()
'routine = P

Dim i As Integer
Dim f8case As Double
Dim tobeprinted As String

f8case = 100

tobeprinted = "4 = CollMot fill"
tobeprinted = tobeprinted + vbCrLf + "0 = probe"
tobeprinted = tobeprinted + vbCrLf + "1 = test rf pulse"
tobeprinted = tobeprinted + vbCrLf + "9 = switch between off res and probe"
tobeprinted = tobeprinted + vbCrLf + "100 = exit to MOT"
tobeprinted = tobeprinted + vbCrLf + "which case? "

Call inputnow(tobeprinted, f8case)

If f8case = 100 Then
    routine = "O"
End If
'f8case = 7

'for changing trap laser frequency
If f8case = 0 Then

    printnow " probe"
    qclearacts
        qvolts ProbeDetuning, 1, 12
    qprep
    qgo
End If

If f8case = 1 Then
'for testing rf pulses
printnow " testing rf pulse"
While routine = "P"
 qclearacts

'   qvolts 2.35, 10, 6      'probe freq
'   qvolts .45, 500, 6      'probe freq  '2.35

'  qturnon ccfive, 0
'  qturnoff ccfive, 10
'  qturnon ccfour, 10
'  qturnoff ccfour, 100
'   qturnon aatwo, 0
'   qturnoff aatwo, 10


'  qturnon cczero, 0
'  qturnoff cczero, 10

'  qprep
'  qgo

'  qwait 200

    Call ibwrt(Agilenttwo%, "VOLT 11 DBM") '11
    qwait 200

    qclearacts
             
    '+9/2 -> -9/2 transfer
        qturnon ccseven, 0  'trigger SRS two
        qturnoff ccseven, 15
   '     qturnon cctwo, 0   'rf switch
    '    qturnoff cctwo, 4   '6.667

    qprep
    qgo

    If GetInputState() <> 0 Then DoEvents
Wend
End If

 'for testing GPIB
 If f8case = 2 Then
    printnow "testing GPIB"
    
    Call ibwrt(Agilenttwo, "SWEEP:SPAC LIN")   'linear frequency sweep
    Call ibwrt(Agilenttwo, "FREQ:START 46.38 MHZ")  'start frequency of sweep
    Call ibwrt(Agilenttwo, "FREQ:STOP 47.45 MHZ")   'end frequency of sweep
    Call ibwrt(Agilenttwo, "SWEEP:TIME 0.001")   'duration of sweep
    Call ibwrt(Agilenttwo, "SWEEP:STAT ON")     'turn on sweep

 End If

If f8case = 3 Then
  'for random testing
  
  qclearacts
    qturnon ccthree, 0
    qturnoff ccthree, 30
  qprep
  qgo
 
End If

If f8case = 4 Then
  'for pulsing CMOT magnetic field
  printnow " pulsing CMOT magnetic field"
  While routine = "P"
    chg Btwo, 0
    qwait 500
    chg Btwo, 1
    qwait 8000
        
    If GetInputState() <> 0 Then DoEvents
  Wend
  
End If

If f8case = 5 Then
  'for scanning parameters
  For i = 0 To 7
  Next i
End If

If f8case = 6 Then
  'for triggering analog board
 
  qclearacts
    qturnon aasix, 10
    qturnoff aasix, 20
  qprep
  qgo

End If
 
If f8case = 7 Then
  'for triggering function generators programmed from Mathematica
  
    qclearacts
   
    qturnon aatwo, 8
    qturnoff aatwo, 18
   
    qturnon cczero, 10
    qturnoff cczero, 20

    qprep
    qgo
    
End If

If f8case = 8 Then
 
  printnow " testing"
  
 'While routine = "P"
  qclearacts

  qturnoff Bseven, 5
 
  qturnon aatwo, 13.334
  qturnoff aatwo, 14.667
 
  qturnon bbtwo, 15
  qturnoff bbtwo, 30
 
  qturnon aathree, 10
  qturnoff aathree, 15
 
  qturnon Bseven, 25

  qprep
  qgo

  qwait 100
 
 'If GetInputState() <> 0 then doevents
 'Wend

End If
 
 'the following switches between "off resonance" detuning
 'and "probe" detuning. This is helpful for making sure the current
 'loop gain is not up too high and causing ringing
If f8case = 9 Then
    printnow " toggling probe/off res"
  While routine = "P"
    clearscreen (2)
    
    printnow "probe"
    qclearacts
        qvolts ProbeDetuning, 1, 12
    qprep
    qgo
    
    chg Bone, 0

    qwait 2000
    
    clearscreen (2)
    printnow "off resonance"
   qclearacts
       qvolts OffResDetuning, 1, 12
   qprep
   qgo
   chg Bone, 1

    qwait 2000

    If GetInputState() <> 0 Then DoEvents
  Wend
End If

Call runroutine

End Sub
Sub hitF10()

'this subroutine is used to program GPIB devices independent
'of the experiment cycle

'program fast B pulses - scaled needs to be set to 0.5
'use this one for seeing a condensate
'CALL simplefastfield(.02, .75, 100, 847, 200, 5000)
'use this one for fast pulses for imaging
'CALL simplefastfield(.025, .75, 115, 884, 20, 500)
'startV, finalV, time1, time2(877), ramptime(now: up and down), holdtime

'for making pulse for slicing beam
Call simplefastfield(0, -2.5, 2000, 200 - 40, 2, 40)

'CALL simplefastfield(.025, .11, 0, 830, 150, 100)

'CALL fastfield(.02, .2, .6, 0, 877, 100, 10, 4000, 0, 0, 0, 10, 0)
'CALL fastfield(.01, .3, .01, 400, 300, 50, 19185, 100, 6000, 100, 2000, 0, 0)
'   startV, holdV, finalV/maxV, aa, bb, cc, dd, ee, ff, gg, hh, jj, kk

'program optical trap evaporation
'CALL optAgilent

'CALL IBWRT(microhp, "SYST:PRES:TYPE NORM")
'CALL IBWRT(microhp, "OUTP:MOD:STAT ON")

'qwait 1000

'CALL IBWRT(microhp, "FREQ 70 MHZ")

qwait 100

printnow "done"
routine = "O"

Call runroutine

End Sub
Sub biasfield(ByVal Vstart As Double, ByVal Vfinal As Double)

Dim a As String

'*** biastime1 and biastime2 need to be defined before entering
'*** this subroutine
  Dim rampTime, tailtime1, tailtime2, totaltime, steptime, sample As Double
  Dim ramp, tail1, tail2, npoints As Integer
  Dim i As Integer
  Dim v As Double
 
  Call ibwrt(srsthree, "*CLS")
  Call ibwrt(srsthree, "*RST")
  add(1) = 8
  add(2) = NOADDR
  Call EnableRemote(0, add()) 'make sure SRS DS345 is in remote mode
  qwait 100

  Call ibwrt(srsthree, "FUNC 5")  'arbitrary waveform
  Call ibwrt(srsthree, "AMPL 10 VP")  'max voltage is 5 V
  Call ibwrt(srsthree, "BCNT 1")  'set burst count to 1
  Call ibwrt(srsthree, "MTYP 5")  'set to burst mode
  Call ibwrt(srsthree, "TSRC 2")  'set trigger source to external, + slope
  Call ibwrt(srsthree, "MENA 1") 'enable modulation
  qwait 100

  rampTime = 10 'ms
  tailtime1 = 10   'ms
  tailtime2 = 10000  'ms '10000
  totaltime = rampTime + tailtime1 + tailtime2
  npoints = 16000  'max is 16,300
  steptime = totaltime / (npoints - 1)
  sample = 40000000 / Int(steptime * 40000)
  Call ibwrt(srsthree, "FSMP " + Str$(sample))
  Call ibwrt(srsthree, "LDWF? 0," + Str$(npoints))
  Call ibrd(srsthree, a)  'wait for response
  qwait 100
 
  printnow "Ramp bias field from " & CStr(Vstart) & "to " & CStr(Vfinal)

  checksum = 0
  tail1 = Int((tailtime1 / totaltime) * npoints)
  tail2 = Int((tailtime2 / totaltime) * npoints)
  ramp = Int((rampTime / totaltime) * npoints)
  For i = 1 To npoints
     If i <= tail1 Then
        v = Vstart
     ElseIf i <= (tail1 + ramp) Then
       v = Vstart + (Vfinal - Vstart) / ramp * (i - tail1)
     ElseIf i <= (tail1 + ramp + tail2 - 1) Then
       v = Vfinal
     Else
       v = Vfinal
     End If
     printnow CStr(v) & "   " & CStr(i - 1)
'     Call srsvolt(v, i - 1)
  Next i
  Voltarray(npoints) = checksum
  Call ibwrti(srsthree, Voltarray(), npoints * 2 + 2)

End Sub
Sub fastfield(ByVal startV As Double, ByVal holdV As Double, ByVal finalV As Double, ByVal aa As Integer, _
  ByVal bb As Integer, ByVal cc As Integer, ByVal dd As Integer, ByVal ee As Integer, ByVal ff As Integer, _
  ByVal gg As Integer, ByVal hh As Integer, ByVal jj As Integer, ByVal kk As Integer)
 
  Dim npoints, i As Integer
  Dim scaled As Double
  Dim a As String

 
  Call ibwrt(srsthree, "*CLS")

  add(1) = 19
  add(2) = NOADDR
  Call EnableRemote(0, add()) 'make sure SRS DS345 is in remote mode
  qwait 200

  Call ibwrt(srsthree, "OFFS 0.1")  'set offset - leave this in or fast pulse circuit may turn on
  Call ibwrt(srsthree, "FUNC 5")  'arbitrary waveform
  Call ibwrt(srsthree, "AMPL 5.12 VP")
  Call ibwrt(srsthree, "MTYP 5")  'set to burst mode
  Call ibwrt(srsthree, "BCNT 1")  'set burst count to 1
  Call ibwrt(srsthree, "TSRC 2")  'set trigger source to external, + slope
' CALL IBWRT(srsthree, "TSRC 0")  'set trigger source to single
  Call ibwrt(srsthree, "MENA 1") 'enable modulation
  qwait 200
    
  npoints = 13

  scaled = 0.2
 
  Call ibwrt(srsthree, "FSMP " + CStr(scaled * 1000000))
  Call ibwrt(srsthree, "LDWF? 1," + CStr(npoints))
  Call ibrd(srsthree, a)  'wait for response
  qwait 200

  checksum = 0   'a public variable

  Voltarray(0) = 0                                    'x
  Voltarray(1) = 0                                    'y
 
  Voltarray(2) = CInt(scaled * (aa))                             'x
  Voltarray(3) = 0
 
  Voltarray(4) = CInt(scaled * (aa + 10))
  Voltarray(5) = -CInt(2000 * startV / 2.5) - 80             'y
 
  Voltarray(6) = CInt(scaled * (aa + bb))             'x
  Voltarray(7) = -CInt(2000 * startV / 2.5) - 80             'y
 
  Voltarray(8) = CInt(scaled * (aa + bb + cc))
  Voltarray(9) = -CInt(2000 * holdV / 2.5) - 80
 
  Voltarray(10) = CInt(scaled * (aa + bb + cc + dd))
  Voltarray(11) = -CInt(2000 * finalV / 2.5) - 80
 
  Voltarray(12) = CInt(scaled * (aa + bb + cc + dd + ee))
  Voltarray(13) = -CInt(2000 * finalV / 2.5) - 80

  Voltarray(14) = CInt(scaled * (aa + bb + cc + dd + ee + ff))
  Voltarray(15) = -CInt(2000 * finalV / 2.5) - 80
 
  Voltarray(16) = CInt(scaled * (aa + bb + cc + dd + ee + ff + gg))
  Voltarray(17) = -CInt(2000 * finalV / 2.5) - 80

  Voltarray(18) = CInt(scaled * (aa + bb + cc + dd + ee + ff + gg + hh))
  Voltarray(19) = -CInt(2000 * finalV / 2.5) - 80

  Voltarray(20) = CInt(scaled * (aa + bb + cc + dd + ee + ff + gg + hh + jj))
  Voltarray(21) = 1000

  Voltarray(22) = CInt(scaled * (aa + bb + cc + dd + ee + ff + gg + hh + jj + kk))
  Voltarray(23) = 1000
 
  Voltarray(24) = CInt(scaled * (aa + bb + cc + dd + ee + ff + gg + hh + jj + kk + 1))
  Voltarray(25) = 0

  For i = 0 To 2 * npoints - 1
    '   printnow CStr(Voltarray(i))
       checksum = checksum + Voltarray(i)
  Next i
 
  printnow CStr(checksum)
  Voltarray(npoints * 2) = checksum
 
  qwait 100

  Call ibwrti(srsthree, Voltarray(), npoints * 4 + 2)

End Sub

Sub microsweep(ByVal fsynthesizer As Double, ByVal fstart As Double, ByVal fend As Double, _
  ByVal duration As Double, ByVal power As Double)

Dim deviation, stepsize, i, vlt, Vend, Vstart As Double

   deviation = 10 'MHz

   Vend = 20 * (fend - fsynthesizer) / deviation
   Vstart = 20 * (fstart - fsynthesizer) / deviation

'PRINT , Vend!

Call ibwrt(micro, "FM:DEV " + CStr(deviation) + " MHZ")
Call ibwrt(micro, "FREQ " + CStr(fsynthesizer) + " MHZ")
Call ibwrt(micro, "POW " + CStr(power) + " dBm")
qwait 100

stepsize = duration / 50

qclearacts

 qturnon Cten, 0

   For i = 0 To duration Step stepsize
       vlt = Vstart + (Vend - Vstart) * i / duration

      ' printnow CStr(vlt) & ",  " & CStr(i)

       If vlt > 10 Then vlt = 10
    
       qvolts vlt, i, 4

   Next i

qturnoff Cten, duration

End Sub


Public Sub pause()
    Stop
End Sub

Public Sub DACtest()
    Dim Test As Long
    Dim test1 As Long
    
    Test = 8
    test1 = 0
    qclearacts
        
       qvolts 0, 1, test1
       qvolts 3, 1500, test1
       qvolts 1, 3000, test1
       qvolts 4, 5000, Test
       qvolts 1, 5500, Test
        qvolts 3, 6000, Test
     
      'qvolts 2, 4, 9
        'qvolts 0, 50, 1
    qprep
    qgo

    Sleep (DACbuffsize)

    If 1 = 0 Then
    qclearacts
        qvolts 1, 0, 9
        qvolts 2, 10, 9
        qvolts 1, 20, 9
        qvolts 1, 40, 9
        qvolts 2, 50, 9
        qvolts 1, 60, 9
        qvolts 3, 70, 9
        qvolts 1, 80, 9
        qvolts 1, 90, 9
        qvolts 2, 100, 9
        qvolts 1, 110, 9
        qvolts 3, 120, 9
        qvolts 0, 130, 9
        qvolts 2, 0, 9
    qprep
    qgo
    
End If
End Sub
Public Sub wait(ByVal time_in_ms As Double)
'waits a time in ms, using the DIO clock
'during the wait the program checks periodically for user input
'Abs(DIOclockstatus - starttime) will cause the wait to exit if for some crazy reason the clock
'   is behind the starttime

Dim starttime As Double
Dim dummy As Integer
time_in_ms = time_in_ms * ClockFix ' This is to re-correct for the clock fix's effect on qwait
starttime = DIOclockstatus   'clockstatus function reads the DIO clock, units are ms
dummy = 0
'InputFrm.BackColor = &H80000003
Do While (time_in_ms - Abs(DIOclockstatus - starttime) > 110) And DIOclockstatus - starttime <> 0 And goflag = True
    Sleep (100)  'this wait is only accurate to a few ms
If GetInputState() <> 0 Then DoEvents     'check for user input
Loop

Do While (time_in_ms - Abs(DIOclockstatus - starttime) > 0.1) And DIOclockstatus - starttime <> 0 And goflag = True
   If GetInputState() <> 0 Then DoEvents     'check for user input
Loop
'InputFrm.BackColor = &H8000000F
'Debug.Print (DIOclockstatus - starttime)

End Sub
Public Sub qwait(ByVal time As Double)
    'for backwards compatibility with old program
    wait time
End Sub

Public Function round(toberounded As Double) As Double

'round a number to one significant digit
    Dim dummy As String
    Dim dummy2 As String
    Dim multiplier As Double
    Dim i As Integer
    Dim k As Integer
    Dim l As Integer
    
    dummy = CStr(CDec(toberounded))

    'find the first non-zero digit
    l = 0
    dummy2 = "0"
    While (dummy2 = "0" Or dummy2 = "-" Or dummy2 = ".") And l < Len(dummy)
        dummy2 = Mid(dummy, l + 1, 1)
        l = l + 1
    Wend
    If l = 0 Then
        round = 0
        Exit Function
    End If
    
    'find the decimal point, if any
    k = 0
    For i = 1 To Len(dummy)
        dummy2 = Mid(dummy, i, 1)
        If dummy2 = "." Then k = i
    Next i
    If k = 0 Then k = Len(dummy) + 1
    
    'multiply
    If toberounded < 1 Then
        multiplier = 10 ^ (l - k)
    Else
        multiplier = 10 ^ (l - k + 1)
    End If
    'Debug.Print "multiplier " + CStr(multiplier)
    'Debug.Print l, k
    
    round = CInt(multiplier * toberounded) / multiplier
    
End Function
Public Sub SortArray(ByRef TheArray As Variant, numcolumns As Integer)

'This subroutine takes the raw DACmatrix and arranges columns by increasing times.
'Called by qDACgather.

    Dim Sorted As Boolean
    Dim Temp() As Variant, X As Integer, Y As Integer

    If numcolumns = 0 Then Exit Sub
    
    ReDim Temp(numcolumns - 1)
    Sorted = False
    Do While Not Sorted
        Sorted = True
        'Debug.Print "numcolumns", numcolumns
        For Y = 0 To numcolumns - 2
            If TheArray(0, Y) > TheArray(0, Y + 1) Then
                'Debug.Print Y, TheArray(0, Y), TheArray(0, Y + 1)
                For X = 0 To UBound(TheArray, 1)  'swap y and y+1 elements
                    Temp(Y) = TheArray(X, Y + 1)
                    TheArray(X, Y + 1) = TheArray(X, Y)
                    TheArray(X, Y) = Temp(Y)
                Next X
                Sorted = False
            End If
        Next Y
    Loop

End Sub
Public Sub inputnow(prompt As String, tobeinput As Double)
   Dim B As String
   Dim a As String
   Dim i As Integer
   
   B = InputBox(prompt, "Input", 0, 200, 800)
   If Len(B) <> 0 Then
        For i = 1 To Len(B)
            a = Mid(B, i, 1)
            If Not (a = "1" Or a = "2" Or a = "3" Or a = "4" Or a = "5" Or a = "6" Or a = "7" Or a = "8" _
              Or a = "9" Or a = "0" Or a = " " Or a = "-" Or a = "+" Or a = "E" Or a = "." Or a = "e") Then Exit Sub
        Next i
        tobeinput = CDbl(B)
    Else
        printnow " Done"
    End If
End Sub
Public Sub printnow(a As String)
    InputFrm.txtMain.Text = InputFrm.txtMain.Text & vbCrLf & a
    InputFrm.txtMain.SelStart = Len(InputFrm.txtMain.Text)
    DoEvents
End Sub
Public Sub removeCharsFromMainText(numCharsToRemove As Integer)
    InputFrm.txtMain.Text = Left(InputFrm.txtMain.Text, Len(InputFrm.txtMain.Text) - numCharsToRemove)
    InputFrm.txtMain.SelStart = Len(InputFrm.txtMain.Text)
    DoEvents
End Sub
Public Sub appendToMainTextLiteral(newText As String)
    InputFrm.txtMain.Text = InputFrm.txtMain.Text & newText
    InputFrm.txtMain.SelStart = Len(InputFrm.txtMain.Text)
    DoEvents
End Sub
Public Sub clearscreen(startline As Integer)
    Dim a() As String
    Dim i As Integer

    If startline = 0 Then
        InputFrm.txtMain.Text = ""
    Else
        If startline <> 0 And InputFrm.txtMain.Text <> "" Then
            a = Split(InputFrm.txtMain.Text, vbCrLf, , vbTextCompare)
        
            For i = 1 To UBound(a) + 1
                If i = 1 Then
                    InputFrm.txtMain.Text = a(0)
                ElseIf i < startline Then
                    InputFrm.txtMain.Text = InputFrm.txtMain.Text + vbCrLf + a(i - 1)
                End If
            Next i
        End If
    End If
    
End Sub





















Sub qbiasramp(xt, flag)

     Rem Ramp should have been programed into SRS previously
     Rem Assume mag. trap is on!!!!!
       Rem Mag trap is turned on (before the ramp)
       Rem   and turned off (after the ramp) outside of this routine.
     Rem Note: ramptime is a global variable.
     Rem Ramp will start at xt.
     Rem This routine does stuff from xt-150 until xt+20+ramptime.

    'start SRS ramp
      qturnon Bzero, xt - 150
      qturnoff Bzero, xt - 130

    'switch in SRS
      qturnon bbzero, xt - 20
      qturnon aaseven, xt - 20
      qturnon bbone, xt - 20

rampback = flag
printnow "rampback flag " + rampback
If rampback = 0 Then
    'switch in new control voltage
      'Vcontrol=4.298 V
      printnow "tight trap"
      qturnon bbzero, xt + 20 + rampTime
      qturnoff aaseven, xt + 20 + rampTime
      qturnoff bbone, xt + 20 + rampTime
End If
If rampback = 1 Then
    'switch back in old control voltage
     printnow "loadtrap"
     qturnoff bbzero, xt + 20 + rampTime ' * 2 + holdtime
     qturnoff aaseven, xt + 20 + rampTime ' * 2 + holdtime
     qturnoff bbone, xt + 20 + rampTime ' * 2 + holdtime
End If
If rampback = 2 Then
     'switch in a third control voltage
     'Vcontrol=4.363 V
     printnow "really tight trap"
     qturnoff bbzero, xt + 20 + rampTime
     qturnoff aaseven, xt + 20 + rampTime
     qturnon bbone, xt + 20 + rampTime
End If
If rampback = 3 Then
     'switch in tight trap control voltage
     'after a srsrampback
     printnow "tight trap/excursion"
     qturnon bbzero, xt + 20 + rampTime * 2 + holdTime
     qturnoff aaseven, xt + 20 + rampTime * 2 + holdTime
     qturnoff bbone, xt + 20 + rampTime * 2 + holdTime
End If

If rampback = 4 Then
     'switch in a fourth control voltage
     printnow "single freq evap trap/now grounded"
     qturnon bbzero, xt + 20 + rampTime
     qturnon aaseven, xt + 20 + rampTime
     qturnoff bbone, xt + 20 + rampTime
End If

If rampback = 5 Then
'leave in SRS

End If

End Sub


Sub TestExperiment()
   'Program, strictly for testing, called by f11.
Dim ct As Integer
Dim starttime, stopTime, startPrepTime, stopPrepTime As Double

Dim inc As Double
inc = 1
ct = 0
starttime = timeGetTime
Do While ct < 150
    qTTL ct * inc, 0, 1 * (ct Mod 2)
    qvolts 1 * (ct Mod 2), ct * inc, 0
    ct = ct + 1
Loop
qTTL ct * inc, 0, 0
qvolts 0, ct * inc, 0

startPrepTime = timeGetTime
qprep
stopPrepTime = timeGetTime

Debug.Print "measured prep time for one whole test is:"; stopPrepTime - startPrepTime

qgo
stopTime = timeGetTime
Debug.Print "measured time for one whole test is:"; stopTime - starttime

End Sub

Sub Test()

    qclearacts
    printnow "Testing"
    dacRamp 1, 21, FreeValue, 0, 0.5, 0.05
 

    qprep
    qgo
  routine = "justwaiting"
While routine = "justwaiting"
   wait (200)
   DoEvents
'   If DIOclockstatus < 1000 Then printnow "restarted DIOclock"
Wend
Call runroutine

End Sub
Sub Test2()
   'Program, strictly for testing, called by f11.
Dim starttime, stopTime As Double

qclearacts

qTTL 3, 1, 0
qTTL 9, 1, 1 'trigger for scope

'qvolts 0, 4, 0
'qvolts 1, 5, 0
'qvolts 0, 6, 0

qTTL 10, 0, 0
qvolts 0, 10, 0
qTTL 11, 0, 1
qvolts 1, 11, 0
qTTL 12, 0, 0
qvolts 0, 12, 0
starttime = timeGetTime
qprep
qgo
stopTime = timeGetTime
Debug.Print "qprep time:"; stopTime - starttime

qclearacts

qTTL 1, 0, 0
qvolts 0, 1, 0
qTTL 2, 0, 1
qvolts 1, 2, 0
qTTL 3, 0, 0
qvolts 0, 3, 0
'Debug.Print "2:"; TTLcounter
qprep
qgo

routine = "O" 'back to MOT
Call runroutine

End Sub


Sub Test_DODAC()
     
'    chg Azero, 0
'    qclearacts
'        qvolts 5, 1, 1
'        qturnon Azero, 2
'    qprep
OTtrig 21000
   
 '  qgo
    
'printnow ("done digital pass")

'Call runroutine

End Sub

Public Sub gusCode()
    
   ' Call atomImage
'    Dim ti As Double
'    ti = 1
    
    'First Triggers:
'    qTTL ti, Bthirteen, 0
'    ti = ti + 0.1
'    qTTL ti, Bthirteen, 1
'   ti = ti + 0.2
'    qTTL ti, Bthirteen, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 1
'  ti = ti + 0.2
'    qTTL ti, Btwelve, 0
'     ti = ti + 0.1
'    qTTL ti, Bthirteen, 0
'     ti = ti + 0.1
'    qTTL ti, Bthirteen, 1
'     ti = ti + 0.2
'    qTTL ti, Bthirteen, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 0
'     ti = ti + 0.1
'    qTTL ti, Btwelve, 1
'    ti = ti + 0.2
'    qTTL ti, Btwelve, 0
'    ti = ti + 0.1
'    qTTL ti, Bthirteen, 0
'     ti = ti + 0.1
'    qTTL ti, Bthirteen, 1
'     ti = ti + 0.2
'    qTTL ti, Bthirteen, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 1
'   ti = ti + 0.2
'    qTTL ti, Btwelve, 0
'     ti = ti + 0.1
'    qTTL ti, Bthirteen, 0
'     ti = ti + 0.1
'    qTTL ti, Bthirteen, 1
'      ti = ti + 0.2
'    qTTL ti, Bthirteen, 0
'     ti = ti + 0.1
'    qTTL ti, Btwelve, 0
'     ti = ti + 0.1
'    qTTL ti, Btwelve, 1
'      ti = ti + 0.2
'    qTTL ti, Btwelve, 0
'    ti = ti + 0.1
'    qTTL ti, Bthirteen, 0
'     ti = ti + 0.1
'    qTTL ti, Bthirteen, 1
'  ti = ti + 0.2
'    qTTL ti, Bthirteen, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 0
'    ti = ti + 0.1
'    qTTL ti, Btwelve, 1
'    ti = ti + 0.2
'    qTTL ti, Btwelve, 0
'     ti = ti + 100

'    Dim reps As Integer
'    For reps = 1 To 10
'        While output = 0
            Call loadAndImage
            printnow ("LOADED AND IMAGED")
    
            Call atomCheck(1000, 1) 'check done thinking line
            printnow ("Done thinking!")
            Call atomCheck(1000, 2) 'check done thinking line
            printnow ("There may be an atom and Gus's code knows")
'        Wend
        
        'this last image is where the rest of the experiment should go:
        Call atomImage
        printnow ("FINAL IMAGE")
        
'    Next reps
    
    routine = "O"
    Call runroutine

End Sub

Public Sub atomCheck(maxtime_in_ms As Double, line As Integer)

'line = 1--> Done thinking line
'line = 2--> Atom check line


'This routine is used in magload to wait for a trigger from an external board (K-42) that tells us the optical trap is ready to proceed.
'Seems like this takes 56 us to run.
Dim DI_array As Byte
Dim output As Long
Dim sampsRead As Long
Dim i As Integer
Dim numloops As Long
Dim numloops2 As Long

If line = 1 Then
'This is the waiting for the optical trap trigger
'This should happen at optical trap turn off minus bias time.
    DAQmxStartTask digitalDAC_0_00
    DI_array = 0
    numloops = 0
     While DI_array = 0 And numloops < 3000 * maxtime_in_ms / 21
         numloops = numloops + 1
         output = DAQmxReadDigitalLines(digitalDAC_0_00, 1, 0.01, DAQmx_Val_GroupByChannel, DI_array, 1, sampsRead, 1, ByVal 0&)
        ' Debug.Print DIOclockstatus
        
     Wend
     If output <> 0 And errormsgflag Then
                
            printnow ("Error thrown: DI array  = " + CStr(DI_array))
             Debug.Print "DAQmxReadDigitalLines failed:" & output
             Call DACerrchk(output)
     End If
     printnow ("Read val = " + CStr(DI_array))
   
    
  '   numloops2 = 0
  '   While DI_array = 1 And numloops2 < 3000 * maxtime_in_ms / 21
  '       numloops2 = numloops2 + 1
  '       output = DAQmxReadDigitalLines(digitalDAC_0_00, 1, 0.01, DAQmx_Val_GroupByChannel, DI_array, 1, sampsRead, 1, ByVal 0&)
  '     '  Debug.Print DIOclockstatus
  '
  '   Wend
    
  '  If output <> 0 And errormsgflag Then
  '          printnow ("Error thrown in second output: DI array  = " + CStr(DI_array))
  '           Debug.Print "DAQmxReadDigitalLines failed:" & output
  '           Call DACerrchk(output)
  '  End If
Else 'Atom line
    DAQmxStartTask digitalDAC_0_01
    DI_array = 0
    numloops = 0
     While DI_array = 0 And numloops < 3000 * maxtime_in_ms / 21
         numloops = numloops + 1
         output = DAQmxReadDigitalLines(digitalDAC_0_01, 1, 0.01, DAQmx_Val_GroupByChannel, DI_array, 1, sampsRead, 1, ByVal 0&)

         'Debug.Print DIOclockstatus
     Wend
     If output <> 0 And errormsgflag Then
                
                printnow ("Error thrown: DI array  = " + CStr(DI_array))
             Debug.Print "DAQmxReadDigitalLines failed:" & output
             Call DACerrchk(output)
     End If
    printnow ("Read val = " + CStr(DI_array))
   
    
    ' numloops2 = 0
    ' While DI_array = 1 And numloops2 < 3000 * maxtime_in_ms / 21
    '     numloops2 = numloops2 + 1
    '     output = DAQmxReadDigitalLines(digitalDAC_0_01, 1, 0.01, DAQmx_Val_GroupByChannel, DI_array, 1, sampsRead, 1, ByVal 0&)
    '   '  output = DAQmxReadDigitalLines(digitalDAC_0_00, DAQmx_Val_Auto, 0.01, DAQmx_Val_GroupByChannel, DI_array, 1, sampsRead, 1, ByVal 0&)
    '   '  Debug.Print DIOclockstatus
    '
    ' Wend
     
    'If output <> 0 And errormsgflag Then
    '        printnow ("Error thrown in second output: DI array  = " + CStr(DI_array))
    '         Debug.Print "DAQmxReadDigitalLines failed:" & output
    '         Call DACerrchk(output)
    'End If
    
End If
If numloops = 1 Then
    printnow "signal was already high"
Else
    printnow "signal was low initially..."
End If

         'Debug.Print DI_array
         'Debug.Print output
         'Debug.Print numloops
         'Debug.Print numloops2
End Sub

Public Sub loadAndImage()

Dim shuttertime As Double
Dim CameraWait As Double
Dim expOffset As Double 'Time between experiments

'CMOT
Dim CMOTTime As Double

'PGC
Dim PGCDetuning, PGCtime, PGCTrapPower, PGCRepPower As Double

'Dipole Trap
Dim trapDepth As Double

'Lattice
Dim latticePower As Double

'Raman Cooling
Dim coolingTime, ROPPower, ramanField, fieldAngle, ROPDetuning, ramanFreq1, ramanFreq2 As Double
Dim ramanPow, ramanPow1, ramanPow2 As Double

'Data set generation declarations
Dim dataSet(10000), indDataPoints(1000) As Double
Dim dataPoint As Double
Dim dataRes, dataRange As Double
Dim dataLower, dataUpper As Double
Dim randValues(1000) As Double
Dim notNew, newVal As Integer
Dim uwArp As Integer
''''''''''''''''''''''''''''''''''''''''''''Parameters declarations'''''''''''''''''''''''''''''''''''''''''''''''''''

'CMOT
CMOTTime = 0

'PGC
PGCtime = 8 '10 '50 for single atom experiments 5/10/2011, AMK  '4.5 for ensembel 5/25/2011
PGCDetuning = -1
PGCRepPower = 0.1  'changed from 0.05 on 5-25-11
PGCTrapPower = 0.11 '0.12 '0.11 '4  '.145 changed from 0.1 on 5-25-11 (both by BL to get powers correct for single atom imaging)

'Dipole Trap
trapDepth = 0.604 'with New Servo Box (Tom's) and pickoff at back of cell

'Lattice
latticePower = 0.22 'full power presently
'LatticeHoldTime = 0

'Raman Cooling
ROPPower = 0.2 '0.053 'for 3D
ramanField = 0.06
fieldAngle = 134.12
ROPDetuning = -1

'Microwave
RSPow = 15 '15 '12 'for axial '18 for EOM (radial) '0 for microwave
uwArp = 0
'Camera
shuttertime = 1 '0.075 ' 0.2 '0.2

'ImagingMode

'IMAGING
Dim imaging, Flourescence As Integer
'1 Flourescence
'2 Absorption
'3 Single atom!
imaging = 3

'Experiment Structure
expOffset = 200 ' 125 ' 125



''''''''''''''''''''''''''''''''''''''''''''''Main Experiment''''''''''''''''''''''''''''''''''''''''''''''''''''

qclearacts
printnow "Running load and image"

'Will adopt a universal time variable t. Before a qprep qgo, we can increment this variable according to the schedule of events. Once qprep and qgo are called, it needs to be set back to something
'greater than zero. Need to work this out, for some reason if you have something at t = 0, it doesn't happen.

Dim i, j, k, X, Index As Integer
t = 1
    
    
   ''''''''SETTING UP FILL
   qTTL t, 15, 1
   t = t + 0.1
   qTTL t, 15, 0
   
   
  
    '''First trigger to Gus code
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.2
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Btwelve, 1
    t = t + 0.2
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.2
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Btwelve, 1
    t = t + 0.2
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.2
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Btwelve, 1
    t = t + 0.2
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.2
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Btwelve, 1
    t = t + 0.2
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.2
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Btwelve, 0
    t = t + 0.1
    qTTL t, Btwelve, 1
    t = t + 0.2
    qTTL t, Btwelve, 0
    t = t + 100
    
    
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.2
    qTTL t, Bthirteen, 0
    
   
   
   
    If False Then 'Dropping MOT
        qvolts 0, t, 6
        t = t + 50
        qvolts 7, t, 6
    End If
   
    qvolts 7, t, 6   'MOT coils on
           
    'MOT position fields for Dipole trap loading
    qvolts 0.342, t, 0 'Right, 0.117 from the perspective of looking east to west across the table
    qvolts -0.342, t, 1 'Left, -0.117, -0.29
    qvolts -0.859, t, 2 'Front, -0.65
    qvolts 0.859, t, 3 'Back, 0.65, 0.685
    qvolts -1.651, t, 4  'Bottom, -1.527, -1.737
    qvolts 1.651, t, 5  'Top, 1.527
 
 
                
    If True Then  'Turning on dipole trap
      
        qvolts 0, t, 19     'Turning off dipole trap to dump atoms
        qTTL t, Asix, 0
        t = t + 2
        qvolts trapDepth, t, 19 'set at top of code with other params
        qTTL t, Asix, 1
    End If

    qTTL t, Aone, 1     ' Repump light on
    qTTL t, Azero, 1    'MOT light on
    qTTL t, Dzero, 1    'trap shutter open
    qTTL t, Done, 1     'repump shutter open
           
    qvolts -0.19, t, 20 'Trap light to 10 MHz red '-.3 = 17 MHz, changed to 0V
    qvolts 0.38, t, 9 'Trap1 light power maximal '.38
    qvolts 0, t, 8 'trap AO set to 0
    qvolts 0.32, t, 11 'repump power
    
    ''''PHASE DITHER PARAMETERS
    If True Then
       ' Call Send(10, "*RST")
        Call Send(10, "OUTP:LOAD INF")
        Call Send(10, "FUNC SIN")
        Call Send(10, "FREQ 950")
        Call Send(10, "VOLT .8")
        Call Send(10, "VOLT:OFFS .4")
        Call Send(10, "OUTP ON")
    Else
        Call Send(10, "OUTP OFF")
    End If

    ''''FILL TIME
    t = t + expOffset

    '''PGC
    qvolts 0, t, 6 'MOT coils off
    
    'PGC FIELDS
  '  qvolts -0.5457 - 0.0609, t, 2  'Front
   ' qvolts 0.5357 - 0.0609, t, 3  'Back
   ' qvolts 0.4601, t, 0  'Right, from the perspective of looking east to west across the table
   ' qvolts -0.3301, t, 1 'Left
   ' qvolts -1.9793, t, 4  'Bottom -1.9
   ' qvolts 1.9593, t, 5
     qvolts -0.7037, t, 2  'Front
    qvolts 0.3719, t, 3  'Back
    qvolts 0.8819, t, 0  'Right,
    qvolts 0.0281, t, 1   'Left
    qvolts -2.0304, t, 4  'Bottom -1.9
    qvolts 2.0104, t, 5  'Top 1.9
   
   
    'PGC
    If PGCFlag = 1 Then
        qvolts PGCRepPower, t, 11  'Repump power reduced by factor of 16
        qvolts PGCDetuning, t, 20   'Trap light freq
        qvolts PGCTrapPower, t, 9 'trap light power

        t = t + PGCtime
        printnow "PG cooled"
        qTTL t, Aone, 0 ' Repump light off
     '   qTTL t - 1.5, Done, 0 ' Repump shutter closed 'NEED TO CHANGE IF DOING OP
        If popLower <> 1 Then
       '     qTTL t - 2, Dzero, 0 'trap shutter off
        End If
        qTTL t, Azero, 0 'Trap light off
    Else
        'qTTL t, Done, 0
        qTTL t, Aone, 0  ' Repump light off
        qTTL t, Azero, 0  'Trap light off
    End If



    Dim killPairs, coolingAxial, pumpingClock, latticeOn, transverseCoolingF1, pumping2Stretch, sidebandSpectroscopy, singleBeamImaging, filtration, pumpingF1, transverseCooling, Pushout, twoP, parametricExcitation, ramptoLowerTrap, trapFrequencyMeasurement, bluePulse, postSelectionSequence, rampSequence, recaptureSequence, delaySecondImage, threeDcooling, secondSBcool, cleanUpCool As Integer
        killPairs = 1
        bluePulse = 0
        postSelectionSequence = 1
        rampSequence = 0
        recaptureSequence = 0
        delaySecondImage = 0
        trapFrequencyMeasurement = 0
        Pushout = 1
        ramptoLowerTrap = 1
        parametricExcitation = 0
        twoP = 0
        filtration = 0
        transverseCoolingF1 = 0
        
        ''''''''''''Cooling Options''''''''''''''''
        transverseCooling = 0
        coolingAxial = 0
        threeDcooling = 1
        secondSBcool = 0 ' option within 3D cooling section
        cleanUpCool = 0 ' option within 3D cooling section
        ''''''''''''''''''''''''''''''''''''''''''''
        
        sidebandSpectroscopy = 1
        pumpingF1 = 0
        singleBeamImaging = 0
        pumpingClock = 0
        pumping2Stretch = 1
        latticeOn = 0
        
        Dim secondImageWaitTime, imageLength, finalTrap, tipField, holdTime, imagingPower, recaptureTime, bluePulseLength, killPairsLength As Double
        killPairsLength = 20 '25 '10 '25 ' length of killpairs pulse (in ms)
        bluePulseLength = 250 ' length of blue detuned pulse (ms)
        recaptureTime = 0 ' time to shut off dipole trap when doing recapture temp measure (vary between 0 and 50 us)
        secondImageWaitTime = 0 ' time (in ms) to wait before taking recapture image
        imageLength = 25 'length (ms) of each image - same for all unless specified later
        imagingPower = 0.19 '0.25 ' '.2
        holdTime = 0
        finalTrap = 0.8 '.453 ' 0.25 before adjusting pickoff
        
         'Zero field for cooling during
     '   imaging: this is necessary
         qvolts -0.7037, t, 2  'Front '-.04
        qvolts 0.3719, t, 3  'Back
        qvolts 0.8819, t, 0  'Right, '-.075
        qvolts 0.0281, t, 1  'Left
        qvolts -2.0304, t, 4  'Bottom -1.9 '-.06 -.15
        qvolts 2.0104, t, 5  'Top 1.9
      '''''
      ' start PGC dither
      qTTL t, Atwelve, 1
      ''''
        
        'start of pulse to kick out multiple atoms - 150ms
        If killPairs Then
            
        'below are te parameters for the pulse -> done before 50ms drop to let things settle
        '    qvolts 0, t, 20  'Trap light ramped to image (detuning)
        '    qvolts 0.09, t, 9  'Trap light power (0.305 for full, 0.168 for half, 0.09 for ~4.5mW)
        '    qvolts 0.11, t, 11  'Repumplight power
        '    qvolts 0, t, 8 'Standard AO frequency
          qvolts -2, t, 20 'Trap light ramped to image (detuning)
          qvolts 0, t, 21
          qvolts 0.2, t, 9 'Trap light power '.168 5/25/2011 ',17
          qvolts 0.11, t, 11  'Repumplight power '.11 5/25/2011
          qvolts 0, t, 8 'Standard AO frequency ' 0 5/25/2011
            
            'Drop cloud
            t = t + 50 'Time during which MOT is dropped and atom(s) are held in tweezer
            printnow "Dumping MOT"
            
            printnow "killing pairs"
            
            qTTL t, Azero, 1    'MOT light on
            qTTL t, Aone, 1     'repump light on
            qTTL t, Dzero, 1  'trap shutter open
            qTTL t, Done, 1     'repump shutter open
            'qvolts 0, t, 20    'Trap light freq for kill two atoms
      
            t = t + killPairsLength 'length of pulse to kick out extra atoms
            
            'end of pulse to kick out multiple atoms - 150ms
            qTTL t, Azero, 0  'MOT and repump light off
            qTTL t, Aone, 0
            qTTL t, Dzero, 0  'trap shutter closed
            qTTL t, Done, 0     'repump shutter closed
            
        End If
          
        
        

        
    '''''''''''''''''''''''''''''''''''''''''''
        
        If singleBeamImaging Then
            qvolts 0.83, t, 20 '.41 for .1 V trap
             qvolts 0.09, t, 22
            t = t + 25
            'qvolts -2, t, 21 '.41 for .1 V trap
        Else
            'below are the imaging parameters for the imaging pulse
            qvolts 0.2, t, 20  '.2'Trap light ramped to image (detuning) - 0.2 before 06-08-12
            qvolts 0, t, 21
        
             'qvolts FreeValue, t, 21  'Trap light ramped to image (detuning)
            '  IMAGING POWER SET BELOW IN SINGLE BEAM CONDITIONAL'.17'Trap light power '.168 5/25/2011
             qvolts 0.11, t, 11 'Repumplight power '.11 5/25/2011
              qvolts 0, t, 8   'Standard AO frequency ' 0 5/25/2011
            'qvolts 0.2, t, 23 'using lattice beam here as a resonant source
        End If
         
        If singleBeamImaging Then
            printnow "no wait"
        Else
            t = t + 25 ' time to let lasers reach lock, and distinguish turning off beams from KP
        End If            'with turning them on in imaging.
        
       If killPairs = 0 Then
            printnow "Dumping MOT"
            t = t + 50  'wait for MOT to fall away
       End If
         
'%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
       'Take images every second to get lifetime measurements
       '    note that the end of each loop is the end of 1 second after
       '    the end of kill pairs pulse (or end of last image)
'     For Image = 1 To 10
         
            'Drop cloud
'            t = t + 400 'Time during which MOT is dropped and atom(s) are held in tweezer

            'IS THERE AN ATOM SHOT!
            'atoms
        
        
     
        
        qTTL t - 0.1, Btwelve, 1
        qTTL t - 0.1, Ctwo, 1 'camTrig
        qTTL t - 4, Dtwo, 1  ' camShutter
        
        
        'Flourescence
        
        If singleBeamImaging Then
            printnow "single beam imaging"
            qTTL t, Afive, 1
        Else
            qTTL t, Azero, 1
            qvolts imagingPower, t, 9
        End If
        
        If singleBeamImaging = 0 Then
            qTTL t, Aone, 1 'repump light on
        End If
       
           
           
        qTTL t - 2.5, Dzero, 1 'trap shutter open
        qTTL t - 2.5, Done, 1   'repump shutter open
            
        If singleBeamImaging = 0 Then
            qTTL t, Afive, 0
        End If
        
            
        printnow "imaging trap"
        t = t + imageLength ' image time
        qTTL t, Ctwo, 0 'camTrig0
        qTTL t - 0.1, Btwelve, 0
'        t = t + 20
'        qTTL t - 1, Ctwo, 1 'camTrig
'        t = t + 100
'        qTTL t, Ctwo, 0 'camTrig0
'        t = t + 20
'        qTTL t - 1, Ctwo, 1 'camTrig
'        t = t + 100
'        qTTL t, Ctwo, 0 'camTrig0
        
        qTTL t, Dtwo, 0  ' camShutter
        
        If singleBeamImaging Then
            qvolts 0, t, 22
           qTTL t, Afive, 0
        End If
        qTTL t, Azero, 0
        qvolts 0, t, 9
        t = t + 1
        qvolts 0, t, 11  'Repump light power
        qTTL t, Aone, 0
        
        qTTL t, Dzero, 0  'trap shutter closed
        qTTL t, Done, 0     'repump shutter closed
   
    If False Then
        qTTL t, Asix, 0
        t = t + FreeValue
        qTTL t, Asix, 1
    End If
    
  
    
     If postSelectionSequence Then
     
        printnow "Post imaging cooling"
       '' below are the tweaked fields, but there seems to be no significant difference, so going back to original...
        'qvolts -0.698, t, 2  'Front '-.04
        'qvolts 0.369, t, 3  'Back
        'qvolts 0.905, t, 0  'Right, '-.075
        'qvolts 0.032, t, 1  'Left
        'qvolts -2.0304, t, 4  'Bottom -1.9 '-.06 -.15
        'qvolts 2.0104, t, 5  'Top 1.9
      ''' below are the old fields (on and before 7-23-12)
        qvolts -0.7037, t, 2  'Front '-.04
        qvolts 0.3719, t, 3  'Back
        qvolts 0.8819, t, 0  'Right, '-.075
        qvolts 0.0281, t, 1  'Left
        qvolts -2.0304, t, 4  'Bottom -1.9 '-.06 -.15
        qvolts 2.0104, t, 5  'Top 1.9
        

        qvolts -0.65, t, 20 'PGC light ramped to cool
        qvolts 0.14, t, 9 'PGC power
        qvolts 0.11, t, 11 'Repumplight power
        qvolts 0, t, 8 'Standard MOT AO frequency
            
          
        t = t + 25

        qTTL t, Azero, 1    'MOT light on
        qTTL t, Aone, 1     'repump light on
          
        t = t + 5 ' 5
         
        qTTL t, Azero, 0  'MOT and repump light off
        qTTL t, Aone, 0
        qvolts 0, t, 9 'Trap light power
        qvolts 0, t, 11 'Repumplight power
    End If
    
        
    'END PGC DITHER AFTER POST COOLING
    qTTL t, Atwelve, 0
    ''''


qprep
qgo

'routine = "O"
'Call runroutine

End Sub

Public Sub atomImage()

Dim shuttertime As Double
Dim CameraWait As Double
Dim expOffset As Double 'Time between experiments

'CMOT
Dim CMOTTime As Double

'PGC
Dim PGCDetuning, PGCtime, PGCTrapPower, PGCRepPower As Double

'Dipole Trap
Dim trapDepth As Double

'Lattice
Dim latticePower As Double

'Raman Cooling
Dim coolingTime, ROPPower, ramanField, fieldAngle, ROPDetuning, ramanFreq1, ramanFreq2 As Double
Dim ramanPow, ramanPow1, ramanPow2 As Double


'Data set generation declarations
Dim dataSet(10000), indDataPoints(1000) As Double
Dim dataPoint As Double
Dim dataRes, dataRange As Double
Dim dataLower, dataUpper As Double
Dim randValues(1000) As Double
Dim notNew, newVal As Integer
Dim uwArp As Integer
''''''''''''''''''''''''''''''''''''''''''''Parameters declarations'''''''''''''''''''''''''''''''''''''''''''''''''''

'CMOT
CMOTTime = 0

'PGC
PGCtime = 8 '10 '50 for single atom experiments 5/10/2011, AMK  '4.5 for ensembel 5/25/2011
PGCDetuning = -1
PGCRepPower = 0.1  'changed from 0.05 on 5-25-11
PGCTrapPower = 0.11 '0.12 '0.11 '4  '.145 changed from 0.1 on 5-25-11 (both by BL to get powers correct for single atom imaging)

'Dipole Trap
trapDepth = 0.604 'with New Servo Box (Tom's) and pickoff at back of cell

'Lattice
latticePower = 0.22 'full power presently
'LatticeHoldTime = 0

'Raman Cooling
ROPPower = 0.2 '0.053 'for 3D
ramanField = 0.06
fieldAngle = 134.12
ROPDetuning = -1

'Microwave
RSPow = 15 '15 '12 'for axial '18 for EOM (radial) '0 for microwave
uwArp = 0
'Camera
shuttertime = 1 '0.075 ' 0.2 '0.2

'ImagingMode

Dim killPairs, coolingAxial, pumpingClock, latticeOn, transverseCoolingF1, pumping2Stretch, sidebandSpectroscopy, singleBeamImaging, filtration, pumpingF1, transverseCooling, Pushout, twoP, parametricExcitation, ramptoLowerTrap, trapFrequencyMeasurement, bluePulse, postSelectionSequence, rampSequence, recaptureSequence, delaySecondImage, threeDcooling, secondSBcool, cleanUpCool As Integer
        killPairs = 1
        bluePulse = 0
        postSelectionSequence = 1
        rampSequence = 0
        recaptureSequence = 0
        delaySecondImage = 0
        trapFrequencyMeasurement = 0
        Pushout = 1
        ramptoLowerTrap = 1
        parametricExcitation = 0
        twoP = 0
        filtration = 0
        transverseCoolingF1 = 0
        
        ''''''''''''Cooling Options''''''''''''''''
        transverseCooling = 0
        coolingAxial = 0
        threeDcooling = 1
        secondSBcool = 0 ' option within 3D cooling section
        cleanUpCool = 0 ' option within 3D cooling section
        ''''''''''''''''''''''''''''''''''''''''''''
        
        sidebandSpectroscopy = 1
        pumpingF1 = 0
        singleBeamImaging = 0
        pumpingClock = 0
        pumping2Stretch = 1
        latticeOn = 0
        
        Dim secondImageWaitTime, imageLength, finalTrap, tipField, holdTime, imagingPower, recaptureTime, bluePulseLength, killPairsLength As Double
        killPairsLength = 20 '25 '10 '25 ' length of killpairs pulse (in ms)
        bluePulseLength = 250 ' length of blue detuned pulse (ms)
        recaptureTime = 0 ' time to shut off dipole trap when doing recapture temp measure (vary between 0 and 50 us)
        secondImageWaitTime = 0 ' time (in ms) to wait before taking recapture image
        imageLength = 25 'length (ms) of each image - same for all unless specified later
        imagingPower = 0.19 '0.25 ' '.2
        holdTime = 0
        finalTrap = 0.8 '.453 ' 0.25 before adjusting pickoff



'IMAGING
Dim imaging, Flourescence As Integer
'1 Flourescence
'2 Absorption
'3 Single atom!
imaging = 3

'Experiment Structure
expOffset = 200 ' 125 ' 125



''''''''''''''''''''''''''''''''''''''''''''''Main Experiment''''''''''''''''''''''''''''''''''''''''''''''''''''

qclearacts
'printnow "Imaging atom"

'Will adopt a universal time variable t. Before a qprep qgo, we can increment this variable according to the schedule of events. Once qprep and qgo are called, it needs to be set back to something
'greater than zero. Need to work this out, for some reason if you have something at t = 0, it doesn't happen.

Dim i, j, k, X, Index As Integer
t = 1
    
    qTTL t, Bthirteen, 0
    t = t + 0.1
    qTTL t, Bthirteen, 1
    t = t + 0.1
    qTTL t, Bthirteen, 0
If True Then ''Second image for either lifetime, or experiment in which previous image
                                    
        If False Then
        
            qvolts 0.83, t, 20 '.41 for .1 V trap
             qvolts 0.09, t, 22
            t = t + 25
            
            qTTL t - 0.1, Ctwo, 1 'camTrig
            qTTL t - 4, Dtwo, 1  ' camShutter
        
            printnow "single beam imaging"
            qTTL t, Afive, 1
       
           
           
            qTTL t - 2.5, Dzero, 1 'trap shutter open
            qTTL t - 2.5, Done, 1   'repump shutter open
        
            
            printnow "imaging trap"
            t = t + imageLength ' image time
            qTTL t, Ctwo, 0 'camTrig0
        
            qTTL t, Dtwo, 0  ' camShutter
        
            qvolts 0, t, 22
           qTTL t, Afive, 0
        
            qTTL t, Dzero, 0  'trap shutter closed
            qTTL t, Done, 0     'repump shutter closed
        
        Else '''MOT IMAGING''''
                                
        
            '''PGC DITHER AGAIN
             ' start PGC dither
             qTTL t, Atwelve, 1
             ''''
        
        
            qvolts -0.7037, t, 2  'Front '-.04
            qvolts 0.3719, t, 3  'Back
            qvolts 0.8819, t, 0  'Right, '-.075
            qvolts 0.0281, t, 1  'Left
            qvolts -2.0304, t, 4  'Bottom -1.9 '-.06 -.15
            qvolts 2.0104, t, 5  'Top 1.9
            
            t = t + 0.1
        'below are the imaging parameters for the imaging pulse
         
            qvolts 0.25, t, 20 'Trap light ramped to image (detuning)
            qvolts 0, t, 21
            qvolts imagingPower, t, 9 'Trap light power
            qvolts 0.11, t, 11  'Repumplight power '.11 5/25/2011
            qvolts 0, t, 8   'Standard AO frequency ' 0 5/25/2011
            'qvolts 0.2, t, 23 'using lattice beam here as a resonant source
                                      
            t = t + 30
        
            If delaySecondImage Then
                    t = t + secondImageWaitTime 'Probability if keeping the atom after this amount of time ********
                End If
                    printnow "Imaging again"
            'IS THE ATOM STILL THERE SHOT
        
        
            'atoms
                qTTL t - 0.1, Btwelve, 1
                qTTL t - 0.1, Ctwo, 1 'camTrig
                qTTL t - 4, Dtwo, 1 ' camShutter
        
            'Flourescence
                 qTTL t, Azero, 1 'MOT and repump light on
                 qTTL t, Aone, 1
            
            
                qTTL t - 2.5, Dzero, 1 'trap shutter open
                qTTL t - 2.5, Done, 1   'repump shutter open
            
                  t = t + imageLength 'length of post select image
                qTTL t, Ctwo, 0 'camTrig0
                qTTL t, Btwelve, 0
       
                qTTL t, Dtwo, 0 ' camShutter
        
                qTTL t, Azero, 0   'MOT and repump light off
                qTTL t, Aone, 0
                qTTL t, Dzero, 0  'trap shutter closed
                qTTL t, Done, 0     'repump shutter closed
            
                'stop PGC dither
                qTTL t, Atwelve, 0
                ''''
            End If
       End If


qprep
qgo

wait (1000)
'routine = "O"
'Call runroutine

End Sub

