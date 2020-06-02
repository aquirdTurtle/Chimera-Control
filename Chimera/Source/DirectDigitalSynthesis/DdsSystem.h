#pragma once
#include "DdsCore.h"

#include "ParameterSystem/ParameterSystemStructures.h"
#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "ConfigurationSystems/Version.h"
#include "GeneralFlumes/ftdiFlume.h"
#include "ParameterSystem/Expression.h"
#include "CustomMfcControlWrappers/functionCombo.h"
#include "DirectDigitalSynthesis/DdsSystemStructures.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "GeneralObjects/ExpWrap.h"
#include "ftd2xx.h"
#include "afxwin.h"
#include "afxcview.h"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qtablewidget.h>
#include <qpushbutton.h>

class Script;
class MainWindow;
class AuxiliaryWindow;

class DdsSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		DdsSystem& operator=(const DdsSystem&) = delete;
		DdsSystem (const DdsSystem&) = delete;

		DdsSystem(bool ftSafemode);
		void redrawListview ( );
		void handleSaveConfig (ConfigStream& file );
		void handleOpenConfig (ConfigStream& file );
		void handleContextMenu (const QPoint& pos);
		void initialize(POINT& pos, IChimeraWindowWidget* master, std::string title );
		void rearrange(UINT width, UINT height, fontMap fonts);
		void handleRampClick();
		void deleteRampVariable();
		void programNow (std::vector<parameterType>& constants);
		std::string getSystemInfo ( );
		std::string getDelim ( );
		DdsCore& getCore ( );
		
	private:
		
		QLabel* ddsHeader;
		QTableWidget* rampListview;
		QPushButton* programNowButton;
		bool controlActive = true;
		std::vector<ddsIndvRampListInfo> currentRamps;
		DdsCore core;
};

