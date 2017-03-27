Attribute VB_Name = "GPIB"

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  The file Gpib-32.dll must be in the systems folder.
'
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Option Base 0

'public Variables and Constants
Option Explicit

Public add(2) As Integer

Public ibsta As Integer
Public iberr As Integer
Public ibcnt As Integer
Public ibcntl As Long

Public Longibsta As Long
Public Longiberr As Long
Public Longibcnt As Long
Public GPIBglobalsRegistered As Integer

Public Const NLend = &H1              ' Send NL with EOI after a transfer
Global Const NOADDR = &HFFFF

' Error messages returned in public variable iberr

Public Const EDVR = 0      ' System error
Public Const ECIC = 1      ' Function requires GPIB board to be CIC
Public Const ENOL = 2      ' Write function detected no listeners
Public Const EADR = 3      ' Interface board not addressed correctly
Public Const EARG = 4      ' Invalid argument to function call
Public Const ESAC = 5      ' Function requires GPIB board to be SAC
Public Const EABO = 6      ' I/O operation aborted
Public Const ENEB = 7      ' Non-existent interface board
Public Const EDMA = 8      ' DMA Error
Public Const EOIP = 10     ' I/O operation started before previous
                           ' operation completed
Public Const ECAP = 11     ' No capability for intended operation
Public Const EFSO = 12     ' File system operation error
Public Const EBUS = 14     ' Command error during device call
Public Const ESTB = 15     ' Serial poll status byte lost
Public Const ESRQ = 16     ' SRQ remains asserted
Public Const ETAB = 20     ' The return buffer is full
Public Const ELCK = 21     ' Address or board is locked

'Functions

Declare Sub Send32 Lib "Gpib-32.dll" Alias "Send" (ByVal ud As Long, ByVal addr As Long, sstr As Any, ByVal cnt As Long, ByVal term As Long)
Declare Function RegisterGpibGlobalsForThread Lib "Gpib-32.dll" (Longibsta As Long, Longiberr As Long, Longibcnt As Long, ibcntl As Long) As Long
Declare Function UnregisterGpibGlobalsForThread Lib "Gpib-32.dll" () As Long
Declare Sub EnableRemote32 Lib "Gpib-32.dll" Alias "EnableRemote" (ByVal ud As Long, arg1 As Any)
Declare Function ibwrt32 Lib "Gpib-32.dll" Alias "ibwrt" (ByVal ud As Long, sstr As Any, ByVal cnt As Long) As Long
Declare Function ibrd32 Lib "Gpib-32.dll" Alias "ibrd" (ByVal ud As Long, sstr As Any, ByVal cnt As Long) As Long
Declare Function ibeot32 Lib "Gpib-32.dll" Alias "ibeot" (ByVal ud As Long, ByVal v As Long) As Long
Declare Function ibdev32 Lib "Gpib-32.dll" Alias "ibdev" (ByVal bdid As Long, ByVal pad As Long, ByVal sad As Long, ByVal tmo As Long, ByVal eot As Long, ByVal eos As Long) As Long

Sub copy_ibvars()
    ibsta = ConvertLongToInt(Longibsta)
    iberr = CInt(Longiberr)
    ibcnt = ConvertLongToInt(ibcntl)
End Sub
Sub ibwrt(ByVal ud As Integer, ByVal buf As String)
    Dim cnt As Long
    
' Check to see if GPIB Global variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If

    cnt = CLng(Len(buf))
    
' Call the 32-bit DLL.
    Call ibwrt32(ud, ByVal buf, cnt)
    
    Call copy_ibvars
End Sub
Sub ibwrti(ByVal ud As Integer, ByRef ibuf() As Integer, ByVal cnt As Long)
' Check to see if GPIB Global variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If

' Call the 32-bit DLL.
    Call ibwrt32(ud, ibuf(0), cnt)

    Call copy_ibvars
End Sub
Sub ibrd(ByVal ud As Integer, buf As String)
    Dim cnt As Long
    
' Check to see if GPIB Global variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If

    cnt = CLng(Len(buf))
    
' Call the 32-bit DLL.
    Call ibrd32(ud, ByVal buf, cnt)
    
    Call copy_ibvars
End Sub
Sub ibeot(ByVal ud As Integer, ByVal v As Integer)
' Check to see if GPIB Global variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If

' Call the 32-bit DLL.
    Call ibeot32(ud, v)
    
    Call copy_ibvars
End Sub
'This Function Controls the GPIB
Sub Send(ByVal addr As Integer, ByVal buf As String)
    
    Dim cnt As Long
    
' Check to see if GPIB public variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If
    
    cnt = CLng(Len(buf))
    
' Call the 32-bit DLL.
    Call Send32(0, addr, ByVal buf, cnt, NLend)

    Call copy_ibvars
End Sub


Sub EnableRemote(ByVal ud As Integer, addrs() As Integer)
' Check to see if GPIB Global variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If

' Call the 32-bit DLL.
    Call EnableRemote32(ud, addrs(0))

    Call copy_ibvars
End Sub

Private Function ConvertLongToInt(LongNumb As Long) As Integer
  
  If (LongNumb And &H8000&) = 0 Then
      ConvertLongToInt = LongNumb And &HFFFF&
  Else
    ConvertLongToInt = &H8000 Or (LongNumb And &H7FFF&)
  End If
    
End Function

Public Sub RegisterGPIBGlobals()
    Dim rc As Long
    
    rc = RegisterGpibGlobalsForThread(Longibsta, Longiberr, Longibcnt, ibcntl)
    If (rc = 0) Then
      GPIBglobalsRegistered = 1
    ElseIf (rc = 1) Then
      rc = UnregisterGpibGlobalsForThread
      rc = RegisterGpibGlobalsForThread(Longibsta, Longiberr, Longibcnt, ibcntl)
      GPIBglobalsRegistered = 1
    ElseIf (rc = 2) Then
      rc = UnregisterGpibGlobalsForThread
      ibsta = &H8000
      iberr = EDVR
      ibcntl = &HDEAD37F0
    ElseIf (rc = 3) Then
      rc = UnregisterGpibGlobalsForThread
      ibsta = &H8000
      iberr = EDVR
      ibcntl = &HDEAD37F0
    Else
      ibsta = &H8000
      iberr = EDVR
      ibcntl = &HDEAD37F0
    End If
End Sub

Public Sub UnregisterGPIBGlobals()
    Dim rc As Long
    
    rc = UnregisterGpibGlobalsForThread
    GPIBglobalsRegistered = 0
    
End Sub

Function ildev(ByVal bdid As Integer, ByVal pad As Integer, ByVal sad As Integer, ByVal tmo As Integer, ByVal eot As Integer, ByVal eos As Integer) As Integer
' Check to see if GPIB Global variables are registered
    If (GPIBglobalsRegistered = 0) Then
      Call RegisterGPIBGlobals
    End If
    
' Call the 32-bit DLL.
    ildev = ConvertLongToInt(ibdev32(bdid, pad, sad, tmo, eot, eos))
    
    Call copy_ibvars
End Function

