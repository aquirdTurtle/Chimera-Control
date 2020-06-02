/****************************************************************************
** Meta object code from reading C++ file 'QtMainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Source/PrimaryWindows/QtMainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtMainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtMainWindow_t {
    QByteArrayData data[9];
    char stringdata0[100];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtMainWindow_t qt_meta_stringdata_QtMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 12), // "QtMainWindow"
QT_MOC_LITERAL(1, 13, 20), // "handleColorboxUpdate"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 5), // "color"
QT_MOC_LITERAL(4, 41, 11), // "systemDelim"
QT_MOC_LITERAL(5, 53, 21), // "handleExpNotification"
QT_MOC_LITERAL(6, 75, 3), // "txt"
QT_MOC_LITERAL(7, 79, 13), // "onRepProgress"
QT_MOC_LITERAL(8, 93, 6) // "repNum"

    },
    "QtMainWindow\0handleColorboxUpdate\0\0"
    "color\0systemDelim\0handleExpNotification\0"
    "txt\0onRepProgress\0repNum"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtMainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   29,    2, 0x0a /* Public */,
       5,    1,   34,    2, 0x0a /* Public */,
       7,    1,   37,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::UInt,    8,

       0        // eod
};

void QtMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtMainWindow *_t = static_cast<QtMainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->handleColorboxUpdate((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->handleExpNotification((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->onRepProgress((*reinterpret_cast< uint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtMainWindow::staticMetaObject = {
    { &IChimeraWindowWidget::staticMetaObject, qt_meta_stringdata_QtMainWindow.data,
      qt_meta_data_QtMainWindow,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *QtMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtMainWindow.stringdata0))
        return static_cast<void*>(this);
    return IChimeraWindowWidget::qt_metacast(_clname);
}

int QtMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IChimeraWindowWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
