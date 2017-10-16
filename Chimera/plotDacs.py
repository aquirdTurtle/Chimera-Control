
# coding: utf-8

# # Helper Functions

# In[1]:

# This function takes in a DAC textfile and converts it into a 2D array where the the first
# column is the time values and the rest of the columns are the different DACs
import numpy as np

def arrayDAC(file):
    # open and read text file
    with open(file, 'r') as txt:
        a=txt.readlines()
    index = 0                           # This while loop replaces every symbol except commas
    c=len(a)                            # and numbers with and empty space in the file
    while index < c:
        a[index]=a[index].replace('\n','')     #index loops through each line of the file
        a[index]=a[index].replace(' ','')
        a[index]=a[index].replace(':','')
        # separate strings in array by ','
        if len(a[index].split(','))>1:
            a[index:index+1]=a[index].split(',')
        c=len(a)
        index = index+1
    index =0
    # loop again deleting empty elements of array
    while index < c :
        if not a[index]:
            del a[index]
        else:
            index += 1
        c = len(a)
    index=0
    while a[index] != "---":                                     # This while loop creates a
        index += 1                                               # a 2D array where each row
    size=int(len(a)/index)                                       # is a new event/time
    arr_size=int((len(a)-size+(size/index))/index)               # the columns are different
    data = [[1 for i in range(index)] for j in range(arr_size)]  # DACs with the 0th column being
    i=0
    j=0
    k=i
    # tcreates the 2D array
    # This while loop fills in the correct elements
    while i < len(a):
        data[j][k]=float(a[i])
        i+=1
        k+=1
        if k%index < 1:
            j+=1
            i+=1
            k=0
    return np.array(data);


# In[2]:

def step(fig, xData, yData, color=None, legend=None, alpha=None):
    xx = np.sort(list(xData) + list(xData))
    xx = xx[:-1]
    yy = list(yData) + list(yData)
    yy[::2] = yData
    yy[1::2] = yData
    yy = yy[:-1]
    yy.insert(0,yy[0])
    fig.line(xx, yy, color=color, legend=legend, alpha=alpha)
    return fig


# In[3]:

def allSame(items):
    return all(x == items[0] for x in items)


# # Graphing Function

# In[9]:

# This function takes in an array produced by arrayDAC() and plots the DAC values with respect
# to time. The function calculates the standard deviation of each DAC for the whole experiment
# to see if the DAC value is changing. If the DAC value is changing, then it is plotted. If
# the DAC value is not changing, then it is listed on the left with the label 'DAC ... OFF'
# The constant value is listed inside the brackets e.x. "[8=0.0]".
# The 24 DACs of the experiment or displayed 8 at a time on three plots.
import bokeh
import bokeh.plotting as bok
import bokeh.layouts as bokLO
import matplotlib as mpl
import matplotlib.cm

def graphDacs(arrayDAC,eventMin,eventMax,ymin,ymax, vsTime=True):
    cmapRGB = mpl.cm.get_cmap('gist_rainbow', 17)
    bokcolors = [cmapRGB(i)[:-1] for i in range(17)]
    bokcolors = ['#%02x%02x%02x' % tuple(int(255*color[i]) for i in range(len(color))) for color in bokcolors]
    if vsTime:
        x = arrayDAC[:,0]
    else:
        x = np.array(list(range(len(arrayDAC[:,0]))))
    bokFigs = [None] * 3
    bokTxtFigs = [None] * 3
    for i in range(len(bokFigs)):
        bokFigs[i] = bok.figure(y_range=(-10, 10), width=500, height=150)
        bokTxtFigs[i] = bok.figure(width=500, height=125)
    markers = ['s', '^', 'o', 'D', '*',  'x', 'p', 'o', 's']
    # loop through each dac board
    for board in range(3):
        string1="                           CONSTANT DACs \n "
        string11=""
        flag1=0
        offset = board * 8
        # loop through each dac on a board
        for num in range(8):
            index = offset + num + 1
            if not allSame(arrayDAC[:, index ]):
                bokFigs[board] = step( bokFigs[board], x, arrayDAC[:, index],
                                       color=bokcolors[num], legend="dac" + str(index),
                                       alpha=0.4 )
            else:
                legendTxt = "[" + str(index) + "=" + str(arrayDAC[2,index]) + "]"
                if len(string11) >= 28 and flag1 < 1:
                    string11 = string11 + legendTxt + " \n "
                    flag1 = 1
                else:
                    string11 = string11 + legendTxt + " "
        bokFigs[board].yaxis.major_tick_line_color = None
        bokFigs[board].yaxis.minor_tick_line_color = None
        # put a glyph on the plot so that bokeh doesn't complain.
        bokTxtFigs[board].circle( 0, 0, color='white' )
        # create text objects listing which ttls are always on / off
        txt1 = bokeh.models.Label( x=0, y=0,
                                  x_units='screen', y_units='screen',
                                  text=string11,
                                  text_font_size='10pt')
        bokTxtFigs[board].add_layout(txt1)
        # clear out grid & axes for text plot
        bokTxtFigs[board].axis.visible = False
        bokTxtFigs[board].xgrid.grid_line_color = None
        bokTxtFigs[board].ygrid.grid_line_color = None
        bokTxtFigs[board].circle(0,0, alpha=0)
        bokFigs[board].circle(0,0,alpha=0)
        bokFigs[board].legend.background_fill_alpha = 0
        bokFigs[board].legend.label_text_font_size = "7pt"
        bokFigs[board].legend.glyph_height = 15
        bokFigs[board].legend.label_height = 7
        bokFigs[board].legend.spacing = 0
        bokFigs[board].yaxis.axis_label = 'Voltage (V)'
    # add single label
    if vsTime:
        bokFigs[-1].xaxis.axis_label = 'Time (ms)'
    else:
        bokFigs[-1].xaxis.axis_label = 'Event #'

    bokeh.io.output_file('DAC_Plot.html', title='DAC Plot')
    f = bokLO.gridplot(
        [bokTxtFigs[0], bokFigs[0]],
        [bokTxtFigs[1], bokFigs[1]],
        [bokTxtFigs[2], bokFigs[2]]
    )
    bok.show(f)


# # Called Function: plotDacs

# In[5]:

# This function takes in a textfile containing the DAC values from the experiment and plots
# either the DAC voltage vs time or DAC voltage vs event. The event/time range and the voltage
# range can be modified using xmin/xmax and ymin/ymax respectively. If
# time = True, then time-dependent plots are shown. If time != True, then event-dependent plots
# are shown. The event-dependent plots are the default plots.

def plotDacs(fileAddress = None,time = None,xmin = None, xmax = None, ymin = None, ymax = None):
    if fileAddress == None or (not type(fileAddress)== str):
        print("ERROR! Need text file!")
    else:
        dac_data = arrayDAC(fileAddress)
        if ymin == None:
            minValue = min(dac_data[:,1])
            for i in range(1,len(dac_data[0,:])):
                if min(dac_data[:,i]) < minValue:
                    minValue = min(dac_data[:,i])
            ymin = minValue-0.5
        if ymax == None:
            maxValue = max(dac_data[:,1])
            for i in range(1,len(dac_data[0,:])):
                if max(dac_data[:,i]) > maxValue:
                    maxValue = max(dac_data[:,i])
            ymax = maxValue+0.5
        if time == True:
            if xmin == None:
                xmin = 0
            if xmax == None:
                xmax = max(dac_data[:,0])+1
        else:
            if xmin == None:
                xmin = 0
            if xmax == None:
                xmax = len(dac_data[:,0])
        graphDacs(dac_data, xmin, xmax, ymin, ymax, vsTime=time)


# ### work

# In[10]:

#plotDacs( "C:\\Users\\Mark-Brown\\Chimera-Control\\Debug-Output\\DAC-Sequence.txt", time=True )


# In[ ]:
