#pragma once

// this class holds the programming thread which sits around and waits until the atom crunching thread passes it some 
// data, at which point it calculates the rearrangement sequence using Kai's code and then calculates the waveforms
// associated with the sequence of moves. Finally, it Programms the corresponding niawg waveform, and triggers the 
// niawg.
class NiawgRealTimeProgrammer
{

};