#include "stdafx.h"
#include "ParameterModel.h"
#include <boost/lexical_cast.hpp>

ParameterModel::ParameterModel (bool isGlobal_, QObject* parent)
    : QAbstractTableModel (parent), isGlobal(isGlobal_){
    rangeInfo.defaultInit ();
    rangeInfo.setNumScanDimensions (1);
    rangeInfo.setNumRanges (0, 1);
}

int ParameterModel::rowCount (const QModelIndex& /*parent*/) const{
    return parameters.size();
}

int ParameterModel::columnCount (const QModelIndex& /*parent*/) const{
    int maxRanges = 0;
    for (auto dimRange : rangeInfo.data){
        if (dimRange.size () > maxRanges){
            maxRanges = dimRange.size ();
        }
    }
    int baseCols = (isGlobal ? 2 : 5 + maxRanges*3);
    return baseCols;
}

QVariant ParameterModel::data (const QModelIndex& index, int role) const{
    int row = index.row ();
    int col = index.column ();
    auto param = parameters[row];
    try {
        switch (role) {
		case Qt::FontRole:{
			if (param.overwritten) {
				QFont font;
				font.setBold (true);
				return QVariant(font);
			}
			return QVariant (); // default
		}
		case Qt::ForegroundRole: { // text color
			if (param.overwritten) {
				return QVariant (QBrush (QColor (255, 0, 0)));
			}
			return QVariant (); // default
		}
		case Qt::BackgroundRole: { // background color
			if (param.active) {
				return QVariant (QBrush (QColor (40,40,80)));
			}
			return QVariant (); // default
		}
        case Qt::EditRole:
            if (!isGlobal && col >= 5) {
                auto rangeNum = int (col - 5) / 3;
                std::string lEnd = rangeInfo (param.scanDimension, rangeNum).leftInclusive ? "[" : "(";
                std::string rEnd = rangeInfo (param.scanDimension, rangeNum).rightInclusive ? "]" : ")";
                switch ((col - 5) % 3) {
					case 0:
						return qstr (param.ranges[rangeNum].initialValue, 9, true);
					case 1:
						return qstr (param.ranges[rangeNum].finalValue, 9, true);
                }
            } // purposely don't break as this is only different for these two. 
        case Qt::DisplayRole:
            if (isGlobal) {
                switch (col) {
                case 0:
                    return qstr(param.name);
                case 1:
                    return qstr (param.constantValue, 9, true);
                default:
                    return QVariant ();
                }
            }
            else {
                switch (col) {
                case 0:
                    return qstr (param.name);
                case 1:
                    return qstr (param.constant ? "Const." : "Var.");
                case 2:
                    return qstr (param.scanDimension);
                case 3:
                    return param.constant ? qstr (param.constantValue, 9, true) : "---";
                case 4:
                    return qstr (param.parameterScope);
                default:
                    if (param.constant) {
                        return QString ("---");
                    }
                    auto rangeNum = int (col - 5) / 3;
                    std::string lEnd = rangeInfo (param.scanDimension, rangeNum).leftInclusive ? "[" : "(";
                    std::string rEnd = rangeInfo (param.scanDimension, rangeNum).rightInclusive ? "]" : ")";
                    switch ((col - 5) % 3) {
						case 0:
							return qstr(lEnd + str (param.ranges[rangeNum].initialValue,9,true));
						case 1:
							return qstr (str(param.ranges[rangeNum].finalValue, 9, true) + rEnd);
						case 2:
							return qstr (rangeInfo (param.scanDimension, rangeNum).variations);
                    }
                    return QVariant ();
                }
            }
        }
    }
    catch (ChimeraError& err){
        errBox (err.trace ());
    }
    return QVariant ();
}

QVariant ParameterModel::headerData (int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (isGlobal) {
            if (section > 1) return QVariant ();
            auto headers = std::vector<std::string> ({ "Symbol", "Value" });
            return QString (headers[section].c_str());
        } else {
            auto headers = std::vector<std::string> ({ " Sym. ", " Type ", " Dim ", " Val. ", " Scope " });
            for (auto rangeInc : range (5)){
                headers.push_back (str (rangeInc + 1) + ". {");
                headers.push_back ("}");
                headers.push_back ("#");
            }
            if (section >= headers.size()) return QVariant ();
            return QString (headers[section].c_str ());
        }
    }
    else if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return QString (cstr (section));
    }
    return QVariant ();
}

Qt::ItemFlags ParameterModel::flags (const QModelIndex& index) const {
    if (index.column () == 1 && !isGlobal || // the "const. / var. option is togglable, not editable.
		(index.column() >= preRangeColumns && parameters[index.row()].constant) || // variation values for constants
		index.column() == 3 && !isGlobal && !parameters[index.row()].constant) { // const value for variables
        return QAbstractTableModel::flags (index);
    }
	return Qt::ItemIsEditable | QAbstractTableModel::flags (index);
};

bool ParameterModel::setData (const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole) {
        try {
            auto& param = parameters[index.row ()];
            if (isGlobal) {
                switch (index.column ()) {
                    case 0:
                        param.name = str (value.toString ());
                        break;
                    case 1:
                        param.constantValue = boost::lexical_cast<double>(cstr (value.toString ()));
                        break;
                }
            }
            else {
                switch (index.column ()) {
                    case 0:
                        param.name = str (value.toString ());
                        break;
                    case 1: // constantValue
                        break;
                    case 2:
                        param.scanDimension = boost::lexical_cast<int> (cstr(value.toString ())); 
                        if (param.scanDimension+1 > rangeInfo.numScanDimensions ()){
                            rangeInfo.setNumScanDimensions (param.scanDimension+1);
                        }
                        break;
                    case 3: 
                        param.constantValue = boost::lexical_cast<double> (cstr(value.toString ()));
                        break;
                    case 4:
                        param.parameterScope = str (value.toString ());
                        break;
                    default:
                        auto rangeNum = int (index.column() - 5) / 3;
                        std::string lEnd = rangeInfo (param.scanDimension, rangeNum).leftInclusive ? "[" : "(";
                        std::string rEnd = rangeInfo (param.scanDimension, rangeNum).rightInclusive ? "]" : ")";

                        switch ((index.column () - 5) % 3) {
							case 0:
								param.ranges[rangeNum].initialValue = boost::lexical_cast<double>(cstr(value.toString ()));
								break;
							case 1:
								param.ranges[rangeNum].finalValue = boost::lexical_cast<double>(cstr (value.toString ()));
								break;
							case 2:
								rangeInfo (param.scanDimension, rangeNum).variations 
									= boost::lexical_cast<unsigned int>(cstr (value.toString ()));
                        }
                }
            }
        }
        catch (boost::bad_lexical_cast&) {
            // hmm
        }
        emit paramsChanged ();
        return true;
    }
    return false;
}

void ParameterModel::setParams (std::vector<parameterType> newParams) {
    beginResetModel ();
    parameters = newParams;
    endResetModel();
    emit paramsChanged ();
}

std::vector<parameterType>& ParameterModel::getParams () {
    return parameters;
}

ScanRangeInfo ParameterModel::getRangeInfo (){
    return rangeInfo;
}

void ParameterModel::setRangeInfo (ScanRangeInfo info){
    beginResetModel ();
    rangeInfo = info;
    endResetModel ();
}

void ParameterModel::checkScanDimensionConsistency () {
    for (auto& param : parameters) {
        if (param.scanDimension >= rangeInfo.numScanDimensions ()) {
            param.scanDimension = 0;
        }
    }
}

void ParameterModel::checkVariationRangeConsistency () {
    unsigned dum = 0;
    for (auto var : parameters) {
        if (var.ranges.size () != rangeInfo.numRanges (var.scanDimension)) {
            if (dum == 0) {
                errBox ("The number of variation ranges of a parameter, " + var.name + ", (and perhaps others) did "
                    "not match the official number. The code will force the parameter to match the official number.");
                dum++; // only dislpay the error message once.
            }
            var.ranges.resize (rangeInfo.numRanges (var.scanDimension));
        }
    }
}


void ParameterModel::setVariationRangeNumber (int num, USHORT dimNumber){
    // -2 for the two +- columns
    int currentVariableRangeNumber = (columnCount () - preRangeColumns) / 3;
    checkScanDimensionConsistency ();
    checkVariationRangeConsistency ();
    if (rangeInfo.numRanges (dimNumber) != currentVariableRangeNumber) {
        errBox ("somehow, the number of ranges the ParameterSystem object thinks there are and the actual number "
            "displayed are off! The numbers are " + str (rangeInfo.numRanges (dimNumber)) + " and "
            + str (currentVariableRangeNumber) + " respectively. The program will attempt to fix this, but "
            "data may be lost.");
        while (rangeInfo.numRanges (dimNumber) != currentVariableRangeNumber) {
            if (rangeInfo.numRanges (dimNumber) > currentVariableRangeNumber) {
                rangeInfo.dimensionInfo (dimNumber).pop_back ();
                for (auto& param : parameters) {
                    param.ranges.pop_back ();
                }
            }
            else {
                rangeInfo.dimensionInfo (dimNumber).push_back (defaultRangeInfo);
                for (auto& param : parameters) {
                    param.ranges.pop_back ();
                }
            }
        }
    }
    if (currentVariableRangeNumber < num) {
        while (currentVariableRangeNumber < num) {
            /// add a range.
            rangeInfo.dimensionInfo (dimNumber).push_back (defaultRangeInfo);
            for (unsigned varInc = 0; varInc < parameters.size (); varInc++) {
                indvParamRangeInfo tempInfo{ 0,0 };
                parameters[varInc].ranges.push_back (tempInfo);
            }
            currentVariableRangeNumber++;
        }
    }
    else if (currentVariableRangeNumber > num) {
        while (currentVariableRangeNumber > num) {
            // delete a range.
            if (rangeInfo.dimensionInfo (dimNumber).size () == 1) {
                return;	// can't delete last range
            }
            for (auto& param : parameters) {
                param.ranges.pop_back ();
            }
            rangeInfo.dimensionInfo (dimNumber).pop_back ();
            currentVariableRangeNumber--;
        }
    }
    setParams (parameters);
    setRangeInfo (rangeInfo);
}