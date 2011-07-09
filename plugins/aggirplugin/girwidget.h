/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef GIRWIDGET_H
#define GIRWIDGET_H

#include <aggirplugin/aggir_exporter.h>

#include <formmanagerplugin/iformwidgetfactory.h>
#include <formmanagerplugin/iformitemdata.h>

#include <QtGui/QWidget>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QAbstractButton;
QT_END_NAMESPACE

/**
 * \file girwidget.h
 * \author Eric MAEKER <eric.maeker@gmail.com>
 * \version 0.5.0
 * \date 10 Apr 2011
*/

namespace Gir {
class GirModel;

namespace Internal {

class GirWidgetFactory : public Form::IFormWidgetFactory
{
    Q_OBJECT
public:
    GirWidgetFactory(QObject *parent = 0);
    ~GirWidgetFactory() {}

    bool initialize(const QStringList &, QString *) {return true;}
    bool extensionInitialized() {return true;}
    bool isInitialized() const {return true;}

    bool isContainer(const int idInStringList) const {Q_UNUSED(idInStringList); return false;}
    QStringList providedWidgets() const {return QStringList() << "aggir" << "gir";}
    Form::IFormWidget *createWidget(const QString &name, Form::FormItem *object, QWidget *parent = 0);
};

//--------------------------------------------------------------------------------------------------------
//--------------------------------------- GirWidget implementation ---------------------------------------
//--------------------------------------------------------------------------------------------------------
namespace Ui {
class GirWidget;
class GirItem;
}

class GirUi : public QWidget
{
    Q_OBJECT
public:
    GirUi(QWidget *parent = 0);
    ~GirUi();

    void setStringfiedGirScore(const QString &gir);
    QString stringfiedGirScore() const;

public Q_SLOTS:
    void updateGirString(QAbstractButton *radio);
    void on_aButton_clicked();
    void on_bButton_clicked();
    void on_cButton_clicked();

private Q_SLOTS:
    void girCalculated(const int gir);

private:
    Ui::GirWidget *m_ui;
    QString m_GirString;
    int m_Gir;
    GirModel *model;
};

class GirWidget : public Form::IFormWidget
{
    Q_OBJECT
public:
    GirWidget(Form::FormItem *linkedObject, QWidget *parent = 0);
    ~GirWidget();

    bool isContainer() const {return false;}

    void setValue(const QVariant &);
    QVariant value() const;

    void setStringfiedGirScore(const QString &gir);
    QString stringfiedGirScore() const;

public Q_SLOTS:
    void retranslate() {}

private:
    GirUi *m_ui;
};

class AGGIR_EXPORT GirItemData : public Form::IFormItemData
{
public:
    explicit GirItemData(Form::FormItem *parent) : Form::IFormItemData(), m_Parent(parent), m_GirWidget(0) {}
    virtual ~GirItemData() {}
    virtual Form::FormItem *parentItem() const {return m_Parent;}

    bool isModified() const;
    void clear();

    void setGirWidget(GirWidget *widget) {m_GirWidget = widget;}

    virtual bool setData(const int ref, const QVariant &data, const int role);
    virtual QVariant data(const int ref, const int role) const;

    virtual void setStorableData(const QVariant &data);
    virtual QVariant storableData() const;

private:
    Form::FormItem *m_Parent;
    QString m_OriginalValue;
    GirWidget *m_GirWidget;
};

} // End Internal
} // End Gir

#endif // GIRWIDGET_H
