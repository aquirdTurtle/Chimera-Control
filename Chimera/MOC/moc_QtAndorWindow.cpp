/****************************************************************************
** Meta object code from reading C++ file 'QtAndorWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Source/PrimaryWindows/QtAndorWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtAndorWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtAndorWindow_t {
    QByteArrayData data[20];
    char stringdata0[259];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtAndorWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtAndorWindow_t qt_meta_stringdata_QtAndorWindow = {
    {
QT_MOC_LITERAL(0, 0, 13), // "QtAndorWindow"
QT_MOC_LITERAL(1, 14, 8), // "newImage"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 11), // "NormalImage"
QT_MOC_LITERAL(4, 36, 16), // "onCameraProgress"
QT_MOC_LITERAL(5, 53, 6), // "picNum"
QT_MOC_LITERAL(6, 60, 17), // "handleNewPlotData"
QT_MOC_LITERAL(7, 78, 36), // "std::vector<std::vector<dataP..."
QT_MOC_LITERAL(8, 115, 12), // "PlottingInfo"
QT_MOC_LITERAL(9, 128, 14), // "onCameraFinish"
QT_MOC_LITERAL(10, 143, 7), // "LRESULT"
QT_MOC_LITERAL(11, 151, 6), // "WPARAM"
QT_MOC_LITERAL(12, 158, 6), // "wParam"
QT_MOC_LITERAL(13, 165, 6), // "LPARAM"
QT_MOC_LITERAL(14, 172, 6), // "lParam"
QT_MOC_LITERAL(15, 179, 17), // "onCameraCalFinish"
QT_MOC_LITERAL(16, 197, 19), // "onCameraCalProgress"
QT_MOC_LITERAL(17, 217, 14), // "onBaslerFinish"
QT_MOC_LITERAL(18, 232, 19), // "handlePrepareForAcq"
QT_MOC_LITERAL(19, 252, 6) // "lparam"

    },
    "QtAndorWindow\0newImage\0\0NormalImage\0"
    "onCameraProgress\0picNum\0handleNewPlotData\0"
    "std::vector<std::vector<dataPoint> >\0"
    "PlottingInfo\0onCameraFinish\0LRESULT\0"
    "WPARAM\0wParam\0LPARAM\0lParam\0"
    "onCameraCalFinish\0onCameraCalProgress\0"
    "onBaslerFinish\0handlePrepareForAcq\0"
    "lparam"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtAndorWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   57,    2, 0x0a /* Public */,
       6,    2,   60,    2, 0x0a /* Public */,
       9,    2,   65,    2, 0x0a /* Public */,
      15,    2,   70,    2, 0x0a /* Public */,
      16,    2,   75,    2, 0x0a /* Public */,
      17,    2,   80,    2, 0x0a /* Public */,
      18,    1,   85,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 8,    2,    2,
    0x80000000 | 10, 0x80000000 | 11, 0x80000000 | 13,   12,   14,
    0x80000000 | 10, 0x80000000 | 11, 0x80000000 | 13,   12,   14,
    0x80000000 | 10, 0x80000000 | 11, 0x80000000 | 13,   12,   14,
    0x80000000 | 10, 0x80000000 | 11, 0x80000000 | 13,   12,   14,
    QMetaType::Void, QMetaType::VoidStar,   19,

       0        // eod
};

void QtAndorWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QtAndorWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newImage((*reinterpret_cast< NormalImage(*)>(_a[1]))); break;
        case 1: _t->onCameraProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->handleNewPlotData((*reinterpret_cast< std::vector<std::vector<dataPoint> >(*)>(_a[1])),(*reinterpret_cast< PlottingInfo(*)>(_a[2]))); break;
        case 3: { LRESULT _r = _t->onCameraFinish((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 4: { LRESULT _r = _t->onCameraCalFinish((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 5: { LRESULT _r = _t->onCameraCalProgress((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 6: { LRESULT _r = _t->onBaslerFinish((*reinterpret_cast< WPARAM(*)>(_a[1])),(*reinterpret_cast< LPARAM(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< LRESULT*>(_a[0]) = std::move(_r); }  break;
        case 7: _t->handlePrepareForAcq((*reinterpret_cast< void*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< NormalImage >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< PlottingInfo >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QtAndorWindow::*)(NormalImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QtAndorWindow::newImage)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QtAndorWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<IChimeraWindowWidget::staticMetaObject>(),
    qt_meta_stringdata_QtAndorWindow.data,
    qt_meta_data_QtAndorWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


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
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtAndorWindow::newImage(NormalImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
