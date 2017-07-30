VERSION 5.00
Begin VB.Form PlotFrm 
   Caption         =   "Plots"
   ClientHeight    =   8865
   ClientLeft      =   7755
   ClientTop       =   840
   ClientWidth     =   13020
   LinkTopic       =   "Form1"
   ScaleHeight     =   8865
   ScaleWidth      =   13020
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   23
      Left            =   10800
      TabIndex        =   220
      Text            =   "0"
      Top             =   1800
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   22
      Left            =   10800
      TabIndex        =   219
      Text            =   "0"
      Top             =   1560
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   21
      Left            =   10800
      TabIndex        =   218
      Text            =   "0"
      Top             =   1320
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   20
      Left            =   10800
      TabIndex        =   217
      Text            =   "0"
      Top             =   1080
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   19
      Left            =   10800
      TabIndex        =   216
      Text            =   "0"
      Top             =   840
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   18
      Left            =   10800
      TabIndex        =   215
      Text            =   "0"
      Top             =   600
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   17
      Left            =   10800
      TabIndex        =   214
      Text            =   "0"
      Top             =   360
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   16
      Left            =   10800
      TabIndex        =   213
      Text            =   "0"
      Top             =   120
      Width           =   975
   End
   Begin VB.CommandButton cmdForceOut 
      Caption         =   "ForceOut"
      Height          =   375
      Index           =   1
      Left            =   11880
      TabIndex        =   212
      Top             =   600
      Width           =   1095
   End
   Begin VB.CommandButton cmdShowDACs 
      Caption         =   "ShowDACs"
      Height          =   375
      Left            =   11880
      TabIndex        =   211
      Top             =   120
      Width           =   1095
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   195
      Index           =   0
      Left            =   1080
      Style           =   1  'Graphical
      TabIndex        =   210
      Top             =   840
      Width           =   200
   End
   Begin VB.CommandButton cmdTTLdefault 
      Caption         =   "TTLdefault"
      Height          =   375
      Left            =   2400
      TabIndex        =   185
      Top             =   120
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   15
      Left            =   8880
      TabIndex        =   182
      Text            =   "0"
      Top             =   1800
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   14
      Left            =   8880
      TabIndex        =   181
      Text            =   "0"
      Top             =   1560
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   13
      Left            =   8880
      TabIndex        =   180
      Text            =   "0"
      Top             =   1320
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   12
      Left            =   8880
      TabIndex        =   179
      Text            =   "0"
      Top             =   1080
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   11
      Left            =   8880
      TabIndex        =   178
      Text            =   "0"
      Top             =   840
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   10
      Left            =   8880
      TabIndex        =   177
      Text            =   "0"
      Top             =   600
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   9
      Left            =   8880
      TabIndex        =   176
      Text            =   "0"
      Top             =   360
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   8
      Left            =   8880
      TabIndex        =   175
      Text            =   "0"
      Top             =   120
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   7
      Left            =   6840
      TabIndex        =   172
      Text            =   "0"
      Top             =   1800
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   6
      Left            =   6840
      TabIndex        =   171
      Text            =   "0"
      Top             =   1560
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   5
      Left            =   6840
      TabIndex        =   170
      Text            =   "0"
      Top             =   1320
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   4
      Left            =   6840
      TabIndex        =   169
      Text            =   "0"
      Top             =   1080
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   3
      Left            =   6840
      TabIndex        =   168
      Text            =   "0"
      Top             =   840
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   2
      Left            =   6840
      TabIndex        =   167
      Text            =   "0"
      Top             =   600
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   1
      Left            =   6840
      TabIndex        =   166
      Text            =   "0"
      Top             =   360
      Width           =   975
   End
   Begin VB.TextBox txtDAC 
      Height          =   285
      Index           =   0
      Left            =   6840
      TabIndex        =   165
      Text            =   "0"
      Top             =   120
      Width           =   975
   End
   Begin VB.CommandButton cmdClear 
      Caption         =   "Clear"
      Height          =   375
      Index           =   1
      Left            =   7920
      TabIndex        =   164
      Top             =   8280
      Width           =   735
   End
   Begin VB.CommandButton cmdReplot 
      Caption         =   "Replot"
      Height          =   375
      Index           =   1
      Left            =   9240
      TabIndex        =   163
      Top             =   7320
      Width           =   735
   End
   Begin VB.TextBox txtDACtmin 
      Height          =   285
      Index           =   3
      Left            =   10080
      TabIndex        =   160
      Text            =   "0"
      Top             =   6960
      Width           =   855
   End
   Begin VB.TextBox txtDACtmin 
      Height          =   285
      Index           =   2
      Left            =   8760
      TabIndex        =   159
      Text            =   "0"
      Top             =   6960
      Width           =   855
   End
   Begin VB.TextBox txtDACtmin 
      Height          =   285
      Index           =   1
      Left            =   7320
      TabIndex        =   156
      Text            =   "0"
      Top             =   6960
      Width           =   855
   End
   Begin VB.TextBox txtDACtmin 
      Height          =   285
      Index           =   0
      Left            =   6000
      TabIndex        =   155
      Text            =   "0"
      Top             =   6960
      Width           =   855
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   15
      Left            =   7560
      TabIndex        =   153
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   14
      Left            =   7320
      TabIndex        =   152
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   13
      Left            =   7080
      TabIndex        =   151
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   12
      Left            =   6840
      TabIndex        =   150
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   11
      Left            =   6480
      TabIndex        =   149
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   10
      Left            =   6240
      TabIndex        =   148
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   9
      Left            =   6000
      TabIndex        =   147
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   8
      Left            =   5760
      TabIndex        =   146
      Top             =   8040
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   7
      Left            =   7560
      TabIndex        =   145
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   6
      Left            =   7320
      TabIndex        =   144
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   5
      Left            =   7080
      TabIndex        =   143
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   4
      Left            =   6840
      TabIndex        =   142
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   3
      Left            =   6480
      TabIndex        =   141
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   2
      Left            =   6240
      TabIndex        =   140
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   1
      Left            =   6000
      TabIndex        =   139
      Top             =   7800
      Width           =   200
   End
   Begin VB.CheckBox ChkDACselect 
      Caption         =   "Check1"
      Height          =   200
      Index           =   0
      Left            =   5760
      TabIndex        =   138
      Top             =   7800
      Width           =   200
   End
   Begin VB.PictureBox Picture2 
      Height          =   4695
      Left            =   5640
      ScaleHeight     =   4635
      ScaleWidth      =   5355
      TabIndex        =   137
      Top             =   2160
      Width           =   5415
   End
   Begin VB.TextBox txtTTLtmin 
      Height          =   285
      Index           =   1
      Left            =   1920
      TabIndex        =   135
      Text            =   "0"
      Top             =   6960
      Width           =   855
   End
   Begin VB.TextBox txtTTLtmin 
      Height          =   285
      Index           =   0
      Left            =   480
      TabIndex        =   133
      Text            =   "0"
      Top             =   6960
      Width           =   855
   End
   Begin VB.CommandButton cmdReplot 
      Caption         =   "Replot"
      Height          =   375
      Index           =   0
      Left            =   3000
      TabIndex        =   132
      Top             =   6960
      Width           =   735
   End
   Begin VB.CommandButton cmdShowTTLs 
      Caption         =   "ShowTTLs"
      Height          =   375
      Left            =   1200
      TabIndex        =   131
      Top             =   120
      Width           =   975
   End
   Begin VB.PictureBox Picture1 
      Height          =   4695
      Left            =   120
      ScaleHeight     =   4635
      ScaleWidth      =   5235
      TabIndex        =   130
      Top             =   2160
      Width           =   5295
   End
   Begin VB.CommandButton cmdClear 
      Caption         =   "Clear"
      Height          =   375
      Index           =   0
      Left            =   4560
      TabIndex        =   129
      Top             =   8280
      Width           =   735
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   63
      Left            =   4200
      TabIndex        =   127
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   62
      Left            =   3960
      TabIndex        =   126
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   61
      Left            =   3720
      TabIndex        =   125
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   60
      Left            =   3480
      TabIndex        =   124
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   59
      Left            =   3240
      TabIndex        =   123
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   58
      Left            =   2880
      TabIndex        =   122
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   57
      Left            =   2640
      TabIndex        =   121
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   56
      Left            =   2400
      TabIndex        =   120
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   55
      Left            =   2160
      TabIndex        =   119
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   54
      Left            =   1920
      TabIndex        =   118
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   53
      Left            =   1560
      TabIndex        =   117
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   52
      Left            =   1320
      TabIndex        =   116
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   51
      Left            =   1080
      TabIndex        =   115
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   50
      Left            =   840
      TabIndex        =   114
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   49
      Left            =   600
      TabIndex        =   113
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   48
      Left            =   240
      TabIndex        =   112
      Top             =   8280
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   47
      Left            =   4200
      TabIndex        =   111
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   46
      Left            =   3960
      TabIndex        =   110
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   45
      Left            =   3720
      TabIndex        =   109
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   44
      Left            =   3480
      TabIndex        =   108
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   43
      Left            =   3240
      TabIndex        =   107
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   42
      Left            =   2880
      TabIndex        =   106
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   41
      Left            =   2640
      TabIndex        =   105
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   40
      Left            =   2400
      TabIndex        =   104
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   39
      Left            =   2160
      TabIndex        =   103
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   38
      Left            =   1920
      TabIndex        =   102
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   37
      Left            =   1560
      TabIndex        =   101
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   36
      Left            =   1320
      TabIndex        =   100
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   35
      Left            =   1080
      TabIndex        =   99
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   34
      Left            =   840
      TabIndex        =   98
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   33
      Left            =   600
      TabIndex        =   97
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   32
      Left            =   240
      TabIndex        =   96
      Top             =   8040
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   31
      Left            =   4200
      TabIndex        =   95
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   30
      Left            =   3960
      TabIndex        =   94
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   29
      Left            =   3720
      TabIndex        =   93
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   28
      Left            =   3480
      TabIndex        =   92
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   27
      Left            =   3240
      TabIndex        =   91
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   26
      Left            =   2880
      TabIndex        =   90
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   25
      Left            =   2640
      TabIndex        =   89
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   24
      Left            =   2400
      TabIndex        =   88
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   23
      Left            =   2160
      TabIndex        =   87
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   22
      Left            =   1920
      TabIndex        =   86
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   21
      Left            =   1560
      TabIndex        =   85
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   20
      Left            =   1320
      TabIndex        =   84
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   19
      Left            =   1080
      TabIndex        =   83
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   18
      Left            =   840
      TabIndex        =   82
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   17
      Left            =   600
      TabIndex        =   81
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   16
      Left            =   240
      TabIndex        =   80
      Top             =   7800
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   15
      Left            =   4200
      TabIndex        =   79
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   14
      Left            =   3960
      TabIndex        =   78
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   13
      Left            =   3720
      TabIndex        =   77
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   12
      Left            =   3480
      TabIndex        =   76
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   11
      Left            =   3240
      TabIndex        =   75
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   10
      Left            =   2880
      TabIndex        =   74
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   9
      Left            =   2640
      TabIndex        =   73
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   8
      Left            =   2400
      TabIndex        =   72
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   7
      Left            =   2160
      TabIndex        =   71
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   6
      Left            =   1920
      TabIndex        =   70
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   5
      Left            =   1560
      TabIndex        =   69
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   4
      Left            =   1320
      TabIndex        =   68
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   3
      Left            =   1080
      TabIndex        =   67
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   2
      Left            =   840
      TabIndex        =   66
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   1
      Left            =   600
      TabIndex        =   65
      Top             =   7560
      Width           =   195
   End
   Begin VB.CheckBox chkTTLSelect 
      Height          =   195
      Index           =   0
      Left            =   240
      TabIndex        =   64
      Top             =   7560
      Width           =   195
   End
   Begin VB.CommandButton cmdForceOut 
      Caption         =   "ForceOut"
      Height          =   375
      Index           =   0
      Left            =   3600
      TabIndex        =   63
      Top             =   120
      Width           =   1215
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   63
      Left            =   5160
      Style           =   1  'Graphical
      TabIndex        =   62
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   62
      Left            =   4920
      Style           =   1  'Graphical
      TabIndex        =   61
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   61
      Left            =   4680
      Style           =   1  'Graphical
      TabIndex        =   60
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   60
      Left            =   4440
      Style           =   1  'Graphical
      TabIndex        =   59
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   59
      Left            =   4200
      Style           =   1  'Graphical
      TabIndex        =   58
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   58
      Left            =   3960
      Style           =   1  'Graphical
      TabIndex        =   57
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   57
      Left            =   3720
      Style           =   1  'Graphical
      TabIndex        =   56
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   56
      Left            =   3480
      Style           =   1  'Graphical
      TabIndex        =   55
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   55
      Left            =   2760
      Style           =   1  'Graphical
      TabIndex        =   54
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   54
      Left            =   2520
      Style           =   1  'Graphical
      TabIndex        =   53
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   53
      Left            =   2280
      Style           =   1  'Graphical
      TabIndex        =   52
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   52
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   51
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   51
      Left            =   1800
      Style           =   1  'Graphical
      TabIndex        =   50
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   50
      Left            =   1560
      Style           =   1  'Graphical
      TabIndex        =   49
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   49
      Left            =   1320
      Style           =   1  'Graphical
      TabIndex        =   48
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   195
      Index           =   48
      Left            =   1080
      Style           =   1  'Graphical
      TabIndex        =   47
      Top             =   1560
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   47
      Left            =   5160
      Style           =   1  'Graphical
      TabIndex        =   46
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   46
      Left            =   4920
      Style           =   1  'Graphical
      TabIndex        =   45
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   45
      Left            =   4680
      Style           =   1  'Graphical
      TabIndex        =   44
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   44
      Left            =   4440
      Style           =   1  'Graphical
      TabIndex        =   43
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   43
      Left            =   4200
      Style           =   1  'Graphical
      TabIndex        =   42
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   42
      Left            =   3960
      Style           =   1  'Graphical
      TabIndex        =   41
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   41
      Left            =   3720
      Style           =   1  'Graphical
      TabIndex        =   40
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   40
      Left            =   3480
      Style           =   1  'Graphical
      TabIndex        =   39
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   39
      Left            =   2760
      Style           =   1  'Graphical
      TabIndex        =   38
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   38
      Left            =   2520
      Style           =   1  'Graphical
      TabIndex        =   37
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   37
      Left            =   2280
      Style           =   1  'Graphical
      TabIndex        =   36
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   36
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   35
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   35
      Left            =   1800
      Style           =   1  'Graphical
      TabIndex        =   34
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   34
      Left            =   1560
      Style           =   1  'Graphical
      TabIndex        =   33
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   33
      Left            =   1320
      Style           =   1  'Graphical
      TabIndex        =   32
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   195
      Index           =   32
      Left            =   1080
      Style           =   1  'Graphical
      TabIndex        =   31
      Top             =   1320
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   31
      Left            =   5160
      Style           =   1  'Graphical
      TabIndex        =   30
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   30
      Left            =   4920
      Style           =   1  'Graphical
      TabIndex        =   29
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   29
      Left            =   4680
      Style           =   1  'Graphical
      TabIndex        =   28
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   28
      Left            =   4440
      Style           =   1  'Graphical
      TabIndex        =   27
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   27
      Left            =   4200
      Style           =   1  'Graphical
      TabIndex        =   26
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   26
      Left            =   3960
      Style           =   1  'Graphical
      TabIndex        =   25
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   25
      Left            =   3720
      Style           =   1  'Graphical
      TabIndex        =   24
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   24
      Left            =   3480
      Style           =   1  'Graphical
      TabIndex        =   23
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   23
      Left            =   2760
      Style           =   1  'Graphical
      TabIndex        =   22
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   22
      Left            =   2520
      Style           =   1  'Graphical
      TabIndex        =   21
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   21
      Left            =   2280
      Style           =   1  'Graphical
      TabIndex        =   20
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   20
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   19
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   19
      Left            =   1800
      Style           =   1  'Graphical
      TabIndex        =   18
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   18
      Left            =   1560
      Style           =   1  'Graphical
      TabIndex        =   17
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   17
      Left            =   1320
      Style           =   1  'Graphical
      TabIndex        =   16
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   195
      Index           =   16
      Left            =   1080
      Style           =   1  'Graphical
      TabIndex        =   15
      Top             =   1080
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   15
      Left            =   5160
      Style           =   1  'Graphical
      TabIndex        =   14
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   14
      Left            =   4920
      Style           =   1  'Graphical
      TabIndex        =   13
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   13
      Left            =   4680
      Style           =   1  'Graphical
      TabIndex        =   12
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   12
      Left            =   4440
      Style           =   1  'Graphical
      TabIndex        =   11
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   11
      Left            =   4200
      Style           =   1  'Graphical
      TabIndex        =   10
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   10
      Left            =   3960
      Style           =   1  'Graphical
      TabIndex        =   9
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   9
      Left            =   3720
      Style           =   1  'Graphical
      TabIndex        =   8
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   8
      Left            =   3480
      Style           =   1  'Graphical
      TabIndex        =   7
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   7
      Left            =   2760
      Style           =   1  'Graphical
      TabIndex        =   6
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   6
      Left            =   2520
      Style           =   1  'Graphical
      TabIndex        =   5
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   5
      Left            =   2280
      Style           =   1  'Graphical
      TabIndex        =   4
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   4
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   3
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   3
      Left            =   1800
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   2
      Left            =   1560
      Style           =   1  'Graphical
      TabIndex        =   1
      Top             =   840
      Width           =   200
   End
   Begin VB.CheckBox chkTTL 
      BackColor       =   &H008080FF&
      Height          =   200
      Index           =   1
      Left            =   1320
      Style           =   1  'Graphical
      TabIndex        =   0
      Top             =   840
      Width           =   200
   End
   Begin VB.Frame FrameTTLSelect 
      Caption         =   "Select TTLs"
      Height          =   1335
      Left            =   120
      TabIndex        =   128
      Top             =   7320
      Width           =   4455
   End
   Begin VB.Frame FrameDACSelect 
      Caption         =   "Select DACs"
      Height          =   1335
      Left            =   5520
      TabIndex        =   154
      Top             =   7320
      Width           =   2415
      Begin VB.Label Label3 
         Caption         =   " 0   1    2   3     4    5    6    7"
         Height          =   255
         Left            =   240
         TabIndex        =   184
         Top             =   240
         Width           =   2175
      End
      Begin VB.Label Label4 
         Caption         =   " 8   9  10  11   12  13  14 15"
         Height          =   255
         Index           =   0
         Left            =   240
         TabIndex        =   183
         Top             =   960
         Width           =   2175
      End
   End
   Begin VB.Label Label2 
      Caption         =   "DAC 2"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2055
      Index           =   2
      Left            =   10080
      TabIndex        =   221
      Top             =   120
      Width           =   615
   End
   Begin VB.Label Label28 
      Caption         =   "15"
      Height          =   255
      Left            =   5150
      TabIndex        =   209
      Top             =   600
      Width           =   255
   End
   Begin VB.Label Label27 
      Caption         =   "14"
      Height          =   255
      Left            =   4910
      TabIndex        =   208
      Top             =   600
      Width           =   255
   End
   Begin VB.Label Label26 
      Caption         =   "13"
      Height          =   255
      Left            =   4680
      TabIndex        =   207
      Top             =   600
      Width           =   255
   End
   Begin VB.Label Label25 
      Caption         =   "12"
      Height          =   255
      Left            =   4430
      TabIndex        =   206
      Top             =   600
      Width           =   255
   End
   Begin VB.Label Label24 
      Caption         =   "11"
      Height          =   255
      Left            =   4200
      TabIndex        =   205
      Top             =   600
      Width           =   255
   End
   Begin VB.Label Label23 
      Caption         =   "10"
      Height          =   255
      Left            =   3950
      TabIndex        =   204
      Top             =   600
      Width           =   255
   End
   Begin VB.Label Label22 
      Caption         =   "9"
      Height          =   255
      Left            =   3720
      TabIndex        =   203
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label21 
      Caption         =   "8"
      Height          =   255
      Left            =   3480
      TabIndex        =   202
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label20 
      Caption         =   "7"
      Height          =   255
      Left            =   2760
      TabIndex        =   201
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label19 
      Caption         =   "6"
      Height          =   255
      Left            =   2520
      TabIndex        =   200
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label18 
      Caption         =   "5"
      Height          =   255
      Left            =   2280
      TabIndex        =   199
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label17 
      Caption         =   "4"
      Height          =   255
      Left            =   2040
      TabIndex        =   198
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label16 
      Caption         =   "3"
      Height          =   255
      Left            =   1800
      TabIndex        =   197
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label15 
      Caption         =   "2"
      Height          =   255
      Left            =   1560
      TabIndex        =   196
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label14 
      Caption         =   "1"
      Height          =   255
      Left            =   1320
      TabIndex        =   195
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label13 
      Caption         =   "0"
      Height          =   255
      Left            =   1080
      TabIndex        =   194
      Top             =   600
      Width           =   135
   End
   Begin VB.Label Label12 
      Caption         =   "d"
      Height          =   255
      Left            =   3240
      TabIndex        =   193
      Top             =   1560
      Width           =   255
   End
   Begin VB.Label Label11 
      Caption         =   "c"
      Height          =   255
      Left            =   3240
      TabIndex        =   192
      Top             =   1320
      Width           =   255
   End
   Begin VB.Label Label10 
      Caption         =   "b"
      Height          =   255
      Left            =   3240
      TabIndex        =   191
      Top             =   1080
      Width           =   255
   End
   Begin VB.Label Label9 
      Caption         =   "a"
      Height          =   255
      Left            =   3240
      TabIndex        =   190
      Top             =   840
      Width           =   255
   End
   Begin VB.Label Label8 
      Caption         =   "d"
      Height          =   255
      Left            =   840
      TabIndex        =   189
      Top             =   1560
      Width           =   135
   End
   Begin VB.Label Label7 
      Caption         =   "c"
      Height          =   255
      Left            =   840
      TabIndex        =   188
      Top             =   1320
      Width           =   135
   End
   Begin VB.Label Label6 
      Caption         =   "b"
      Height          =   255
      Left            =   840
      TabIndex        =   187
      Top             =   1080
      Width           =   135
   End
   Begin VB.Label Label5 
      Caption         =   "a"
      Height          =   255
      Left            =   840
      TabIndex        =   186
      Top             =   840
      Width           =   135
   End
   Begin VB.Label Label2 
      Caption         =   "DAC 1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2055
      Index           =   1
      Left            =   8160
      TabIndex        =   174
      Top             =   120
      Width           =   615
   End
   Begin VB.Label Label2 
      Caption         =   "DAC 0"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2055
      Index           =   0
      Left            =   6120
      TabIndex        =   173
      Top             =   120
      Width           =   615
   End
   Begin VB.Label Label1 
      Caption         =   "ymax"
      Height          =   255
      Index           =   5
      Left            =   9720
      TabIndex        =   162
      Top             =   6960
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "ymin"
      Height          =   255
      Index           =   4
      Left            =   8400
      TabIndex        =   161
      Top             =   6960
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "tmax"
      Height          =   255
      Index           =   3
      Left            =   6960
      TabIndex        =   158
      Top             =   6960
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "tmin"
      Height          =   255
      Index           =   2
      Left            =   5640
      TabIndex        =   157
      Top             =   6960
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "tmax"
      Height          =   255
      Index           =   1
      Left            =   1560
      TabIndex        =   136
      Top             =   6960
      Width           =   375
   End
   Begin VB.Label Label1 
      Caption         =   "tmin"
      Height          =   255
      Index           =   0
      Left            =   120
      TabIndex        =   134
      Top             =   6960
      Width           =   375
   End
End
Attribute VB_Name = "PlotFrm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Option Base 0
Private userxrange As Boolean

Public Sub PlotTTL()
    Dim i As Integer
    Dim j As Integer
    
    For j = 0 To 3
        For i = 0 To 15
            chkTTL(j * 16 + i).VALUE = -CInt(TTLarray(j, i))
            If chkTTL(j * 16 + i).VALUE = 1 Then
                chkTTL(j * 16 + i).BackColor = &HC000&
            Else: chkTTL(j * 16 + i).BackColor = &H8080FF
            End If
        Next i
    Next j
    'InputFrm.SetFocus  'this makes it difficult to change things on the PlotFrm
End Sub
Public Sub PlotDAC()
    Dim i As Integer
    
    For i = 0 To numDACs - 1
        txtDAC(i).Text = CStr(CDec(DACarray(i)))
    Next i
    InputFrm.SetFocus
End Sub
Public Sub PlotqloopDACs()

    Dim i As Integer    'DAC line
    Dim k As Integer    'time index
    Dim l As Integer    'index for DAC channel to be plotted
    Dim xmin As Double
    Dim xmax As Double
    Dim ymin As Double
    Dim ymax As Double
    Dim ystep As Double
    Dim data() As Double
    
    If plotflag = True Then
    
        'find out how many DAC channels are selected for plotting
        l = 0
        For i = 0 To 15
            If ChkDACselect(i) = 1 Then l = l + 1
        Next i
        If l = 0 Or DACbuffsize = 0 Then Exit Sub
    
        'redim data array for plotting
        ReDim data(l, DACbuffsize)
        ReDim plotDACdata(1, DACbuffsize)
    
        'set data array values for plotting
        For k = 0 To DACbuffsize - 1
            data(0, k) = DACdatatoboard(0, k) / 1000 'time in ms
            'Debug.Print data(0,k)
            l = 0
            For i = 0 To 15
                If ChkDACselect(i) = 1 Then
                    data(l + 1, k) = DACdatatoboard(i + 1, k)
                    l = l + 1
                    'Debug.Print l, k, data(l, k)
                End If
            Next i
        Next k
        plotDACdata = data
        plotbuffsize = DACbuffsize
    Else
        data = plotDACdata
    End If
    
    'call plotgraph subroutine
    If userxrange Then
        xmin = CDbl(txtDACtmin(0).Text)
        xmax = CDbl(txtDACtmin(1).Text)
        ymin = CDbl(txtDACtmin(2).Text)
        ymax = CDbl(txtDACtmin(3).Text)
        userxrange = False
    Else
        xmin = data(0, 0)
        xmax = data(0, DACbuffsize - 1)
        ymin = DACmatrix(2, 0) / 10 ^ 6
        ymax = 1.1 * (DACmatrix(2, 0) / 10 ^ 6)
        'Debug.Print "hello", ymin, ymax
        For i = 1 To DACcounter - 1
            If DACmatrix(2, i) / 10 ^ 6 < ymin Then ymin = DACmatrix(2, i) / 10 ^ 6
            If DACmatrix(2, i) / 10 ^ 6 > ymax Then ymax = DACmatrix(2, i) / 10 ^ 6
        Next i
        txtDACtmin(0).Text = CStr(xmin)
        txtDACtmin(1).Text = CStr(xmax)
        txtDACtmin(2).Text = CStr(ymin)
        txtDACtmin(3).Text = CStr(ymax)
    End If
    ystep = round((ymax - ymin) / 5)
    'Debug.Print "xrange ", xmin, xmax
    'Debug.Print "yrange ", ymin, ymax, ystep
    'Debug.Print userxrange
        
    Call Plotgraph(Picture2, data, CLng(plotbuffsize), xmin, xmax, ymin, ymax, ystep)

    InputFrm.SetFocus
    
End Sub
Public Sub PlotqloopTTLs()

    Dim i As Integer    'TTL line
    Dim j As Integer    'TTL port
    Dim k As Integer    'time index
    Dim l As Integer    'index for TTL channel to be plotted
    Dim xmin As Double
    Dim xmax As Double
    Dim ymin As Double
    Dim ymax As Double
    Dim ystep As Double
    Dim time As Double   'in ms
    Dim VALUE As Double
    Dim data() As Double
    
     'find out how many TTL channels are selected for plotting
    l = 0
    For j = 0 To 3
        For i = 0 To 15
            If chkTTLSelect(j * 16 + i) = 1 Then l = l + 1
        Next i
    Next j
    If l = 0 Or TTLbuffsize = 0 Then Exit Sub
    
    If plotflag = True Then
        'redim data array for plotting
        ReDim data(l, TTLbuffsize)
        ReDim plotTTLdata(1, TTLbuffsize)
        
        'set data array values for plotting
        For k = 0 To TTLbuffsize - 1
            time = (converttounsigned(TTLdatatoboard(0, k)) + _
              converttounsigned(TTLdatatoboard(1, k)) * 65535) / 1000
            data(0, k) = time
            'Debug.Print time
            l = 0
            For j = 0 To 3
                VALUE = converttounsigned(TTLdatatoboard(j + 2, k))
                For i = 0 To 15
                    If chkTTLSelect(j * 16 + i) = 1 Then
                        data(l + 1, k) = ((VALUE \ (2 ^ i)) Mod 2) + l * 2
                        'Debug.Print l + 1, k, data(l + 1, k)
                        l = l + 1
                    End If
                Next i
            Next j
        Next k
        plotTTLdata = data
        plotTTLbuffsize = TTLbuffsize
    Else
        'ReDim data(l, plotTTLbuffsize)
        data = plotTTLdata
        'Debug.Print data(1, 2)
    End If
                
    'call plotgraph subroutine
    ymin = 0
    ymax = l * 2
    ystep = 1
    If l > 5 Then ystep = 2
    If userxrange Then
        xmin = CDbl(txtTTLtmin(0).Text)
        xmax = CDbl(txtTTLtmin(1).Text)
        userxrange = False
    Else
        xmin = data(0, 0)
        xmax = data(0, TTLbuffsize - 1)
        txtTTLtmin(0).Text = CStr(xmin)
        txtTTLtmin(1).Text = CStr(xmax)
    End If
    'Debug.Print "xrange ", xmin, xmax
    'Debug.Print userxrange
    'Debug.Print data(1, 2), CLng(plotTTLbuffsize), xmin, xmax, ymin, ymax, ystep
    Call Plotgraph(Picture1, data, CLng(plotTTLbuffsize), xmin, xmax, ymin, ymax, ystep)
    InputFrm.SetFocus
    
End Sub
Public Sub Plotgraph(win As PictureBox, data() As Double, num As Long, xmin As Double, _
    xmax As Double, ymin As Double, ymax As Double, ystep As Double)

    Dim xmin2, xmax2, ymin2, ymax2 As Integer
    Dim i As Integer
    Dim j As Integer
    Dim k As Integer
    Dim step As Double
    Dim xoffset As Double

    If xmin = xmax Or ymin = ymax Then
        'Debug.Print "problem with xmin..."
        Exit Sub
    End If
    
    'Debug.Print "everything is ok..."
    
    win.Cls
    win.ForeColor = QBColor(0)
    win.BackColor = QBColor(15)

    'find a step size by dividing by 5 and rounding to 1 significant digit
    step = round((xmax - xmin) / 5)
    'Debug.Print "step " & step

    ' establish scale, leaving room for titles, scales, etc.
    xmin2 = xmin - 0.2 * (xmax - xmin)
    xmax2 = xmax + 0.1 * (xmax - xmin)
    ymax2 = ymax + 0.3 * (ymax - ymin)
    ymin2 = ymin - 0.3 * (ymax - ymin)
    win.Scale (xmin2, ymax2)-(xmax2, ymin2)

    'draw a box for the graph
    win.Line (xmin - 0.3 * win.TextWidth("x"), ymin + 0.3 * win.TextHeight("x"))- _
      (xmax + 0.4 * win.TextWidth("x"), ymax - 0.3 * win.TextHeight("x")), , B

    'draw x-axis scale
    xoffset = CInt(xmin / step) * step
    'Debug.Print xoffset
    For i = 0 To CInt((xmax - xmin) / step)
        win.Line (xoffset + i * step, ymin)-(xoffset + i * step, ymin + win.TextHeight("x") / 2)
        If (i Mod 1) = 0 Then
            win.CurrentX = win.CurrentX - win.TextWidth(Trim(Str(i))) / 2
            win.Print Trim(Str(xoffset + i * step));
        End If
    Next i

    ' draw y-axis scale
    For i = 0 To CInt((ymax - ymin) / ystep)
        win.Line (xmin, i)-(xmin - win.TextWidth("x"), i)
        If (i Mod 1) = 0 Then
            win.CurrentX = win.CurrentX - win.TextWidth(Trim(Str(i)))
            win.CurrentY = win.CurrentY - win.TextHeight("x") / 2
            win.Print Trim(Str(i));
        End If
    Next i

    ' put a title at the top
    Title$ = "qloop TTLs"
    win.CurrentX = (xmin + xmax) / 2 - win.TextWidth(Title$) / 2
    win.CurrentY = ymax - win.TextHeight("x") * 2
    win.Print Title$;

    ' label the y scale (this is the tricky part)
    Title$ = "value"
    win.CurrentX = xmin - win.TextWidth("XXXXX")
    win.CurrentY = (ymax + ymin) / 2
    win.ScaleMode = 3
    xstart = win.CurrentX
    ystart = win.CurrentY + win.TextWidth(Title$) / 2
    win.CurrentX = 0
    win.CurrentY = 0
    win.Print Title$;
    For i = 0 To win.TextWidth(Title$) - 1
        For j = 0 To win.TextHeight(Title$) - 1
            If win.Point(i, j) <> win.BackColor Then
                win.PSet (xstart + j, ystart - i)
                win.PSet (i, j), win.BackColor
            End If
        Next j
    Next i
    win.Scale (xmin2, ymax2)-(xmax2, ymin2)

    ' label the x scale
    Title$ = "time (ms)"
    win.CurrentX = (xmin + xmax) / 2 - win.TextWidth(Title$) / 2
    win.CurrentY = ymin + win.TextHeight("x") * 2
    win.Print Title$;

    '
    ' draw the line
    '
    For j = 1 To UBound(data, 1)  ' loop through number of channels to be plotted
        win.CurrentX = data(0, 0)
        win.CurrentY = data(j, 0)
        For i = 1 To num - 1
            win.Line -(data(0, i), data(j, i - 1)), QBColor(0 + j)
            win.Line -(data(0, i), data(j, i)), QBColor(0 + j)
        Next i
    Next j
    InputFrm.SetFocus
    
End Sub



'edited: AMK 11/9/2010
Private Sub cmdClear_Click(Index As Integer)
    Dim i As Integer
    Dim j As Integer
    
    If Index = 0 Then
        For j = 0 To 3
            For i = 0 To 15
                chkTTLSelect(j * 16 + i).VALUE = 0
            Next i
        Next j
    End If
    If Index = 1 Then
        For j = 0 To numDACs / 8
            For i = 0 To 7
                ChkDACselect(j * 8 + i).VALUE = 0
            Next i
        Next j
    End If
    
End Sub
'edited: AMK 11/9/2010
Private Sub cmdForceOut_Click(Index As Integer)
    Dim i As Integer
    Dim j As Integer
    Dim output As Integer
    
    If Index = 0 Then
        For j = 0 To 3
            For i = 0 To 15
                TTLarray(j, i) = chkTTL(j * 16 + i).VALUE
            Next i
        Next j
        Call TTLarrayConvert
        Call DIOforceout
        Call PlotTTL
    End If
    
    If Index = 1 Then
        For i = 0 To numDACs - 1
            DACarray(i) = CDbl(txtDAC(i).Text)
        Next i
        DACforce2 DACarray(0), 0   'Note: this will set all the DACs
        Call PlotDAC
    End If
    
    InputFrm.SetFocus
    
End Sub

Private Sub cmdReplot_Click(Index As Integer)
    userxrange = True
   ' Debug.Print Index
    If Index = 0 Then Call PlotqloopTTLs
    If Index = 1 Then Call PlotqloopDACs
End Sub

Private Sub cmdShowDACs_Click()
    Call PlotDAC
End Sub

Private Sub cmdShowTTLs_Click()
    Call PlotTTL
End Sub

Private Sub cmdTTLdefault_Click()
    Call TTLdefault
End Sub

'edited: AMK 11/9/2010
Private Sub Form_Load()
'    ComputerControlFrm.Show
    Label2(0).Caption = "DAC 0       1            2            3            4            5            6            7"
    Label2(1).Caption = "    8            9           10          11          12          13          14          15"
    Label2(2).Caption = "    16          17          18          19          20          21          22          23"
    userxrange = False
    InputFrm.SetFocus
End Sub


Private Sub FrameShowTTLs_DragDrop(Source As Control, X As Single, Y As Single)

End Sub

Private Sub Label29_Click(Index As Integer)
End Sub

