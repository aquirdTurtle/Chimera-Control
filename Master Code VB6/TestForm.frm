VERSION 5.00
Begin VB.Form TestFrm 
   Caption         =   "Tests"
   ClientHeight    =   3615
   ClientLeft      =   255
   ClientTop       =   5850
   ClientWidth     =   7050
   LinkTopic       =   "Form1"
   ScaleHeight     =   3615
   ScaleMode       =   0  'User
   ScaleWidth      =   7050
   Begin VB.TextBox Text5 
      Height          =   375
      Left            =   5400
      TabIndex        =   12
      Top             =   3120
      Width           =   1455
   End
   Begin VB.CommandButton cmdShowDACmatrix 
      Caption         =   "ShowDACMatrix"
      Height          =   375
      Left            =   5520
      TabIndex        =   11
      Top             =   2160
      Width           =   1335
   End
   Begin VB.CommandButton cmdShowTTLmatrix 
      Caption         =   "ShowTTLMatrix"
      Height          =   375
      Left            =   4080
      TabIndex        =   10
      Top             =   2160
      Width           =   1335
   End
   Begin VB.TextBox Text4 
      Height          =   2055
      Left            =   4080
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   9
      Top             =   120
      Width           =   2775
   End
   Begin VB.CommandButton cmdTest 
      Caption         =   "Test"
      Height          =   375
      Left            =   6000
      TabIndex        =   8
      Top             =   2640
      Width           =   855
   End
   Begin VB.CommandButton cmdRound 
      Caption         =   "Round"
      Height          =   255
      Left            =   3360
      TabIndex        =   7
      Top             =   3240
      Width           =   1815
   End
   Begin VB.TextBox Text3 
      Enabled         =   0   'False
      Height          =   285
      Index           =   1
      Left            =   1680
      TabIndex        =   6
      Text            =   "0"
      Top             =   3240
      Width           =   1455
   End
   Begin VB.TextBox Text3 
      Height          =   285
      Index           =   0
      Left            =   0
      TabIndex        =   5
      Text            =   "0"
      Top             =   3240
      Width           =   1455
   End
   Begin VB.CommandButton cmdConverttounsigned 
      Caption         =   "Converttounsigned"
      Height          =   255
      Left            =   3360
      TabIndex        =   4
      Top             =   2760
      Width           =   1815
   End
   Begin VB.TextBox Text2 
      Enabled         =   0   'False
      Height          =   285
      Index           =   1
      Left            =   1680
      Locked          =   -1  'True
      TabIndex        =   3
      Text            =   "0"
      Top             =   2760
      Width           =   1455
   End
   Begin VB.TextBox Text2 
      Height          =   285
      Index           =   0
      Left            =   0
      TabIndex        =   2
      Text            =   "0"
      Top             =   2760
      Width           =   1455
   End
   Begin VB.CheckBox chkShowTTLdatatoboard 
      Caption         =   "ShowTTLdatatoboard"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   2280
      Value           =   1  'Checked
      Width           =   2295
   End
   Begin VB.TextBox Text1 
      Height          =   2175
      Left            =   120
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   0
      Top             =   120
      Width           =   3855
   End
End
Attribute VB_Name = "TestFrm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Option Base 0
Public Sub TestTTLdatatoboard()
    Dim i As Integer
    Dim j As Integer
    Dim time As Double
    
    Text1.Text = ""
    For i = 0 To TTLbuffsize - 1
        time = converttounsigned(TTLdatatoboard(0, i)) + converttounsigned(TTLdatatoboard(1, i)) * 65535
        time = time / 1000
        Text1.Text = Text1.Text + CStr(time)
        'Debug.Print "time ", time
        For j = 2 To 5
            Text1.Text = Text1.Text & vbTab & CStr(converttounsigned(TTLdatatoboard(j, i)))
        Next j
        Text1.Text = Text1.Text + vbCrLf
    Next i
End Sub



Private Sub cmdConverttounsigned_Click()
    Dim val As Long
    Dim dummy As String
    
    'check that text2(0).text is a number, if not exit sub
    If Not (Text2(0).Text = "") Then
    For i = 1 To Len(Text2(0).Text)
        dummy = Mid(Text2(0).Text, i, 1)
        If dummy = "0" Or dummy = "1" Or dummy = "2" Or dummy = "3" Or dummy = "4" Or dummy = "5" Or _
           dummy = "6" Or dummy = "7" Or dummy = "8" Or dummy = "9" Or dummy = "." Or dummy = "-" Then
        Else: Exit Sub
        End If
    Next i

    'check that val is an integer
    val = CLng(Text2(0).Text)
    If val > 32767 Or val < -32768 Then
        Text2(1).Text = "not an integer"
    Else
        Text2(1).Text = CStr(converttounsigned(CInt(Text2(0).Text)))
    End If
    End If
    
End Sub

Private Sub cmdRound_Click()
    Text3(1).Text = CStr(round(CDbl(Text3(0).Text)))
End Sub

Private Sub cmdShowDACmatrix_Click()
    Dim i As Integer
    Dim j As Integer
    
    Text4.Text = ""
    For i = 0 To DACcounter - 1
        For j = 0 To 2
            If Not j = 0 Then Text4.Text = Text4.Text + vbTab
            Text4.Text = Text4.Text + CStr(DACmatrix(j, i))
        Next j
        Text4.Text = Text4.Text + vbCrLf
    Next i
End Sub

Private Sub cmdShowTTLmatrix_Click()
    Dim i As Integer
    Dim j As Integer
    
    Text4.Text = 0
    For i = 0 To TTLcounter - 1
        For j = 0 To 2
            If Not j = 0 Then Text4.Text = Text4.Text + vbTab
            Text4.Text = Text4.Text + CStr(TTLmatrix(j, i))
        Next j
        Text4.Text = Text4.Text + vbCrLf
    Next i
End Sub

Private Sub cmdTest_Click()
    'Call SortArray(TTLmatrix, TTLcounter)
    'Call cmdShowTTLmatrix_Click
    
    'Call SortArray(DACmatrix, DACcounter)
    'Call cmdShowDACmatrix_Click
        
    TestFrm.Text5.Text = "type"
    TestFrm.Text5.SetFocus
    
End Sub


Private Sub Text5_KeyPress(KeyAscii As Integer)
    Dim a As String
    
    a = Chr(KeyAscii)
    Debug.Print "a", a
End Sub
