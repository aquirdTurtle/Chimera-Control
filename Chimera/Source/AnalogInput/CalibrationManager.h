#pragma once
// created by Mark O. Brown
#pragma once
#include <AnalogInput/calInfo.h>
#include "AnalogInput/AiSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "DigitalOutput/DoSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/Version.h"
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <Agilent/AgilentCore.h>
#include <Python/NewPythonHandler.h>
#include <type_traits>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>



class CalibrationManager : public IChimeraSystem {
public:
	// THIS CLASS IS NOT COPYABLE.
	CalibrationManager& operator=(const CalibrationManager&) = delete;
	CalibrationManager (const CalibrationManager&) = delete;
	CalibrationManager (IChimeraQtWindow* parent);

	void handleContextMenu (const QPoint& pos);
	static std::string calTtlConfigToString (std::vector<std::pair<DoRows::which, unsigned> > ttlConfig);
	static std::string calDacConfigToString (std::vector<std::pair<unsigned, double>> aoConfig);
	void initialize (QPoint& pos, IChimeraQtWindow* parent, AiSystem* ai, AoSystem* ao, DoSystem* ttls_in,
		std::vector<std::reference_wrapper<AgilentCore>> agilentsIn, NewPythonHandler* python_in);
	//void setChangeVal (unsigned which, double change);
	void runAll ();
	void calibrate (calInfo& cal, unsigned which);
	bool wantsExpAutoCal ();
	void handleSaveMasterConfig (std::stringstream& configStream);
	void handleOpenMasterConfig (ConfigStream& configStream);
	//void setControlDisplay (unsigned which, double value);
	std::vector<calInfo> getCalibrationInfo ();
	std::string dblVecToString (std::vector<double> ctrls);
	void refreshListview ();
private:
	QLabel* calsHeader;
	CQPushButton* calibrateAllButton;
	CQCheckBox* expAutoCalButton;
	QTableWidget* calibrationTable;
	QPushButton* cancelCalButton;
	PlotCtrl calibrationViewer;
	void updateCalibrationView (calInfo& cal);
	void handleSaveMasterConfigIndvCal (std::stringstream& configStream, calInfo& servo);
	calInfo handleOpenMasterConfigIndvCal (ConfigStream& configStream);
	std::vector<calInfo> calibrations;
	void addCalToListview (calInfo& cal);
	AiSystem* ai;
	AoSystem* ao;
	DoSystem* ttls;
	std::vector<std::reference_wrapper<AgilentCore>> agilents;
	NewPythonHandler* pythonHandler;
};



