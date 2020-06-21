/****************************************************************************
** Meta object code from reading C++ file 'QtAuxiliaryWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Source/PrimaryWindows/QtAuxiliaryWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtAuxiliaryWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtAuxiliaryWindow_t {
    QByteArrayData data[6];
    char stringdata0[91];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtAuxiliaryWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtAuxiliaryWindow_t qt_meta_stringdata_QtAuxiliaryWindow = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QtAuxiliaryWindow"
QT_MOC_LITERAL(1, 18, 18), // "handleDoAoPlotData"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 38), // "std::vector<std::vector<plotD..."
QT_MOC_LITERAL(4, 77, 6), // "doData"
QT_MOC_LITERAL(5, 84, 6) // "aoData"

    },
    "QtAuxiliaryWindow\0handleDoAoPlotData\0"
    "\0std::vector<std::vector<plotDataVec> >\0"
    "doData\0aoData"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtAuxiliaryWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,

       0        // eod
};

void QtAuxiliaryWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QtAuxiliaryWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->handleDoAoPlotData((*reinterpret_cast< const std::vector<std::vector<plotDataVec> >(*)>(_a[1])),(*reinterpret_cast< const std::vector<std::vector<plotDataVec> >(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QtAuxiliaryWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<IChimeraWindowWidget::staticMetaObject>(),
    qt_meta_stringdata_QtAuxiliaryWindow.data,
    qt_meta_data_QtAuxiliaryWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtAuxiliaryWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtAuxiliaryWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtAuxiliaryWindow.stringdata0))
        return static_cast<void*>(this);
    return IChimeraWindowWidget::qt_metacast(_clname);
}

int QtAuxiliaryWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IChimeraWindowWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
