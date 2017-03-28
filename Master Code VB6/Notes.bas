Attribute VB_Name = "Notes"
'FAQs
'
'What runs when you start the code?
'   1.  InputFrm_Load
'           This calls "reset", which is in TheMainProgram module.'
'                   Reset sets routine="O" and call "runroutine", which is also
'                   in TheMainProgram module

'Where are all the public (global) variables defined?
'    at the beginning of TheMainProgram module!

'What is the best way to loop subroutines, like MOT, but allow a controlled
'   exit to run another routine?
'
'   While routine="O"
'       blah blah blah
'       wait(1000)  'waits at time in ms
'       DoEvents  'DoEvents is a Basic command that allows the computer to see key inputs
'                               (ie, effectively allowing getinput to run)
'   Wend
'   runroutine
'
'How to set up a run one-time routine?
'
'    blah blah blah
'    routine = "O" 'back to MOT
'    Call runroutine
'
'How do I change the default values for the TTLs?
'     "reset" in TheMainProgram module resets all the TTLs to their default values,
'         and then calls MOT  (MOT alone does not set all the TTLs to their default values)
'     The default values are constants (Azero0, Aone0, etc) set in the beginning of the "DIO64" module
'
'How do I plot DACs or TTLS in a qloop?
'     add statement "plotflag=True" right before qprep
'         (plotflag is set False in qprep after the plotting)
'     check boxes for what TTLs and what DACs you want to plot
'
'DIOboard frequency is set by a public const scan,
'  that is set at beginning of "DIO64" module
'
'DACforce2 really sets all the DAC channels
'     DACforce doesn't work
'
'qprep adds a bunch of qvolt commands to set the initial DAC values at time 0
'
'So to fix the clock we can put in a fudge factor to multiply the times.  It seems
'the clock is 16MHz but the board thinks it is 20MHz. So, if we scale the times by
'the correct factor in qTTLgather then we may be okay.  Lets make a public variable
' called "ClockFix"jjz 11-19-08
'
' I talked to Bob at Viewpoint Systems.  To set the PXI clock Use: DIO64_Set_ATTR(0,12,1)
' he will send a better manual to me. jjz 11-20-08
'
'
'
'
'
'
'
'
'
