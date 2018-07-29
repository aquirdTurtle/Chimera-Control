#pragma once
#include "Control.h"
#include <array>

/// just roughly right now...

struct optimizationAlgorithm
{
	enum class which
	{
		HillCimbing,
		NeuralNetwork,
		Genetic,
		RandomizedSearch
	};
	which fromStr ( std::string txt );
	std::string toStr ( which alg );
};


struct optimizationSettings
{
	optimizationAlgorithm::which alg;
	double tolerance;
	double gain;
	std::string config;
};


class MachineOptimizer
{
	public:
		void initialize ( );
		void handleOpenConfig ( );
		void handleSaveConfig ( );
		void handleNewConfig ( );
		void runOptimization ( );
		void rearrange ( );
		void updateParams( );
	private:
		// controls
		Control<CStatic> header;
		Control<CButton> optimizeButton;
		// optimization settings
		Control<CStatic> optimizationParametersHeader;
		Control<CStatic> algorithmsHeader;
		std::array<Control<CButton>, 4> algorithmRadios;
		std::vector<double> history;
};