// created by Mark O. Brown
#pragma once
#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
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
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qpushbutton.h>

/**/
class AuxiliaryWindow;

class DoSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		DoSystem& operator=(const DoSystem&) = delete;
		DoSystem (const DoSystem&) = delete;

		DoSystem ( bool ftSafemode, bool serialSafemode );
		~DoSystem();
		/// config handling
		std::string getDoSystemInfo ();
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile);
		void initialize(POINT& startLocation, IChimeraWindowWidget* master );
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		void zeroBoard();
		void handleTTLPress (DigitalOutput& out);
		void handleHoldPress();
		void setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status);
		bool getFtFlumeSafemode ();

		void rearrange(UINT width, UINT height, fontMap fonts);

		std::pair<UINT, UINT> getTtlBoardSize();

		void setName( DoRows::which row, UINT number, std::string name);
		std::string getName ( DoRows::which row, UINT number );
		Matrix<std::string> getAllNames ();
		bool getTtlStatus ( DoRows::which row, int number );
		void updateDefaultTtl( DoRows::which row, UINT column, bool state);
		bool getDefaultTtl( DoRows::which row, UINT column);
		std::pair<USHORT, USHORT> calcDoubleShortTime( double time );
		std::array< std::array<bool, 16>, 4 > getCurrentStatus( );
		void updatePush( DoRows::which row, UINT col );
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
		bool holdStatus; 
};

