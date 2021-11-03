#pragma once
#include "DdsCore.h"

#include "ParameterSystem/ParameterSystemStructures.h"
#include "ConfigurationSystems/Version.h"
#include "GeneralFlumes/ftdiFlume.h"
#include "ParameterSystem/Expression.h"
#include "CustomQtControls/functionCombo.h"
#include "DirectDigitalSynthesis/DdsSystemStructures.h"
#include "GeneralObjects/ExpWrap.h"
#include "ftd2xx.h"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qtablewidget.h>
#include <qpushbutton.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <GeneralObjects/IChimeraSystem.h>

class DdsSystem : public IChimeraSystem{	
	public:
		// THIS CLASS IS NOT COPYABLE.
		DdsSystem& operator=(const DdsSystem&) = delete;
		DdsSystem (const DdsSystem&) = delete;

		DdsSystem(IChimeraQtWindow* parent, bool ftSafemode);
		void redrawListview ( );
		void handleSaveConfig (ConfigStream& file );
		void handleOpenConfig (ConfigStream& file );
		void handleContextMenu (const QPoint& pos);
		void initialize(QPoint& pos, IChimeraQtWindow* master, std::string title );
		void refreshCurrentRamps ();
		void programNow (std::vector<parameterType>& constants);
		std::string getSystemInfo ( );
		std::string getDelim ( );
		DdsCore& getCore ( );

	private:
		QLabel* ddsHeader;
		QTableWidget* rampListview;
		QPushButton* programNowButton;
		QPushButton* stepButton;
		QPushButton* resetButton;
		CQCheckBox* controlCheck;
		bool controlActive = true;
		std::vector<ddsIndvRampListInfo> currentRamps;

		DdsCore core;
};

