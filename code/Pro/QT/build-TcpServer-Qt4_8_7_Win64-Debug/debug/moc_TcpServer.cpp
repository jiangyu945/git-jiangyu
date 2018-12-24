/****************************************************************************
** Meta object code from reading C++ file 'mymonitor.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Myonitor/mymonitor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mymonitor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Mymonitor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      40,   10,   10,   10, 0x08,
      65,   10,   10,   10, 0x08,
      86,   10,   10,   10, 0x08,
     110,   10,   10,   10, 0x08,
     131,   10,   10,   10, 0x08,
     168,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Mymonitor[] = {
    "Mymonitor\0\0on_pushButton_Bind_clicked()\0"
    "doProcessNewConnection()\0doProcessConnected()\0"
    "doProcessDisconnected()\0doProcessreadyRead()\0"
    "on_pushButton_sendToClient_clicked()\0"
    "on_pushButton_clearRecv_clicked()\0"
};

void Mymonitor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Mymonitor *_t = static_cast<Mymonitor *>(_o);
        switch (_id) {
        case 0: _t->on_pushButton_Bind_clicked(); break;
        case 1: _t->doProcessNewConnection(); break;
        case 2: _t->doProcessConnected(); break;
        case 3: _t->doProcessDisconnected(); break;
        case 4: _t->doProcessreadyRead(); break;
        case 5: _t->on_pushButton_sendToClient_clicked(); break;
        case 6: _t->on_pushButton_clearRecv_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Mymonitor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Mymonitor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Mymonitor,
      qt_meta_data_Mymonitor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Mymonitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Mymonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Mymonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Mymonitor))
        return static_cast<void*>(const_cast< Mymonitor*>(this));
    return QWidget::qt_metacast(_clname);
}

int Mymonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
