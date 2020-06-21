/****************************************************************************
** Meta object code from reading C++ file 'ScopeThreadWorker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Source/Plotting/ScopeThreadWorker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScopeThreadWorker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ScopeThreadWorker_t {
    QByteArrayData data[13];
    char stringdata0[99];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ScopeThreadWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ScopeThreadWorker_t qt_meta_stringdata_ScopeThreadWorker = {
    {
QT_MOC_LITERAL(0, 0, 17), // "ScopeThreadWorker"
QT_MOC_LITERAL(1, 18, 6), // "notify"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 7), // "newData"
QT_MOC_LITERAL(4, 34, 15), // "QVector<double>"
QT_MOC_LITERAL(5, 50, 5), // "xdata"
QT_MOC_LITERAL(6, 56, 4), // "xmin"
QT_MOC_LITERAL(7, 61, 4), // "xmax"
QT_MOC_LITERAL(8, 66, 5), // "ydata"
QT_MOC_LITERAL(9, 72, 4), // "ymin"
QT_MOC_LITERAL(10, 77, 4), // "ymax"
QT_MOC_LITERAL(11, 82, 8), // "traceNum"
QT_MOC_LITERAL(12, 91, 7) // "process"

    },
    "ScopeThreadWorker\0notify\0\0newData\0"
    "QVector<double>\0xdata\0xmin\0xmax\0ydata\0"
    "ymin\0ymax\0traceNum\0process"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ScopeThreadWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       3,    7,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Double, QMetaType::Double, 0x80000000 | 4, QMetaType::Double, QMetaType::Double, QMetaType::Int,    5,    6,    7,    8,    9,   10,   11,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void ScopeThreadWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScopeThreadWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->notify((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->newData((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< const QVector<double>(*)>(_a[4])),(*reinterpret_cast< double(*)>(_a[5])),(*reinterpret_cast< double(*)>(_a[6])),(*reinterpret_cast< int(*)>(_a[7]))); break;
        case 2: _t->process(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 3:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ScopeThreadWorker::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ScopeThreadWorker::notify)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ScopeThreadWorker::*)(const QVector<double> & , double , double , const QVector<double> & , double , double , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ScopeThreadWorker::newData)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ScopeThreadWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ScopeThreadWorker.data,
    qt_meta_data_ScopeThreadWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ScopeThreadWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScopeThreadWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ScopeThreadWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ScopeThreadWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void ScopeThreadWorker::notify(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ScopeThreadWorker::newData(const QVector<double> & _t1, double _t2, double _t3, const QVector<double> & _t4, double _t5, double _t6, int _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
