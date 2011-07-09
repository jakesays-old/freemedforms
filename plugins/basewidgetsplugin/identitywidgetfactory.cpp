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
#include "identitywidgetfactory.h"

#include <patientbaseplugin/identitywidget.h>
#include <patientbaseplugin/patientmodel.h>
#include <patientbaseplugin/patientwidgetmanager.h>
#include <patientbaseplugin/patientselector.h>

#include <formmanagerplugin/iformitem.h>

#include <coreplugin/icore.h>
#include <coreplugin/ipatient.h>

#include <translationutils/constanttranslations.h>

using namespace BaseWidgets;
using namespace Trans::ConstantTranslations;

static inline Core::IPatient *patient() {return Core::ICore::instance()->patient();}

IdentityWidgetFactory::IdentityWidgetFactory(QObject *parent) :
        IFormWidgetFactory(parent)
{
}

IdentityWidgetFactory::~IdentityWidgetFactory()
{}

bool IdentityWidgetFactory::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    return true;
}

bool IdentityWidgetFactory::extensionInitialized()
{
    return true;
}

bool IdentityWidgetFactory::isInitialized() const
{
    return true;
}

QStringList IdentityWidgetFactory::providedWidgets() const
{
    return QStringList() << "identity" << "ident";
}

bool IdentityWidgetFactory::isContainer(const int idInStringList) const
{
    Q_UNUSED(idInStringList);
    return true;
}

Form::IFormWidget *IdentityWidgetFactory::createWidget(const QString &name, Form::FormItem *formItem, QWidget *parent)
{
    return new IdentityFormWidget(formItem,parent);
}


inline static int getNumberOfColumns(Form::FormItem *item, int defaultValue = 1)
{
    if (!item->extraDatas().value("column").isEmpty())
        return item->extraDatas().value("column").toInt();
    else
        return defaultValue;
}

IdentityFormWidget::IdentityFormWidget(Form::FormItem *formItem, QWidget *parent)
        : Form::IFormWidget(formItem,parent), m_ContainerLayout(0)
{
    setObjectName("IdentityFormWidget");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    QWidget *mainWidget = new QWidget;
    mainLayout->addWidget(mainWidget);
    mainLayout->addStretch();

    m_ContainerLayout = new QGridLayout(mainWidget);
    m_ContainerLayout->setMargin(0);
    m_ContainerLayout->setSpacing(0);
//    IFormWidget::createLabel(tkTr(Trans::Constants::IDENTITY_TEXT), Qt::AlignCenter);
//    m_Label->setFrameStyle(FormLabelFrame);
//    QFont font = m_Label->font();
//    font.setBold(true);
//    m_Label->setFont(font);
//    QSizePolicy sizePolicy = m_Label->sizePolicy();
//    sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
//    m_Label->setSizePolicy(sizePolicy);

    // Retrieve the number of columns for the gridlayout (lays in extraDatas() of linked FormItem)
    numberColumns = getNumberOfColumns(m_FormItem);
//    if (isCompactView(m_FormItem)) {
//        mainLayout->setMargin(0);
//        mainLayout->setSpacing(2);
//        m_ContainerLayout->setMargin(0);
//        m_ContainerLayout->setSpacing(2);
//    }
//    m_ContainerLayout->addWidget(m_Label, 0, 0,  1, numberColumns);
    if (formItem->extraDatas().value("option", QString()).compare("readonly", Qt::CaseInsensitive) == 0)
        m_Identity = new Patients::IdentityWidget(this);
    else
        m_Identity = new Patients::IdentityWidget(this, Patients::IdentityWidget::ReadWriteMode);
    m_Identity->setCurrentPatientModel(Patients::PatientModel::activeModel());
    m_ContainerLayout->addWidget(m_Identity, 1, 0,  1, numberColumns);
    i = numberColumns;
    row = 1;
    col = 0;

    connect(patient(), SIGNAL(currentPatientChanged()), this, SLOT(onCurrentPatientChanged()));

    // create itemdata
    IdentityWidgetData *datas = new IdentityWidgetData(m_FormItem);
    datas->setIdentityFormWiget(this);
    m_FormItem->setItemDatas(datas);
}

IdentityFormWidget::~IdentityFormWidget()
{
}

void IdentityFormWidget::addWidgetToContainer(IFormWidget *widget)
{
    if (!widget)
        return;
    if (!m_ContainerLayout)
        return;
    col = (i % numberColumns);
    row = (i / numberColumns);
    m_ContainerLayout->addWidget(widget , row, col);
    i++;
}

void IdentityFormWidget::onCurrentPatientChanged()
{
    m_Identity->setCurrentIndex(patient()->currentPatientIndex());
}

void IdentityFormWidget::retranslate()
{
    /** \todo iformitem --> one spec per language ? */
    //     m_Label->setText(m_FormItem->spec()->label());
}



bool IdentityWidgetData::isModified() const
{
    return m_Widget->m_Identity->isModified();
}

QVariant IdentityWidgetData::storableData() const
{
    m_Widget->m_Identity->submit(); return QVariant();
}
