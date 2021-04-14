// created by Mark O. Brown
#include "stdafx.h"
#include <algorithm>
#include <numeric>
#include "PictureStats.h"


// as of right now, the position of this control is not affected by the mode or the trigger mode.
void PictureStats::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();
	long size = 315;
	pictureStatsHeader = new QLabel ("Raw Counts", parent);
	pictureStatsHeader->setGeometry (px, py, size, 25);
	repetitionIndicator = new QLabel ("Repetition ?/?", parent);
	repetitionIndicator->setGeometry (px, py+=25, size, 25);
	/// Picture labels ////////////////////////////////////////////////////////////
	collumnHeaders[0] = new QLabel ("Pic:", parent);
	collumnHeaders[0]->setGeometry (px, py += 25, size / 5, 25);
	int inc = 0;
	for (auto& control : picNumberIndicators){
		inc++;
		control = new QLabel (qstr ("#" + str (inc) + ":"), parent);
		control->setGeometry (px, py+=25, size / 5, 25);
	}
	/// Max Count 
	py -= 100;
	px += size / 5;
	collumnHeaders[1] = new QLabel ("Max:", parent);
	collumnHeaders[1]->setGeometry (px, py, size / 5,25);
	for (auto& control : maxCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	/// Min Counts
	py -= 100;
	px += size / 5;
	collumnHeaders[2] = new QLabel ("Min:", parent);
	collumnHeaders[2]->setGeometry (px, py, size / 5, 25);
	for (auto& control : minCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	/// Average Counts
	py -= 100;
	px += size / 5;
	collumnHeaders[3] = new QLabel ("Avg:", parent);
	collumnHeaders[3]->setGeometry (px, py, size / 5, 25);
	for (auto& control : avgCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	/// Selection Counts
	py -= 100;
	px += size / 5;
	collumnHeaders[4] = new QLabel ("Avg:", parent);
	collumnHeaders[4]->setGeometry (px, py, size / 5, 25);
	for (auto& control : selCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	px -= 4./5*size;
	py += 25;
}


void PictureStats::reset(){
	for (auto& control : maxCounts)	{
		control->setText("-");
	}
	for (auto& control : minCounts)	{
		control->setText ("-");
	}
	for (auto& control : avgCounts)	{
		control->setText ("-");
	}
	for (auto& control : selCounts){
		control->setText ("-");
	}
	repetitionIndicator->setText ( "Repetition ---/---" );
}


void PictureStats::updateType(std::string typeText){
	displayDataType = typeText;
	pictureStatsHeader->setText (cstr(typeText));
}


statPoint PictureStats::getMostRecentStats ( ){
	return mostRecentStat;
}


