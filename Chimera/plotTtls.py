
# coding: utf-8

# # Helper Functions

# In[24]:

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


# In[15]:

def allSame(items):
    return all(x == items[0] for x in items)


# In[7]:

import numpy as np

def loadTtlFile(file):
    """
    This function takes in a TTL textfile and converts it into a 2D array where the the first
    column is the time values and the rest of the columns are the different TTLs.
    """
    # open and read text file
    txt=open(file,'r')
    a=txt.readlines()

    index = 0
    c=len(a)
    # This while loop replaces every symbol except commas
    # and numbers with and empty space in the file
    while index < c:
        #index loops through each line of the file
        a[index]=a[index].replace('\n','')
        a[index]=a[index].replace(' ','')
        a[index]=a[index].replace(':','')
        a[index]=a[index].replace('A','')
        a[index]=a[index].replace('B','')
        a[index]=a[index].replace('C','')
        a[index]=a[index].replace('D','')
        # if statement separates strings in array by ','
        # if statement used to create additional indices due to splitting
        if len(a[index].split(','))>1:
            a[index:index+1]=a[index].split(',')
        c=len(a)
        index = index+1
    index = 0
    # loop again deleting empty elements of array
    while index < c :
        if not a[index]:
            del a[index]
        else:
            index += 1
        c = len(a)
    index = 0
    # This while loop creates  a 2D array where each row is a new event/time
    # the columns are different TTLs with the 0th column being
    while a[index] != "---":
        index += 1
    size=int(len(a)/index)
    arr_size=int((len(a)-size+(size/index))/index)
    data = [[1 for i in range(index)] for j in range(arr_size)]
    i=0
    j=0
    k=i
    # Create the 2D array This while loop fills in the correct elements
    while i < len(a):
        data[j][k]=float(a[i])
        i+=1
        k+=1
        if k%index < 1:
            j+=1
            i+=1
            k=0
    return np.array(data);


# # Graphing Function

# In[27]:

# This function takes in an array produced by arrayTTL() and plots the TTL values with respect
# to time. The function calculates the standard deviation of each TTL for the whole experiment
# to see if the TTL value is changing. If the TTL value is changing, then it is plotted. If
# the TTL value is not changing, then it is listed on the left stating whether the TTL value is
#0 or 1. The 64 TTLs of the experiment or displayed 16 at a time on four plots. The TTL values in
# a given plot have been shifted relative to each other so that we can distinguish them if
# many TTLs have 1 or 0 at the same time. This spacing is dictacted by the POS variable

import bokeh
import bokeh.plotting as bok
import bokeh.layouts as bokLO
import matplotlib as mpl
import matplotlib.cm
def graphTtls(arrayTTL, eventMin, eventMax, ymin, ymax, vsTime):
    bokeh.io.output_file('TTL_Plot.html', title='TTL Plot')
    cmapRGB = mpl.cm.get_cmap('gist_rainbow', 17)
    bokcolors = [cmapRGB(i)[:-1] for i in range(17)]
    bokcolors = ['#%02x%02x%02x' % tuple(int(255*color[i]) for i in range(len(color))) for color in bokcolors]
    if vsTime:
        x = arrayTTL[:,0]
    else:
        x = np.array(list(range(len(arrayTTL[:,0]))))
    ttlOffset=0.02
    bokFigs = [None] * 4
    bokTxtFigs = [None] * 4
    for i in range(len(bokFigs)):
        bokFigs[i] = bok.figure(y_range=(-0.2, 1.2), width=500, height=125)
        bokTxtFigs[i] = bok.figure(width=500, height=125)
    markers = ['s', '^', 'o', 'D', '*',  'x', 'p', 'o', 's']
    rowNames = ['A', 'B', 'C', 'D']
    # loop through rows
    for row in range(4):
        offset = row * 16
        flag11, flag10 = 0, 0
        string1="TTL \n"
        string11=""
        string10=""
        # loop through numbers in each row
        for num in range(16):
            # if it changes
            if not allSame(arrayTTL[:,offset+num+1]):
                # plot it.
                bokFigs[row] = step(bokFigs[row], x, arrayTTL[:,offset+num+1] + ttlOffset * (7-num),
                                    color=bokcolors[num], legend=rowNames[row] + str(num), alpha=0.5)
            else:
                # add to list of constant lines.
                txt = '[' + rowNames[row] + str(num+1) +']'
                if arrayTTL[2,num] > 0:
                    if len(string11) >= 33 and flag11 < 1:
                        string11=string11+"\n                       " + txt + " "
                        flag11 = 1
                    else:
                        string11=string11 + txt + " "
                else:
                    if len(string10) >= 33 and flag10 < 1:
                        string10=string10 + "\n                       " + txt + " "
                        flag10 = 1
                    else:
                        string10=string10 + txt + " "

        label_dict = {0: 'Off', 1: 'On'}
        bokFigs[row].yaxis.major_tick_line_color = None
        bokFigs[row].yaxis.minor_tick_line_color = None
        bokFigs[row].yaxis.ticker = bokeh.models.tickers.FixedTicker( ticks=[0, 1] )
        bokFigs[row].yaxis.formatter = bokeh.models.FuncTickFormatter( code="""var labels = %s;
                                                                              return labels[tick];
                                                                           """ % label_dict )
        # put a glyph on the plot so that bokeh doesn't complain.
        bokTxtFigs[row].circle( 0, 0, color='white' )
        # create text objects listing which ttls are always on / off
        txt1 = bokeh.models.Label( x=0, y=0,
                                  x_units='screen', y_units='screen',
                                  text="Always ON:  " + string11,
                                  text_font_size='8pt')
        txt2 = bokeh.models.Label( x=0, y=10,
                                  x_units='screen', y_units='screen',
                                  text="Always OFF: " + string10,
                                  text_font_size='8pt')
        bokTxtFigs[row].add_layout(txt1)
        bokTxtFigs[row].add_layout(txt2)
        # clear out grid & axes for text plot
        bokTxtFigs[row].axis.visible = False
        bokTxtFigs[row].xgrid.grid_line_color = None
        bokTxtFigs[row].ygrid.grid_line_color = None
        bokFigs[row].legend.background_fill_alpha = 0
        bokFigs[row].legend.label_text_font_size = "7pt"
        bokFigs[row].legend.glyph_height = 15
        bokFigs[row].legend.label_height = 7
        bokFigs[row].legend.spacing = 0

    # add single label
    if vsTime:
        bokFigs[-1].xaxis.axis_label = 'Time (ms)'
    else:
        bokFigs[-1].xaxis.axis_label = 'Event #'
    bok.show(bokLO.gridplot(
        [bokTxtFigs[0], bokFigs[0]],
        [bokTxtFigs[1], bokFigs[1]],
        [bokTxtFigs[2], bokFigs[2]],
        [bokTxtFigs[3], bokFigs[3]]
    ))


# # Calling function: plotTtls

# In[29]:

#This function takes in a textfile containing the DAC values from the experiment and plots
# either the TTL value vs time or TTL value vs event. The event/time range and the voltage
# range can be modified using xmin/xmax and ymin/ymax respectively. If
# time = True, then time-dependent plots are shown. If time != True, then event-dependent plots
# are shown. The event-dependent plots are the default plots.

def plotTtls(fileAddress = None,time = False,xmin = None, xmax = None, ymin = None, ymax = None):
    if fileAddress == None or (not type(fileAddress)== str):
        print("ERROR! Need text file!")
    else:
        ttl_data = loadTtlFile(fileAddress)
        if ymin == None:
            minValue = min(ttl_data[:,1])
            for i in range(1,len(ttl_data[0,:])):
                if min(ttl_data[:,i]) < minValue:
                    minValue = min(ttl_data[:,i])
            ymin = minValue-.2
        if ymax == None:
            maxValue = max(ttl_data[:,1])
            for i in range(1,len(ttl_data[0,:])):
                if max(ttl_data[:,i]) > maxValue:
                    maxValue = max(ttl_data[:,i])
            ymax = maxValue+.2

        if time == True:
            if xmin == None:
                xmin = 0
            if xmax == None:
                xmax = max(ttl_data[:,0])+1

        else:
            if xmin == None:
                xmin = 0
            if xmax == None:
                xmax = len(ttl_data[:,0])
        graphTtls(ttl_data, xmin, xmax, ymin, ymax, time)


# # Example

# In[30]:

#plotTTL("C:\\Users\\Mark-Brown\\Chimera-Control\\Debug-Output\\TTL-Sequence.txt", time=True )


# In[ ]:
