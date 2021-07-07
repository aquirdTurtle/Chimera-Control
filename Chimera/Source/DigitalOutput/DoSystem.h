// created by Mark O. Brown
#pragma once
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include "DigitalOutput/DoStructures.h"
#include "ParameterSystem/Expression.h"
#include "Plotting/PlotCtrl.h"
#include "GeneralFlumes/ftdiStructures.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/ExpWrap.h"
#include "GeneralObjects/Matrix.h"

#include "DigitalOutput.h"
#include "DoCore.h"

#include <array>
#include <sstream>
#include <unordered_map>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <GeneralObjects/IChimeraSystem.h>

/**/
class AuxiliaryWindow;

class DoSystem : public IChimeraSystem {
	public:
		// THIS CLASS IS NOT COPYABLE.
		DoSystem& operator=(const DoSystem&) = delete;
		DoSystem (const DoSystem&) = delete;

		DoSystem (IChimeraQtWindow* parent, bool ftSafemode, bool serialSafemode );
		~DoSystem();
		/// config handling
		std::string getDoSystemInfo ();
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile);
		void initialize(QPoint& startLocation, IChimeraQtWindow* master );
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		void zeroBoard();
		void handleTTLPress (DigitalOutput& out);
		void handleHoldPress();
		void setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status);
		bool getFtFlumeSafemode ();

		std::pair<unsigned, unsigned> getTtlBoardSize();

		void setName( DoRows::which row, unsigned number, std::string name);
		std::string getName ( DoRows::which row, unsigned number );
		Matrix<std::string> getAllNames ();
		bool getTtlStatus ( DoRows::which row, int number );
		void updateDefaultTtl( DoRows::which row, unsigned column, bool state);
		bool getDefaultTtl( DoRows::which row, unsigned column);
		std::pair<USHORT, USHORT> calcDoubleShortTime( double time );
		std::array< std::array<bool, 16>, 4 > getCurrentStatus( );
		void updatePush( DoRows::which row, unsigned col );
		allDigitalOutputs& getDigitalOutputs();
		DoCore& getCore ();

	private:
		DoCore core;
		/// other.
		void handleInvert( );
		// one control for each TTL
		QLabel* ttlTitle;
		CQPushButton* ttlHold;
		CQPushButton* zeroTtls;
		std::array< QLabel*, 16 > ttlNumberLabels;
		std::array< QLabel*, 16 > ttlRowLabels;
		allDigitalOutputs outputs;

		// tells whether the hold button is down or not.
		// bool holdStatus; 
};

