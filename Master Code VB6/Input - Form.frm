VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form InputFrm 
   Caption         =   "Main"
   ClientHeight    =   10665
   ClientLeft      =   315
   ClientTop       =   1635
   ClientWidth     =   7665
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   9.75
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   KeyPreview      =   -1  'True
   LinkTopic       =   "Form1"
   ScaleHeight     =   10665
   ScaleWidth      =   7665
   Begin MSWinsockLib.Winsock tcpserver 
      Left            =   1200
      Top             =   9600
      _ExtentX        =   741
      _ExtentY        =   741
      _Version        =   393216
      LocalPort       =   10010
   End
   Begin VB.CheckBox chkAbort 
      Caption         =   "Abort"
      Height          =   375
      Left            =   4320
      Style           =   1  'Graphical
      TabIndex        =   5
      Top             =   9600
      Width           =   855
   End
   Begin VB.CommandButton cmdPause 
      Caption         =   "Pause"
      Height          =   375
      Left            =   4320
      TabIndex        =   4
      Top             =   10080
      Width           =   855
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "Exit"
      Height          =   375
      Left            =   3240
      TabIndex        =   3
      Top             =   10080
      Width           =   855
   End
   Begin VB.CommandButton cmdReset 
      Caption         =   "Reset"
      Height          =   375
      Left            =   3240
      TabIndex        =   2
      Top             =   9600
      Width           =   855
   End
   Begin VB.CommandButton cmdGetInput 
      Caption         =   "GetInput"
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   9600
      Width           =   855
   End
   Begin VB.TextBox txtMain 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   9495
      Left            =   120
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   0
      Top             =   0
      Width           =   5175
   End
   Begin VB.Label Label1 
      Caption         =   "Label1"
      Height          =   10335
      Left            =   5400
      TabIndex        =   6
      Top             =   0
      Width           =   1695
   End
   Begin VB.Menu View 
      Caption         =   "View"
      Begin VB.Menu menuPlots 
         Caption         =   "Plots"
      End
      Begin VB.Menu menuTests 
         Caption         =   "Tests"
      End
   End
End
Attribute VB_Name = "InputFrm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Option Base 0
Dim adjustparam As Boolean
Dim entervalue As Boolean
Dim paramindex As Integer
Dim keyshift As Boolean
Dim B As String



Private Sub chkAbort_Click()
    Call abortclick
End Sub

Private Sub cmdExit_Click()
    Call exitprog
End Sub

Private Sub cmdGetInput_Click()
    Call getinput
End Sub

Private Sub cmdPause_Click()
    Call pause
End Sub

Private Sub cmdReset_Click()
    Call reset
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    key = KeyCode
    If adjustparam = True Then
        Call paramadjust(paramindex)
    ElseIf entervalue = True Then
        Call valueenter(paramindex)
    ElseIf keyshift = True Then
        Call capital
    ElseIf inputflag = 1 Then
        Call inputvalue
    Else
        Call getinput
    End If
    
End Sub

Private Sub Form_Load()
    Dim i As Integer
    
    tcpserver.Protocol = sckTCPProtocol
    tcpserver.LocalPort = 10010
    
    Call reset
    
    'Call MOT  'not necessary, since reset calls MOT
    
    adjustparam = False
    entervalue = False
    paramindex = 0
    B = ""

End Sub

Private Sub tcpserver_ConnectionRequest(ByVal requestID As Long)
    ' Check if the control's state is closed. If not, close the connection before accepting the new connection.
    If tcpserver.State <> sckClosed Then tcpserver.Close
    ' Accept the request with the requestID parameter.
    tcpserver.Accept (requestID)
End Sub


Private Sub tcpserver_DataArrival(ByVal bytesTotal As Long)
    Dim strData As String
    tcpserver.GetData strData
    
    If strData = "go" Then
        flagFromNIAWG = True
    ElseIf strData = "Accumulations?" Then
        sendAccumsFlag = True
    ElseIf strData = "next variable" Then
        varFlagFromNIAWG = True
    Else
        commFromNIAWG = strData
    End If
    
    printnow CStr("received: " + CStr(strData))
    
End Sub


Private Sub menuPlots_Click()
    PlotFrm.Show
End Sub

Private Sub menuTests_Click()
    TestFrm.Show
End Sub


Private Sub txtMain_KeyDown(KeyCode As Integer, Shift As Integer)
    key = KeyCode
End Sub

Public Sub getinput()

    Dim a As String
    Dim B As String
    Dim i As Integer
    Dim j As Integer
    
    adjustparam = False
    
    'Debug.Print "key", key
    If key = 0 Then Exit Sub
    'Call clearscreen(0)
    Call paramfix("read")

    If key < 65 Or key > 90 Then  'not a letter
        Select Case key  'for function keys
            Case vbKeyF1:    'function key F1
                routine = "O"
                printnow "mot routine requested"
                InputFrm.txtMain.BackColor = vbYellow
                'Call runroutine
            Case vbKeyF2:    'function key F2
                routine = "M"
                printnow "DAC timing test"
                'Call runroutine
            Case vbKeyF3:    'function key F3
                routine = "H"
                printnow "load and Image"
                'Call runroutine
            Case vbKeyF4:    'function key F4
                routine = "R"
                printnow "Single atoms requested cw"
                'Call runroutine
            Case vbKeyF5:    'function key F5
                routine = "G"
                printnow "Main experiment requested"
                'Call runroutine
            Case vbKeyF7:    'function key F7
                routine = "C"
                printnow "Camera test requested"
                'Call runroutine
            Case vbKeyF8:    'function key F8
                routine = "P"
                printnow "MOT fill requested"
                'Call runroutine
            Case vbKeyF9:    'function key F9
                routine = "A"
                printnow "Test Andor"
                'Call runroutine
            Case vbKeyF10:    'function key F10
                routine = "L"
                printnow "routine hitF10"
                'Call runroutine
            Case vbKeyF11:    'function key F11
                routine = "B"
                printnow "routine test"
                'Call runroutine
            Case vbKeyF12:    'function key F12
                routine = "D"
                printnow "routine digital DAC"
                'Call runroutine
            Case vbKeyUp:    'up key
                printnow "increment key out of order"
            Case vbKeyDown:    'down key
                printnow "decrement key out of order"
            Case 187:  ' "+"
                printnow "Ready to increment mirror upwards."
                routine = "SU"
            Case 189: ' "_"
                printnow "Ready to increment mirror downwards."
                routine = "SD"
            Case vbKeyShift:
                keyshift = True
                printnow "shift key "
        End Select
        
    ElseIf key > 64 And key < 91 Then ' a lower case letter
        paramindex = 0
       
        For i = 1 To noparams
            If plist(i).letter = Chr$(key) Then
                paramindex = i
                printnow plist(paramindex).label + " present value: " + CStr(plist(paramindex).VALUE)
                adjustparam = True
            End If
        Next i
        If paramindex = 0 Then
            printnow "unknown parameter; hit enter"
        End If
        
    End If
    key = 0
 End Sub
Public Sub inputvalue()
Dim a As String
    
    If key = vbKeyReturn Then
        inputvariable = CDbl(B)
        inputflag = 2
        B = ""
      '  Call inputnow("", a)
    Else
        a = Chr$(key)
       'Debug.Print "key", key, a
        If a = "1" Or a = "2" Or a = "3" Or a = "4" Or a = "5" Or a = "6" Or a = "7" Or a = "8" _
          Or a = "9" Or a = "0" Or a = " " Or a = "-" Or a = "+" Or a = "E" Or a = "." Or a = "e" Then
            
            B = B + Chr$(key)
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + a
        ElseIf key = 189 Then
            B = B + "-"
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + "-"
        ElseIf key = 187 Then
            B = B + "+"
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + "+"
        End If
    End If
    key = 0
    Call runroutine

End Sub
Public Sub capital()
     
     'Debug.Print "capital key", key
     clearscreen (0)
     Select Case key
            Case vbKeyG:
                routine = "g"
                printnow " routine antigrav"
            Case vbKeyX:
                routine = "x"
                printnow " routine qdirection"
            Case vbKeyF:
                routine = "F"
                printnow " routine F unused"
            Case vbKeyC:
                routine = "L"
                printnow " routine hitF10"
            Case vbKeyO:
                 routine = "E"  'E for efficiency
                 printnow " routine E unused"
            Case vbKeyQ:
                routine = "Q"
                printnow " quit program"
            Case vbKeyN:
    End Select
   
    keyshift = False
    key = 0
    Call runroutine
End Sub
Public Sub paramadjust(j As Integer)
    InputFrm.txtMain.Text = InputFrm.txtMain.Text + vbCrLf + plist(j).label
        
    If key = vbKeyReturn Then           'enter-key as end of input
        adjustparam = False
        Call paramfix("write")
        clearscreen (0)
        printnow plist(j).label + "set to " + CStr(plist(j).VALUE)
        Exit Sub
    End If
    If key = 191 Then  ' "/"
        adjustparam = False
        entervalue = True
        Exit Sub
    Else
        Call clearscreen(1)
        printnow vbCrLf
        Select Case key
            Case vbKeyUp:    'up key
                plist(j).VALUE = plist(j).VALUE + plist(j).inc
                printnow " increment " + CStr(plist(j).inc) + " to " + CStr(plist(j).VALUE)
            Case vbKeyDown:    'down key
                plist(j).VALUE = plist(j).VALUE - plist(j).inc
                printnow " decrement " + CStr(plist(j).inc) + " to " + CStr(plist(j).VALUE)
            Case vbKeyPageUp  'page up key
                plist(j).VALUE = plist(j).VALUE + plist(j).biginc
                printnow " big inc " + CStr(plist(j).inc) + " to " + CStr(plist(j).VALUE)
            Case vbKeyPageDown   'page down key
                plist(j).VALUE = plist(j).VALUE - plist(j).biginc
                printnow " big dec " + CStr(plist(j).inc) + " to " + CStr(plist(j).VALUE)
        End Select
        
        If plist(j).VALUE > plist(j).max Then
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + " set to max value " + CStr(plist(j).max)
            plist(j).VALUE = plist(j).max
        End If
        If plist(j).VALUE < plist(j).min Then
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + " set to min value " + CStr(plist(j).min)
            plist(j).VALUE = plist(j).min
        End If
        Call paramfix("write")
    End If
    key = 0
End Sub
Sub valueenter(j As Integer)
    Dim a As String
       
    If key = vbKeyReturn And Len(B) > 0 Then
        
        
        plist(j).VALUE = CSng(B)
        InputFrm.txtMain.Text = InputFrm.txtMain.Text + vbCrLf + "Set to " + CStr(plist(j).VALUE)
        
        If plist(j).VALUE > plist(j).max Then
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + " set to max value " + CStr(plist(j).max)
            plist(j).VALUE = plist(j).max
        End If
        If plist(j).VALUE < plist(j).min Then
            InputFrm.txtMain.Text = InputFrm.txtMain.Text + " set to min value " + CStr(plist(j).min)
            plist(j).VALUE = plist(j).min
        End If
        Call paramfix("write")
        entervalue = False
        B = ""
    Else
        
        If key > 95 And key < 106 Then key = key - 48 'This is to take keypad numbers to "real" numbers.
        
        a = Chr$(key)
        If key = 190 Or key = 110 Then
            a = "."
            key = 46
        End If
        Debug.Print "key", key, a
        If a = "1" Or a = "2" Or a = "3" Or a = "4" Or a = "5" Or a = "6" Or a = "7" Or a = "8" _
          Or a = "9" Or a = "0" Or a = " " Or a = "-" Or a = "+" Or a = "E" Or a = "." Or a = "e" Then
            B = B + Chr$(key)
            InputFrm.txtMain.Text = InputFrm.txtMain.Text & a
            InputFrm.txtMain.SelStart = Len(InputFrm.txtMain.Text)
        ElseIf key = 189 Then
            B = B + "-"
            InputFrm.txtMain.Text = InputFrm.txtMain.Text & a
            InputFrm.txtMain.SelStart = Len(InputFrm.txtMain.Text)
        ElseIf key = 187 Then
            B = B + "+"
            InputFrm.txtMain.Text = InputFrm.txtMain.Text & a
            InputFrm.txtMain.SelStart = Len(InputFrm.txtMain.Text)
        ElseIf key = vbKeyBack Then
            If Len(B) > 0 Then B = Left(B, Len(B) - 1)
            InputFrm.txtMain.Text = Left(InputFrm.txtMain.Text, Len(InputFrm.txtMain.Text) - 1)
        End If
    End If
    key = 0
End Sub
Public Sub paramfix(iflag As String)
    Dim i As Integer
    
   i = 1 ' Experiment Repitions
        If iflag = "init" Then
           plist(i).label = "expRep": plist(i).letter = "R"
           plist(i).VALUE = 1: plist(i).inc = 1: plist(i).biginc = 10
           plist(i).max = 10000: plist(i).min = 1
        End If
        If iflag = "read" Then plist(i).VALUE = expRep
        If iflag = "write" Then expRep = plist(i).VALUE
   
    i = 2 'Polarization Gradient cooling flag, 0 false, 1 true
        If iflag = "init" Then
           plist(i).label = "PGCFlag": plist(i).letter = "P"
           plist(i).VALUE = 1: plist(i).inc = 1: plist(i).biginc = 1
           plist(i).max = 1: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = PGCFlag
        If iflag = "write" Then PGCFlag = plist(i).VALUE
   
    i = 3  'moltime
        If iflag = "init" Then
           plist(i).label = "Raman Cool": plist(i).letter = "C"
           plist(i).VALUE = 0: plist(i).inc = 0.333: plist(i).biginc = 1
           plist(i).max = 500: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = ramanTime
        If iflag = "write" Then ramanTime = plist(i).VALUE
   
   i = 4 'microwave freq
        If iflag = "init" Then
           plist(i).label = "mwPulseLength": plist(i).letter = "M"
           plist(i).VALUE = 0: plist(i).inc = 0.01: plist(i).biginc = 0.1
           plist(i).max = 1000: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = mwPulseLength
        If iflag = "write" Then mwPulseLength = plist(i).VALUE

   i = 5 'Flourescence imaging light frequency
        If iflag = "init" Then
           plist(i).label = "Probe frequency": plist(i).letter = "I"
           plist(i).VALUE = 0#: plist(i).inc = 0.01: plist(i).biginc = 0.1
           plist(i).max = 1: plist(i).min = -5
        End If
        If iflag = "read" Then plist(i).VALUE = flourFrequency
        If iflag = "write" Then flourFrequency = plist(i).VALUE
  
   i = 6 'droptime
        If iflag = "init" Then
           plist(i).label = "droptime": plist(i).letter = "D"
           plist(i).VALUE = 0: plist(i).inc = 0.333: plist(i).biginc = 2
           plist(i).max = 1000: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = droptime
        If iflag = "write" Then droptime = plist(i).VALUE
   
   i = 7 'Automate flag
        If iflag = "init" Then
           plist(i).label = "Automate": plist(i).letter = "A"
           plist(i).VALUE = 0: plist(i).inc = 1: plist(i).biginc = 10
           plist(i).max = 1: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = automateFlag
        If iflag = "write" Then automateFlag = plist(i).VALUE
   
   i = 8 'Bz
        If iflag = "init" Then
           plist(i).label = "image F=1": plist(i).letter = "Z"
           plist(i).VALUE = 0: plist(i).inc = 0.01: plist(i).biginc = 0.2
           plist(i).max = 1: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = imageLower
        If iflag = "write" Then imageLower = plist(i).VALUE
   
   i = 9 'By
        If iflag = "init" Then
           plist(i).label = "MicroFreq": plist(i).letter = "Y"
           plist(i).VALUE = 6.8: plist(i).inc = 0.000001: plist(i).biginc = 0.000001
           plist(i).max = 8: plist(i).min = 1
        End If
        If iflag = "read" Then plist(i).VALUE = RSFreq
        If iflag = "write" Then RSFreq = plist(i).VALUE
   
   i = 10 'Bx
        If iflag = "init" Then
           plist(i).label = "Bx": plist(i).letter = "X"
           plist(i).VALUE = 0: plist(i).inc = 1: plist(i).biginc = 1
           plist(i).max = 5: plist(i).min = -5
        End If
        If iflag = "read" Then plist(i).VALUE = Bx
        If iflag = "write" Then Bx = plist(i).VALUE
  
 
   i = 12 'Populate F=1
        If iflag = "init" Then
           plist(i).label = "Pop. F=1": plist(i).letter = "F"
           plist(i).VALUE = 0!: plist(i).inc = 0.1: plist(i).biginc = 1
           plist(i).max = 1!: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = popLower
        If iflag = "write" Then popLower = plist(i).VALUE
 
    i = 14 'optime
        If iflag = "init" Then
           plist(i).label = "Accumulations": plist(i).letter = "O"
           plist(i).VALUE = 1: plist(i).inc = 1: plist(i).biginc = 5
           plist(i).max = 100000: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = accumulations
        If iflag = "write" Then accumulations = plist(i).VALUE

    i = 15 'lastrf value
        If iflag = "init" Then
           plist(i).label = "Lattice Holdtime": plist(i).letter = "V"
           plist(i).VALUE = 0: plist(i).inc = 0.01: plist(i).biginc = 0.1
           plist(i).max = 200: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = LatticeHoldTime
        If iflag = "write" Then LatticeHoldTime = plist(i).VALUE

    i = 16 'Blow away
        If iflag = "init" Then
           plist(i).label = "Kill F=2 ": plist(i).letter = "B"
           plist(i).VALUE = 0: plist(i).inc = 0.002: plist(i).biginc = 10
           plist(i).max = 1: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = blowAway
        If iflag = "write" Then blowAway = plist(i).VALUE

    i = 17 'latticeFlag
        If iflag = "init" Then
           plist(i).label = "Lattice Flag": plist(i).letter = "L"
           plist(i).VALUE = 0: plist(i).inc = 0.333: plist(i).biginc = 5
           plist(i).max = 1: plist(i).min = 0
        End If
        If iflag = "read" Then plist(i).VALUE = latticeFlag
        If iflag = "write" Then latticeFlag = plist(i).VALUE
        
    i = 18 'Free value for use in test code
        If iflag = "init" Then
           plist(i).label = "Free Value": plist(i).letter = "K"
           plist(i).VALUE = 0: plist(i).inc = 0.002: plist(i).biginc = 10
           plist(i).max = 1000000: plist(i).min = -10
        End If
        If iflag = "read" Then plist(i).VALUE = FreeValue
        If iflag = "write" Then FreeValue = plist(i).VALUE
        
        
    noparams = i

End Sub
