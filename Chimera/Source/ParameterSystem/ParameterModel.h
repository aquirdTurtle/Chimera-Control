#pragma once

#include <QAbstractTableModel>
#include "ParameterSystemStructures.h"

class ParameterModel : public QAbstractTableModel
{
    Q_OBJECT
    public:
        ParameterModel (bool isGlobal, QObject* parent = nullptr);
        int rowCount (const QModelIndex& parent = QModelIndex ()) const override;
        int columnCount (const QModelIndex& parent = QModelIndex ()) const override;
        QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData (int section, Qt::Orientation orientation, int role) const;
        Qt::ItemFlags flags (const QModelIndex& index) const override;
        bool setData (const QModelIndex& index, const QVariant& value, int role);
        void setParams (std::vector<parameterType> newParams); 
        std::vector<parameterType> getParams ();
        ScanRangeInfo getRangeInfo ();
        void setRangeInfo (ScanRangeInfo info);
        const USHORT preRangeColumns = 5;
        void checkScanDimensionConsistency ();
        void checkVariationRangeConsistency ();
        void setVariationRangeNumber (int num, USHORT dimNumber);
        const bool isGlobal;
    private:
        const IndvRangeInfo defaultRangeInfo = { 2,false,true };
        std::vector<parameterType> parameters;
        ScanRangeInfo rangeInfo;
    signals:
        void paramsChanged ();
        void editCompleted (const QString&);
};