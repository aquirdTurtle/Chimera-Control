/****************************************************************************
** Meta object code from reading C++ file 'AtomCruncherWorker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Source/RealTimeDataAnalysis/AtomCruncherWorker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AtomCruncherWorker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CruncherThreadWorker_t {
    QByteArrayData data[13];
    char stringdata0[121];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CruncherThreadWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CruncherThreadWorker_t qt_meta_stringdata_CruncherThreadWorker = {
    {
QT_MOC_LITERAL(0, 0, 20), // "CruncherThreadWorker"
QT_MOC_LITERAL(1, 21, 5), // "error"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 6), // "errstr"
QT_MOC_LITERAL(4, 35, 9), // "atomArray"
QT_MOC_LITERAL(5, 45, 9), // "atomQueue"
QT_MOC_LITERAL(6, 55, 6), // "aqueue"
QT_MOC_LITERAL(7, 62, 8), // "pixArray"
QT_MOC_LITERAL(8, 71, 12), // "PixListQueue"
QT_MOC_LITERAL(9, 84, 7), // "pixlist"
QT_MOC_LITERAL(10, 92, 4), // "init"
QT_MOC_LITERAL(11, 97, 11), // "handleImage"
QT_MOC_LITERAL(12, 109, 11) // "NormalImage"

    },
    "CruncherThreadWorker\0error\0\0errstr\0"
    "atomArray\0atomQueue\0aqueue\0pixArray\0"
    "PixListQueue\0pixlist\0init\0handleImage\0"
    "NormalImage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CruncherThreadWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    1,   42,    2, 0x06 /* Public */,
       7,    1,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   48,    2, 0x0a /* Public */,
      11,    1,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12,    2,

       0        // eod
};

void CruncherThreadWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CruncherThreadWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->error((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->atomArray((*reinterpret_cast< atomQueue(*)>(_a[1]))); break;
        case 2: _t->pixArray((*reinterpret_cast< PixListQueue(*)>(_a[1]))); break;
        case 3: _t->init(); break;
        case 4: _t->handleImage((*reinterpret_cast< NormalImage(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< atomQueue >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< PixListQueue >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< NormalImage >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CruncherThreadWorker::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CruncherThreadWorker::error)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CruncherThreadWorker::*)(atomQueue );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CruncherThreadWorker::atomArray)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CruncherThreadWorker::*)(PixListQueue );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CruncherThreadWorker::pixArray)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CruncherThreadWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CruncherThreadWorker.data,
    qt_meta_data_CruncherThreadWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CruncherThreadWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CruncherThreadWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CruncherThreadWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CruncherThreadWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void CruncherThreadWorker::error(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CruncherThreadWorker::atomArray(atomQueue _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CruncherThreadWorker::pixArray(PixListQueue _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
