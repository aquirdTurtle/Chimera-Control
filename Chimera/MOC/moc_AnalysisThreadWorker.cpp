/****************************************************************************
** Meta object code from reading C++ file 'AnalysisThreadWorker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Source/RealTimeDataAnalysis/AnalysisThreadWorker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AnalysisThreadWorker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AnalysisThreadWorker_t {
    QByteArrayData data[12];
    char stringdata0[161];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AnalysisThreadWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AnalysisThreadWorker_t qt_meta_stringdata_AnalysisThreadWorker = {
    {
QT_MOC_LITERAL(0, 0, 20), // "AnalysisThreadWorker"
QT_MOC_LITERAL(1, 21, 11), // "newPlotData"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 36), // "std::vector<std::vector<dataP..."
QT_MOC_LITERAL(4, 71, 4), // "init"
QT_MOC_LITERAL(5, 76, 12), // "handleNewPic"
QT_MOC_LITERAL(6, 89, 9), // "atomQueue"
QT_MOC_LITERAL(7, 99, 12), // "handleNewPix"
QT_MOC_LITERAL(8, 112, 12), // "PixListQueue"
QT_MOC_LITERAL(9, 125, 7), // "pixlist"
QT_MOC_LITERAL(10, 133, 7), // "setXpts"
QT_MOC_LITERAL(11, 141, 19) // "std::vector<double>"

    },
    "AnalysisThreadWorker\0newPlotData\0\0"
    "std::vector<std::vector<dataPoint> >\0"
    "init\0handleNewPic\0atomQueue\0handleNewPix\0"
    "PixListQueue\0pixlist\0setXpts\0"
    "std::vector<double>"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AnalysisThreadWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   44,    2, 0x0a /* Public */,
       5,    1,   45,    2, 0x0a /* Public */,
       7,    1,   48,    2, 0x0a /* Public */,
      10,    1,   51,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    2,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    2,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,    2,

       0        // eod
};

void AnalysisThreadWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AnalysisThreadWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newPlotData((*reinterpret_cast< std::vector<std::vector<dataPoint> >(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->init(); break;
        case 2: _t->handleNewPic((*reinterpret_cast< atomQueue(*)>(_a[1]))); break;
        case 3: _t->handleNewPix((*reinterpret_cast< PixListQueue(*)>(_a[1]))); break;
        case 4: _t->setXpts((*reinterpret_cast< std::vector<double>(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< atomQueue >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< PixListQueue >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AnalysisThreadWorker::*)(std::vector<std::vector<dataPoint>> , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnalysisThreadWorker::newPlotData)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AnalysisThreadWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AnalysisThreadWorker.data,
    qt_meta_data_AnalysisThreadWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AnalysisThreadWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AnalysisThreadWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AnalysisThreadWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AnalysisThreadWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void AnalysisThreadWorker::newPlotData(std::vector<std::vector<dataPoint>> _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
