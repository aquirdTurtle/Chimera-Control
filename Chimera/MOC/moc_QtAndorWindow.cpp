/****************************************************************************
** Meta object code from reading C++ file 'QtAndorWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Source/PrimaryWindows/QtAndorWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtAndorWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtAndorWindow_t {
    QByteArrayData data[15];
    char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtAndorWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtAndorWindow_t qt_meta_stringdata_QtAndorWindow = {
    {
QT_MOC_LITERAL(0, 0, 13), // "QtAndorWindow"
QT_MOC_LITERAL(1, 14, 16), // "onCameraProgress"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 6), // "picNum"
QT_MOC_LITERAL(4, 39, 14), // "onCameraFinish"
QT_MOC_LITERAL(5, 54, 7), // "LRESULT"
QT_MOC_LITERAL(6, 62, 6), // "WPARAM"
QT_MOC_LITERAL(7, 69, 6), // "wParam"
QT_MOC_LITERAL(8, 76, 6), // "LPARAM"
QT_MOC_LITERAL(9, 83, 6), // "lParam"
QT_MOC_LITERAL(10, 90, 17), // "onCameraCalFinish"
QT_MOC_LITERAL(11, 108, 19), // "onCameraCalProgress"
QT_MOC_LITERAL(12, 128, 14), // "onBaslerFinish"
QT_MOC_LITERAL(13, 143, 19), // "handlePrepareForAcq"
QT_MOC_LITERAL(14, 163, 6) // "lparam"

    },
    "QtAndorWindow\0onCameraProgress\0\0picNum\0"
    "onCameraFinish\0LRESULT\0WPARAM\0wParam\0"
    "LPARAM\0lParam\0onCameraCalFinish\0"
    "onCameraCalProgress\0onBaslerFinish\0"
    "handlePrepareForAcq\0lparam"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtAndorWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x0a /* Public */,
       4,    2,   47,    2, 0x0a /* Public */,
      10,    2,   52,    2, 0x0a /* Public */,
      11,    2,   57,    2, 0x0a /* Public */,
      12,    2,   62,    2, 0x0a /* Public */,
      13,    1,   67,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    0x80000000 | 5, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    0x80000000 | 5, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    0x80000000 | 5, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    0x80000000 | 5, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    QMetaType::Void, QMetaType::VoidStar,   14,

       0        // eod
};

void QtAndorWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtAndorWindow *_t = static_cast<QtAndorWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onCameraProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: { LRESULT _r = _t->onCameraFinish((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 2: { LRESULT _r = _t->onCameraCalFinish((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 3: { LRESULT _r = _t->onCameraCalProgress((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 4: { LRESULT _r = _t->onBaslerFinish((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->handlePrepareForAcq((*reinterpret_cast< void*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QtAndorWindow::staticMetaObject = {
    { &IChimeraWindowWidget::staticMetaObject, qt_meta_stringdata_QtAndorWindow.data,
      qt_meta_data_QtAndorWindow,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *QtAndorWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtAndorWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtAndorWindow.stringdata0))
        return static_cast<void*>(this);
    return IChimeraWindowWidget::qt_metacast(_clname);
}

int QtAndorWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IChimeraWindowWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
