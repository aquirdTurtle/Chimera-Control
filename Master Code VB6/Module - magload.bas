Attribute VB_Name = "Magmodule"
Option Base 0 'This just indicates that arrays are zero-indexed.
'This Module was reorganized and partially rewritten by BJL on 150910.
'       I am trying to incorporate more conditional statements and subroutine calls to make
'       setting up and switching between different experiments fairly simple, while still
'       maintaining the flexibility of the code to be altered for customized experimental
'       sequences, as much as possible.
' Of course, some reorganization will be necessary as the sequences become more complex,
'       and its possible that we would benefit by making a couple of separate subroutines that
'       contain entire sequences for very different types of experiments, but at the moment I
'       do not think its necessary...


' ''''''''''''''''''''''''''''''''''''''''
'
'   UPDATES
'
' (1) change of the 795loading and change tunneling parameter form vertical to horizontal 5/18/2016
' (2) Pressing Abort doesn't try to print the "executing" mesage a bunch of times any more. (Mark, 5/~20/2016)
' (3) "Executing #" Statement changed to only output one line and to replace the # on subsequent runs.\
' All aditions after this had to be reprogrammed after the power outage on 5/25/2016 cause the computer to shut off suddenly.
' (4) String settings set at the beginning of the code are no longer case-sensitive (Mark 5/25/2016)
' (5) Unclicks abort button at end of aborting and notifies user (Mark, 5/25/2016)
' (6) Changed "TakeStandardPGCImage" from subroutine to function, taking in the imaging length as a parameter. (Mark, 5/25/2016)
' (7) Experiment no longer outputs 1 number for every variation, but rather the total number of variations as well as the variation # the system is
' Currently executing.

'=========================================================================================
'--> Start of main experimental sequence (Magload):
Public Sub Magload()
    '-------------------------------------------------------------------------------------
    '-----> Relevant variable Declarations for the full experimental procedure (temp variables set within subroutines)
    Dim i, j, k, numRSfreqs As Integer ' these are the indices used for stepping through freevalues
    Dim defaultTweezerDepth, enhancedLoadTrapDepth, carrierFreq, secondMicroFreq, triggerOffset, alignmentPulseLength, alignmentPulsePower, depumpTestPulseLength, repumpTestPulseLength, RamseyEvolveTime As Double
    Dim specAxis, tuningParam, RemseyTestParam, initiateTunneling As String
    Dim fillMOTtime, dataLower, dataUpper, fieldShim As Double
    Dim importDataset As Boolean
    
    Dim controlWithNIAWG, sendToNIAWG, enhancedLoading, loadTesting, newProceduresTest, alignProbe, depumpAtoms, ramanSBCooling As Boolean
    Dim carrierRabiCurve, sidebandSpec, tunnelingDynamics, spinExchange, entangleVerify, microSingleAtom, microGlobalRotate, carrierCal As Boolean
    Dim pushoutF2, pushoutImaging, specDifferentDepth, calCoolingFreqs, sigmaRepumpingTest, sigmaDepumpingTest, sigmaPolarizationTest, RamseyEchoTests As Boolean
    Dim RamseyNonEchoTest, oneWayARPtesting, rotateQuantAxisFB, rotateQuantAxisBack, probeImageTest, repeatMicroWave, atomShuttlingImaging As Boolean
    '-------------------------------------------------------------------------------------
    'Setting global constants that should not change on a regular basis:
    '       For example, sideband cooling frequencies, which may need to be recalibrated,
    '       and trap configurations that only change between experiments.
    
    'Traps configurations: (*This will likely need to be significantly modified once we move to NI AWG...*)
   ' defaultTweezerDepth = -9.45 <==== commenting out so errors are thrown if i try to use this!
   ' enhancedLoadTrapDepth = -9.45 'Will have to calibrate - currently this is not deep enough (should be ~3.5 mK)
    rightWellFreq = 82000000
    leftWellImageFreq = 74500000
    leftWellTunnelFreq = 77915000
    AOholdTime = 35
    
    'Tunneling, spin exchange, and ent. verification constants:
    tunnelDACdepth = -8.416 'Ramp to this depth for transfer to ARB control
    exchangeARPtime = 18 'Length of the bias ARP used to load/unload atoms for spin exchange
    exchangeARPrange = 0.1 'How far above/below the bias resonance the ARP should start/end (a 0.1V range ramps from 0.1 above to 0.1 below)
    gradField = 7 'This is the gradient used for entanglement verification, do not exceed 8 V, 7 V is typical.
       
    exchangeTime = 15 '4.5
    gradientTime = 6.5
    
    'Sideband cooling:
    topRSBcooling = 69855000 'may switch back to: '69855000 '69852000 'was 19858000 when using mixer boxes, now directly using tek AFGs
    botRSBcooling = 69857000 'may switch back to: '69857500  'was 19858000 when using mixer boxes, now directly using tek AFGs
    axialRSBcooling = 69965000 '69958000 '69940000 'was 19957000 when using mixer boxes, now directly using tek AFGs

    'Microwaves:
    RSPow = 16 'standard power for both microwave rotations and Raman spectroscopy/cooling 150911
    mwPulseLength = 0.0132 '0.0132 '0.066 '0.01312 for the square pulse '0.03 for the Gaussian pi pulse, 0.014 for square pi pulse '0.045 'Length of microwave pi-pulse. 30us for full gaussian pulse 150911 (*Note that this is a public variable*)

    'Pumping beam test parameters:
    depumpTestPulseLength = 8 '0.015 '8 '0.5
    repumpTestPulseLength = 0.02 '0.5
    alignmentPulseLength = 0.01 '0.015 '0.004 '0.025 'Length of pushout pulse used for aligning beams - typically 0.01 ms, sometimes 0.004
    alignmentPulsePower = 0.09 ' 0.01 'power in probe beam during pulse. typically 0.01 V, and don't go above 0.04 V without a shutter on side MOT beam!! (and with shutter and ND 3+1 on launcher, use 0.05 V for EO beam path)
    
    'probe imaging parameters
    imagingLengthProbe = 26 '7.5 '0.4 '7 ' length of images, which is used for lossless images
    imDetuneProbe = 1.1 '1.255 '1.37 ' detuning used for probe images - should be resonant with cycling transition in imaging trap
    
    'Standard parameters
    imagingLength = 25 ' length of images, which is used for standard PGC images
    fillMOTtime = 175 'Length of time to fill MOT before initiating load into tweezers
    triggerOffset = 0.11 ' Length of trigger sequences (used to keep tunnelingTimeTracker in sync)
    'finalTrapDepth = -0.45 'final trap depth ramped to for "spectroscopy in different depth" <== irrelevant, so want error to be thrown when called!
    '-------------------------------------------------------------------------------------
    'Setting default values for variables that are changed/varied frequently:
    '       For example, the carrier frequency and RSFreq are regularly varied and then set
    '       between different runs, so we set these values in a separate section.
            
    tuningParam = "RSFreq"  'what is varied? -->  "RSFreq", "piezoMirror", "tunnelBias", "tunnelTime", "rabiPulseLength",
    '"RSBcoolingFreq", "depumpPulseLength", "repumpPulseLength", "microwavePulseLength", "RamseyTimeScan",
    '"probeDetune", "arpHold", "arpRange", "arpTime","relativeBias", "sideBiases", "RSFreqSpinExchange", "CarrFreqSpinExchange" , "fieldRotatingBackSpinExchange"
    '"piezoMirrorSpinExchange" etc.
    importDataset = False
    dataLower = 6.830915 '0.47  '0.521 '6.83092  '0 '0.0117 '0.0117 '6.830925 '6.830905 '6.5  '6.83092  '0.0115 '6.830925 '0.521 '6.830915 '6.830925 '0.0118 '6.830925 '0.463  '0.478 '0.521 '6.841275 '95000 '6.84102 '6.840985 '0.46 '0.48  '0.53 '0.521 '6.840985 '0 '0.497 '6.83091 '6.841 '0 '5 '0.507 ' 0.497  '6.83091 ' 0.497  '0.4945 '6.83092 '6.840985  '6.83092   ' 0 '0.505 '0.4942 '6.83092 '0 '0.5158  '0.5105  '0.4995  '2.5 '0.4955 '6.83093  '0.4945 '6.84091 '0.509  '6.83092 '6.84091 '0.95 '0.509 '0.35 '6.83088  '0.536   '0.536  '0.5012 '0.503 '6.83088 '0.503 '0.50275 '0.5025 '0.502 '6.83093 '6.83088   '69890000 '6.83088  '6.83089 '0  '0.503 '0.522 '6.830905 '6.840975 '6.8309
    dataUpper = 6.831005 '0.474  '0.526  '6.831025 '5 '0.015 '0.015 '0.3 '6.83103   '6.831005   '10 '6.831025   '0.015 '6.83103 '0.53 '6.831005 '6.831025 '0.01311 ' 6.84108  '0.498 '0.488 '0.53   '6.841475 '160000 '6.84135 '6.84115 '0.5  '0.487  '0.545  '0.53  '6.84115   '8 '0.503 '6.831   '6.8412 '5 '10 '0.515 '0.503  '6.831   '0.503  '0.499 '6.83102 '6.84121 '6.83102  ' 4 '1.5 '0.512 '0.4992 '6.83103   '10 '0.5188  '0.5215  '0.4995 '0.5075 '5 '0.4995 '6.83102  '0.498 '6.84107 '0.53 '6.83104 '6.84107 '1.45 '0.53  '1.1 '6.83104 '0.556  '0.551 '0.5062 '0.51  '6.83104 '0.517 '0.50675 '0.5055 '0.507 '6.83104 '6.83104   '69990000 '6.83105   '6.83104 '0.075  '0.509 '0.527 '6.831015 '6.841125 '6.83105
        
    'carrierFreq cal 6.830915 - 6.831005 for polynomial fit with 200 accuum and 8 pts, old: 6.83088 - 6.83104, typically use the top beam, remeber to set to square waveform and back to Gaussian afterwards
    'rabiPulseLength range 0 - 0.075, pi time 12 us for top beam, need to set the carrierFreq, use 20 points
    'axial SBspectrum 6.830925 - 6.83103 ' zoommed in 6.83099 - 101, set RSFreq = 6.83*** below for rabi scan, typically 30 points, typically gaussian pulses
    'axialRSBcooling cal 69890000 - 69990000
    'top/botRSBcooling cal 69890000 - 69990000
    'top SB spec with cooling, set import data to be true and the waveform gen to Gaussian, set numRSfreqs to 2
    'top/botRSB rabi range 0 - 0.075, typically 15 points
    'vertical config tunnelBias cal: 0.5015 - 0.507
    'horizontal config tunnelBias cal: somewhere between 0.490 - 0.510, but not yet found...
    'tunnelTime valuse for bias:
            ' - 0.45 ms for vertical, 808nm spacing, 28.8 uW
            ' - 0.8 ms for vertical, 808 nm spacing, 42 uW
    'tunnelBias range 0.4925 - 0.4995 '200 accum 15 pt, 0.501 - 0.507, tunnel time 0.45 at 12/30/2015, tunnel time 1.1ms at 02/15/2016, 150 accum, 20 pts
    ' tunnelBias vert, 900nm spacing, gg tun, 5 uW: 0.521 - 0.529, tunnel time 0.9
    'RamseyEvolveTime scan variable 0.1 - 0.25 for the EO beam for alignment, need to set RSFreq for uwave to be resonant e.g. 6.841007
    'Raman beam alignment with Ramsey, set piezoMirror range 0-10, need to set RSFreq for uwave to be resonant e.g. 6.841007
            'DAC 23, 15 points, need to set to square pulse 305.5mV, microwave pi time for square pulse 0.014
            '
            'top beam: set RamseyEvolveTime to 75 us, Ramsey pi time 31 us, PZT X 4.5-6.1, PZT Y 2.9-5.9
            'EO beam: set to 167 us on 160104, pi time 25.6 us
            'axial beam: set to 160 us, pi time 100 us on 160104,
    
    'microwave spectrum, nothing special: 6.840875 - 6.841075 GHz for single pulse, 6.840965 - 6.84102 for five pulse (6.84092 - 6.84108 for 14us pulse, 97-102 for 55us pulse), Gaussian or square pulse. Square for ramsey experiments.
    'piezoMirror range for Raman beam alignment  0 - 10 in volts (??? That's not volts...)
    'depumpPulseLength scan range 0-0.06
    'repumpPulseLength scan range 0-0.04
    'light shifted resonance freq cal 6.84065 - 6.8409, rotated filed unshifted 6.841 - 6.8411
    'CarrFreqSpinExchange 6.8309615 - 6.830963
    
    numRSfreqs = 1 ' RSFreq is always programmed as the last freq. for 2, carrier freq is 1st and RSfreq second
    carrierFreq = 6.830966     '9695    'Frequency used for 3D cooling
    secondMicroFreq = 6.8409995    '6.841032 '**Only used when programming 3 freqs to RS (this is middle freq in list)
    readoutMicroFreq = 6.841067  ' for microwav e pi pulse during pushout imaging sequence
    RSFreq = 6.840998    '6.8409975  '6.8409 '6.8409931 '6.840996 '6.840999 '6.830961 '6.8409981   '6.8409975 '6.840994 '6.8410755 '6.840975 '6.841007 ' for uwave '6.84112 '6.841042 'When not varied, set RSfreq here (for spin flips, etc.)
    
    tunnelTime = 1.2 '1.5 '1.1 '1.75 '0.8 '2 '0.8 '0.6265 '1.1 ' 0.5 '0.9 '0.85 '1.5 '0.9 'This variable is mainly used to set the tunneling time for bias curves.
    tunnelBias = 0.5247 '0.5391 '0.5254 '0.49945 '0.497 '0.5097 '0.4965 '0.5172 '0.51625 '0.4966  '0.5443 'This variable is sent to the NIAWG to set the bias during time traces -- use no more than 6 digits of resolution after the dcimal place!!
    initiateTunneling = "frequency" ' options are "intensity" or "frequency" or "bias"
    
    AOfreqRampTime = 7.5 '+++ was 16 change happen 5/18/2016 5 '10 ' total length of freq ramps (currently do bias during this, so bias ramp time (below) is 0!
    rampToTunnelBiasTime = 1.5 '+++ was 0.25, change happed 5/18/2016 '0 '0.25 '0.2 '0.3 ' 0.25 '0.5 '5.6 '0.25 '5 '2 'in ms, length of hold for ramp to tunneling bias (or initial bias for spin exch)
    intRampDownTime = 20 '15 '20
    intRampUpTime = 20 '10
    
    ARPinitBias = 0.495  '0.505  ' the bias at which to start the ARP (and thus, to ramp to during the frequency sweep together)
    ARPfinalBias = 0.47 '0.47  '0.4835 '0.463 ' the bias at which to end the ARP (and thus to ramp from during freq sweep apart)
    timeOfARP = 3.1 '3.1 05112016 '3.1  '6 '21 '17.5 '3 ' how long should the ARP take?
    holdAfterARP = 0.1 ' time to hold after arp is done before either ARPing back or ramping apart
    
    '-------------------------------------------------------------------------------------
    'Series of flags/selections for setting the experimental sequence below:
    controlWithNIAWG = False ' flag to determine whether to coordinate run with NI AWG
    sendToNIAWG = controlWithNIAWG 'False
    
    enhancedLoading = False
    loadTesting = False
    newProceduresTest = False            'section for temp code hidden inside of "loadTesting" section

    carrierCal = True
    ' setup to do square pulses
    carrierRabiCurve = False 'True ' setup to do square pulses
    calCoolingFreqs = False  ' scan the shifted freq (FSK) on the AFG, to be used with "sidebandSpec" section, no cooling!
    
    ramanSBCooling = False
    sidebandSpec = False
    specAxis = "top" ' Choices are "top", "bottom", and "axial" <-case sensitive!

    specDifferentDepth = False 'True
    microGlobalRotate = False
    rotateQuantAxisFB = False
    rotateQuantAxisBack = rotateQuantAxisFB ' false ' At the moment, this will always turn on when we rotate the QA, but dont need to
    microSingleAtom = False
    repeatMicroWave = False
    
    tunnelingDynamics = False
    oneWayARPtesting = False
    twoWayARPtesting = False
    spinExchange = False ' still need to modify this for using NI AWG!!!
    entangleVerify = False ' still need to modify this for using NI AWG!!!
    
    pushoutF2 = True
    pushoutImaging = False ' if this is on, make sure pushoutF2 is false!
    alignProbe = False
    depumpAtoms = False ' this  is not set up at the moment, but would be for depumping the atoms before other experiments, not to test pumping beams (those are below)
    
    'Add more flags for testing/characterization experiments below:
    sigmaDepumpingTest = False ' this section applies a repump pulse with the MOT repump before varying the length of an OP pulse (which will depump to F=1)
    sigmaRepumpingTest = False ' this section first depumps the atom (using depumpAtoms via OP beam only) and then varies length of sigma repump pulse
    sigmaPolarizationTest = False ' this section tests the polarization purity by trying to scatter out of the dark state with OP light (after pump2stretch)
    
    probeImageTest = False
    atomShuttlingImaging = False
    
    RamseyEchoTests = False ' use square microwave pulse, do not use Gaussian pulse, also change Raman beam(s) to square pulses
    RemseyTestParam = "axial" '"EObeam","top","bottom","axial", "LSbeam"   <---- to choose which beam to turn on during 1st leg of Ramsey echo. Mainly for beam alignment, but can also measure light shifts
    RamseyEvolveTime = 0.06   'at most 60 us for the top beam Ramsey 2pi time. Scan this to find a good time before beam alignment
    RamseyNonEchoTest = False
    
    ''''
    'Maybe set up a conditional here that automatically sets the "numRSfreqs"?
    ''''
    
    If automateFlag Then
        Call createDataSet(importDataset, dataLower, dataUpper)
    End If
    
    'Start listening for a message from the NI AWG:
    If controlWithNIAWG Then
        InputFrm.tcpserver.Listen
    End If
    Dim niawgReady As Boolean
    niawgReady = True
    If sendToNIAWG And controlWithNIAWG Then
        Call sendMessagesToNIAWG(tuningParam)
        If InputFrm.tcpserver.State <> sckConnected Then
            ' so don't try to start experiment
            niawgReady = False
        End If
    End If
    
    ' big loop to skip if canceled without connecting to niawg.
    If (niawgReady = True) Then
        '-------------------------------------------------------------------------------------
        ' Beginning Main Experiment loop(s) below:
        printnow "//////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\"
        printnow "//////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\"
        printnow "Beginning Experiment."
    
        For i = 1 To expRep
        'Beginning of the loop to program a sequence before repeating for accumulations
            printnow ("Executing Variation " + Str(i) + " / " + Str(expRep))
            t = 1
            qclearacts
            flagFromNIAWG = False
            writeNewExperiment = 1
    
            If automateFlag Then
                FreeValue = indDataPoints(i - 1)
                
                ' The case-structure below sets the relevant variable to be varied (as
                '       compared to all the "default" values above):
                Select Case LCase(tuningParam)
                'The string "tuningParam" decides what is going to be varied during the run via a
                '       "Select" casestructure at the beginning of the for loop. The main values it can
                '       take on are: "RSFreq", "piezoMirror", "tunnelBias", "tunnelTime", etc.
                    Case "rsfreq": RSFreq = FreeValue 'Vary RSfreq for spectra (microwave, sideband, etc.)
                    Case "rsfreqtunneling": RSFreq = FreeValue 'Vary RSfreq for spectra (microwave, sideband, etc.)
                    Case "tunnelbias": tunnelBias = FreeValue
                    Case "tunneltime": tunnelTime = FreeValue
                    Case "exchangetime": exchangeTime = FreeValue
                    Case "gradienttime": gradientTime = FreeValue
                    Case "piezomirror": 'qvolts FreeValue, t, 23 'Set piezo mirror angle for beam alignment
                        dacRamp t, 23, 5, 0, FreeValue, 0.05 'considering using a ramp here, since jumps can be bad, but would need a ramp back at the end...
                        t = t + 5
                    Case "rabipulselength": rabiPulseLength = FreeValue
                    Case "rsbcoolingfreq":
                        topRSBcooling = FreeValue
                        botRSBcooling = FreeValue
                        axialRSBcooling = FreeValue
                    Case "microwavepulselength": mwPulseLength = FreeValue
                    Case "repumppulselength": repumpTestPulseLength = FreeValue
                    Case "depumppulselength": depumpTestPulseLength = FreeValue
                    Case "ramseytimescan": RamseyEvolveTime = FreeValue
                    Case "probedetune": imDetuneProbe = FreeValue
                    Case "arptime":
                        timeOfARP = FreeValue
                        tunnelTime = timeOfARP + arpHold ' because this is the actual time used to hold for the ARP
                    Case "arphold":
                        holdAfterARP = FreeValue
                        tunnelTime = timeOfARP + holdAfterARP ' because this is the actual time used to hold for the ARP
                    Case "secondmicrofreq": secondMicroFreq = FreeValue
                    Case "rsfreqspinexchange":
                        RSFreq = FreeValue
                    Case "fieldrotatingbackspinexchange"
                        fieldShim = FreeValue
                    Case "carrfreqspinexchange"
                        carrierFreq = FreeValue
                    Case "piezomirrorspinexchange": 'qvolts FreeValue, t, 23 'Set piezo mirror angle for beam alignment
                        dacRamp t, 23, 5, 0, FreeValue, 0.05 'considering using a ramp here, since jumps can be bad, but would need a ramp back at the end...
                        t = t + 5
                    Case "none":
                End Select
                
                printnow CStr(Format(FreeValue, "0.000000000"))
                
                '''TEMPORARY!!!
                'topRSBcooling = FreeValue
                'botRSBcooling = FreeValue
                'axialRSBcooling = FreeValue
                '----------
                
            End If
    
    
            '---------------------------------------------------------------------------------
            ' Turning on the Dipole trap with standard parameters:
            qTTL t, Dten, 0 'Switch for left-well RF (for dropping left-well atoms, if desired)
    
            qTTL t, Asix, 1
            'qvolts defaultTweezerDepth, t, 19  'set at top of code with other params
            
            '---------------------------------------------------------------------------------
            'Explicitly initializing important TTLs at the beginning of each sequence:
            qTTL t, Bfifteen, 0 'Trigger for tweezer freq ramps initialized to 0
            qTTL t, Bsix, 0 'Trigger for Arb control of tweezer intensity ramps initialized to 0
            qTTL t, Bfive, 0 ' Switch for trap intensity control initialized for DAC control (1 is for Arb Control)
            qTTL t, Dthree, 0 'Trigger for Arb control (Gaussians) of µw/axial Raman initialized to 0
            qTTL t, Athirteen, 0 'Trigger for Arb control (Gaussians) of bot/top Raman initialized to 0
            qTTL t, Cthree, 0 'Trigger for RS generator (freq switching) is initialized to 0
            qTTL t, Celeven, 0 'Oscilloscope trigger initialized to 0
            qTTL t, Afifteen, 0 ' Oscilloscope marker (used for timing checks) set to 0
            qTTL t, Dtwelve, 0 'This may have been the trigger for microwave ARPs (but that doesnt look set up at the moment).
                    'Dtwelve was unused in the tunneling code, so may just want to delete, unless we figure out what it is for...
            
            'Setting Shutters: (Note specifically, that most shutters are not actuated within
            '       subroutines (both for flexibility and longevity of the shutter lifetimes, the
            '       main exception is the camera shutter), so we need to manually open/close
            '       these throughout the code!
            qTTL t, Cfour, 0 ' Raman Beams shutter closed (default, independent of LS now)
            qTTL t, Bthree, 0 ' LS beam shutter closed (default, independent of Raman now)<----need to set up separate TTL!!
            qTTL t, Dzero, 1 'MOT shutter open (from Slave laser)
            qTTL t, Bone, 1 'Repump shutter open (labelled F=1; blocks both repump fibers)
            qTTL t, Cthirteen, 1 'This is now used for the side MOT beam (independent of other beams)
            qTTL t, Ctwelve, 0 'alignment/probe shutter closed (should be closed most of the time)
            qTTL t, Btwo, 0 ' this is the OP shutter, separate from side MOT adn probe
            '----> May have to add some shutters here for the enhanced loading and magic LS beam (when available)
            
            
            t = t + 0.1
            
            '---------------------------------------------------------------------------------
            'Starting Experimental Sequence:
            Call triggerOscilloscope
            Call turnOnMOT
            Call initRamanAFGs
            Call initLowFreqFGs
            
            Select Case numRSfreqs
                Case 1:
                    Call programOneRSMicroFreq(pushoutImaging, readoutMicroFreq)
                Case 2:
                    Call programTwoRSMicroFreqs(carrierFreq, pushoutImaging, readoutMicroFreq)
                Case 3:
                    Call programThreeRSmicroFreqs(carrierFreq, secondMicroFreq, pushoutImaging, readoutMicroFreq)
            End Select
            printnow "All FGs initialized"
            
            '*********************************************************************************
            '***** This is where we will want to add code for any initial communications with
            '               the NI AWG related to telling it what frequencies to use (if we go that
            '               way), but the place where we will add holds to make sure that the
            '               waveforms are programmed before starting is below (after the
            
            
            
            If enhancedLoading Then
                'Will want to add commands here to open/close the enhanced loading shutters
                qTTL t, Bnine, 1
                qTTL t, Bten, 1
                
                Call loadTrap795(fillMOTtime)
                
                qTTL t, Bnine, 0
                qTTL t, Bten, 0
            Else
                t = t + fillMOTtime
                Call loadTrapPGC
                printnow "PGC loading complete"
            End If
            
            'Closing MOT shutter (slave laser path) here, but others stay open until after image.
            '       -> Not closing the shutter if there is no SB cooling (to save shutter, and it wont matter much without SB cooling...)
            If ramanSBCooling Then
                qTTL t, Dzero, 0 '<---------------------------------------------------uncomment this when shutter is back in slave path!!!
            End If
            If Not atomShuttlingImaging Then
                Call takeStandardPGCimage(imagingLength)
            End If
            ''qTTL t, Cthirteen, 0
            Call inTrapPGC
            
             
            If sigmaDepumpingTest Then
                printnow "not calling pump2stretch, but still need to add subroutines for OP tests!"
                Call populateF1(depumpTestPulseLength)
                'Call setQuantAxis
            ElseIf sigmaRepumpingTest Then
                Call populateF1(depumpTestPulseLength)  'typically use: 0.5 ms, but for this want to be sure!
                Call sigmaRepumpTest(repumpTestPulseLength)
            ElseIf sigmaPolarizationTest Then
                Call pump2stretch
                Call sigmaDepumpTest(depumpTestPulseLength)
            Else
                Call pump2stretch
            End If
            
            '---------------------------------------------------------------------------------
            '==> At this point in the code, we have loaded and imaged the traps, then
            '               initialized all of the loaded atoms in the |2,2> state.
            '        Any new procedures/experiments should be added below. Integrating into the
            '               current structure is good practice once everything is working, but for
            '               testing, you can just add a new temp section that skips the rest by
            '               putting it inside of the "loadTesting" section.
                
            If loadTesting Then
                
                If newProceduresTest Then
                        'Insert temporary code and calls for alternate procedures here, so that
                        '       the current code written out below stays the same!
                        ' If you are trying to integrate into code below, then just copy the
                        '       relevant calls into this section along with the new code, and when
                        '       it is finalized, integrate into the main code below!!!
                       
                        Call setQuantAxis '<--- this should already be set, but if not, we may need to add extra settling time?
                        t = t + 50
                        
                        qTTL t, Dzero, 0
                        qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers) <-- currently not installed, but will add Randall's shutters here 160228
                        qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                        qTTL t, Cfour, 0
                        printnow "Close shutters for probe image (if not already)"
                        
                        qTTL t, Dtwo, 1 ' camShutter open
                        qTTL t, Ctwelve, 1
                        printnow "open camera and probe shutters early"
                        
                        t = t + 20 ' we should be able to compact this a bit more by moving the shutter triggers around, but for now, just testing!
                        
                        printnow "Probe beam imaging test"
                        imagingPowerProbe = 0.275 '0.09 '0.11 '0.15 ' max power corresponds to ~25nW on 160228
                        dacRamp t, 20, 4, 0.735, imDetuneProbe, 0.1 '0.735 is the typical setpoint for pump2stretch and cooling
                                                                    '  - it may change after that, but jumping there shouldnt be a problem.
                        t = t + 5 ' this serves as an offset time for the probe shutter to open, so do not add time between calling this subroutine and trigerring shutter!
                        
                        'Camera triggers (camera and shutter)
                        qTTL t - 0.05, Ctwo, 1 'camTrig
                        'qTTL t - 4, Dtwo, 1 ' camShutter
                        
                        'Applying probe pulse!! 'copied from the pushout subroutine
                        qvolts imDetuneProbe, t, 20 'probe detuning set to final value for image
                        qvolts imagingPowerProbe, t, 22
                        
                        qTTL t, Afive, 1
                        t = t + imagingLengthProbe
                        qTTL t, Afive, 0
                        qTTL t, Dtwo, 0     ' CamShutter
                        
                        qvolts 0.022, t, 22 ' this is closer to the minimum power than 0V...
                        qTTL t, Ctwo, 0     ' CamTrig0
                        
                        t = t + 25 ' this is a hold time to make sure we arent actuating the camera shutter too fast!
                                    ' Might actually wnat to move this before and then the probe shutter starts the image while the camera shutter ends it, just to make sure there is no extra leakage will try this first, since there seems to be nearly no leakage from probe...?
                        
                        'Shutting off all light and closing the relevant shutters:
                        
                        qTTL t, Dtwo, 0     ' CamShutter
                        
                        'qTTL t, Asix, 0
                      '  Call triggerArbIntensityRamps
                      '  t = t + 5
                      '  Call triggerArbIntensityRamps
                      '  t = t + 10
                        
                        t = t + 1
                        
                        t = t + 0.01
                        
                        
                        
                Else
                    printnow "Load testing - Only one image per experiment!"
                    
                    'qTTL t, Asix, 0
                    Call triggerArbIntensityRamps
                    t = t + 5
                    printnow "  Ramp traps to min to loose atoms"
                    Call triggerArbIntensityRamps
                    'qTTL t, Asix, 1
                    t = t + 1
                    
                End If
            Else
                        
                If ramanSBCooling Then
                    qTTL t, Cfour, 1 ' Raman and LS Beams shutter opened here
                    
                    '--> Thius should not be necessary! Removed to make sure there is no leakage 160418
                   ' If microSingleAtom Then
                   '     qTTL t, Bthree, 1
                   ' End If
                    
                    Call threeDcooling   ' Note: theres a 25 ms hold at beginning of 3D cooling, so dont need to hold for shutter...
                    Call advanceRSgen
                End If
                
                If False Then 'formerly specDifferentDepth, but this is no longer correct -- have to use Arb programming instead!
                    Call rampToDifferentDepth(defaultTweezerDepth, finalTrapDepth, 10, 0.05)
                End If
                
                '=================== turn this off =============================
                'qTTL t, Cfour, 0 ' Raman and LS Beams shutter opened here
                'qTTL t, Bone, 0 ' repump
                'qTTL t, Dzero, 0 'mot shutter
                               
                't = t + FreeValue
                'Call triggerArbIntensityRamps
                't = t + 4
                '===============================================================
                '===============================================================
                'REMOVE THIS HOLD!!! <------------------------------------------------------------*********************************
                't = t + 100
                '===============================================================
                '===============================================================
                'Call triggerArbIntensityRamps
                't = t + 4
                
                'qTTL t, Cfour, 1 'sidemot beam
                'qTTL t, Cthirteen, 1 'sidemot beam
                
                If carrierCal Then
                    'make sure FSK triggers are off for spectroscopy!
                    qTTL t, Dfour, 0
                    qTTL t, Dfive, 0
                    qTTL t, Dsix, 0
                    
                    qTTL t, Cfour, 1 ' Raman and LS Beams shutter opened here
                    t = t + 10
                    printnow "Carrier: Gaussian pulses should be off!"
                    
                    Select Case LCase(specAxis)
                        Case "top":
                            Call topSBspec(0.012) 'usually 0.012 ms for carriers taken with ~40 kHz rabi rate
                        Case "bottom":
                            Call botSBspec(0.012)
                        Case "axial":
                            printnow "=>Are you actually taking an axial carrier??"
                            Call axialSBspec(0.02)
                        Case Else:
                            printnow "Improper Raman beam selection!"
                    End Select
                    Call advanceRSgen
                    printnow "  50 ms hold after spec pulse"
                    t = t + 50
                ElseIf carrierRabiCurve Then
                    If calCoolingFreqs Then
                        printnow "  to calibrate RSB freqs"
                        qTTL t, Dfour, 1
                        qTTL t, Dfive, 1
                        qTTL t, Dsix, 1
                    Else
                        qTTL t, Dfour, 0
                        qTTL t, Dfive, 0
                        qTTL t, Dsix, 0
                    End If
                    
                    qTTL t, Cfour, 1 ' Raman and LS Beams shutter opened here
                    t = t + 10
                    printnow "Carrier Rabi: no cooling, square pulses"
                    
                    Select Case LCase(specAxis)
                        Case "top":
                            Call topSBspec(rabiPulseLength)
                        Case "bottom":
                            Call botSBspec(rabiPulseLength)
                        Case "axial":
                            Call axialSBspec(rabiPulseLength)
                        Case Else:
                            printnow "Improper Raman beam selection!"
                    End Select
                    Call advanceRSgen
                    
                    If ramanSBCooling = False Then
                        printnow "  50 ms hold after Rabi pulse"
                        t = t + 50
                    End If
                    
                ElseIf sidebandSpec And ramanSBCooling Then
                    qTTL t, Dfour, 0
                    qTTL t, Dfive, 0
                    qTTL t, Dsix, 0
                    t = t + 5 ' hold for letting AFGs settle down, but may not need to be so long...
                    
                    Select Case LCase(specAxis)
                        Case "top":
                            printnow "-> top spec after 3D cool"
                            Call topSBspec(0.12)
                        Case "bottom":
                            printnow "-> bot spec after 3D cool"
                            Call botSBspec(0.12)
                        Case "axial":
                            printnow "-> axial spec after 3D cool"
                            Call axialSBspec(0.138)
                        Case Else:
                            printnow "Improper Raman beam selection!"
                    End Select
                    Call advanceRSgen
                ElseIf sidebandSpec Then
                    If calCoolingFreqs Then
                        printnow "  to calibrate RSB freqs"
                        qTTL t, Dfour, 1
                        qTTL t, Dfive, 1
                        qTTL t, Dsix, 1
                    Else
                        qTTL t, Dfour, 0
                        qTTL t, Dfive, 0
                        qTTL t, Dsix, 0
                    End If
                    
                    qTTL t, Cfour, 1 ' Raman and LS Beams shutter opened here
                    t = t + 10
                    printnow "Thermal sideband spectroscopy"
                    
                    t = t + 5 ' wait for AFG to settle down, but probably want to shorten!****************
                    
                    Select Case LCase(specAxis)
                        Case "top":
                            Call topSBspec(0.08) 'was 0.12, but I think we should chirp these pulses for thermal SB spec... try 0.08 first (what is uesd in first stage of cooling)
                        Case "bottom":
                            Call botSBspec(0.08) '0.12)
                        Case "axial":
                            Call axialSBspec(0.09) '0.138)
                        Case Else:
                            printnow "Improper Raman beam selection!"
                    End Select
                    Call advanceRSgen
                    printnow "  50 ms hold after spec pulse"
                    t = t + 50
                End If
                
                
                
                If rotateQuantAxisFB Then
                    If tunnelingDynamics Or spinExchange Or entangleVerify Then
                        qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                        qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                        printnow "Close pumping shutters while rotating B"
                    End If
                    Call rotateFieldsFB
                End If
                
                
                If RamseyNonEchoTest Then
                    printnow "Entering Ramsey-non-echo sequence, draft 1"
                    
                    Call microwaveRotation(0.0066) ' full power pi by 2 pulse, ARB for uwave set to DC with 305.5mV, need to check if pulses repeated
                        
                    t = t + RamseyEvolveTime - 0.01 'FreeValue; 75us for top beam; 167us for EO beam; 140 for axial beam (on the way down here);
                    
                    Call microwaveRotation(0.0066) ' full power pi by 2 pulse, ARB for uwave set to DC with 305.5mV
                    
                        
                End If
                
                If RamseyEchoTests Then
                    printnow "Entering Ramsey-echo sequence, draft 1"
                    
                    If LCase(RemseyTestParam) = "lsbeam" Then
                        qTTL t, Bthree, 1
                        t = t + 15
                    Else
                        qTTL t - 3, Cfour, 1
                    End If
                        
                    Call microwaveRotation(0.0066) ' full power pi by 2 pulse, ARB for uwave set to DC with 305.5mV
                        
                    Select Case LCase(RemseyTestParam)
                        Case "eobeam"
                            qTTL t - 0.01, Aeight, 1
                            printnow " - EO Beam on during hold"
                        Case "top"
                            qTTL t - 0.01, Aten, 1
                            printnow " - Top Beam on during hold"
                        Case "bottom"
                            qTTL t - 0.01, Atwo, 1
                            printnow " - Bottom Beam on during hold"
                        Case "axial"
                            qTTL t - 0.01, Aeleven, 1
                            printnow " - Axial Beam on during hold"
                        Case "lsbeam"
                            qTTL t - 0.01, Csix, 1
                            qvolts 0.075, t - 0.01, 16
                        Case Else
                            printnow "No beams on during Ramsey!"
                    End Select
                    
                    t = t + RamseyEvolveTime - 0.01 'FreeValue; 75us for top beam; 167us for EO beam; 140 for axial beam (on the way down here);
                        
                    Select Case LCase(RemseyTestParam)
                        Case "eobeam"
                            qTTL t, Aeight, 0
                        Case "top"
                            qTTL t, Aten, 0
                        Case "bottom"
                            qTTL t, Atwo, 0
                        Case "axial"
                            qTTL t, Aeleven, 0
                        Case "lsbeam"
                            qTTL t, Csix, 0
                            qvolts 0, t, 16
                        Case Else
                    End Select
                      
                    Call microwaveRotation(0.0132) ' full power pi pulse
                        
                    t = t + RamseyEvolveTime - 0.01 'FreeValue
                        
                    Call microwaveRotation(0.0066) ' full power pi by 2 pulse
                        
                    t = t + 30
                    qTTL t + 10, Cfour, 0
                    qTTL t + 10, Bthree, 0
                    
                        
                End If
                
                
                If microSingleAtom Then
                    't = t + 50 ' +++ to make sure field is settled, 4/19/16
                    
                    qTTL t, Cfour, 0
                    qTTL t, Bthree, 1 ' Raman and LS Beams shutter opened here <--- this should be changed to be just the LS beam shutter!!***
                    t = t + 4
                    t = t + 15
                    'Call singleSpinFliptest
                    Call singleSpinFlip
                    'Call singleSpinFlipModulation
                    
                    If False Then
                        If ramanSBCooling = False Then
                            printnow "  50 ms hold after microwave pulse for shutter"
                            t = t + 50
                        End If
                    End If
                    
                    Call advanceRSgen
                End If
                
                
                If False Then
                    qTTL t, Cfour, 0
                    qTTL t, Bthree, 1 ' Raman and LS Beams shutter opened here <--- this should be changed to be just the LS beam shutter!!***
                    t = t + 4
                    
                    
                    qvolts 0.125, t, 16 'set ls beam power (may need to switch to different DAC soon!  was 0.067
                    t = t + 0.8 ' vary this to check the settling of the fields over time
        
                    qTTL t, Csix, 1
                    printnow ("LS beam on")
                    t = t + 0.03 '0.01
                    
                    qTTL t, Csix, 0
                    qvolts 0, t, 16
                    
                    t = t + 50
                    qTTL t, Bthree, 1
                End If
                
                'Closing the shutter for Raman and LS Beams, if it was opened
                qTTL t, Bthree, 0
                qTTL t, Cfour, 0
                t = t + 4
                          
                          
                If microGlobalRotate Then
                    If specDifferentDepth Then
                        Call triggerArbIntensityRamps
                        t = t + 10
                    End If
                    
                  '  Call markerOscilloscope ' +++ test uwave precision 04/23/16
                    
                    
                     
                    Call microwaveRotation(mwPulseLength)
                   ' If FreeValue > 1.5 Then
                   '     Call microwaveRotation(mwPulseLength)
                   ' End If
                    
                    t = t + 0.1
                  '  Call markerOscilloscope ' +++ test uwave precision 04/23/16
                     
                    Call advanceRSgen
                
                    If specDifferentDepth Then
                        Call triggerArbIntensityRamps
                        t = t + 10
                    End If
                End If
                
                
                If rotateQuantAxisBack Then
                    Call rotateFieldsBack
                    'Call rotateFieldsBackTest(fieldShim) ' testing for spin exchange 05122016
                    printnow " Rotating back to original QA"
                End If
                
                
                
                '---These two sections below can probably be deleted... 160420
                If False Then ' formerly specDifferentDepth, but this is no longer correct!
                    Call rampToDifferentDepth(finalTrapDepth, defaultTweezerDepth, 10, 0.05)
                End If
                If False Then 'was using a flag "tunnelingBias" here, but the way it is
                                'currently set up, I dont think that makes a lot of sense...
                    'Shutters:
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    printnow "pumping shutters closed for tunneling"
    
                    Call tunnelingSequenceIntensityInit
                End If
                '---These two sections above can probably be deleted... 160420
                
                
                
                If tunnelingDynamics Then
                    'Shutters:
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    printnow "pumping shutters closed for tunneling"
                    
                    Select Case LCase(initiateTunneling)
                        Case "intensity"
                            Call tunnelingSequenceIntensityInit
                        Case "frequency"
                            Call tunnelingSequenceFreqInit
                        Case "bias"
                            Call tunnelingSequenceBiasInit
                        Case Else
                            printnow "NOT TUNNELING!!!"
                    End Select
                End If
                
                If oneWayARPtesting Then
                    'Shutters:
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    printnow "pumping shutters closed for ARPs"
                    
                    printnow "-> Manually setting tunnel parameters for ARPs!!"
                    tunnelTime = timeOfARP ' this is to set the hold times correctly, regardless of whether it is varied!
                    AOfreqRampTime = 16.5 ' this is the combined time of the two frequency ramps. The intensity ramp times are set above and are typically 20 ms for ARPs.
                    rampToTunnelBiasTime = 0.25 ' this is effectively just a buffer time...
                    
                    Call tunnelingSequenceFreqInit ' this has the same sequence of triggers as is needed... can modify later for spin exhcange
                    
                End If
                
                
                
                If twoWayARPtesting Then
                    Dim ARPtestHoldTime As Double
                    
                    'Shutters:
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    printnow "pumping shutters closed for ARPs"
                    
                    printnow "-> Manually setting tunnel parameters for ARPs!!"
                    
                    AOfreqRampTime = 16.5 '12 ' this is the combined time of the two frequency ramps. The intensity ramp times are set above and are typically 20 ms for ARPs.
                    rampToTunnelBiasTime = 0.25 ' this is effectively just a buffer time...
                    ARPtestHoldTime = 1 ' hold for 1 ms before ARPing atom back out of other well!
                    
                    Call twoWayARPsequence(holdAfterARP)  ' was doing twoWayARPsequence(ARPtestHoldTime) 05112016
                End If
                
                If atomShuttlingImaging Then
                    printnow "-> shuttling atoms while doing imaging !!"
                    
                    AOfreqRampTime = 12 ' this is the combined time of the two frequency ramps. The intensity ramp times are set above and are typically 20 ms for ARPs.
                    rampToTunnelBiasTime = 0.25 ' this is effectively just a buffer time...
                    ARPtestHoldTime = 1 ' hold for 1 ms before ARPing atom back out of other well!
                    
                    Call atomShuttlingImagingSequence
                End If
                
                
                
                If spinExchange Then
                    'Shutters:
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    printnow "pumping shutters closed before tunneling"
    
                    Call spinExchangeSequence(defaultTweezerDepth, True) ' the true/false is for doing/not-doing a ARP back - need to make a variable...
    
                End If
        
                If entangleVerify Then
                    'Shutters:
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers)
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    printnow "pumping shutters closed before tunneling"
    
                    Call entangleVerifyClose(defaultTweezerDepth)
                End If
        
                If probeImageTest Then
                    Call probeImagingTest
                    t = t + 50
                End If
        
                If False Then 'switch back to probeImageTest when done!
                    Call setQuantAxis '<--- this should already be set, but if not, we may need to add extra settling time?
                    qTTL t, Dzero, 0
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers) <-- currently not installed, but will add Randall's shutters here 160228
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    qTTL t, Cfour, 0
                    printnow "Close shutters for probe image (if not already)"
                    
                    t = t + 10 ' we should be able to compact this a bit more by moving the shutter triggers around, but for now, just testing!
                    
                    qTTL t, Ctwelve, 1
                    Call probeImaging
                    qTTL t, Ctwelve, 0
                    
                    t = t + 50 ' time to settle (and to protect from overdriving the shutters) before taking PGC image!
                    
                End If
                
        
        
                'Repump and side MOT beam shutters are opened here for taking the final image:
                qTTL t, Bone, 1
                qTTL t, Cthirteen, 1
                
                If pushoutF2 Then
                    Call Pushout
                End If
                
                If alignProbe Then
                    '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
                    'qTTL t, Dzero, 0 ''---> Currently using the MOT slave shutter for this path in order to go to
                                    ''      higher powers (and do trap depth measurements), but will want to add
                                    '       shutters so that we have separate shutters for these, at which point
                                    '       we will need to modify this TTL as necessary
                    
                    'typically using standard pushout pulse length of 0.01ms, but this can be adjusted below:
                    Call alignmentPush(alignmentPulseLength, alignmentPulsePower, defaultTweezerDepth)
                    
                    'qTTL t, Dzero, 1
                End If
                
                If pushoutImaging Then
                    ''Call setQuantAxis '<--- this should already be set, but if not, we may need to add this back in
                    qTTL t, Afive, 0 'just making sure probe beam is off!
                    
                    qTTL t, Dzero, 0
                    qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers) <-- currently not installed, but will add Randall's shutters here 160228
                    qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
                    qTTL t, Cfour, 0
                    printnow "Close shutters before probe imaging (if not already)"
                    
                    qTTL t, Ctwelve, 1 'opening probe shutter to prepare!
                    
                    t = t + 10
                    
                    printnow "Pushout imaging readout sequence:"
                    imagingPowerProbe = 0.32  ' max power corresponds to ~30nW on 160424
                    dacRamp t, 20, 4, 0.735, imDetuneProbe, 0.1 '0.735 is the typical setpoint for pump2stretch and cooling
                                                                '  - it may change after that, but jumping there shouldnt be a problem.
                    qvolts imagingPowerProbe, t, 22
                    t = t + 5 ' this serves as an offset time for the probe shutter to open, so do not add time between calling this subroutine and trigerring shutter!
                    
                    'Camera shutter open for readout pulses
                    qTTL t - 4, Dtwo, 1 ' camShutter
                    
                    'Applying probe pulse!! 'copied from the pushout subroutine
                    qvolts imDetuneProbe, t, 20 'probe detuning set to final value for image
                    qvolts imagingPowerProbe, t, 22
                    
                    qTTL t - 0.05, Ctwo, 1 'camTrig
                    qTTL t, Afive, 1
                    t = t + imagingLengthProbe
                    qTTL t, Afive, 0
                    qTTL t, Ctwo, 0     ' CamTrig0
                    
                    qvolts 0.022, t, 22 ' this is closer to the minimum power than 0V...
    
                    '----------------------------------------------------
                    'ramp down to pushout rtrap depth to loose hot atoms!
                    Call triggerArbIntensityRamps
                    t = t + 5
                    
                    t = t + 1 ' hold at shallow depth for getting rid of hot atoms
                    
                    Call triggerArbIntensityRamps
                    t = t + 10
                    
                    ' microwave pi-pulse before other image!
                    Call microwaveRotation(0.01316) ' manually set pi pulse!
                    Call advanceRSgen
                    '----------------------------------------------------
                    
                    'Camera triggers (camera and shutter)
                    qTTL t - 0.05, Ctwo, 1 'camTrig
                
                    'Applying probe pulse!! 'copied from the pushout subroutine
                    qvolts imDetuneProbe, t, 20 'probe detuning set to final value for image
                    qvolts imagingPowerProbe, t, 22
                
                    qTTL t, Afive, 1
                    t = t + imagingLengthProbe
                    qTTL t, Afive, 0
                    qTTL t, Dtwo, 0     ' CamShutter
                
                    qvolts 0.022, t, 22 ' this is closer to the minimum power than 0V...
                    
                    qTTL t, Ctwo, 0     ' CamTrig0
                    qTTL t, Dtwo, 0     ' CamShutter
                    qTTL t, Ctwelve, 0 ' probe shutter!
                   
                Else
                    If Not (atomShuttlingImaging) Then
                        Call takeStandardPGCimage(imagingLength) '+++ commented for atom shutting imaging 05182016
                    End If
                End If
                
                If False Then 'controlWithNIAWG
                    t = t + 0.01
                    printnow "Triggering NI AWG: End of script" ' same as subroutine triggerTweezerRamps
                    qTTL t, Bfourteen, 1
                    t = t + 0.1
                    qTTL t, Bfourteen, 0
                End If
                
               
                
                If LCase(tuningParam) = "piezomirror" Then
                    dacRamp t, 23, 5, FreeValue, 0, 0.05 'considering using a ramp here, since jumps can be bad, but would need a ramp back at the end...
                    t = t + 5
                End If
                    
                
            End If ' Closing the "If loadTests" conditional, which just skips over everything after the first image...
                
            '==> These next things should happen no matter whether we do load testing or not!
            
            printnow "  (Triggering ARB at the end of sequence)" ' same as subroutine triggerArbIntensityRamps
            qTTL t, Bsix, 1
            t = t + 0.1
            qTTL t, Bsix, 0
            
            t = t + 0.01
            printnow "Triggering NI AWG: End of script" ' same as subroutine triggerTweezerRamps
            qTTL t, Bfourteen, 1
            t = t + 0.1
            qTTL t, Bfourteen, 0
            
            Call turnOnMOT
                
                
            
            '*********************************************************************************
            '**** Initially, this is where I want to put all of the checks and associated
            '               holds that make sure the NI AWG is programmed and ready to go before
            '               initiating the experiment.
            '       ->If this is a significant hold and we notice that there is a subsequent long
            '               hold for programming the TTLs/DACs (which I thiuk there probably will be),
            '               then we should try to put this code between the first "qprep" and "qgo"
            '               commands -- this should allow us to get everything programmed to the TTL
            '               and DAC boards while the NI AWG is being programmed, thus saving time.
            '               My concern is that there could be some error that gets thrown, or if the
            '               timing will get off, if there is a significant hold between the qprep and
            '               qgo commands (which we will probably just have to try)...
    
           
            Dim q As Integer
            
            For q = 1 To accumulations
                If writeNewExperiment = 1 Then
                    printnow "Programming DACs"
                End If
                
                qprep
                
                If writeNewExperiment = 1 And controlWithNIAWG Then
                    'This is the loop that waits to hear that the NI AWG is ready to go:
                    printnow " ->Waiting on NI AWG..."
                    Do While flagFromNIAWG = False And goflag = True
                        DoEvents
                        wait (200)
                        ''Was trying to add something that will break out of this loop when you
                        ''   press "Abort". Not yet working, but would be nice to add...
                        ' NEW. This should allow abort to cause immediate exiting of the whole damn thing.
                        If InputFrm.chkAbort.VALUE = 1 Then
                            Exit Do
                        End If
                    Loop
                End If
                ' NEW. This should allow abort to cause immediate exiting of the whole damn thing.
                If InputFrm.chkAbort.VALUE = 1 Then
                    Exit For
                End If
                If (q <> 1) Then
                    removeCharsFromMainText (Len(CStr(q - 1)) + Len(CStr(accumulations)) + 3)
                Else
                    printnow "executing "
                End If
                appendToMainTextLiteral (CStr(q) & " / " & CStr(accumulations))
                qgo
            
                writeNewExperiment = 0
            Next q
            writeNewExperiment = 1 'So that this variable is set to 1 for other parts, (forceDac,etc)
        Next i
    End If
    'Closing connection to NI AWG computer adn shutting off RS generator:
    If controlWithNIAWG Then
        InputFrm.tcpserver.Close
    End If
    Call Send(28, "OUTP OFF")
    If InputFrm.chkAbort.VALUE = 1 Then
        ' set this to unclicked
        InputFrm.chkAbort.VALUE = 0
        ' notify the user
        printnow "Finished Aborting!"
    End If
    routine = "justwaiting"
    While routine = "justwaiting"
            wait (200)
            DoEvents
    Wend
    Call runroutine
End Sub



'=========================================================================================
'======= Below are all of the subroutines that we can call from the main sequence: =======
'-----------------------------------------------------------------------------------------
'       Feel free to add more subroutines, as convenient, but remember that you will need to
'               restart (recompile) the code to use them.

Public Sub createDataSet(ByVal importKeyFlag As Boolean, ByVal dataLow As Double, ByVal dataHigh As Double)
    'The array "indDataPoints" is (or, should be) declared as a Public variable in the MainProgram
    '       routine, which allows us to use this subroutine to assemble the array and
    '       then access it throughout the program, without having to worry about passing and
    '       returning the relevant values.
        
        
    Dim j, k, intLoop As Integer
    Dim numDataPoints, dataRes, dataRange, dataPoint, tempDoub, randomValue As Double
    Dim randValues(1000) As Double
    Dim newVal As Boolean
    Dim dataString, parsedDataString() As String
        
    numDataPoints = expRep
    dataRange = dataHigh - dataLow
    dataRes = dataRange / numDataPoints

        
    If importKeyFlag Then
        Open "\\Andor\share\Data and documents\Data repository\Key_file\key.txt" For Input As #1
        dataString = Input(LOF(1), #1)
        Close #1
        parsedDataString = Split(dataString, vbCrLf)
        For intLoop = 0 To UBound(parsedDataString)
            tempDoub = CDbl(parsedDataString(intLoop))
            indDataPoints(intLoop) = tempDoub
            printnow CStr(intLoop)
            'printnow CStr(gothere)
            'printnow "All's good!"
        Next intLoop
        expRep = UBound(parsedDataString) + 1
    Else
        'Might need to clear set here
        For j = 0 To numDataPoints - 1
            Do
                randomValue = CInt(Int((numDataPoints + 1) * Rnd()))
                'Check if data point already used
                newVal = True
                For k = 0 To j
                    If randValues(k) = randomValue Then
                        newVal = False
                    End If
                Next k
                If newVal = True Then
                    randValues(j) = randomValue
                End If
            Loop While newVal = False
        
            dataPoint = dataLow + randomValue * dataRes
            indDataPoints(j) = dataPoint

            'printnow CStr(j)
                
        Next j
            
        'Writing key file
        ' TODO: FIX THIS DAMNED THING
        Open "\\Andor\share\Data and documents\Data repository\Key_file\key.txt" For Output As #1
        Dim f As Integer
        For f = 0 To numDataPoints - 1
            Write #1, indDataPoints(f)
        Next f
        Close #1
    End If
End Sub

Public Sub programOneRSMicroFreq(ByVal pushoutSeqence As Boolean, ByVal readoutFreq As Double)
    ' Setting this up in a new way using both the global variables RSPow and RSFreq for
    '       programming the RS generator, so they need to be set before calling this function.
    If pushoutsequence Then
        If RSPow < 18 Then
            Call Send(28, "OUTP ON")
            Call Send(28, "SOURce:LIST:SEL 'freqList2'")
            Call Send(28, "SOURce:LIST:FREQ " + CStr(RSFreq) + " GHz, " + CStr(readoutFreq) + " GHz")
            printnow ("Programming RSG with frequencies: " + CStr(RSFreq) + " and " + CStr(readoutFreq))
            Call Send(28, "SOURce:LIST:POW " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm") 'two tones, same output power, RSPow
            Call Send(28, "SOURce:LIST:MODE STEP")
            Call Send(28, "SOURce:LIST:TRIG:SOURce EXT")
            Call Send(28, "SOURce:FREQ:MODE LIST")            
        Else
            printnow "RSPow too high!!"
            Call Send(28, "OUTP OFF")
        End If
    Else
        If RSPow < 18 Then
            Call Send(28, "SOURce:FREQuency:MODE CW")
            Call Send(28, "FREQ " + CStr(RSFreq) + " GHz")
            printnow ("Programming RSG with frequencies: " + CStr(RSFreq))
            Call Send(28, "POW " + CStr(RSPow) + " dBm")
            Call Send(28, "OUTP ON")
        Else
            printnow "RSPow too high!!"
            Call Send(28, "OUTP OFF")
        End If
    End If
    t = t + 0.01
End Sub

Public Sub programTwoRSMicroFreqs(ByVal carrier As Double, ByVal pushoutSeqence As Boolean, ByVal readoutFreq As Double)
    ' Setting this up in a new way using both the global variables RSPow and RSFreq for
    '       programming the RS generator, so they need to be set before calling this function.
    If pushoutsequence Then
        If RSPow < 18 Then
            Call Send(28, "OUTP ON")
            Call Send(28, "SOURce:LIST:SEL 'freqList3'")
            Call Send(28, "SOURce:LIST:FREQ " + CStr(carrier) + " GHz, " + CStr(RSFreq) + " GHz, " + CStr(readoutFreq) + " GHz")
            printnow ("Programming RSG with frequencies: " + CStr(carrier) + " and " + CStr(RSFreq) + " and " + CStr(readoutFreq))
            Call Send(28, "SOURce:LIST:POW " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm") 'two tones, same output power, RSPow, plus the readout freq power
            Call Send(28, "SOURce:LIST:MODE STEP")
            Call Send(28, "SOURce:LIST:TRIG:SOURce EXT")
            Call Send(28, "SOURce:FREQ:MODE LIST")
            
        Else
            printnow "RSPow too high!!"
            Call Send(28, "OUTP OFF")
        End If
    Else
        If RSPow < 18 Then
            Call Send(28, "OUTP ON")
            Call Send(28, "SOURce:LIST:SEL 'freqList2'")
            Call Send(28, "SOURce:LIST:FREQ " + CStr(carrier) + " GHz, " + CStr(RSFreq) + " GHz")
            printnow ("Programming RSG with frequencies: " + CStr(carrier) + " and " + CStr(RSFreq))
            Call Send(28, "SOURce:LIST:POW " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm") 'two tones, same output power, RSPow
            Call Send(28, "SOURce:LIST:MODE STEP")
            Call Send(28, "SOURce:LIST:TRIG:SOURce EXT")
            Call Send(28, "SOURce:FREQ:MODE LIST")
            
        Else
            printnow "RSPow too high!!"
            Call Send(28, "OUTP OFF")
        End If
    End If
    
    t = t + 0.01
End Sub

Public Sub programThreeRSmicroFreqs(ByVal carrier As Double, ByVal pulse1Freq As Double, ByVal pushoutSeqence As Boolean, ByVal readoutFreq As Double)
    ' Setting this up in a new way using both the global variables RSPow and RSFreq for
    '       programming the RS generator, so they need to be set before calling this function.
    ' *** Note that the power is set to be the same for all frequencies!!**
    '         --> Can change this, but right now this is what we typically use...
    If pushoutsequence Then
        If RSPow < 18 Then
            Call Send(28, "OUTP ON")
            Call Send(28, "SOURce:LIST:SEL 'freqList4'")
            Call Send(28, "SOURce:LIST:FREQ " + CStr(carrier) + " GHz, " + CStr(pulse1Freq) + " GHz, " + CStr(RSFreq) + " GHz, " + CStr(readoutFreq) + " GHz")
            printnow ("Programming RSG with frequencies: " + CStr(carrier) + " and " + CStr(pulse1Freq) + " and " + CStr(RSFreq) + " and " + CStr(readoutFreq))
            Call Send(28, "SOURce:LIST:POW " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm") 'four! tones, same output power, RSPow, plus the readout freq power
            Call Send(28, "SOURce:LIST:MODE STEP")
            Call Send(28, "SOURce:LIST:TRIG:SOURce EXT")
            Call Send(28, "SOURce:FREQ:MODE LIST")
            
        Else
            printnow "RSPow too high!!"
            Call Send(28, "OUTP OFF")
        End If
    Else
        If RSPow < 18 Then
            Call Send(28, "OUTP ON")
            Call Send(28, "SOURce:LIST:SEL 'freqList3'")
            Call Send(28, "SOURce:LIST:FREQ " + CStr(carrier) + " GHz, " + CStr(pulse1Freq) + " GHz, " + CStr(RSFreq) + " GHz")
            printnow ("Programming RSG with frequencies: " + CStr(carrier) + " and " + CStr(pulse1Freq) + " and " + CStr(RSFreq))
            Call Send(28, "SOURce:LIST:POW " + CStr(RSPow) + "dBm, " + CStr(RSPow) + "dBm" + CStr(RSPow) + "dBm") 'Three tones, same output power, RSPow
            Call Send(28, "SOURce:LIST:MODE STEP")
            Call Send(28, "SOURce:LIST:TRIG:SOURce EXT")
            Call Send(28, "SOURce:FREQ:MODE LIST")
            
        Else
            printnow "RSPow too high!!"
            Call Send(28, "OUTP OFF")
        End If
    End If
    t = t + 0.01
End Sub



Public Sub initRamanAFGs()
    'Variables for setting the AFGs that define the double-well potential:
    Dim topRamanPow, botRamanPow, axialRamanPow, EOramanPow, topRamanCoolFreq, botRamanCoolFreq, axialRamanCoolFreq, EOramanFreq, specFreqs As Double

    '-------------------------------------------------------------------------------------
    'Programming AFG3102, GPIB 25, used for generating the top and bottom Raman RF tones:
    specFreqs = 70000000 ' 70 MHz spectroscopy frequency for both top/bottom beams
    
    topRamanCoolFreq = topRSBcooling ' this is the standard value, but may need to be a global variable that we can set above/vary...
    topRamanPow = -3.03 '-2.8 'this is the power we use to get high efficiency deflection (and 41 kHz carrier Rabi on 151210)
    
    botRamanCoolFreq = botRSBcooling ' this is the standard value, but may need to be a global variable that we can set above/vary...
    botRamanPow = -4.95 '-2.32 '-2.1 '-3.8 'this is the power we use to get 41 kHz carrier Rabi on 151210

    If True Then
        Call Send(25, "SOURCE1:FREQ " + CStr(specFreqs))
        Call Send(25, "SOURCE1:FSKey:STATe On")
        Call Send(25, "SOURCE1:FSKey:FREQ " + CStr(topRamanCoolFreq))
        Call Send(25, "SOURCE1:FSKey:SOURce External")
        Call Send(25, "SOURCE1:VOLT:UNIT DBM")
        If topRamanPow < -2 Then
            Call Send(25, "SOURCE1:VOLT " + CStr(topRamanPow))
        Else
                printnow "AO power set too high!"
        End If
        Call Send(25, "SOURCE1:VOLT:OFFS 0")
        Call Send(25, "OUTput1:STATe ON")
        
        '----------------------------------------
        
        Call Send(25, "SOURCE2:FREQ " + CStr(specFreqs))
        Call Send(25, "SOURCE2:FSKey:STATe On")
        Call Send(25, "SOURCE2:FSKey:FREQ " + CStr(botRamanCoolFreq))
        Call Send(25, "SOURCE2:FSKey:SOURce External")
        Call Send(25, "SOURCE2:VOLT:UNIT DBM")
        If botRamanPow < -2 Then
            Call Send(25, "SOURCE2:VOLT " + CStr(botRamanPow))
        Else
                printnow "AO power set too high!"
        End If
        Call Send(25, "SOURCE2:VOLT:OFFS 0")
        Call Send(25, "OUTput2:STATe ON")
    Else
        Call Send(25, "OUTPut1:STATe OFF")
        Call Send(25, "OUTPut2:STATe OFF")
    End If



    '-------------------------------------------------------------------------------------
    'Programming AFG3102, GPIB 24, used for generating Axial and EO Raman beam RF tones:
    specFreqs = 70000000 ' 70 MHz spectroscopy frequency for both top/bottom beams
    
    EOramanFreq = 80000000
    EOramanPow = -20 '-18.5 ' -5.5 '-3 'this is the power we use to get high efficiency deflection (and 41 kHz carrier Rabi on 151210)
    
    axialRamanCoolFreq = axialRSBcooling ' this is the standard value, but may need to be a global variable that we can set above/vary...
    axialRamanPow = -5.5 '-4.5 '-2.56 'this is the power we use to get 41 kHz carrier Rabi on 151210
    
    If True Then
        Call Send(24, "SOURCE1:FUNC SIN")
        Call Send(24, "SOURCE1:FREQuency:MODE CW")
        Call Send(24, "SOURCE1:FREQ " + CStr(EOramanFreq))
        Call Send(24, "SOURCE1:VOLT:UNIT DBM")
        If EOramanPow < -2 Then
            Call Send(24, "SOURCE1:VOLT " + CStr(EOramanPow))
        Else
                printnow "Raman EO RF power too high!"
        End If
        Call Send(24, "SOURCE1:VOLT:OFFS 0")
        Call Send(24, "OUTput1:STATe ON")
        
        '-----------------------
        
        Call Send(24, "SOURCE2:FREQ " + CStr(specFreqs))
        Call Send(24, "SOURCE2:FSKey:STATe On")
        Call Send(24, "SOURCE2:FSKey:FREQ " + CStr(axialRamanCoolFreq))
        Call Send(24, "SOURCE2:FSKey:SOURce External")
        Call Send(24, "SOURCE2:VOLT:UNIT DBM")
        If axialRamanPow < -2 Then
            Call Send(24, "SOURCE2:VOLT " + CStr(axialRamanPow))
        Else
                printnow "Axial Raman power too high!"
        End If
        Call Send(24, "SOURCE2:VOLT:OFFS 0")
        Call Send(24, "OUTput2:STATe ON")
    Else
        Call Send(24, "OUTPut1:STATe OFF")
        Call Send(24, "OUTPut2:STATe OFF")
    End If

    t = t + 0.01
End Sub

Public Sub initLowFreqFGs()
    '**Note that the variable "FreeValue" is a Public variable, so we do not have to
    '               pass the value through to use it for calibrating SBs!
    
    'Declaring the variables to be used in this subroutine below (definitions are in sections for the specific devices):
    Dim topRamanFreq1, topRamanFreq2, botRamanFreq1, botRamanFreq2, axialRamanFreq1, eoBeamRamanFreq, FG4extraFreq1, FG4extraFreq2 As Double
    Dim topRamanPow1, topRamanPow2, botRamanPow1, botRamanPow2, axialRamanPow1, eoBeamRamanPow, FG4amp1, FG4amp2, FG4offset1, FG4offset2 As Double
    
    '-------------------------------------------------------------------------------------
    'Parametric and RF photon transitions: RF FG 3320A Agilent. GPIB 10
    If False Then
        'Call Send(10, "OUTP:LOAD INF")
        'Call Send(10, "FUNC SIN")
        'Call Send(10, "FREQ " + CStr(FreeValue))
        'Call Send(10, "VOLT .1") ''0.3 V is a typical amplitude for parametric excitation
        'Call Send(10, "VOLT:OFFS 0")
        'Call Send(10, "OUTP ON")
       
        Call Send(10, "OUTP:LOAD 50")
        Call Send(10, "FUNC SIN")
        'Call Send(10, "FREQ 280000")
        Call Send(10, "FREQ " + CStr(FreeValue))
        Call Send(10, "VOLT 0.5")
        Call Send(10, "VOLT:OFFS 0")
        Call Send(10, "OUTP ON")
    Else
        Call Send(10, "OUTP OFF")
    End If


    '-------------------------------------------------------------------------------------
    '-------> Now available for providing other frequencies!!*******
    'RF FG1 for RAMAN transitions EO and AXIAL BEAMs: 335522A Agilent GPIB 11
    axialRamanFreq1 = axialRSBcooling 'axial cooling freq (also used for axial Spectroscopy)
    eoBeamRamanFreq = 30000000 'EO Beam
    axialRamanPow1 = 5 'axial beam power to mixer - what is currently set, but not actually being programmed...
    eoBeamRamanPow = -16.3
     
    If False Then ' was for EO and axial Raman beams (when using mixer boxes), but now this is an extra FG that we can use for other things...
        'CHANNEL 1: DO NOT EXCEED -4dBM
        Call Send(11, "OUTPUT1:LOAD 50")
        Call Send(11, "SOURCE1:FUNC SIN")
        Call Send(11, CStr("SOURCE1:FREQ " + CStr(axialRamanFreq1)))
        Call Send(11, "SOURCE1:VOLT:UNIT DBM")
        If axialRamanPow1 < 5.1 Then
            Call Send(11, CStr("SOURCE1:VOLT " + CStr(axialRamanPow1)))
        Else
            printnow "FIX THIS!! - Power set out of range in FG1, Channel 1"
        End If
        Call Send(11, "SOURCE1:VOLT:OFFS 0")
        Call Send(11, "OUTPUT1 ON")
        
        'CHANNEL 2: DO NOT EXCEED -11dBM
        Call Send(11, "OUTPUT2:LOAD 50")
        Call Send(11, "SOURCE2:FUNC SIN")
        Call Send(11, CStr("SOURCE2:FREQ " + CStr(eoBeamRamanFreq)))
        Call Send(11, "SOURCE2:VOLT:UNIT DBM")
        If eoBeamRamanPow < -10 Then
            Call Send(11, CStr("SOURCE2:VOLT " + CStr(eoBeamRamanPow)))
        Else
            printnow "Power set out of range for FG1, channel 2"
        End If
        Call Send(11, "SOURCE2:VOLT:OFFS 0")
        Call Send(11, "OUTPUT2 ON")
    Else
        Call Send(11, "OUTPUT1 OFF")
        Call Send(11, "OUTPUT2 OFF")
    End If

    
    '-------------------------------------------------------------------------------------
    '---------->    THIS FG NO LONGER CONNECTED!!************
    'RF FG2 for RAMAN transitions BOTTOM BEAM: 335522A Agilent GPIB 12
    botRamanFreq1 = botRSBcooling 'was 19858000, but recently set to 19852300
    botRamanFreq2 = 20000000
    botRamanPow1 = -2.1 '-3.2 on 150107, changed to -2.1 for gaussian pulses
    botRamanPow2 = -2.1 '-15.5


    If False Then ' no longer connected!!
        'CHANNEL 1 SETTINGS:
        Call Send(12, "OUTPUT1:LOAD 50")
        Call Send(12, "SOURCE1:FUNC SIN")
        Call Send(12, "SOURCE1:FREQ " + CStr(botRamanFreq1))
        Call Send(12, "SOURCE1:VOLT:UNIT DBM")
        If botRamanPow1 < 3 Then
            Call Send(12, "SOURCE1:VOLT " + CStr(botRamanPow1))
        Else
            printnow "Power set out of range for FG2, ch1"
        End If
        Call Send(12, "SOURCE1:VOLT:OFFS 0")
        Call Send(12, "OUTPUT1 ON")
        
        'CHANNEL 2 SETTINGS:
        Call Send(12, "OUTPUT2:LOAD 50")
        Call Send(12, "SOURCE2:FUNC SIN")
        Call Send(12, "SOURCE2:FREQ " + CStr(botRamanFreq2))
        Call Send(12, "SOURCE2:VOLT:UNIT DBM")
        If botRamanPow2 < 3 Then
            Call Send(12, "SOURCE2:VOLT " + CStr(botRamanPow2))
        Else
                printnow "Power set out of range for FG2, ch2"
        End If
        Call Send(12, "SOURCE2:VOLT:OFFS 0")
        Call Send(12, "OUTPUT2 ON")
        
    Else
        Call Send(12, "OUTPUT1 OFF")
        Call Send(12, "OUTPUT2 OFF")
    End If

    

    '-------------------------------------------------------------------------------------
    '---------->    THIS FG NO LONGER CONNECTED!!************
    'RF FG3 for RAMAN transitions TOP BEAM: 335522A Agilent GPIB 13
    topRamanFreq1 = topRSBcooling '19852300
    topRamanFreq2 = 20000000
    topRamanPow1 = 3 '-2 on 150107, changed to +3 for gaussian pulses
    topRamanPow2 = 3

    If False Then ' THIS FG IS NO LONGER CONNECTED!!!
        'CHANNEL 1 SETTINGS:
        Call Send(13, "OUTPUT1:LOAD 50")
        Call Send(13, "SOURCE1:FUNC SIN")
        Call Send(13, "SOURCE1:FREQ " + CStr(topRamanFreq1))
        Call Send(13, "SOURCE1:VOLT:UNIT DBM")
        If topRamanPow1 < 4 Then
            Call Send(13, "SOURCE1:VOLT " + CStr(topRamanPow1))
        Else
            printnow "Power is set out of range for FG3, ch1"
        End If
        Call Send(13, "SOURCE1:VOLT:OFFS 0")
        Call Send(13, "OUTPUT1 ON")
        
        'CHANNEL 2 SETTINGS:
        Call Send(13, "OUTPUT2:LOAD 50")
        Call Send(13, "SOURCE2:FUNC SIN")
        Call Send(13, "SOURCE2:FREQ " + CStr(topRamanFreq2))
        Call Send(13, "SOURCE2:VOLT:UNIT DBM")
        If topRamanPow2 < 4 Then
            Call Send(13, "SOURCE2:VOLT " + CStr(topRamanPow2))
        Else
            printnow "Power is set out of range for FG3, ch2"
        End If
        Call Send(13, "SOURCE2:VOLT:OFFS 0")
        Call Send(13, "OUTPUT2 ON")
        
    Else
        Call Send(13, "OUTPUT1 OFF")
        Call Send(13, "OUTPUT2 OFF")
    End If


    '-------------------------------------------------------------------------------------
    'RF FG4 for Arb Intensity Ramps and Parametric Excitation: 335522B Agilent GPIB 14
    FG4extraFreq1 = 19853000 ' Not useable at the moment - this output is used to ramp
                                '   the tweezer intensity down for tunneling.
    FG4extraFreq2 = FreeValue ' +++ setto Freevalue for LS beam kick out '19935000 ' This is not currently set up to connect to the axial
                                '   Raman AO, but could definitely do that, if we wanted to...
    FG4amp1 = 0.01
    FG4amp2 = 0.2
    FG4offset1 = 0
    FG4offset2 = FG4amp2 / 2 '0.07

    If False Then  'Programming arbitrary ramps of tweezer intensity to Channel 1: ' +++ True '
        'printnow "Program ARB for intensity ramps"
        Call Send(14, "DATA1:VOL:CLEar")
                
        'Strings used to define what segments need to be loaded into the arbitrary waveform:
      '  tmpStr = """dtRamp"",""Int:\DCHold_New_NEGSPD_B.arb"",0,onceWaitTrig,highAtStart,4,""Int:\RAMP12_p42p0uW15ms_151226unCal_B.arb"",0,onceWaitTrig,highAtStart,4,""Int:\RAMP3_p42p0uW10ms_fastUp_151226unCal_B.arb"",0,onceWaitTrig,highAtStart,4"
        tmpStr = """dtRamp"",""Int:\DCHold_New_NEGSPD_B.arb"",0,onceWaitTrig,highAtStart,4,""Int:\RAMP12_p28p8uW20ms_150824cal_B.arb"",0,onceWaitTrig,highAtStart,4,""Int:\RAMP3_p28p8uW10ms_fastUp_150824cal_B.arb"",0,onceWaitTrig,highAtStart,4"
        'tmpStr = """dtRamp"",""Int:\DCHold_New_NEGSPD_B.arb"",0,onceWaitTrig,highAtStart,4,""Int:\RAMP12_p28p8uW20ms_150824cal_B.arb"",0,onceWaitTrig,highAtStart,4,""Int:\RAMP3_p28p8uW20ms_symUp_150824cal_B.arb"",0,onceWaitTrig,highAtStart,4"
        lenStr = Len(tmpStr)

       'Constant hold in deeper trap:
        Call Send(14, "MMEM:LOAD:DATA1 ""Int:\DCHold_New_NegSPD_B.arb""")
        
        'temp files for testing (151226):
        'Call Send(14, "MMEM:LOAD:DATA1 ""Int:\RAMP12_p42p0uW15ms_151226unCal_B.arb""")
        'Call Send(14, "MMEM:LOAD:DATA1 ""Int:\RAMP3_p42p0uW10ms_fastUp_151226unCal_B.arb""")
        
        
        'Ramps down/up for the updated depth calibration from 150825:
        Call Send(14, "MMEM:LOAD:DATA1 ""Int:\RAMP12_p28p8uW20ms_150824cal_B.arb""")
        'Call Send(14, "MMEM:LOAD:DATA1 ""Int:\RAMP3_p28p8uW20ms_symUp_150824cal_B.arb""")
        Call Send(14, "MMEM:LOAD:DATA1 ""Int:\RAMP3_p28p8uW10ms_fastUp_150824cal_B.arb""")
        
        Call Send(14, "DATA1:SEQ #" + CStr(Len(CStr(lenStr))) + CStr(lenStr) + CStr(tmpStr))
        Call Send(14, "SOURCE1:FUNC ARB")
        Call Send(14, "SOURCE1:FUNC:ARB dtRamp")
        Call Send(14, "OUTPUT1 ON")
    Else
        Call Send(14, "OUTPUT1 OFF")
    End If

    If False Then 'If we want to switch back to using this for RF tones, reenable this section:
        'CHANNEL 1: DO NOT EXCEED **
        Call Send(14, "OUTPUT1:LOAD 50")
        Call Send(14, "SOURCE1:FUNC SIN")
        Call Send(14, "SOURCE1:FREQ " + CStr(FG4extraFreq1))
        If FG4amp1 < 0.3 Then
                Call Send(14, "SOURCE1:VOLT " + CStr(FG4amp1))
        Else
                printnow "Power set out of range for FG4, ch1"
        End If
        Call Send(14, "SOURCE1:VOLT:OFFS " + CStr(FG4offset1))
        Call Send(14, "OUTPUT1 ON")
    End If
    
    If False Then ' Programming channel 2 of FG4 ' +++ False '
        'CHANNEL 2: DO NOT EXCEED **
        'Call Send(14, "OUTPUT2:LOAD 50") ' +++ commented '
        Call Send(14, "SOURCE2:FUNC SIN")
        Call Send(14, "SOURCE2:FREQ " + CStr(FG4extraFreq2))
        'Call Send(14, "SOURCE2:VOLT:UNIT DBM") ' +++ commented '
        If FG4amp2 < 3 Then
                Call Send(14, "SOURCE2:VOLT " + CStr(FG4amp2))
        Else
                printnow "Power set out of range for FG4, ch2"
        End If
        Call Send(14, "SOURCE2:VOLT:OFFS " + CStr(FG4offset2))  ' +++ FG4offset2 wasn't there '
        Call Send(14, "OUTPUT2 ON")
    Else
        Call Send(14, "OUTPUT2 OFF")
    End If
    
    t = t + 0.01
End Sub

Public Sub turnOnMOT()
     qvolts 7, t, 6   'MOT coils on '7

     'MOT position fields for Dipole trap loading
     qvolts 0.58, t, 0 'Right, 0.117 from the perspective of looking east to west across the table '0.64 ' 0.59
     qvolts -0.58, t, 1 'Left, -0.64 ' 0.59
     qvolts -1.06, t, 2 'Front, -1.272  '1.07
     qvolts 1.06, t, 3 'Back, 1.272
     qvolts -1.31, t, 4  'Bottom, -1.24 '1.3
     qvolts 1.31, t, 5 'Top, 1.24 ' 1.3
     
     ' Setting the beam detuning and powers:
     qvolts -0.1, t, 20 'CycOffset: Trap light detuning to 10MHz red '-0.1


     qvolts 0.064, t, 22 'side MOT power (from master laser)
     qvolts 0.2, t, 9 'Trap1 light power (from Slave laser)
     qvolts 0.32, t, 11 'repump power (detuning is always resonant)


     qTTL t, Aone, 1         ' Repump light on

     qTTL t, Bone, 1         ' Repump shutter open (150909, this is the new RP shutter, D1 is not connected now)
     'qTTL t, Bthree, 1         ' Repump shutter open this is currently the repump shutter, may switch back to bone!
     qTTL t, Cthirteen, 1    ' F=2 light (OP) shutter open - this shutter is not connected at the moment, but we may want to re-add it when we have a working shutter — possibly one of Randall’s)

     qTTL t, Azero, 1    'MOT light on
     qTTL t, Afive, 1    'Side MOT beam TTL on
     qTTL t, Dzero, 1    'trap shutter open
     qTTL t, Ctwelve, 0  ' probe shutter closed when using for alignment beam (otherwise it kills the MOT)

     t = t + 0.01
End Sub

Public Sub loadTrap795(ByVal timeMOTon As Double)
    Dim blueLoadFreq, blueLoadPow, depumpFreq, depumpPower, loadingTimeAfterMOT As Double
    Dim PGCafterLoad As Boolean
    ' **No shutters are actuated within this subroutine, so if we want to have any of the
    '       MOT shutters close immediately after the gradient is shut off, we will have to
    '       subtract an offset time to handle this (but probably unnecessary).
    ' This subroutine is designed to be used for the entire loading procedure, after
    '       initially turning on the MOT. Specifically, I expect to call "turnOnMOT()",
    '       followed by this function (possibly with some initialization calls in between
    '       the function calls). At the end of the subroutine, the traps will be at the
    '       "finalTrapDepth" and, other than the tweezers, all light will be off.
    ' Note that the "timeMOTon" is the total time the MOT will remain on after this
    '       function is called. The total time before this function is completed will depend
    '       on the 795 pulse length ("loadingTimeAfterMOT", 35ms currently), plus 10ms for
    '       the frequency ramps (5ms for each of the 795 and depump beams), and finally the
    '       time for the trap to ramp to the final depth (currently 5ms).
    '               -> The current total extra time is: 45 ms  (=35+10; 35 for collisions, 10 for freq ramps...)

    'Defining the relevant parameters for enhanced (795) loading:
    loadingTimeAfterMOT = 35  'length of 795 pulse after dropping the MOT (to kick out remaining pairs)
    blueLoadFreq = -0.5 '-0.2  '795 freq during load pulse
    blueLoadPow = 0.13 '0.115   '795 power during load pulse (0 is min, 0.3 max)
    depumpFreq = 1.4 '1.5    '2-2 frequency during loading sequence
    depumpPower = 5 '4              '2-2 light power (0-10V), typically either 4 or 5.5 V
    
    PGCafterLoad = False ' turns on/off a section of PGC during rampdown of the traps after enhanced loading...
    
    Call triggerArbIntensityRamps 'triggering rapid ramp of trap intensity up to enhanced loading depths!
    ' Call triggerSimultaneousTweezerIntRamps '+++ was using this line, change to above 5/18/2016 ' this will trigger both the NIAWG and the Arb -- should work even for spectroscopy!
    
    'Ramping to the correct loading frequencies to preserve the locks:
    '       *** May want to test if this is necessary? ***
    If depumpFreq <> 0 Then 'Make sure this matches the default DAC value *AND* the ramp back below
        dacRamp t, 10, 5, 0, depumpFreq, 0.1
    End If
    t = t + 5
    
    If blueLoadFreq <> 0 Then 'Make sure this matches the default DAC value *AND* the ramp back below
        dacRamp t, 17, 5, 0, blueLoadFreq, 0.1
    End If
    t = t + 5
            
            
    'turn up RF power in beams during MOT loading
    qvolts depumpPower, t, 8  'set 2-2 power
    qvolts blueLoadPow, t, 18 'set 795 power (0 is min, 0.3 max)
            
        'turn on beams for the remainder of MOT load
    qTTL t, Cnine, 1  ' 2-2'' light on
    qTTL t, Cten, 1 ' 795 light on
    
    
    ' Vary MOT parameters during 795 load?
    qvolts 0.095, t, 11 'Repump power
    qvolts 0.21, t, 9 ' MOT slave power
    qvolts 0.06, t, 22 ' side MOT power
     
     
    t = t + timeMOTon - 10  '10 ms offset is to compensate for frequency ramps before loading
    
    'Shutting off gradient from MOT loading:
    qvolts 0, t, 6 'MOT coils off
    
    'Zeroed B-field for PGC (as of 150910, but last modified much earlier...)
    qvolts -0.7037, t, 2    'Front
    qvolts 0.3719, t, 3         'Back
    qvolts 0.8819, t, 0         'Right
    qvolts 0.0281, t, 1     'Left
    qvolts -2.0304, t, 4    'Bottom
    qvolts 2.0104, t, 5     'Top
    
    'Shut off all MOT light and set DACs to standard values:
    qTTL t, Aone, 0 ' repump on ''TURNING OFF PGC/REPUMP FOR NOW...?
    qTTL t, Azero, 0 ' diagonal MOT light off
    qTTL t, Afive, 0 ' side MOT light off

    qvolts 0, t, 9 'Trap light power
    qvolts 0, t, 11 'Repumplight power
    qvolts 0, t, 22 'side mot power
    qvolts 0, t, 20 'cycOffset freq back to 0

    'Perform loading pulse after dropping MOT:
    printnow "Blue loading after MOT dropped"
    t = t + loadingTimeAfterMOT 'length of loading pulse

    'Shutting off the loading beams:
    qTTL t, Cnine, 0    'depumping light off
    qTTL t, Cten, 0     '795 light off

    qvolts 0, t, 8  'DAC power for 2-2 laser -> shut off after loading
    qvolts 0, t, 18 'DAC power for 795 laser -> shut off after loading
    
    
    If PGCafterLoad Then 'try adding some PGC here?
        qvolts 0.2, t, 11  'Repump power reduced by factor of 16
        qvolts -3, t, 20   'Trap light freq
        qvolts 0.065, t, 9 'trap light power
        qvolts 0.06, t, 22 'side MOT power
        qTTL t, Aone, 1 ' Repump light on
        qTTL t, Afive, 1 'Side MOT beam on
        qTTL t, Azero, 1 'Trap light on
        
        t = t + 0.1
    End If
    
    Call triggerArbIntensityRamps 'triggering ramp of the trap back to standard imaging depth in <10 ms (this will occur during freq ramps)
    'Call triggerSimultaneousTweezerIntRamps '+++ was using this line, change to above 5/18/2016 ' this will trigger both the NIAWG and the Arb -- should work even for spectroscopy!
    '========> Note that we should probably eventually get rid of the extra ramp times below? wasting time!
    
    
    'Ramping the loading freqs back to preserve locks :
    If blueLoadFreq <> 0 Then 'Make sure this matches the first ramp above
        dacRamp t, 17, 5, blueLoadFreq, 0, 0.1
    End If
    t = t + 5
    
    If depumpFreq <> 0 Then 'Make sure this matches the first ramp above
        dacRamp t, 10, 5, depumpFreq, 0, 0.1
    End If
    t = t + 5
    
    If PGCafterLoad Then ' shutting off the PGC light...
        qTTL t, Aone, 0 ' repump on ''TURNING OFF PGC/REPUMP FOR NOW...?
        qTTL t, Azero, 0 ' diagonal MOT light off
        qTTL t, Afive, 0 ' side MOT light off

        qvolts 0, t, 9 'Trap light power
        qvolts 0, t, 11 'Repumplight power
        qvolts 0, t, 22 'side mot power
        qvolts 0, t, 20 'cycOffset freq back to 0
        
        t = t + 1
    End If
    
    t = t + 0.01
End Sub

Public Sub loadTrapPGC()
    'As before, no shutters are actuated within this subrouting - Be sure all the relevant
    '       shutters are open before calling this routine and closed after!!
    Dim PGCtime, PGCDetuning, PGCpowerMOTlight, PGCsideMOTpower, PGCRepPower As Double
    PGCtime = 10 ' do not need much PGC to cool near the bottom of the trap -> 10 is plenty
    PGCDetuning = -3 ' increase detuning significantly for PGC
    PGCRepPower = 0.32 'Full power - does not appear to need to be lower...
    PGCpowerMOTlight = 0.065 ' reducing trap power for cooling
    PGCsideMOTpower = 0.064 ' standard side-MOT beam power for MOT
    
    'Shutting off gradient from MOT loading:
    qvolts 0, t, 6 'MOT coils off
    
    'Zeroed B-field for PGC (as of 150910, but last modified much earlier...)
    qvolts -0.7037, t, 2    'Front
    qvolts 0.3719, t, 3     'Back
    qvolts 0.8819, t, 0     'Right
    qvolts 0.0281, t, 1     'Left
    qvolts -2.0304, t, 4    'Bottom
    qvolts 2.0104, t, 5     'Top
    
    'Set beam powers for PGC and turn on beams (should already be on, but just in case):
    qvolts PGCRepPower, t, 11  'Repump power reduced by factor of 16
    qvolts PGCDetuning, t, 20   'Trap light freq
    qvolts PGCpowerMOTlight, t, 9 'trap light power
    qvolts PGCsideMOTpower, t, 22 'side MOT power
    qTTL t, Aone, 1 ' Repump light on
    qTTL t, Afive, 1 'Side MOT beam on
    qTTL t, Azero, 1 'Trap light on

    t = t + PGCtime
    

    qTTL t, Aone, 0 ' Repump light off
    qTTL t, Afive, 0 'side mot beam off
    qTTL t, Azero, 0 'Trap light off
    
    qvolts 0, t, 11  'Repump power reduced by factor of 16
    qvolts 0, t, 20   'Trap light freq
    qvolts 0, t, 9 'trap light power
    qvolts 0, t, 22

    t = t + 0.01
End Sub

Public Sub takeStandardPGCimage(ByVal imageLength As Double)
    Dim imagingPower, imDetune As Double
    '**IMPORTANT NOTE!!!--> The shutter for the imaging beams (both the side-MOT and
    '       Repump shutters) will not be actuated within this subroutine, so that we have
    '       the flexibility to minimize the number of times it opens and closes (so we do
    '       not break the shutter so often) by only closing it for the main portion of the
    '       experiment (which will be done in the main code).
    ' *To be clear: The only shutter being actuated in the code is the shutter for the
    '       Andor camera, which we always run at this high rate (though, if we go to shorter
    '       imaging times, we will need to add an hold time to extend the shutter lifetime.
    '------------------------------------------------------------------------------------
    
    'Setting the relevant imaging parameters at the top:
    imDetune = 0.17
    imagingPower = 0.2
    
    'Setting PGC fields for imaging while cooling with side MOT beam:
    qvolts -0.7037, t, 2        'Front
    qvolts 0.3719, t, 3             'Back
    qvolts 0.8819, t, 0         'Right
    qvolts 0.0281, t, 1         'Left
    qvolts -2.0304, t, 4        'Bottom
    qvolts 2.0104, t, 5         'Top
           
    t = t + 0.1

    'below are the imaging parameters for the MOT light (specifically, the side MOT beam)
            
    qvolts imagingPower, t, 22 ' was 0.2 on 150408
    qvolts imDetune, t, 20 'Trap light ramped to image (detuning)
    qvolts 0, t, 21
    qvolts 0, t, 9
    qvolts 0.32, t, 11  'Repumplight power
           
            
    t = t + 5
    
    printnow "Take PGC Image"
    
    'Turning on the repunmp light <<**Be sure to open the repump shutter before calling this subroutine!!**>>
    qTTL t, Aone, 1 ' repump on 1 ms before (141121)
    t = t + 1
    
    'Camera triggers (camera and shutter)
    qTTL t - 0.1, Ctwo, 1 'camTrig
    qTTL t - 4, Dtwo, 1 ' camShutter
    
    'Flourescence
    qTTL t, Azero, 0
    qTTL t, Afive, 1
    
    t = t + imageLength
    
    'Shutting off all light and closing the relevant shutters:
    qTTL t, Ctwo, 0     ' CamTrig0
    qTTL t, Dtwo, 0     ' CamShutter

    qTTL t, Afive, 0    ' Side MOT beam off
    qTTL t, Azero, 0    ' MOT light off
    qTTL t, Aone, 0     ' Repump light off
    
    qvolts 0, t, 22 ' was 0.2 on 150408
    qvolts 0, t, 20
    qvolts 0, t, 21
    qvolts 0, t, 9
    qvolts 0, t, 11  'Repumplight power

    'Sending a pulse to oscilloscope trace, for observing the timing sequence
    Call markerOscilloscope
    
    t = t + 0.01
    
    'we found the camera needs at least 5ms gap time before immediately turning on the next takePGCimage subroutine 05/18/2016
    t = t + 5
End Sub

Public Sub probeImagingTest()
    'Call populateF1(5)  'not going to use this typically, just for testing!
    't = t + 10
    Call setQuantAxis '<--- this should already be set, but if not, we may need to add extra settling time?
            qTTL t, Dzero, 0
            qTTL t, Bone, 0 'Repump shutter closed here (labelled F=1; blocks both repump fibers) <-- currently not installed, but will add Randall's shutters here 160228
            qTTL t, Cthirteen, 0 'F=2 light shutter closed here (blocks side MOT beam and OP fiber paths)
            qTTL t, Cfour, 0
            qTTL t, Afive, 0 'just making sure probe beam is off!
            printnow "Close shutters for probe image (if not already)"
            
            ''qTTL t, Dtwo, 1 ' camShutter open
            qTTL t, Ctwelve, 1
            printnow "open probe shutters early"
            
            t = t + 20 ' we should be able to compact this a bit more by moving the shutter triggers around, but for now, just testing!
            
            '----------------- turning on the microwave before the first probe ------------------
            ' test of readout procedure!
            'Call microwaveRotation(0.01316) ' manually set pi pulse!
            'Call advanceRSgen
            't = t + 25 ' just being safe here - should be able to nmake shorter!
            ' ---------------- end of microwave pulse ----------------
            
            printnow "Probe beam imaging test"
            imagingPowerProbe = 0.32 '0.2 '0.32 '0.05 ' 0.05 ' 0.11 '0.11 '0.15 ' max power corresponds to ~30nW on 160424
            dacRamp t, 20, 4, 0.735, imDetuneProbe, 0.1 '0.735 is the typical setpoint for pump2stretch and cooling
                                                        '  - it may change after that, but jumping there shouldnt be a problem.
            qvolts imagingPowerProbe, t, 22
            t = t + 5 ' this serves as an offset time for the probe shutter to open, so do not add time between calling this subroutine and trigerring shutter!
            
            'Camera triggers (camera and shutter)
            qTTL t - 0.05, Ctwo, 1 'camTrig
            qTTL t - 4, Dtwo, 1 ' camShutter
            
            'Applying probe pulse!! 'copied from the pushout subroutine
            qvolts imDetuneProbe, t, 20 'probe detuning set to final value for image
            qvolts imagingPowerProbe, t, 22
            
            qTTL t, Afive, 1
            t = t + imagingLengthProbe
            qTTL t, Afive, 0
           ' qTTL t, Dtwo, 0     ' CamShutter
            
            qvolts 0.022, t, 22 ' this is closer to the minimum power than 0V...
            qTTL t, Ctwo, 0     ' CamTrig0
            
            '----------------- turning on the microwave before the second probe ------------------
            ' test of readout procedure!
            'Call microwaveRotation(0.01316) ' manually set pi pulse!
            'Call advanceRSgen
            
            't = t + 25 ' just being safe here - should be able to nmake shorter!
        
        
            'Camera triggers (camera and shutter)
            'printnow "Probe beam imaging test second pulse"
            'qTTL t - 0.05, Ctwo, 1 'camTrig
            'qTTL t - 4, Dtwo, 1 ' camShutter ' shutter is already open!
        
            'Applying probe pulse!! 'copied from the pushout subroutine
            'qvolts imDetuneProbe, t, 20 'probe detuning set to final value for image
            'qvolts imagingPowerProbe, t, 22
        
            'qTTL t, Afive, 1
            't = t + imagingLengthProbe
            'qTTL t, Afive, 0
            'qTTL t, Dtwo, 0     ' CamShutter
        
            'qvolts 0.022, t, 22 ' this is closer to the minimum power than 0V...
            'qTTL t, Ctwo, 0     ' CamTrig0
        
            '------------------------------------------------------
            
            
            t = t + 25 ' this is a hold time to make sure we arent actuating the camera shutter too fast!
                        ' Might actually wnat to move this before and then the probe shutter starts the image while the camera shutter ends it, just to make sure there is no extra leakage will try this first, since there seems to be nearly no leakage from probe...?
            
            'Shutting off all light and closing the relevant shutters:
            
            qTTL t, Dtwo, 0     ' CamShutter
            qTTL t, Ctwelve, 0 ' probe shutter!
            
            t = t + 20
            
            t = t + 1
            
        t = t + 0.01
End Sub



Public Sub probeImaging()
    Dim imagingPowerProbe As Double ' L stands for lossless imaging
    '**IMPORTANT NOTE!!!--> The shutter for the imaging beams (both the side-MOT and
    '       Repump shutters) will not be actuated within this subroutine, so that we have
    '       the flexibility to minimize the number of times it opens and closes (so we do
    '       not break the shutter so often) by only closing it for the main portion of the
    '       experiment (which will be done in the main code).
    ' *To be clear: The only shutter being actuated in the code is the shutter for the
    '       Andor camera, which we always run at this high rate (though, if we go to shorter
    '       imaging times, we will need to add an hold time to extend the shutter lifetime.
    '------------------------------------------------------------------------------------
    
    'Setting the relevant imaging parameters at the top:
    'imageLengthL = imagingLengthProbe ' this is already a global variable...
    'imDetuneProbe = 1.1 '<--- this should be a clobal variable
    
    
    'Call setQuantAxis --> Not calling this because it should alredy be set, and if it isnt, it should get called outside of this subroutine

    't = t + 18 - 0.01  'to let field settle and lasers settle
        
        
    
        
    printnow "Probe beam imaging"
    imagingPowerProbe = 0.05 ' 0.11 '0.11 '0.15 ' max power corresponds to ~25nW on 160228
    dacRamp t, 20, 4, 0.735, imDetuneProbe, 0.1 '0.735 is the typical setpoint for pump2stretch and cooling
                                                '  - it may change after that, but jumping there shouldnt be a problem.
    
    t = t + 5 ' this serves as an offset time for the probe shutter to open, so do not add time between calling this subroutine and trigerring shutter!
    
    'Camera triggers (camera and shutter)
    qTTL t - 0.1, Ctwo, 1 'camTrig
    qTTL t - 4, Dtwo, 1 ' camShutter
    
    'Applying probe pulse!! 'copied from the pushout subroutine
    qvolts imDetuneProbe, t, 20 'probe detuning set to final value for image
    qvolts imagingPowerProbe, t, 22
    
    qTTL t, Afive, 1
    t = t + imagingLengthProbe
    qTTL t, Afive, 0
    
    qvolts 0.022, t, 22 ' this is closer to the minimum power than 0V...
    
    t = t + 15 ' this is a hold time to make sure we arent actuating the camera shutter too fast!
                ' Might actually wnat to move this before and then the probe shutter starts the image while the camera shutter ends it, just to make sure there is no extra leakage will try this first, since there seems to be nearly no leakage from probe...?
    
    'Shutting off all light and closing the relevant shutters:
    qTTL t, Ctwo, 0     ' CamTrig0
    qTTL t, Dtwo, 0     ' CamShutter
    
    t = t + 0.01
End Sub

Public Sub inTrapPGC()
    'As before, no shutters are actuated within this subrouting - Be sure all the relevant
    '       shutters are open before calling this routine and closed after!!
    Dim PGCtime, PGCDetuning, PGCpowerMOTlight, PGCsideMOTpower, PGCRepPower As Double
    PGCtime = 3 ' do not need much PGC to cool near the bottom of the trap -> 10 is plenty
    PGCDetuning = -2.5 ' slightly different detuning than for initial loading
    PGCRepPower = 0.11 'lower power - may not have varied in a long time...
    PGCpowerMOTlight = 0 ' only side-mot beam for this cooling
    PGCsideMOTpower = 0.25 ' standard side-MOT beam power for MOT
    
    'Zeroed B-field for PGC (as of 150910, but last modified much earlier...)
    qvolts -0.7037, t, 2    'Front
    qvolts 0.3719, t, 3     'Back
    qvolts 0.8819, t, 0     'Right
    qvolts 0.0281, t, 1     'Left
    qvolts -2.0304, t, 4    'Bottom
    qvolts 2.0104, t, 5     'Top
    
    'Set beam powers for PGC and turn on beams (should already be on, but just in case):
    qvolts PGCRepPower, t, 11  'Repump power reduced by factor of 16
    qvolts PGCDetuning, t, 20   'Trap light freq
    qvolts PGCpowerMOTlight, t, 9 'trap light power
    qvolts PGCsideMOTpower, t, 22 'side MOT power
    qTTL t, Aone, 1 ' Repump light on
    qTTL t, Afive, 1 'Side MOT beam on
    qTTL t, Azero, 0 'Trap light on

    t = t + PGCtime
    printnow "PG cooled"

    qTTL t, Aone, 0 ' Repump light off
    qTTL t, Afive, 0 'side mot beam off
    qTTL t, Azero, 0 'Trap light off
    
    t = t + 0.01
End Sub

Public Sub setQuantAxis()
        azimuthalPump = 25 * 3.141592 / 180 '349, measured on 131028; 80 measured on 151221; 25 measured on 160325
        tipField = 0.047 '0.0345 '0.016 '0.10, measured on 131028; measured 0.016 on 151221; 0.047 measured on 160325
       
        'Setting the quantization axis:
        qvolts -0.7037 + tipField * 0.935 * Cos(azimuthalPump), t, 2 'Front
        qvolts 0.3719 - tipField * 0.935 * Cos(azimuthalPump), t, 3 'Back
        qvolts 0.8819 - 3 * 1.206, t, 0 'Right,
        qvolts 0.0281 + 3 * 1.206, t, 1 'Left
        qvolts -2.0304 - 1.4451 * tipField * Sin(azimuthalPump), t, 4  'Bottom -1.9
        qvolts 2.0104 + 1.4451 * tipField * Sin(azimuthalPump), t, 5  'Top 1.9
        
        t = t + 0.01
        
        
End Sub

Public Sub pump2stretch()
        printnow "Populating F=|22>"
                 
        'Setting the pumping beam frequencies:
        dacRamp t, 21, 10, 0, -1.65, 0.1 'Ramp of DAC 21, F1 offset lock to 2.32 for 2.2 mK trap and -.5V on dac14 AO, -1.5 for 1.1 mK
        qvolts 0, t, 14 'ROP AO, -.5 for 2mK, trap, 0 for 1.1 mK 141217,AMK
        qvolts 0.735, t, 20 'F2 light 1.1 for 2.1 mK and adjusted AO, .75 for 1.1mK
        
        Call setQuantAxis

        t = t + 18 - 0.01  'to let field settle and lasers settle
         
        'Setting the pumping beam powers:
        qvolts 0.3, t, 13 ' sigma+ OP beam power
        qvolts 0.3, t, 15       'sigma+ Repump beam power
         
        'Setting OP frequency based on trap depth:
        qvolts 0.75, t, 12 'frequency for 1.1mK trap depth (may need recalibration)
        'qvolts 2.5, t, 12 ''frequency for 2.2mK trap depth (may need recalibration)
         
        'Performing OP pulse:
        qTTL t, Afour, 1   'OP light on *****
        qTTL t, Athree, 1 'Sigma + repump beam *****
        
        t = t + 1.5
        
        qTTL t, Afour, 0
        qTTL t, Athree, 0
        
        qvolts 0, t, 13  'power min
        qvolts 0, t, 15

        t = t + 0.01
End Sub

Public Sub rampToDifferentDepth(ByVal inTrapDepth As Double, ByVal outTrapDepth As Double, ByVal rampTime As Double, ByVal rampRes As Double)

        printnow "====> DON'T USE THIS!!!!"
       ' Call dacRamp(t, 19, rampTime, inTrapDepth, outTrapDepth, rampRes)
       ' t = t + rampTime

End Sub

Public Sub threeDcooling()
        Dim coolSelect As Integer
        
        coolSelect = 1  '1 for gaussian pulsed cooling,
                                        '2 for continuous cooling,
                                        '3 for square pulsed cooling
                                        
        Select Case coolSelect
            Case 1:
                Dim cycles, OPpulseLength1, OPpower1, coolPulseLengthRadial, coolPulselengthAxial, coolPulseLengthRadial2, coolPulseLengthAxial2 As Double

                printnow "3D Gaussian-Pulsed cooling!"

                'General parameters of cooling routine
                cycles = 100 '200
                longerSplit = 65 '130 '65 '35 '50 'previously was 25

                qTTL t, Cfour, 1 'RB shutter opens, if not already...

                'Radial cool parameters
                coolPulseLengthRadial = 0.08 'for chirped pulses (to get any hotter atoms...)
                coolPulseLengthRadial2 = 0.12
    
               'Axial cool parameters
                 coolPulselengthAxial = 0.09 'for chirped pulses (to get hotter atoms)
                 coolPulseLengthAxial2 = 0.138
 
                 'Optical pumping parameters
                 OPpulseLength1 = 0.25 '0.5
                 OPpower1 = 0.3 '0.1
                 sigmaRepPow = 0.3 '0.2 '0.07
         
                 
                 'Switch to Cooling freqs (trigger FSK on tek AFGs)
                 '   (D4 is top, D5 is bottom, D6 is axial)
                 qTTL t, Dfour, 1
                 qTTL t, Dfive, 1
                 qTTL t, Dsix, 1
                 
                 
                 t = t + 25      'Delay for fields to completely settle, was ~12 ms longer (due to
                                         '       activation of continuous cooling section), but I dont think that was
                                         '       completely necessary, but should still keep it in mind...
                 
                 
                 qvolts 0.735, t, 20 'Offset lock for F2 light 0.735 for standard trap depths 160326; this should already be set, but confirming!
                 qvolts 0.75, t, 12 'OP freq, may want to set the offset lock voltages within the subroutine as well (just need to confirm what they should be)
                 qvolts -1.65, t, 21 'Offset lock for F1 light -1.65 for standard trap depths 160326; this should already be set, but confirming!
                 qvolts 0, t, 14 ' standard sigma+ repump AO freq
 
                 qvolts OPpower1, t, 13  'OP power
                 qvolts sigmaRepPow, t, 15  'sigma+ repump power


                 For p = 1 To cycles

                    If p < (cycles - longerSplit + 1) Then
             
                        'Axial cooling pulses (Raman + OP):
                        qTTL t, Dthree, 1 'trigger gaussian waveform
                        qTTL t, Aeleven, 1
                        qTTL t, Aeight, 1
                        t = t + coolPulselengthAxial
                        qTTL t, Aeleven, 0
                        qTTL t, Aeight, 0
                        qTTL t, Dthree, 0
            
                        qTTL t, Afour, 1   'OP light on
                        qTTL t, Athree, 1 'for 22 pumping
                        t = t + OPpulseLength1
                        qTTL t, Afour, 0
                        qTTL t, Athree, 0 'for 22 pumping
        
        
                        'Radial (only one radial direction per cycle, for initial cooling cycles)
                        If p Mod 2 = 1 Then
                            qTTL t, Athirteen, 1
                            qTTL t, Aten, 1
                            qTTL t, Aeight, 1
                            t = t + coolPulseLengthRadial '0.045 '
                            qTTL t, Aten, 0
                            qTTL t, Aeight, 0
                            qTTL t, Athirteen, 0

                            qTTL t, Afour, 1   'OP light on
                            qTTL t, Athree, 1 'for 22 pumping
                            t = t + OPpulseLength1
                            qTTL t, Afour, 0
                            qTTL t, Athree, 0 'for 22 pumping
                        Else
                            qTTL t, Athirteen, 1
                            qTTL t, Atwo, 1
                            qTTL t, Aeight, 1
                            t = t + coolPulseLengthRadial
                            qTTL t, Atwo, 0
                            qTTL t, Aeight, 0
                            qTTL t, Athirteen, 0
                                            
                            qTTL t, Afour, 1   'OP light on
                            qTTL t, Athree, 1 'for 22 pumping
                            t = t + OPpulseLength1
                            qTTL t, Afour, 0
                            qTTL t, Athree, 0 'for 22 pumping
    
                        End If
            
                    Else
    
                        'Top radial pulses (we do both radial in each cycle for the final cycles,
                        '       but now we will also put the axial pulse between!)
                        qTTL t, Athirteen, 1
                        qTTL t, Aten, 1
                        qTTL t, Aeight, 1
                        t = t + coolPulseLengthRadial2 '0.055 '
                        qTTL t, Aten, 0
                        qTTL t, Aeight, 0
                        qTTL t, Athirteen, 0
    
                        qTTL t, Afour, 1   'OP light on
                        qTTL t, Athree, 1 'for 22 pumping
                        t = t + OPpulseLength1
                        qTTL t, Afour, 0
                        qTTL t, Athree, 0 'for 22 pumping
    
    
                        'Axial pulses in final cycles:
                        qTTL t, Dthree, 1 'trigger gaussian waveform
                        qTTL t, Aeleven, 1
                        qTTL t, Aeight, 1
                        t = t + coolPulseLengthAxial2
                        qTTL t, Aeleven, 0
                        qTTL t, Aeight, 0
                        qTTL t, Dthree, 0
        
                        qTTL t, Afour, 1   'OP light on
                        qTTL t, Athree, 1 'for 22 pumping
                        t = t + OPpulseLength1
                        qTTL t, Afour, 0
                        qTTL t, Athree, 0 'for 22 pumping
    
    
                        'Bottom radial pulses
                        qTTL t, Athirteen, 1
                        qTTL t, Atwo, 1
                        qTTL t, Aeight, 1
                        t = t + coolPulseLengthRadial2
                        qTTL t, Atwo, 0
                        qTTL t, Aeight, 0
                        qTTL t, Athirteen, 0
    
                        qTTL t, Afour, 1   'OP light on
                        qTTL t, Athree, 1 'for 22 pumping
                        t = t + OPpulseLength1
                        qTTL t, Afour, 0
                        qTTL t, Athree, 0 'for 22 pumping
                    End If
         
                 Next p

                 t = t + 0.1
 
                 qTTL t, Afour, 1   'OP light on
                 qTTL t, Athree, 1 'for 22 pumping
                 
                 t = t + 0.5 ' Addig extra ms of OP to make sure state is pure! added on 160217** may want to remove if OP is working better later
                
 
                 qTTL t, Athree, 0 'Turn off repump sigma
                 qTTL t, Afour, 0 'Turn off 2-2'
                 qTTL t, Aeight, 0
                 qTTL t, Atwo, 0
                 qTTL t, Aten, 0
                 qTTL t, Athirteen, 0
                 qTTL t, Aeleven, 0
                 qTTL t, Dthree, 0
 
                 qvolts 0, t, 11
                 qvolts 0, t, 13
                 qvolts 0, t, 15
 
                 t = t + 0.1
         
         Case 2: 'continuous cooling
                 printnow "No cooling - need to set up this style of cooling first!"
         Case 3: 'square pulsed cooling
                 printnow "No cooling - need to set up this style of cooling first!"
    End Select

    t = t + 0.01
End Sub

Public Sub topSBspec(ByVal pulseLength As Double)
    '--> Note that there are no shutters actuated within this subroutine - need to make
    '       sure shutters are opened/closed appropriately in the main script section!


    'Not sure I need to do this anymore (not using another axial SB freq, so could just
    '       get rid of it, or maybe set up to actually do centered axial SB spectroscopy as well??
    qTTL t, Deleven, 0 'use 1sb for spectroscopy (no longer a 2nd SB freq...)
    qTTL t, Aseven, 0 '2sb off (no longer a 2nd SB freq...)
    t = t + 0.1
    
    'Turning on Dfour and Dsix selects the radial cooling freqs from the RF generator,
    '   while Dfive and Dseven are used to select the radial spectroscopy freqs (20 MHz).
    ''qTTL t, Dfour, 0
    ''qTTL t, Dfive, 1
    ''qTTL t, Dsix, 0
    ''qTTL t, Dseven, 1
  
    t = t + 1.1 'delay for microwave synth to settle after freq switch and also for the FG1 and FG2 Raman switch
    
    
    printnow "Top beam spectroscopy pulse"
    qTTL t, Aeight, 1 '
    qTTL t, Aten, 1
    qTTL t, Athirteen, 1 ' trigger for Gaussian pulse (wont matter if FG is set to DC, so leaving trigger)

    t = t + pulseLength
    
    qTTL t, Aeight, 0
    qTTL t, Aten, 0
    qTTL t, Athirteen, 0
    
    printnow CStr("  Pulse length " + CStr(pulseLength) + " ms")
    

    t = t + 0.01
End Sub

Public Sub botSBspec(ByVal pulseLength As Double)
    '--> Note that there are no shutters actuated within this subroutine - need to make
    '       sure shutters are opened/closed appropriately in the main script section!


    'Not sure I need to do this anymore (not using another axial SB freq, so could just
    '       get rid of it, or maybe set up to actually do centered axial SB spectroscopy as well??
    qTTL t, Deleven, 0 'use 1sb for spectroscopy (no longer a 2nd SB freq...)
    qTTL t, Aseven, 0 '2sb off (no longer a 2nd SB freq...)
    t = t + 0.1
    
    'Turning on Dfour and Dsix selects the radial cooling freqs from the RF generator,
    '   while Dfive and Dseven are used to select the radial spectroscopy freqs (20 MHz).
    ''qTTL t, Dfour, 0
    ''qTTL t, Dfive, 1
    ''qTTL t, Dsix, 0
    ''qTTL t, Dseven, 1
  
    t = t + 1.1 'delay for microwave synth to settle after freq switch and also for the FG1 and FG2 Raman switch
    
    
    printnow "Bottom beam spectroscopy pulse"
    qTTL t, Aeight, 1 '
    qTTL t, Atwo, 1
    qTTL t, Athirteen, 1 ' trigger for Gaussian pulse (wont matter if FG is set to DC, so leaving trigger)

    t = t + pulseLength
    
    qTTL t, Aeight, 0
    qTTL t, Atwo, 0
    qTTL t, Athirteen, 0
    
    printnow CStr("  Pulse length " + CStr(pulseLength) + " ms")

    t = t + 0.01
End Sub

Public Sub axialSBspec(ByVal pulseLength As Double)
    '--> Note that there are no shutters actuated within this subroutine - need to make
    '       sure shutters are opened/closed appropriately in the main script section!
    ' Additionally, be aware that the function generators are also not being advanced
    '       within these subroutines (so RS triggers need to be sent in main script**


    'Not sure I need to do this anymore (not using another axial SB freq, so could just
    '       get rid of it, or maybe set up to actually do centered axial SB spectroscopy as well??
    qTTL t, Deleven, 0 'use 1sb for spectroscopy (no longer a 2nd SB freq...)
    qTTL t, Aseven, 0 '2sb off (no longer a 2nd SB freq...)
    t = t + 0.1
      
    t = t + 1.1 'delay for microwave synth to settle after freq switch and also for the FG1 and FG2 Raman switch
    
    
    printnow "Axial beam spectroscopy pulse"
    qTTL t, Aeight, 1 '
    qTTL t, Aeleven, 1
    qTTL t, Dthree, 1 ' trigger for Gaussian pulse (wont matter if FG is set to DC, so leaving trigger)

    t = t + pulseLength
        
    qTTL t, Aeight, 0
    qTTL t, Aeleven, 0
    qTTL t, Dthree, 0
        
    printnow CStr("  Pulse length " + CStr(pulseLength) + " ms")
        
    t = t + 0.01
End Sub

Public Sub rotateFieldsFB()

    azimuthalPump = 25 * 3.141592 / 180 '349, measured on 131028, 80 is more recent (in 1512...)
    tipField = 0.047  '0.11 is th eold value  '0.0345 from 1512..
    printnow "Rotating fields to FB, plus 10ms settle time"
    
    'Ramp RL to 1G
    dacRamp t, 0, 2, 0.8819 - 3 * 1.206, 0.8819 - 1 * 1.206, 0.2 'Right,
    dacRamp t, 1, 2, 0.0281 + 3 * 1.206, 0.0281 + 1 * 1.206, 0.2 'Left
    t = t + 2
    'Ramp FB to 2G
    dacRamp t, 2, 2, -0.7037 + tipField * 0.935 * Cos(azimuthalPump), -0.7037 + 2 * 0.935, 0.2  'Front
    dacRamp t, 3, 2, 0.3719 - tipField * 0.935 * Cos(azimuthalPump), 0.3719 - 2 * 0.935, 0.2 'Back
    t = t + 2
    'Ramp RL to 0G
    dacRamp t, 0, 2, 0.8819 - 1 * 1.206, 0.8819 - 0 * 1.206, 0.2 'Right,
    dacRamp t, 1, 2, 0.0281 + 1 * 1.206, 0.0281 + 0 * 1.206, 0.2 'Left,
    t = t + 2
    'Ramp FB 3G
    dacRamp t, 2, 2, -0.7037 + 2 * 0.935, -0.7037 + 3 * 0.935, 0.2  'Front
    dacRamp t, 3, 2, 0.3719 - 2 * 0.935, 0.3719 - 3 * 0.935, 0.2   'Back
    t = t + 2
    
    t = t + 15 'settling time for fields - should be good enough '+++ change from 10 to 15, 05142016
    
    t = t + 0.01
End Sub


Public Sub rotateFieldsBack()
    azimuthalPump = 25 * 3.141592 / 180 '349, measured on 131028, 80 is more recent (in 1512...)
    tipField = 0.047  '0.11 is th eold value  '0.0345 from 1512..
    printnow "Rotating fields to FB, plus 10ms settle time"
    
    'Ramp FB from 3G down to 2G
    dacRamp t, 2, 2, -0.7037 + 3 * 0.935, -0.7037 + 2 * 0.935, 0.2  'Front
    dacRamp t, 3, 2, 0.3719 - 3 * 0.935, 0.3719 - 2 * 0.935, 0.2   'Back
    t = t + 2
    'Ramp RL from 0G back up to 1 G
    dacRamp t, 0, 2, 0.8819 - 0 * 1.206, 0.8819 - 1 * 1.206, 0.2 'Right,
    dacRamp t, 1, 2, 0.0281 + 0 * 1.206, 0.0281 + 1 * 1.206, 0.2 'Left,
    t = t + 2
    'Ramp FB from 2G back to 0ish G
    dacRamp t, 2, 2, -0.7037 + 2 * 0.935, -0.7037 + tipField * 0.935 * Cos(azimuthalPump), 0.2  'Front
    dacRamp t, 3, 2, 0.3719 - 2 * 0.935, 0.3719 - tipField * 0.935 * Cos(azimuthalPump), 0.2 'Back
    t = t + 2
    'Ramp RL from 1G back to 3G
    dacRamp t, 0, 2, 0.8819 - 1 * 1.206, 0.8819 - 3 * 1.206, 0.2 'Right,
    dacRamp t, 1, 2, 0.0281 + 1 * 1.206, 0.0281 + 3 * 1.206, 0.2 'Left
    t = t + 2
    
    
    t = t + 10 'settling time for fields - should be good enough
    
    t = t + 0.01
End Sub

Public Sub rotateFieldsBackTest(ByVal fieldShim As Double)
    azimuthalPump = 25 * 3.141592 / 180 '349, measured on 131028, 80 is more recent (in 1512...)
    tipField = 0.047  '0.11 is th eold value  '0.0345 from 1512..
    printnow "Rotating fields to FB, plus 10ms settle time"
    
    'Ramp FB from 3G down to 2G
    dacRamp t, 2, 2, -0.7037 + 3 * 0.935, -0.7037 + 2 * 0.935, 0.2  'Front
    dacRamp t, 3, 2, 0.3719 - 3 * 0.935, 0.3719 - 2 * 0.935, 0.2   'Back
    t = t + 2
    'Ramp RL from 0G back up to 1 G
    dacRamp t, 0, 2, 0.8819 - 0 * 1.206, 0.8819 - 1 * 1.206, 0.2 'Right,
    dacRamp t, 1, 2, 0.0281 + 0 * 1.206, 0.0281 + 1 * 1.206, 0.2 'Left,
    t = t + 2
    'Ramp FB from 2G back to 0ish G
    dacRamp t, 2, 2, -0.7037 + 2 * 0.935, -0.7037 + tipField * 0.935 * Cos(azimuthalPump), 0.2  'Front
    dacRamp t, 3, 2, 0.3719 - 2 * 0.935, 0.3719 - tipField * 0.935 * Cos(azimuthalPump), 0.2 'Back
    t = t + 2
    'Ramp RL from 1G back to 3G
    dacRamp t, 0, 2, 0.8819 - 1 * 1.206, 0.8819 - 3 * 1.206, 0.2 'Right,
    dacRamp t, 1, 2, 0.0281 + 1 * 1.206, 0.0281 + 3 * 1.206, 0.2 'Left
    t = t + 2
    
    'Ramp TopBottom from 0G back to other value
    dacRamp t, 4, 2, -2.0304, -2.0304 - fieldShim, 0.05 'Bottom
    dacRamp t, 5, 2, 2.0104, 2.0104 + fieldShim, 0.05 'Top
    t = t + 2
    
    printnow ("setting Top field as " + CStr(fieldShim))
    
    
    t = t + 10 'settling time for fields - should be good enough
    
    t = t + 0.01
End Sub


Public Sub singleSpinFliptest()
    'We are not closing/opening and shutters in this subroutine! Make sure that the Raman
    '       shutter is open and then closes immediately after this. Also that the microwave
    '       generator is programmed and triggered correctly!
    
    If True Then ' adding extra spin flip before the LS beam on, and remove micraowave during the LS beam on
    
        Call microwaveRotation(0.0135)
    
    End If
    
    '     If False Then
    If True Then
        'Setting LS beam power:
        qvolts FreeValue, t, 16
        'qvolts 0.125, t, 16 '+++' ' --- 0.125 'set ls beam power (may need to switch to different DAC soon!  was 0.067, now 0.125 on 160322
        t = t + 0.8 ' vary this to check the settling of the fields over time
        
        qTTL t, Csix, 1
        printnow ("LS beam on")
    Else
        qTTL t, Csix, 1
        printnow ("LS beam RAMPING on")
        dacRamp t, 16, 0.1, 0, FreeValue, 0.005 '+++ was 0.125, time was 0.5'
        t = t + 0.1 '+++ was 0.5'
    End If
    '    End If
    
    
        If False Then 'turn off the microwave
    't = t + 0.01
    
    qTTL t, Dthree, 1 'arb waveform control of mw
    If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
        '  printnow ("   Second microwave")
        qTTL t, Czero, 1
        qTTL t, Cone, 1
    Else
        printnow ("Reduce mw power below 1 dBm, did not fire")
    End If
         End If
    
    t = t + 0.03 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
    
       '    If False Then
    If True Then
        qTTL t, Csix, 0
        qvolts 0, t, 16
    Else
        printnow ("LS beam RAMPING off")
        dacRamp t, 16, 0.1, FreeValue, 0, 0.005 '+++ was 0.125, time was 0.5'
        t = t + 0.1 '+++ was 0.5'
        qTTL t, Csix, 0
    End If
    '    End If
    
    If True Then ' adding extra spin flip after the LS beam on
    
        Call microwaveRotation(0.0135)
    
    End If
    
End Sub

Public Sub singleSpinFlip()
    'We are not closing/opening and shutters in this subroutine! Make sure that the Raman
    '       shutter is open and then closes immediately after this. Also that the microwave
    '       generator is programmed and triggered correctly!
    
      
    If True Then
        'Setting LS beam power:
        qvolts 0.3, t, 16 '+++ change from 0.1 to 0.27 after adding ND10 before fiber '0.09 '0.15 from fits at 4/18/2016' ' --- 0.125 'set ls beam power (may need to switch to different DAC soon!  was 0.067, now 0.125 on 160322
        t = t + 0.8 ' vary this to check the settling of the fields over time
        
        qTTL t, Csix, 1 '+++ test LS shift on the unwanted atom 05142016
        printnow ("LS beam on")
    Else
        qTTL t, Csix, 1
        printnow ("LS beam RAMPING on")
        dacRamp t, 16, 0.1, 0, FreeValue, 0.005 '+++ was 0.125, time was 0.5'
        t = t + 0.1 '+++ was 0.5'
    End If
    

    't = t + 0.01
    
    qTTL t, Dthree, 1 'arb waveform control of mw
    
    If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
        '  printnow ("   Second microwave")
        qTTL t, Czero, 1
        qTTL t, Cone, 1
    Else
        printnow ("Reduce mw power below 1 dBm, did not fire")
    End If

    printnow CStr("applying microwave pulse for" + CStr(mwPulseLength) + "at frequency of" + CStr(RSFreq))
    t = t + mwPulseLength 'for varying length of uwave
    't = t + 0.03 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
    
    qTTL t, Czero, 0
    qTTL t, Cone, 0
    
    If True Then
        qTTL t, Csix, 0
        qvolts 0, t, 16
    Else
        printnow ("LS beam RAMPING off")
        dacRamp t, 16, 0.1, 0.17, 0, 0.005 '+++  DAC value was 0.125'
        t = t + 0.2
        qTTL t, Csix, 0
    End If
    
    
    'delay for arb trigger
    t = t + 0.01
    qTTL t, Dthree, 0

    t = t + 0.01
End Sub


Public Sub singleSpinFlipModulation()
    'We are not closing/opening and shutters in this subroutine! Make sure that the Raman
    '       shutter is open and then closes immediately after this. Also that the microwave
    '       generator is programmed and triggered correctly!
    
      
    qTTL t, Csix, 1
    printnow ("LS beam on")
    

    t = t + 40 'for modulation to kick out the atom
    
    qTTL t, Csix, 0
    qvolts 0, t, 16
    
    Call triggerArbIntensityRamps
    t = t + 10 ' for the Ramps to happen
    
    Call triggerArbIntensityRamps
    t = t + 10 ' for the Ramps to happen

    t = t + 0.01
    
    
End Sub
Public Sub microwaveRotation(ByVal pulseLength As Double)
    'We are not closing/opening and shutters in this subroutine! Make sure that the Raman
    '       shutter is open and then closes immediately after this. Also that the microwave
    '       generator is programmed and triggered correctly!

    'applying trigger for microwave pulse:
    qTTL t, Dthree, 1 'arb waveform control of mw
    
    'delay for arb trigger
    qTTL t + 0.1, Dthree, 0
    
    '+++ now repeat it five times to increase signal
    If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                printnow "Applying triggered microwave pulse"
        qTTL t, Czero, 1
        qTTL t, Cone, 1
    Else
        printnow ("Reduce mw power below 1 dBm, did not fire")
    End If
    
    t = t + pulseLength '+++ 2pi pulse!! need to change this back!!!' 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
    
    qTTL t, Czero, 0
    qTTL t, Cone, 0
        
    If False Then ' one repetitions!
        '+++ start of repeating, four times extra
        t = t + 0.05 ' delay between the pulses
        
        
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        
        ' +++ end of repeating
    End If
    
    
    If False Then ' two repetitions!
        '+++ start of repeating, four times extra
        t = t + 0.05 ' delay between the pulses
        
        
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        
        t = t + 0.05 ' delay between the pulses
        
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        
        ' +++ end of repeating
    End If
    
    If False Then
        '+++ start of repeating, four times extra
        t = t + 0.05 ' delay between the pulses
        
        If True Then
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        
        t = t + 0.05 ' delay between the pulses
        
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        
        t = t + 0.05 ' delay between the pulses
        
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        
        t = t + 0.05 ' delay between the pulses
        
        If RSPow < 19 Then '******May want to actually set things up so RSpow actually is used to define microwave power?***
                    printnow "Applying triggered microwave pulse"
            qTTL t, Czero, 1
            qTTL t, Cone, 1
        Else
            printnow ("Reduce mw power below 1 dBm, did not fire")
        End If
        
        t = t + pulseLength 'length of gaussian pulses used for pi-pulses (will have different routine for general, global spin rotations
        
        qTTL t, Czero, 0
        qTTL t, Cone, 0
        End If
        ' +++ end of repeating
    End If
        
    t = t + 0.01
End Sub

'****************************************************************************************
'Double-check, but I think this subroutine is set up correctly for tunneling - still need to set up exchange and entanglement verification!!!!
Public Sub tunnelingSequenceIntensityInit()
    Dim tunnelTimeTracker As Double
    'tunnelTimeTracker = 0
    
    printnow "Entering main tunneling section:"
  '  dacRamp t, 19, 4, inTrapDepth, tunnelDACdepth, 0.01
  '  t = t + 4

  '  qTTL t, Bfive, 1 'Switching to arb waveform intensity control (Agilent 33552B, GPIB 14)

    'Sending a trigger to the oscilloscope before initiating rampdown, for timing checks:
    ''Call markerOscilloscope

    'Send trigger to NI AWG for ramping tweezers together:
    printnow "  -Ramping traps together"
    Call triggerTweezerRamps
    t = t + rampToTunnelBiasTime ' first wait for bias ramp to tunneling bias (in 2 ms)
    t = t + AOfreqRampTime
    t = t + 1 - triggerOffset ' hold for correction waveform (maximum time)

    'Trigger intensity ramps down to initiate tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity down in " + CStr(intRampDownTime) + " ms, total")
    t = t + intRampDownTime - triggerOffset
    'tunnelTimeTracker = tunnelTimeTracker + intRampDownTime
    
    ''''TEMPORARY RAMP FREQ AFTER!
   ' printnow "  -Ramping traps together"
   ' Call triggerTweezerRamps
   ' t = t + rampToTunnelBiasTime - triggerOffset ' first wait for bias ramp to tunneling bias (in 2 ms)
   ' t = t + AOfreqRampTime - triggerOffset '<--- this ofset is for the oscilloscope trigger below!
    'tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + rampToTunnelBiasTime
    ''-----------------------
    
   ' Call markerOscilloscope
    
    'Evolution for tunneling:
    printnow CStr("  -Tunneling for " + CStr(tunnelTime) + " ms")
    t = t + tunnelTime
    'tunnelTimeTracker = tunnelTimeTracker + tunnelTime
            
    'Extra hold --> LEAVE ME ALWAYS 'took out on 151227 - I think this was more for spin exchange?? Can add back in, if necessary...
    ''t = t + 0.2
    ''tunnelTimeTracker = tunnelTimeTracker + 0.2
    
    'Call markerOscilloscope
    't = t - triggerOffset
    
    ''--MORE TEMPORARY TRIGGERS!
   '' Call triggerTweezerRamps '<---- commenting in/out trigger here because we are explicityly programming in the hold in the script
    'Wait for traps to ramp apart (linear freq ramp)
   ' t = t + AOfreqRampTime '
   ' t = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG
  ''  tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + rampToTunnelBiasTime
    '**THIS NEXT LINE IS IMPORTANT!!**
   ' t = t + 2 ' this extra time makes sure the intensity ramps do not happen before the tunneling sequence is done, so there should be no affect on tunneling
    ''-----------------------
    
    'Trigger intensity ramp up after tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity up in " + CStr(intRampUpTime) + " ms, total")
    t = t + intRampUpTime - triggerOffset
    'tunnelTimeTracker = tunnelTimeTracker + intRampUpTime
    
    'Trigger to reset intensity ramp sequence (back to constant hold at deep trap):
    'Call triggerArbIntensityRamps
    'tunnelTimeTracker = tunnelTimeTracker + triggerOffset
    
    'Checking whether the wells remained close together for long enough...
   ' If AOholdTime < tunnelTimeTracker Then
   '         printnow "  -No wait, trigger ramp up immediately"
   ' Else
   '         t = t + (AOholdTime - tunnelTimeTracker)
   '         printnow "  -Holding before trap separation"
   ' End If
   ' printnow CStr("Offset before separation: " + CStr(AOholdTime - tunnelTimeTracker) + " ms")
    
    Call triggerTweezerRamps
    
    'Wait for traps to ramp apart (linear freq ramp)
    t = t + AOfreqRampTime '
    t = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG
    t = t + 1 + 1 ' hold for correction waveform (maximum time) second +1, for worst case of center waveform of repeating 1 ms
    
    'Trigger to show on scope end of ramps
    Call markerOscilloscope

    printnow "Wells separated, tunneling complete"

    'Now that the dynamics are completed, set back to the original trap config:
  '  qvolts tunnelDACdepth, t, 19

  '  qTTL t, Bfive, 0 'Switching back to DAC control
    
  '  dacRamp t, 19, 4, tunnelDACdepth, inTrapDepth, 0.01 ' ramping back to original depth
  '  t = t + 4
    
    t = t + 0.01
End Sub
'****************************************************************************************
'Double-check, but I think this subroutine is set up correctly for tunneling - still need to set up exchange and entanglement verification!!!!
Public Sub tunnelingSequenceFreqInit()
    Dim tunnelTimeTracker As Double
    'tunnelTimeTracker = 0
    
    printnow "Entering main tunneling section:"
  '  dacRamp t, 19, 4, inTrapDepth, tunnelDACdepth, 0.01
  '  t = t + 4

  '  qTTL t, Bfive, 1 'Switching to arb waveform intensity control (Agilent 33552B, GPIB 14)

    'Sending a trigger to the oscilloscope before initiating rampdown, for timing checks:
    ''Call markerOscilloscope

    'Send trigger to NI AWG for ramping tweezers together:
   '' printnow "  -Ramping traps together"
   '' Call triggerTweezerRamps
   '' t = t + rampToTunnelBiasTime ' first wait for bias ramp to tunneling bias (in 2 ms)
   '' t = t + AOfreqRampTime
   '' t = t + 1 - triggerOffset ' hold for correction waveform (maximum time)

    'Trigger intensity ramps down to initiate tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity down in " + CStr(intRampDownTime) + " ms, total")
    t = t + intRampDownTime - triggerOffset
    'tunnelTimeTracker = tunnelTimeTracker + intRampDownTime
    
    
    ''''TEMPORARY RAMP FREQ AFTER!
    printnow "  -Ramping traps together"
    Call triggerTweezerRamps
    t = t + rampToTunnelBiasTime - triggerOffset ' first wait for bias ramp to tunneling bias (in 2 ms)
    t = t + AOfreqRampTime - triggerOffset '<--- this ofset is for the oscilloscope trigger below!
    'tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + rampToTunnelBiasTime
    ''-----------------------
    
    Call markerOscilloscope
    
    'Evolution for tunneling:
    printnow CStr("  -Tunneling for " + CStr(tunnelTime) + " ms")
    t = t + tunnelTime
    'tunnelTimeTracker = tunnelTimeTracker + tunnelTime
            
    'Extra hold --> LEAVE ME ALWAYS 'took out on 151227 - I think this was more for spin exchange?? Can add back in, if necessary...
    ''t = t + 0.2
    ''tunnelTimeTracker = tunnelTimeTracker + 0.2
    
    Call markerOscilloscope
    t = t - triggerOffset
    
    ''--MORE TEMPORARY TRIGGERS!
   '' Call triggerTweezerRamps '<---- commenting in/out trigger here because we are explicityly programming in the hold in the script
    'Wait for traps to ramp apart (linear freq ramp)
    t = t + AOfreqRampTime '
    t = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG
  ''  tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + rampToTunnelBiasTime
    '**THIS NEXT LINE IS IMPORTANT!!**
    t = t + 2 ' this extra time makes sure the intensity ramps do not happen before the tunneling sequence is done, so there should be no affect on tunneling
    ''-----------------------
    
    'Trigger intensity ramp up after tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity up in " + CStr(intRampUpTime) + " ms, total")
    t = t + intRampUpTime - triggerOffset
    'tunnelTimeTracker = tunnelTimeTracker + intRampUpTime
    
    'Trigger to reset intensity ramp sequence (back to constant hold at deep trap):
    'Call triggerArbIntensityRamps
    'tunnelTimeTracker = tunnelTimeTracker + triggerOffset
    
    'Checking whether the wells remained close together for long enough...
   ' If AOholdTime < tunnelTimeTracker Then
   '         printnow "  -No wait, trigger ramp up immediately"
   ' Else
   '         t = t + (AOholdTime - tunnelTimeTracker)
   '         printnow "  -Holding before trap separation"
   ' End If
   ' printnow CStr("Offset before separation: " + CStr(AOholdTime - tunnelTimeTracker) + " ms")
    
   '' Call triggerTweezerRamps
    
    'Wait for traps to ramp apart (linear freq ramp)
  ''  t = t + AOfreqRampTime '
  ''  t = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG
  ''  t = t + 1 + 1 ' hold for correction waveform (maximum time) second +1, for worst case of center waveform of repeating 1 ms
    
    'Trigger to show on scope end of ramps
    Call markerOscilloscope

    printnow "Wells separated, tunneling complete"

    'Now that the dynamics are completed, set back to the original trap config:
  '  qvolts tunnelDACdepth, t, 19

  '  qTTL t, Bfive, 0 'Switching back to DAC control
    
  '  dacRamp t, 19, 4, tunnelDACdepth, inTrapDepth, 0.01 ' ramping back to original depth
  '  t = t + 4
    
    t = t + 0.01
End Sub

Public Sub twoWayARPsequence(ByVal holdTime As Double)
    Dim tunnelTimeTracker As Double
    'tunnelTimeTracker = 0
    
    printnow "Beginning two-way ARP sequence:"
  
    'Trigger intensity ramps down to get to shallow traps!
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity down in " + CStr(intRampDownTime) + " ms, total")
    t = t + intRampDownTime - triggerOffset
    
    
    ''''TEMPORARY RAMP FREQ AFTER!
    printnow "  -Ramping traps to start position for ARP"
    Call triggerTweezerRamps
    't = t + rampToTunnelBiasTime - triggerOffset ' first wait for bias ramp to tunneling bias (in 2 ms) 'commented at 05112016
    t = t + AOfreqRampTime - triggerOffset '<--- this ofset is for the oscilloscope trigger below!
    ''-----------------------
    
    Call markerOscilloscope
    t = t - triggerOffset
    
    'Vary bias to ARP into other well:
    printnow CStr("  -ARP1 into well in " + CStr(timeOfARP) + " ms")
    t = t + timeOfARP
            
    
    Call markerOscilloscope
    t = t - triggerOffset
    
    'Call triggerArbIntensityRamps
    't = t + 0.1 - triggerOffset ' ramping the intensity up
    
     'Evolution for Spin exchange?:
    printnow CStr("  -Hold after ARP for " + CStr(holdTime) + " ms")
    t = t + holdTime
    
    'Call triggerArbIntensityRamps
    't = t + 0.1 - triggerOffset ' ramping the intensity back down
    
    Call markerOscilloscope
    t = t - triggerOffset
    
    'Vary bias to ARP out of other well:
    printnow CStr("  -ARP2 out of well in " + CStr(timeOfARP) + " ms")
    t = t + timeOfARP
    
    t = t + AOfreqRampTime '
    't = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG 'commented at 05112016
    
    'Trigger intensity ramp up after tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity up in " + CStr(intRampUpTime) + " ms, total")
    t = t + intRampUpTime - triggerOffset
   
    Call markerOscilloscope

    printnow "Wells separated, ARP sequence complete"
    
    t = t + 0.01
End Sub

Public Sub atomShuttlingImagingSequence()
    printnow "Beginning atom shutting and imaging sequence:"
  
    ''-----------------------
    Dim imageLength, imagingPower, imDetune As Double
    '**IMPORTANT NOTE!!!--> The shutter for the imaging beams (both the side-MOT and
    '       Repump shutters) will not be actuated within this subroutine, so that we have
    '       the flexibility to minimize the number of times it opens and closes (so we do
    '       not break the shutter so often) by only closing it for the main portion of the
    '       experiment (which will be done in the main code).
    ' *To be clear: The only shutter being actuated in the code is the shutter for the
    '       Andor camera, which we always run at this high rate (though, if we go to shorter
    '       imaging times, we will need to add an hold time to extend the shutter lifetime.
    '------------------------------------------------------------------------------------
    
    'Setting the relevant imaging parameters at the top:
    imageLength = imagingLength
    imDetune = 0.17
    imagingPower = 0.2
    
    'Setting PGC fields for imaging while cooling with side MOT beam:
    qvolts -0.7037, t, 2        'Front
    qvolts 0.3719, t, 3             'Back
    qvolts 0.8819, t, 0         'Right
    qvolts 0.0281, t, 1         'Left
    qvolts -2.0304, t, 4        'Bottom
    qvolts 2.0104, t, 5         'Top
           
    t = t + 0.1

    'below are the imaging parameters for the MOT light (specifically, the side MOT beam)
            
    qvolts imagingPower, t, 22 ' was 0.2 on 150408
    qvolts imDetune, t, 20 'Trap light ramped to image (detuning)
    qvolts 0, t, 21
    qvolts 0, t, 9
    qvolts 0.32, t, 11  'Repumplight power
           
           

            
    t = t + 5
    
    printnow "Take PGC Image"
    
    'Turning on the repunmp light <<**Be sure to open the repump shutter before calling this subroutine!!**>>
    qTTL t, Aone, 1 ' repump on 1 ms before (141121)
    t = t + 1
    
    'Camera triggers (camera and shutter)
    'qTTL t - 0.1, Ctwo, 1 'camTrig
    qTTL t - 4, Dtwo, 1 ' camShutter
    
    'Flourescence
    qTTL t, Azero, 0
    qTTL t, Afive, 1

    Call markerOscilloscope
    t = t - triggerOffset
    'Bring in well1 close to well2:
    Call triggerTweezerRamps
  
    ' Loop that takes all of the pictures for the movie.
    For picLoopInc = 1 To 18
        qTTL t - 0.1, Ctwo, 1 'camTrig
        t = t + 26
        qTTL t - 0.1, Ctwo, 0 'camTrig
        t = t + 25
    Next picLoopInc
    
    Call markerOscilloscope
    t = t - triggerOffset
    '------------------------- end of the fist half of the ramp  ----------------------------------
    'Call takeStandardPGCimage(imagingLength)
    
    'bring well1 back to original location:
    'Call triggerTweezerRamps
    printnow CStr("  -bring well1 back " + " 200 ms")
    't = t + 350 - triggerOffset
    
    'Call markerOscilloscope
    't = t - triggerOffset
       
    'printnow "Wells separated, ARP sequence complete"
    
    't = t + 300
    't = t + 0.01
    ' ---------------------------- end of insertion of the frequency ramps of the trap -----------------------------
    't = t + imageLength
    
    'Shutting off all light and closing the relevant shutters:
    'qTTL t, Ctwo, 0     ' CamTrig0
    qTTL t, Dtwo, 0     ' CamShutter

    qTTL t, Afive, 0    ' Side MOT beam off
    qTTL t, Azero, 0    ' MOT light off
    qTTL t, Aone, 0     ' Repump light off
    
    qvolts 0, t, 22 ' was 0.2 on 150408
    qvolts 0, t, 20
    qvolts 0, t, 21
    qvolts 0, t, 9
    qvolts 0, t, 11  'Repumplight power

    'Sending a pulse to oscilloscope trace, for observing the timing sequence
    Call markerOscilloscope
    
    t = t + 0.01
End Sub


Public Sub tunnelingSequenceBiasInit()
    Dim tunnelTimeTracker As Double
    'tunnelTimeTracker = 0
    
    printnow "Entering main tunneling section:"
  '  dacRamp t, 19, 4, inTrapDepth, tunnelDACdepth, 0.01
  '  t = t + 4

  '  qTTL t, Bfive, 1 'Switching to arb waveform intensity control (Agilent 33552B, GPIB 14)

    'Sending a trigger to the oscilloscope before initiating rampdown, for timing checks:
    ''Call markerOscilloscope

    'Send trigger to NI AWG for ramping tweezers together:
   '' printnow "  -Ramping traps together"
   '' Call triggerTweezerRamps
   '' t = t + rampToTunnelBiasTime ' first wait for bias ramp to tunneling bias (in 2 ms)
   '' t = t + AOfreqRampTime
   '' t = t + 1 - triggerOffset ' hold for correction waveform (maximum time)

    'Trigger intensity ramps down to initiate tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity down in " + CStr(intRampDownTime) + " ms, total")
    t = t + intRampDownTime - triggerOffset
    'tunnelTimeTracker = tunnelTimeTracker + intRampDownTime
    
    qTTL t - 1, Dten, 1
    qTTL t, Dten, 0
        
    ''''TEMPORARY RAMP FREQ AFTER!
    printnow "  -Ramping traps together"
    Call triggerTweezerRamps
    t = t + rampToTunnelBiasTime - triggerOffset ' first wait for bias ramp to tunneling bias (in 2 ms)
    t = t + AOfreqRampTime - triggerOffset '<--- this ofset is for the oscilloscope trigger below!
    'tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + rampToTunnelBiasTime
    ''-----------------------
    
    Call markerOscilloscope
    
    'Evolution for tunneling:
    printnow CStr("  -Tunneling for " + CStr(tunnelTime) + " ms")
    t = t + tunnelTime
    'tunnelTimeTracker = tunnelTimeTracker + tunnelTime
            
    'Extra hold --> LEAVE ME ALWAYS 'took out on 151227 - I think this was more for spin exchange?? Can add back in, if necessary...
    ''t = t + 0.2
    ''tunnelTimeTracker = tunnelTimeTracker + 0.2
    
    Call markerOscilloscope
    t = t - triggerOffset
    
    
    ''--MORE TEMPORARY TRIGGERS!
    'Call triggerTweezerRamps '<---- was commenting in/out trigger here because we are explicityly programming in the hold in the script,
                             '<---- changed for bias ramp
    'Wait for traps to ramp apart (linear freq ramp)
    t = t + AOfreqRampTime '- triggerOffset ' first wait for bias ramp to tunneling bias (in 2 ms)
    t = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG
  ''  tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + rampToTunnelBiasTime
    '**THIS NEXT LINE IS IMPORTANT!!**
    t = t + 2 ' this extra time makes sure the intensity ramps do not happen before the tunneling sequence is done, so there should be no affect on tunneling
    ''-----------------------
    
    'Trigger intensity ramp up after tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity up in " + CStr(intRampUpTime) + " ms, total")
    t = t + intRampUpTime - triggerOffset
    'tunnelTimeTracker = tunnelTimeTracker + intRampUpTime
    
    'Trigger to reset intensity ramp sequence (back to constant hold at deep trap):
    'Call triggerArbIntensityRamps
    'tunnelTimeTracker = tunnelTimeTracker + triggerOffset
    
    'Checking whether the wells remained close together for long enough...
   ' If AOholdTime < tunnelTimeTracker Then
   '         printnow "  -No wait, trigger ramp up immediately"
   ' Else
   '         t = t + (AOholdTime - tunnelTimeTracker)
   '         printnow "  -Holding before trap separation"
   ' End If
   ' printnow CStr("Offset before separation: " + CStr(AOholdTime - tunnelTimeTracker) + " ms")
    
   '' Call triggerTweezerRamps
    
    'Wait for traps to ramp apart (linear freq ramp)
  ''  t = t + AOfreqRampTime '
  ''  t = t + rampToTunnelBiasTime 'hold time for bias ramp from NI AWG
  ''  t = t + 1 + 1 ' hold for correction waveform (maximum time) second +1, for worst case of center waveform of repeating 1 ms
    
    'Trigger to show on scope end of ramps
    Call markerOscilloscope

    printnow "Wells separated, tunneling complete"

    'Now that the dynamics are completed, set back to the original trap config:
  '  qvolts tunnelDACdepth, t, 19

  '  qTTL t, Bfive, 0 'Switching back to DAC control
    
  '  dacRamp t, 19, 4, tunnelDACdepth, inTrapDepth, 0.01 ' ramping back to original depth
  '  t = t + 4
    
    t = t + 0.01
End Sub

Public Sub spinExchangeSequence(ByVal inTrapDepth As Double, ByVal ARPback As Boolean)
    If rotateQuantAxisFB = False Then
            printnow "Note: Fields not rotated to FB!"
    End If
    
    Dim tunnelTimeTracker As Double
    tunnelTimeTracker = 0
    
    printnow "Entering spin exchange section:"
    dacRamp t, 19, 4, inTrapDepth, tunnelDACdepth, 0.01
    t = t + 4

    qTTL t, Bfive, 1 'Switching to arb waveform intensity control (Agilent 33552B, GPIB 14)


    'Sending a trigger to the oscilloscope before initiating rampdown, for timing checks:
    Call markerOscilloscope

    'Send trigger to NIAWG for ramping tweezers together and initializing bias for ARP:
    printnow "  -Ramping traps together"
    Call triggerTweezerRamps
    t = t + AOfreqRampTime
    t = t + 2 ' for bias ramp
    
    tunnelTimeTracker = tunnelTimeTracker + AOfreqRampTime + 2

    'Trigger intensity ramps down to initiate tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity down in " + CStr(intRampDownTime) + " ms, total")
    t = t + intRampDownTime - triggerOffset
    tunnelTimeTracker = tunnelTimeTracker + intRampDownTime - triggerOffset


    'ARP for eg prep
    t = t + 0.2
    tunnelTimeTracker = tunnelTimeTracker + 0.2

    Call triggerTweezerRamps
    t = t + exchangeARPtime
    tunnelTimeTracker = tunnelTimeTracker + exchangeARPtime + triggerOffset
    printnow "  -ARP left well into right"

    '==========
    '--> This is where we would alter the traps to, for example, observe spin exchange
    '               in deeper traps (or to shut off off one well completely, if desired).
    '       Note that you will also have to reverse after the evolve time for unloading...
    '==========

    'Exchange
    t = t + exchangeTime
    tunnelTimeTracker = tunnelTimeTracker + exchangeTime

    If ARPback Then
        'ARP back with NI AWG
        Call triggerTweezerRamps
        t = t + exchangeARPtime  '12
        tunnelTimeTracker = tunnelTimeTracker + exchangeARPtime + triggerOffset '12
        printnow "  -ARP back to separate wells"
    End If
    
    t = t + 0.1
    tunnelTimeTracker = tunnelTimeTracker + 0.1


    'LEAVE ME ALWAYS
    t = t + 0.2
    tunnelTimeTracker = tunnelTimeTracker + 0.2
    qTTL t, Dthree, 0


    'Trigger intensity ramp up after tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity up in " + CStr(intRampUpTime) + " ms, total")
    t = t + intRampUpTime - triggerOffset
    tunnelTimeTracker = tunnelTimeTracker + intRampUpTime
    
    'Trigger to reset intensity ramp sequence (back to constant hold at deep trap):
    Call triggerArbIntensityRamps
    tunnelTimeTracker = tunnelTimeTracker + triggerOffset

    'Checking whether the wells remained close together for long enough...
    If AOholdTime < tunnelTimeTracker Then
            printnow "  -No wait, trigger ramp up immediately"
    Else
            t = t + (AOholdTime - tunnelTimeTracker)
            printnow "  -Holding before trap separation"
    End If
    printnow CStr("Offset before separation: " + CStr(AOholdTime - tunnelTimeTracker) + " ms")
    
    Call triggerTweezerRamps

    'Now, hold for traps to ramp apart:
    t = t + AOfreqRampTime 'linear ramp of spacing
    t = t + 2 ' bias ramp back to original bias

    'Trigger to show on scope end of ramps
    Call markerOscilloscope
    
    printnow "Wells separated, exchange sequence completed"
    If ARPback = False Then
        printnow " - actually, only ramp forward!"
    End If

    'Now that the dynamics are completed, set back to the original trap config:
    qvolts -8.416, t, 19 'setting back to original depth
    t = t + 3 'bias settle
    
    qTTL t, Bfive, 0 'Switching back to DAC control
    
    dacRamp t, 19, 4, -8.416, inTrapDepth, 0.01
    t = t + 4

    t = t + 0.01
End Sub


Public Sub entangleVerifyClose(ByVal inTrapDepth As Double)

    Dim tunnelTimeTracker As Double
    tunnelTimeTracker = 0
    
    printnow "Entering spin exchange section:"
    dacRamp t, 19, 4, inTrapDepth, tunnelDACdepth, 0.01
    t = t + 4

    qTTL t, Bfive, 1 'Switching to arb waveform intensity control (Agilent 33552B, GPIB 14)

    'Sending a trigger to the oscilloscope before initiating rampdown, for timing checks:
    Call markerOscilloscope

    'Send trigger to AFG for ramping tweezers together:
    printnow "  -Ramping traps together"
    Call triggerTweezerRamps
    t = t + AOfreqRampTime
    
    'Ramping quantization axis down to 750 mG along FB during intensity ramp-down
    If rotateQuantAxisFB Then
            printnow "  -Ramping FB quant. axis to 750 mG"
            dacRamp t + 0.5, 2, 2, -0.7037 + 3 * 0.935, -0.7037 + 0.75 * 0.935, 0.2 'Front
            dacRamp t + 0.5, 3, 2, 0.3719 - 3 * 0.935, 0.3719 - 0.75 * 0.935, 0.2 'Back
    Else
            printnow "**Why aren't you rotating fields to FB initially???"
    End If

    'Trigger intensity ramps down to initiate tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity down in " + CStr(intRampDownTime) + " ms, total")
    t = t + intRampDownTime - triggerOffset
    tunnelTimeTracker = tunnelTimeTracker + intRampDownTime


    'ARP for eg prep
    t = t + 0.2
    tunnelTimeTracker = tunnelTimeTracker + 0.2

    Call triggerTweezerRamps
    t = t + exchangeARPtime
    tunnelTimeTracker = tunnelTimeTracker + exchangeARPtime + triggerOffset
    printnow "  -ARP left well into right"


    'Exchange
    t = t + exchangeTime
    tunnelTimeTracker = tunnelTimeTracker + exchangeTime


    'ARP back
    Call triggerTweezerRamps
    t = t + exchangeARPtime '12
    tunnelTimeTracker = tunnelTimeTracker + exchangeARPtime + triggerOffset '12
    printnow "  -ARP back to separate wells"
    
    t = t + 0.1
    tunnelTimeTracker = tunnelTimeTracker + 0.1


    'turning on the gradient:
    qvolts gradField, t, 6
    printnow "  -Turn on gradient, evolve"
    t = t + 3 ' letting fields settle...
    tunnelTimeTracker = tunnelTimeTracker + 3
    

    'set time to evolve in gradient
    t = t + gradientTime
    tunnelTimeTracker = tunnelTimeTracker + gradientTime


    'Shutting off gradient:
    qvolts 0, t, 6
    t = t + 3 ' field settling time
    tunnelTimeTracker = tunnelTimeTracker + 3
    printnow "  -Shut off gradient, pi/2 pulse"

    
    'Applying a pi/2 pulse (manually, to control the exact timing)
    Call microwaveRotation(mwPulseLength / 2)
    tunnelTimeTracker = tunnelTimeTracker + mwPulseLength / 2 + 0.02 'extra time within subroutine should be 0.02 ms
        
    t = t + 0.1
    Call advanceRSgen
    tunnelTimeTracker = tunnelTimeTracker + 0.1 + triggerOffset


    'Offset evolve time -> Leave me always!
    t = t + 0.2
    tunnelTimeTracker = tunnelTimeTracker + 0.2


    'Trigger intensity ramp up after tunneling:
    Call triggerArbIntensityRamps
    printnow CStr("  -Ramping intensity up in " + CStr(intRampUpTime) + " ms, total")
    t = t + intRampUpTime - triggerOffset
    tunnelTimeTracker = tunnelTimeTracker + intRampUpTime
    
    'Trigger to reset intensity ramp sequence (back to constant hold at deep trap):
    Call triggerArbIntensityRamps
    tunnelTimeTracker = tunnelTimeTracker + triggerOffset


    'Checking whether the wells remained close together for long enough...
    If AOholdTime < tunnelTimeTracker Then
            printnow "  -No wait, trigger ramp up immediately"
    Else
            t = t + (AOholdTime - tunnelTimeTracker)
            printnow "  -Holding before trap separation"
    End If
    printnow CStr("Offset before separation: " + CStr(AOholdTime - tunnelTimeTracker) + " ms")
    
    Call triggerTweezerRamps
    'Now, hold for traps to ramp apart:
    t = t + AOfreqRampTime 'linear ramp of spacing

    'Trigger to show on scope end of ramps
    Call markerOscilloscope
    
    printnow "Wells separated, entanglement"
    printnow "  verification complete!"


    'Now that the dynamics are completed, set back to the original trap config:
    qvolts -8.416, t, 19 'setting back to original depth
    qvolts 0, t, 6
    t = t + 3 'bias settle
    
    qTTL t, Bfive, 0 'Switching back to DAC control
    
    dacRamp t, 19, 4, -8.416, finalTrap, 0.01
    t = t + 4

    t = t + 0.01
End Sub

Public Sub Pushout()
    ' No shutters are actuated within this subroutine to give flexibility for opening
    '       and closing times - immediately before is a good time to open a shutter, for
    '       instance, but we want to make sure there is plenty of time before closing again
    '       (so be sure to have sufficient delay  after until closing the shutter).
    Call triggerArbIntensityRamps

    qvolts 0.42, t, 20 'was 0.40, but this seems to be centered 160421
    t = t + 5 - triggerOffset
    
    printnow "ramping for pushout"

    'Applying probe pulse!!
    'qvolts 0.3, t, 22
    qvolts 0.05, t, 22 'was at full power: 0.3 V, but turning down seems to help for some reason (160421).
    ' qvolts 0.005, t, 22 ' +++ test of the push out on 11 state 043016
    qTTL t, Afive, 1
    t = t + 1
    't = t + 0.2 ' +++ test of the push out on 11 state 043016
    qTTL t, Afive, 0
    qvolts 0, t, 22
    
    Call triggerArbIntensityRamps
    t = t + 10 - triggerOffset

    t = t + 0.01
End Sub

Public Sub Pushout_old(ByVal pushoutTrapDepth As Double, ByVal inTrapDepth As Double)
    ' No shutters are actuated within this subroutine to give flexibility for opening
    '       and closing times - immediately before is a good time to open a shutter, for
    '       instance, but we want to make sure there is plenty of time before closing again
    '       (so be sure to have sufficient delay  after until closing the shutter).
    dacRamp t, 19, 5, inTrapDepth, pushoutTrapDepth, 0.05
    qvolts 0.4, t, 20 '.4 is for -0.5 V depth pushout
    t = t + 5
    
    printnow "ramping for pushout"

    'Applying probe pulse!!
    qvolts 0.3, t, 22
    qTTL t, Afive, 1
    t = t + 1
    qTTL t, Afive, 0
    qvolts 0, t, 22
    
    dacRamp t, 19, 10, pushoutTrapDepth, inTrapDepth, 0.025
    t = t + 10

    t = t + 0.01
End Sub

Public Sub alignmentPush(ByVal pushPulseLength As Double, ByVal pushPulsePow As Double, ByVal inTrapDepth As Double)
    Dim pushoutTrapDepth As Double
    ' The only shutter actuated within the subroutine is the probe beam shutter, since we
    '       wont ever need to keep it open for another part of the code, if we are using this
    '       section of the code...
    ' However - be sure to actuate any other necessary shutters (especially the Repump!!**)
    
    'Setting the standard pushout trap depth, -0.5 V
   ' pushoutTrapDepth = -0.5
   '
   ' dacRamp t, 19, 5, inTrapDepth, pushoutTrapDepth, 0.05
   ' qvolts 0.4, t, 20 '.4 is for -0.5 V depth pushout
    Call triggerArbIntensityRamps
    t = t + 5 - triggerOffset
    
    printnow "ramping for pushout"
        
    'Setting power for pushout to be 0.01 V, typical for final alignments
    '   Can increase, if necessary, but dont exceed 0.04 (and the less the better),
    '   unless you have added a separate shutter for the side MOT beam (probably should).
    qvolts pushPulsePow, t, 22
    
    'Opening the alignment shutter before pushout pulse
    qTTL t, Cthirteen, 0
    qTTL t, Ctwelve, 1 'Currently probe-beam shutter
    t = t + 20

    '1 ms Repump pulse before pushout during alignment
    qvolts 0.32, t, 11
    qTTL t, Aone, 1
    t = t + 1
    qTTL t, Aone, 0
    qvolts 0, t, 11
    
    'Applying probe pulse!!
    qTTL t, Afive, 1
    t = t + pushPulseLength
    qTTL t, Afive, 0
    qvolts 0, t, 22
    
    'Extra hold time before closing alignment shutter - dont want to kill it too fast...
    t = t + 30
    qTTL t, Ctwelve, 0
    qTTL t, Cthirteen, 1
    t = t + 10
    
    'dacRamp t, 19, 10, pushoutTrapDepth, inTrapDepth, 0.025
    't = t + 10
    
    Call triggerArbIntensityRamps
    t = t + 10 - triggerOffset
    
    t = t + 0.01
End Sub

Public Sub trapDepthCalibration(ByVal inTrapDepth As Double, ByVal pushoutTrapDepth As Double, ByVal probeFreq As Double, ByVal probePower As Double, ByVal probePulseLength As Double)
    ' The only shutter actuated within this subroutine is the shutter for the probe beam
    '        (Ctwelve), so make sure to open repump shutter, etc, if necessary!!
    dacRamp t, 19, 5, inTrapDepth, pushoutTrapDepth, 0.05
    qvolts probeFreq, t, 20
    t = t + 5
       
    printnow "ramping for pushout"
        
    'Setting power for pushout to be 0.01 V, typical for final alignments
    '   Can increase, if necessary, but dont exceed 0.04 (and the less the better),
    '   unless you have added a separate shutter for the side MOT beam (probably should).
    qvolts probePower, t, 22
    
    'Opening the alignment shutter before pushout pulse
    qTTL t, Ctwelve, 1 'Currently probe-beam shutter
    t = t + 20

        '1 ms Repump pulse before pushout during alignment
    qvolts 0.32, t, 11
    qTTL t, Aone, 1
    t = t + 1
    qTTL t, Aone, 0
    qvolts 0, t, 11
    
    'Applying probe pulse!!
    qTTL t, Afive, 1
    t = t + probePulseLength
    qTTL t, Afive, 0
    qvolts 0, t, 22
    
    'Extra hold time before closing alignment shutter - dont want to kill it too fast...
    t = t + 30
    qTTL t, Ctwelve, 0
    t = t + 10
    
    dacRamp t, 19, 10, pushoutTrapDepth, inTrapDepth, 0.025
    t = t + 10
    
    t = t + 0.01
End Sub

Public Sub wellCombineSequence(ByVal inTrapDepth As Double, ByVal togetherTime As Double)
    printnow ("Combining wells")
    
    'lowering the trap power so we do not saturate when shutting off left well
    dacRamp t, 19, 5, inTrapDepth, -7.5, 0.1
    t = t + 5

    qTTL t, Bfifteen, 0 'trigger RF sweep **-> Be sure to program correct together to generator!
    qTTL t, Afifteen, 0
    t = t + 0.1
    qTTL t, Bfifteen, 1
    qTTL t, Afifteen, 1
    printnow "RF ramp initiated"
    t = t + 0.1
    qTTL t, Bfifteen, 0
    qTTL t, Afifteen, 0
        
    t = t + 10
        
    'Ramping off left trap
    printnow "**Need to set up this sequence within the NIAWG!! - cannot ramp off trap via DACs!"
    t = t + 5
    
    t = t + AOholdTime - 5

    printnow "  Ramping traps apart"
    t = t + 10 'linear ramp of spacing

    printnow "      Ramping left well back on"
    t = t + 5

    t = t + 0.01
End Sub

Public Sub pushout795(ByVal inTrapDepth As Double, ByVal pushout795TrapDepth As Double, ByVal freq795 As Double, ByVal pow795 As Double, ByVal pulseLength795 As Double)
        Dim depumpPower, depumpFreq As Double
        
        'Setting various parameters:
        depumpPower = 4
        depumpFreq = 1.5 ' this should probably be set to resonance, but depends how this section is used... can customize and make other versions too, if relevant
        
        qvolts -0.7037, t, 2  'Front '-.04
    qvolts 0.3719, t, 3  'Back
    qvolts 0.8819, t, 0  'Right, '-.075
    qvolts 0.0281, t, 1  'Left
    qvolts -2.0304, t, 4  'Bottom -1.9 '-.06 -.15
        qvolts 2.0104, t, 5  'Top 1.9
            
    printnow "Zero fields for 795 pushout -> 10ms hold"
    t = t + 10
    
        dacRamp t, 19, 5, inTrapDepth, pushout795TrapDepth, 0.05
        t = t + 5
        
    If depumpFreq <> 0 Then
        dacRamp t, 10, 5, 0, depumpFreq, 0.05
        End If
        t = t + 5
    If freq795 <> 0 Then
        dacRamp t, 17, 5, 0, freq795, 0.05
    End If
    t = t + 5

    'Applying pulse of 795 and 2-2 light!!
    qvolts depumpPower, t, 8 'power of 2-2 beam (setting standard power)
    qvolts pow795, t, 18

    printnow "Applying pulse of 795 and 2-2' light"

        qTTL t, Cten, 1
        qTTL t, Cnine, 1
    t = t + pulseLength795
    qTTL t, Cten, 0
    qTTL t, Cnine, 0

    qvolts 0, t, 8 'power of 2-2 beam (setting standard power)
    qvolts 0, t, 18

    If depumpFreq <> 0 Then
        dacRamp t, 10, 5, depumpFreq, 0, 0.05
        End If
        t = t + 5
    If freq795 <> 0 Then
        dacRamp t, 17, 5, freq795, 0, 0.05
    End If
    t = t + 5


        dacRamp t, 19, 10, pushout795TrapDepth, inTrapDepth, 0.025
        t = t + 10

        t = t + 0.01
End Sub


Public Sub rampDownSequence(ByVal inTrapDepth As Double, ByVal minTrapDepth As Double, ByVal shallowHoldTime As Double)
        dacRamp t, 19, 10, inTrapDepth, -1, 0.05
        t = t + 10
        
        dacRamp t, 19, 10, -1, minTrapDepth, 0.002
        t = t + 10
        
        printnow CStr("Holding in shallow trap for " + CStr(shallowHoldTime) + " ms")
        t = t + shallowHoldTime
        
        
        dacRamp t, 19, 10, -0.1, -4.6, 0.04 ' was 0.015 for bottom value, changed to 0.010 after cal
        t = t + 10
        
        dacRamp t, 19, 10, -4.6, finalTrap, 0.1 ' was 0.015 for initial value
        t = t + 10
        
End Sub



Public Sub triggerOscilloscope()
        'This trigger is typically used to indicate to the oscilloscope the start of each
        '       run, but we can also move it around, if its useful (probably not).
        qTTL t, Celeven, 1
        t = t + 0.1
        qTTL t, Celeven, 0
        
        t = t + 0.01
End Sub

Public Sub markerOscilloscope()
    'Send TTL pulse to an Oscilloscope trace (for seeing/measuring timing sequences)
    qTTL t, Afifteen, 1
    t = t + 0.2 ' +++ was 0.1, incomplete square shape 4/23/16
    qTTL t, Afifteen, 0
        
    t = t + 0.01
End Sub

Public Sub advanceRSgen()
    printnow "Advancing Rhode Schwartz Generator"
    qTTL t, Cthree, 1
    t = t + 0.1
    qTTL t, Cthree, 0
    
    t = t + 0.01
End Sub

Public Sub triggerSimultaneousTweezerIntRamps()
        'Sending trigger for RF sweeps of AWG:
        '       Currently this starts sweeping together traps, but with NI AWG, this will be a general trigger!
        printnow "  (Triggering NI AWG and Arb ramps)"
        qTTL t, Bfourteen, 1
        qTTL t, Bsix, 1
        t = t + 0.1
        qTTL t, Bfourteen, 0
        qTTL t, Bsix, 0
        
        t = t + 0.01
End Sub

Public Sub triggerTweezerRamps()
        'Sending trigger for RF sweeps of AWG:
        '       Currently this starts sweeping together traps, but with NI AWG, this will be a general trigger!
        printnow "  (Triggering NI AWG: Next waveform)"
        qTTL t, Bfourteen, 1
        t = t + 0.1
        qTTL t, Bfourteen, 0
        
        t = t + 0.01
End Sub

Public Sub triggerArbIntensityRamps()
        'Sending trigger for RF sweeps of AWG:
        '       Currently this starts sweeping together traps, but with NI AWG, this will be a general trigger!
        printnow "  (Triggering ARB for int. ramp)"
        qTTL t, Bsix, 1
        t = t + 0.1
        qTTL t, Bsix, 0
        
        t = t + 0.01
End Sub

Public Sub sigmaDepumpTest(ByVal depumpPulseLength As Double)
        printnow "Test darkness of state"
        
        'Use the section below only for tweaking the quantization axis (then set final values in the QA section)
        '----> currently only use this after pump2stretch, so the field should already be set!
        '-----------------------------------
       ' azimuthalPump = FreeValue * 3.141592 / 180 '349, measured on 131028
       ' tipField = 0.0345 '.11'0.10, measured on 131028
       '
       ' qvolts -0.7037 + tipField * 0.935 * Cos(azimuthalPump), t, 2 'Front
       ' qvolts 0.3719 - tipField * 0.935 * Cos(azimuthalPump), t, 3 'Back
       ' qvolts 0.8819 - 3 * 1.206, t, 0  'Right,
       ' qvolts 0.0281 + 3 * 1.206, t, 1  'Left
       ' qvolts -2.0304 - 1.4451 * tipField * Sin(azimuthalPump), t, 4 'Bottom -1.9
       ' qvolts 2.0104 + 1.4451 * tipField * Sin(azimuthalPump), t, 5 'Top 1.9
       '
       ' t = t + 18
       ' printnow CStr(azimuthalPump)
        '-----------------------------------
        
        
        
        'SET BEAM FREQUENCIES:
        qvolts 0.735, t, 20 '''0.71 from 160217 'DAC12 .75V = 69.5MHz: .95 for .8V,.7 for .453,.92 for .65 (all of these with the .75 V on teh dac 12)
        qvolts 0.75, t, 12 '0 for .1V trap
        
        'set beam power:
        qvolts 0.3, t, 13
        
        t = t + 3
        
        qTTL t, Afour, 1 'OP light on
        t = t + depumpPulseLength
        qTTL t, Afour, 0 'off
         
        qvolts 0, t, 13  'power min
        qvolts 0, t, 11
         
        t = t + 0.01
End Sub

Public Sub sigmaRepumpTest(ByVal OPpulseLength As Double)
        'Setting the pumping beam frequencies:
        dacRamp t, 21, 10, 0, -1.65, 0.1 'Ramp of DAC 21, F1 offset lock to 2.32 for 2.2 mK trap and ..5V on dac14 AO, -1.5 for 1.1 mK
        qvolts 0, t, 14 'ROP AO, -.5 for 2mK, trap, 0 for 1.1 mK 141217,AMK
        qvolts 0.735, t, 20 'F2 light 1.1 for 2.1 mK and adjusted AO, .75 for 1.1mK
        qvolts 0.75, t, 12 'frequency for 1.1mK trap depth (may need recalibration), OP frequency
             
         printnow "Populating F=|22>"
         'azimuthalPump = 349 * 3.141592 / 180 '349, measured on 131028
         'tipField = 0.11 '0.10, measured on 131028
        
         'Setting the quantization axis, same as in pump2stretch:
         'qvolts -0.7037 + tipField * 0.935 * Cos(azimuthalPump), t, 2 'Front
         'qvolts 0.3719 - tipField * 0.935 * Cos(azimuthalPump), t, 3 'Back
         'qvolts 0.8819 - 3 * 1.206, t, 0 'Right,
         'qvolts 0.0281 + 3 * 1.206, t, 1 'Left
         'qvolts -2.0304 - 1.4451 * tipField * Sin(azimuthalPump), t, 4  'Bottom -1.9
         'qvolts 2.0104 + 1.4451 * tipField * Sin(azimuthalPump), t, 5  'Top 1.9
         Call setQuantAxis ' this is now separated out to make sur ethe quantization axis is always set to be the same (when we wnat it to be good)
         
         t = t + 18 'to let field settle and lasers settle
        
        
         qvolts 0.3, t, 13 ' sigma+ OP beam power
         qvolts 0.3, t, 15 'sigma+ Repump beam power
        
        
         'Performing OP pulse:
         qTTL t, Afour, 1   'OP light on *****
         qTTL t, Athree, 1 'Sigma + repump beam *****
        
         t = t + OPpulseLength
        
         qTTL t, Afour, 0
         qTTL t, Athree, 0
        
         qvolts 0, t, 13  'power min
         qvolts 0, t, 15

         t = t + 0.01
End Sub
Public Sub populateF1(ByVal depumpPulseLength As Double)
        printnow "Populating F=1"
         
        qvolts 0.3, t, 11
        qTTL t, Aone, 1
        t = t + 1
        qTTL t, Aone, 0
        qvolts 0, t, 11
         
        qvolts 0.735, t, 20 '0.735 on 160326 from polarization purity measurmeents
       ' qvolts -2, t, 21 '.41 for .1 V trap
        
         
        qvolts -0.7037 - 0.935, t, 2 'Front
        qvolts 0.3719 + 0.935, t, 3 'Back
        qvolts 0.8819 - 1.206, t, 0 'Right,
        qvolts 0.0281 + 1.206, t, 1 'Left
        qvolts -2.0304, t, 4  'Bottom -1.9
        qvolts 2.0104, t, 5  'Top 1.9
        
        qvolts 0.3, t, 13 '0.3
        'qvolts 2.5, t, 12 '0 for 2 mK
        qvolts 0.75, t, 12 'I think 0.75 V is standard for 1 mK traps... may want to calibrate '' 0.75
        
        t = t + 3
         
        qTTL t, Afour, 1   'OP light on
        t = t + depumpPulseLength 'was typically 0.5 ms in old code (for preparing F=1)
        qTTL t, Afour, 0 'off
        qvolts 0, t, 13  'power min

        t = t + 0.01
End Sub

Public Sub sendMessagesToNIAWG(ByVal tunedParam As String)
    
    Dim accumFactor As Double
   
    Dim messageToNIAWG1, messageToNIAWG2, messageToNIAWG3, messageToNIAWG4, messageToNIAWG5 As String
    If goflag = True Then
        varFlagFromNIAWG = False
        sendAccumsFlag = False
   
   
        printnow "Waiting for open TCP connection..."
        While InputFrm.tcpserver.State <> sckConnected And goflag = True
            wait (200)
            DoEvents
        Wend
        If InputFrm.tcpserver.State <> sckConnected Then
            ' then break out of subroutine, don't try to talk to niawg.
            Exit Sub
        End If
        
        accumFactor = 1 ' this is way to change number sent to NI AWG
        
        printnow "   connection open!"
        If tunedParam = "tunnelBias" Or tunedParam = "tunnelTime" Or tunedParam = "arpTime" Or tunedParam = "arpRange" Or tunedParam = "arpHold" Or tunedParam = "CarrFreqSpinExchange" Or tunedParam = "piezoMirrorSpinExchange" Or tunedParam = "CarrFreqSpinExchange" Or tunedParam = "fieldRotatingBackSpinExchange" Or tunedParam = "RSFreqSpinExchange" Then
            printnow "--> assuming enhanced loading!!!"
            printnow "   telling NIAWG accums/" + CStr(Format(accumFactor, "0.")) + "!!"
            messageToNIAWG1 = "Accumulations: " + CStr(Format(accumulations / accumFactor, "0.0"))
        Else
            messageToNIAWG1 = "Accumulations: " + CStr(Format(accumulations, "0.0"))
        End If
        
        While sendAccumsFlag = False And goflag = True
            wait (100)
            DoEvents
        Wend
        InputFrm.tcpserver.SendData (messageToNIAWG1)
        
        While varFlagFromNIAWG = False And goflag = True
            wait (100)
            DoEvents
        Wend
        
        Select Case tunedParam
            Case "tunnelBias":
            
                If goflag = True Then
                    varFlagFromNIAWG = False
                    
                    printnow "Now, sending variables!"
                    
                    messageToNIAWG1 = "b " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "d " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(1 - indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "t " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(tunnelTime, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                End If
            Case "tunnelTime":
                
                If goflag = True Then
                    varFlagFromNIAWG = False
                        
                    printnow "Now, sending variables!"
                        
                    messageToNIAWG1 = "t " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "b " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(tunnelBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "d " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - tunnelBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                End If
            
            Case "RSFreqTunneling":
                
                If goflag = True Then
                    varFlagFromNIAWG = False
                        
                    printnow "Now, sending variables!"
                        
                    messageToNIAWG1 = "t " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "b " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(tunnelBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "d " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - tunnelBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                End If
                
            Case "arpRange":
                 
                If goflag = True Then
                    varFlagFromNIAWG = False
                        
                    printnow "Now, sending variables!"
                        
                    messageToNIAWG1 = "tr " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(timeOfARP, "0.000000")) + " " ' was using the line below but NIAWG dislike
                        'messageToNIAWG1 = messageToNIAWG1 + CStr(Format((ARPinitBias - indDataPoints(i)) * timeOfARP / (ARPinitBias - ARPfinalBias), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG1 = "th " 'time to jhold before arping bakc (if relevant)
                    For i = 0 To expRep - 1
                        'messageToNIAWG1 = messageToNIAWG1 + CStr(Format(timeOfARP, "0.000000")) + " "
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(holdAfterARP, "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "bi " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG2 = messageToNIAWG2 + CStr(Format(tunnelBias - indDataPoints(i), "0.000000")) + " "
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(ARPinitBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "di " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - (tunnelBias - indDataPoints(i)), "0.000000")) + " "
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - ARPinitBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG4 = "bf " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG4 = messageToNIAWG4 + CStr(Format(tunnelBias + indDataPoints(i), "0.000000")) + " "
                        messageToNIAWG4 = messageToNIAWG4 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG4 = messageToNIAWG4 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG4)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG5 = "df " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG5 = messageToNIAWG5 + CStr(Format(1 - (tunnelBias + indDataPoints(i)), "0.000000")) + " "
                        messageToNIAWG5 = messageToNIAWG5 + CStr(Format(1 - (indDataPoints(i)), "0.000000")) + " "
                    Next i
                    messageToNIAWG5 = messageToNIAWG5 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG5)
                    
                End If
                
            Case "arpHold":
                 
                If goflag = True Then
                    varFlagFromNIAWG = False
                        
                    printnow "Now, sending variables!"
                        
                    messageToNIAWG1 = "tr " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(timeOfARP, "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG1 = "th " 'time to jhold before arping bakc (if relevant)
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "bi " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG2 = messageToNIAWG2 + CStr(Format(tunnelBias - indDataPoints(i), "0.000000")) + " "
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(ARPinitBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "di " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - (tunnelBias - indDataPoints(i)), "0.000000")) + " "
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - ARPinitBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG4 = "bf " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG4 = messageToNIAWG4 + CStr(Format(tunnelBias + indDataPoints(i), "0.000000")) + " "
                        messageToNIAWG4 = messageToNIAWG4 + CStr(Format(ARPfinalBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG4 = messageToNIAWG4 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG4)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG5 = "df " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        'messageToNIAWG5 = messageToNIAWG5 + CStr(Format(1 - (tunnelBias + indDataPoints(i)), "0.000000")) + " "
                        messageToNIAWG5 = messageToNIAWG5 + CStr(Format(1 - ARPfinalBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG5 = messageToNIAWG5 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG5)
                    
                End If
                
            Case "arpTime":
                
                If goflag = True Then
                    varFlagFromNIAWG = False
                        
                    printnow "Now, sending variables!"
                        
                    messageToNIAWG1 = "tr " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    
                    messageToNIAWG1 = "th " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(holdAfterARP, "0.000000")) + " "
                        'messageToNIAWG1 = messageToNIAWG1 + CStr(Format(1, "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    
                    messageToNIAWG2 = "bi " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(ARPinitBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "di " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - (ARPinitBias), "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG4 = "bf " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG4 = messageToNIAWG4 + CStr(Format(ARPfinalBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG4 = messageToNIAWG4 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG4)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG5 = "df " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG5 = messageToNIAWG5 + CStr(Format(1 - ARPfinalBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG5 = messageToNIAWG5 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG5)
                    
                End If
            
            Case "sideBiases":
                If goflag = True Then
                    varFlagFromNIAWG = False
                    
                    printnow "Now, sending variables!"
                    
                    messageToNIAWG1 = "btop " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "dtop " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(1 - indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    
                    varFlagFromNIAWG = False
                    
                    printnow "Now, sending variables!"
                    
                    messageToNIAWG1 = "bbot " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "dbot " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(1 - indDataPoints(i), "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend

                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "t " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(tunnelTime, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                End If
                
            Case "relativeBias":
                 Dim topPairBias, botPairBias As Double
                 topPairBias = 0.5265 ' set from initial run on 160426
                 botPairBias = 0.548
                                  
                 If goflag = True Then
                    varFlagFromNIAWG = False
                    
                    printnow "Now, sending variables!"
                    
                    messageToNIAWG1 = "btop " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(indDataPoints(i) * topPairBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "dtop " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(indDataPoints(i) * (1 - topPairBias), "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    
                    varFlagFromNIAWG = False
                    
                    printnow "Now, sending variables!"
                    
                    messageToNIAWG1 = "bbot " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format((1 - indDataPoints(i)) * botPairBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "dbot " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format((1 - indDataPoints(i)) * (1 - botPairBias), "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    
                    
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "t " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(tunnelTime, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                End If
                
            Case "none":
                printnow " "
                printnow "--> THIS WILL BE A CONSTANT TUNNELING RUN!!"
                printnow "    No tunneling parameters varied (vary cooling)!"
                printnow "  "
                
                
                If goflag = True Then
                    varFlagFromNIAWG = False
                        
                    printnow "Now, sending variables!"
                        
                    messageToNIAWG1 = "b " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG1 = messageToNIAWG1 + CStr(Format(tunnelBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG1)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG2 = "d " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG2 = messageToNIAWG2 + CStr(Format(1 - tunnelBias, "0.000000")) + " "
                    Next i
                    messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG2)
                    
                    While varFlagFromNIAWG = False And goflag = True
                        wait (100)
                        DoEvents
                    Wend
                    varFlagFromNIAWG = False
                    
                    messageToNIAWG3 = "t " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                    For i = 0 To expRep - 1
                        messageToNIAWG3 = messageToNIAWG3 + CStr(Format(tunnelTime, "0.000000")) + " "
                    Next i
                    messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                    InputFrm.tcpserver.SendData (messageToNIAWG3)
                    
                End If

            
            Case Else
                If tunedParam = "piezoMirrorSpinExchange" Or tunedParam = "CarrFreqSpinExchange" Or tunedParam = "fieldRotatingBackSpinExchange" Or tunedParam = "RSFreqSpinExchange" Then
                    If goflag = True Then
                        varFlagFromNIAWG = False
                            
                        printnow "Now, sending variables!"
                            
                        messageToNIAWG1 = "tr " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                        For i = 0 To expRep - 1
                            messageToNIAWG1 = messageToNIAWG1 + CStr(Format(timeOfARP, "0.000000")) + " "
                        Next i
                        messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                        InputFrm.tcpserver.SendData (messageToNIAWG1)
                        
                        While varFlagFromNIAWG = False And goflag = True
                            wait (100)
                            DoEvents
                        Wend
                        varFlagFromNIAWG = False
                        
                        
                        messageToNIAWG1 = "th " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                        For i = 0 To expRep - 1
                            messageToNIAWG1 = messageToNIAWG1 + CStr(Format(holdAfterARP, "0.000000")) + " "
                            'messageToNIAWG1 = messageToNIAWG1 + CStr(Format(1, "0.000000")) + " "
                        Next i
                        messageToNIAWG1 = messageToNIAWG1 + "done!" + Chr(0)
                        InputFrm.tcpserver.SendData (messageToNIAWG1)
                        
                        While varFlagFromNIAWG = False And goflag = True
                            wait (100)
                            DoEvents
                        Wend
                        varFlagFromNIAWG = False
                        
                        
                        messageToNIAWG2 = "bi " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                        For i = 0 To expRep - 1
                            messageToNIAWG2 = messageToNIAWG2 + CStr(Format(ARPinitBias, "0.000000")) + " "
                        Next i
                        messageToNIAWG2 = messageToNIAWG2 + "done!" + Chr(0)
                        InputFrm.tcpserver.SendData (messageToNIAWG2)
                        
                        While varFlagFromNIAWG = False And goflag = True
                            wait (100)
                            DoEvents
                        Wend
                        varFlagFromNIAWG = False
                        
                        messageToNIAWG3 = "di " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                        For i = 0 To expRep - 1
                            messageToNIAWG3 = messageToNIAWG3 + CStr(Format(1 - (ARPinitBias), "0.000000")) + " "
                        Next i
                        messageToNIAWG3 = messageToNIAWG3 + "done!" + Chr(0)
                        InputFrm.tcpserver.SendData (messageToNIAWG3)
                        
                        While varFlagFromNIAWG = False And goflag = True
                            wait (100)
                            DoEvents
                        Wend
                        varFlagFromNIAWG = False
                        
                        messageToNIAWG4 = "bf " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                        For i = 0 To expRep - 1
                            messageToNIAWG4 = messageToNIAWG4 + CStr(Format(ARPfinalBias, "0.000000")) + " "
                        Next i
                        messageToNIAWG4 = messageToNIAWG4 + "done!" + Chr(0)
                        InputFrm.tcpserver.SendData (messageToNIAWG4)
                        
                        While varFlagFromNIAWG = False And goflag = True
                            wait (100)
                            DoEvents
                        Wend
                        varFlagFromNIAWG = False
                        
                        messageToNIAWG5 = "df " 'Expecting to mainly use this for varying the bias, hence naming the variable "b"
                        For i = 0 To expRep - 1
                            messageToNIAWG5 = messageToNIAWG5 + CStr(Format(1 - ARPfinalBias, "0.000000")) + " "
                        Next i
                        messageToNIAWG5 = messageToNIAWG5 + "done!" + Chr(0)
                        InputFrm.tcpserver.SendData (messageToNIAWG5)
                   End If
                Else
                    printnow "you crazy -> invalid message to NI AWG"
                End If
            End Select
        End If
End Sub
