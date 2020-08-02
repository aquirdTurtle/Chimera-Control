// created by Mark O. Brown
#include "stdafx.h"

#include "ExperimentThread/ExperimentThreadManager.h"
#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "GeneralObjects/CodeTimer.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "NIAWG/NiawgWaiter.h"
#include "NIAWG/NiawgSystem.h"
#include "ParameterSystem/Expression.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "Andor/CameraSettingsControl.h"
#include "Scripts/ScriptStream.h"
#include <ExperimentThread/ExpThreadWorker.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <PrimaryWindows/QtAndorWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include <PrimaryWindows/QtBaslerWindow.h>
#include <qthread.h>
#include <RealTimeDataAnalysis/AnalysisThreadWorker.h>
#include <RealTimeDataAnalysis/AtomCruncherWorker.h>
#include "nidaqmx2.h"
#include <qdebug.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>
  