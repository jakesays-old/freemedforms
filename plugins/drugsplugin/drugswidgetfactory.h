/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                      *
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef DRUGSWIDGETFACTORY_H
#define DRUGSWIDGETFACTORY_H

namespace DrugsDB {
class DrugsModel;
}

// include Qt headers
#include <QWidget>
QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

#include <formmanagerplugin/iformwidgetfactory.h>


/**
 * \file drugswidget.h
 * \author Eric MAEKER <eric.maeker@free.fr>
 * \version 0.4.0
 * \date 05 Apr 2010
*/

namespace DrugsWidget {
namespace Internal {

class DrugsWidgetsFactory : public Form::IFormWidgetFactory
{
    Q_OBJECT
public:
    DrugsWidgetsFactory(QObject *parent = 0);
    ~DrugsWidgetsFactory();

    bool initialize(const QStringList &arguments, QString *errorString);
    bool extensionInitialized();

    bool isContainer( const int idInStringList ) const;
    QStringList providedWidgets() const;
    bool isInitialized() const { /** \todo here */ return true; }

    Form::IFormWidget *createWidget(const QString &name, Form::FormItem *linkedObject, QWidget *parent = 0);
};


//--------------------------------------------------------------------------------------------------------
//------------------------------------ mfDrugsWidget implementation --------------------------------------
//--------------------------------------------------------------------------------------------------------
class DrugsPrescriptorWidget : public Form::IFormWidget
{
    Q_OBJECT
public:
    DrugsPrescriptorWidget(Form::FormItem *linkedObject, QWidget *parent);
    ~DrugsPrescriptorWidget();

private:
    DrugsDB::DrugsModel *m_PrescriptionModel;
    QString     m_iniPath;
    bool        m_WithPrescribing, m_WithPrinting;
};

}  // End Internal
}  // End DrugsWidget

#endif  // DRUGSWIDGETFACTORY_H
