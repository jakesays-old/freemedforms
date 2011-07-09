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
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef IFORMITEMDATA_H
#define IFORMITEMDATA_H

#include <formmanagerplugin/formmanager_exporter.h>

#include <QtCore/QVariant>

namespace Form {
class FormItem;

class FORM_EXPORT IFormItemData : public QObject
{
    Q_OBJECT
public:
    enum RoleRepresentation {
        ID_EpisodeDate = 0,  // used by BaseFormData to set episode date
        ID_EpisodeLabel,     // used by BaseFormData to set episode label
        ID_UserName,         // used by BaseFormData to set episode label
        ID_ForPatientModel,
        ID_ForCalculations
    };

    IFormItemData() {}
    virtual ~IFormItemData() {}

    virtual void clear() = 0;

    virtual Form::FormItem *parentItem() const = 0;
    virtual bool isModified() const = 0;

    // ref makes references to patient's data -> Core::IPatient
    virtual bool setData(const int ref, const QVariant &data, const int role) = 0;
    virtual QVariant data(const int ref, const int role) const = 0;

    virtual void setStorableData(const QVariant &data) = 0;
    virtual QVariant storableData() const = 0;

Q_SIGNALS:
    void dataChanged(const int ref);

};
} // namespace Form

//Q_DECLARE_METATYPE(Form::IFormItemData*);

#endif // IFORMITEMDATA_H
