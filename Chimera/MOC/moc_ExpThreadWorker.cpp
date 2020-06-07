/****************************************************************************
** Meta object code from reading C++ file 'ExpThreadWorker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Source/ExperimentThread/ExpThreadWorker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ExpThreadWorker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ExpThreadWorker_t {
    QByteArrayData data[14];
    char stringdata0[161];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ExpThreadWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ExpThreadWorker_t qt_meta_stringdata_ExpThreadWorker = {
    {
QT_MOC_LITERAL(0, 0, 15), // "ExpThreadWorker"
QT_MOC_LITERAL(1, 16, 14), // "updateBoxColor"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 6), // "notify"
QT_MOC_LITERAL(4, 39, 4), // "warn"
QT_MOC_LITERAL(5, 44, 9), // "debugInfo"
QT_MOC_LITERAL(6, 54, 9), // "repUpdate"
QT_MOC_LITERAL(7, 64, 12), // "prepareAndor"
QT_MOC_LITERAL(8, 77, 13), // "prepareBasler"
QT_MOC_LITERAL(9, 91, 8), // "doAoData"
QT_MOC_LITERAL(10, 100, 38), // "std::vector<std::vector<plotD..."
QT_MOC_LITERAL(11, 139, 6), // "doData"
QT_MOC_LITERAL(12, 146, 6), // "aoData"
QT_MOC_LITERAL(13, 153, 7) // "process"

    },
    "ExpThreadWorker\0updateBoxColor\0\0notify\0"
    "warn\0debugInfo\0repUpdate\0prepareAndor\0"
    "prepareBasler\0doAoData\0"
    "std::vector<std::vector<plotDataVec> >\0"
    "doData\0aoData\0process"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ExpThreadWorker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   59,    2, 0x06 /* Public */,
       3,    1,   64,    2, 0x06 /* Public */,
       4,    1,   67,    2, 0x06 /* Public */,
       5,    1,   70,    2, 0x06 /* Public */,
       6,    1,   73,    2, 0x06 /* Public */,
       7,    1,   76,    2, 0x06 /* Public */,
       8,    0,   79,    2, 0x06 /* Public */,
       9,    2,   80,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    0,   85,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::UInt,    2,
    QMetaType::Void, QMetaType::VoidStar,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 10,   11,   12,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void ExpThreadWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ExpThreadWorker *_t = static_cast<ExpThreadWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateBoxColor((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->notify((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->warn((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->debugInfo((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->repUpdate((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 5: _t->prepareAndor((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 6: _t->prepareBasler(); break;
        case 7: _t->doAoData((*reinterpret_cast< const std::vector<std::vector<plotDataVec> >(*)>(_a[1])),(*reinterpret_cast< const std::vector<std::vector<plotDataVec> >(*)>(_a[2]))); break;
        case 8: _t->process(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (ExpThreadWorker::*_t)(QString , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::updateBoxColor)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::notify)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::warn)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::debugInfo)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)(unsigned int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::repUpdate)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)(void * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::prepareAndor)) {
                *result = 5;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::prepareBasler)) {
                *result = 6;
                return;
            }
        }
        {
            typedef void (ExpThreadWorker::*_t)(const std::vector<std::vector<plotDataVec>> & , const std::vector<std::vector<plotDataVec>> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExpThreadWorker::doAoData)) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject ExpThreadWorker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ExpThreadWorker.data,
      qt_meta_data_ExpThreadWorker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ExpThreadWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExpThreadWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ExpThreadWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ExpThreadWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void ExpThreadWorker::updateBoxColor(QString _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ExpThreadWorker::notify(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ExpThreadWorker::warn(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ExpThreadWorker::debugInfo(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ExpThreadWorker::repUpdate(unsigned int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ExpThreadWorker::prepareAndor(void * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ExpThreadWorker::prepareBasler()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ExpThreadWorker::doAoData(const std::vector<std::vector<plotDataVec>> & _t1, const std::vector<std::vector<plotDataVec>> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
