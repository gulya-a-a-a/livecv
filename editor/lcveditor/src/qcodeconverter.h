/****************************************************************************
**
** Copyright (C) 2014-2017 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#ifndef QCODECONVERTER_H
#define QCODECONVERTER_H

#include <QObject>
#include "qlcveditorglobal.h"
#include "qabstractcodeserializer.h"

namespace lcv{

class Q_LCVEDITOR_EXPORT QCodeConverter : public QObject{

    Q_OBJECT
    Q_PROPERTY(QAbstractCodeSerializer* serialize READ serialize  WRITE setSerialize  NOTIFY serializeChanged)
    Q_PROPERTY(QString type                       READ type       WRITE setType       NOTIFY typeChanged)
    Q_PROPERTY(QString typeObject                 READ typeObject WRITE setTypeObject NOTIFY typeObjectChanged)

public:
    explicit QCodeConverter(QObject *parent = 0);
    virtual ~QCodeConverter();

    QAbstractCodeSerializer* serialize();
    void setSerialize(QAbstractCodeSerializer* serialize);

    void setType(const QString& type);
    const QString& type() const;

    void setTypeObject(const QString& typeObject);
    const QString& typeObject() const;

signals:
    void serializeChanged();
    void typeChanged();
    void typeObjectChanged();

private:
    Q_DISABLE_COPY(QCodeConverter)

    QAbstractCodeSerializer* m_serialize;
    QString m_type;
    QString m_typeObject;
};

inline QAbstractCodeSerializer *QCodeConverter::serialize(){
    return m_serialize;
}

inline const QString &QCodeConverter::type() const{
    return m_type;
}

inline const QString &QCodeConverter::typeObject() const{
    return m_typeObject;
}

inline void QCodeConverter::setSerialize(QAbstractCodeSerializer *serialize){
    if (m_serialize == serialize)
        return;

    m_serialize = serialize;
    emit serializeChanged();
}

inline void QCodeConverter::setType(const QString &type){
    if (m_type == type)
        return;

    m_type = type;
    emit typeChanged();
}

inline void QCodeConverter::setTypeObject(const QString &typeObject){
    if (m_typeObject == typeObject)
        return;

    m_typeObject = typeObject;
    emit typeObjectChanged();
}

}// namespace

#endif // QCODECONVERTER_H
