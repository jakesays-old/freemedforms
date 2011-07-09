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
#ifndef XMLFORMIO_H
#define XMLFORMIO_H

#include <formmanagerplugin/iformio.h>

#include <QDomNode>
#include <QObject>
#include <QCache>

/**
 * \file xmlformio.h
 * \author Eric MAEKER <eric.maeker@gmail.com>
 * \version 0.6.0
 * \date 10 May 2011
*/

namespace Category {
class CategoryItem;
}

namespace Form {
class FormItem;
class FormMain;
class IFormWidgetFactory;
}

namespace XmlForms {

class XmlFormIO : public Form::IFormIO
{
     Q_OBJECT
public:
    XmlFormIO(QObject *parent=0);
    ~XmlFormIO();

    static QString lastestXmlVersion();

    // Form::IForm interface
    QString name() const {return "XmlFormIO";}

    void muteUserWarnings(bool state) {m_Mute = state;}

    bool canReadForms(const QString &uuidOrAbsPath) const;

    Form::FormIODescription *readFileInformations(const QString &uuidOrAbsPath);
    QList<Form::FormIODescription *> getFormFileDescriptions(const Form::FormIOQuery &query);

    QList<Form::FormMain *> loadAllRootForms(const QString &uuidOrAbsPath = QString::null);
    bool loadPmhCategories(const QString &uuidOrAbsPath);

    bool saveForm(QObject *treeRoot) { Q_UNUSED(treeRoot); return true; }

    QString lastError() const {return m_Error.join("\n");}
    // End Form::IForm interface

private:
    void getAllFormsFromDir(const QString &absPath, QList<Form::FormIODescription *> *list);
    bool createCategory(const QDomElement &element, Category::CategoryItem *parent, const QString &readingAbsPathFile);

private:
     mutable QString m_AbsFileName;
     mutable QStringList m_Error;
     mutable QDomDocument m_MainDoc;
     bool m_Mute;
     Form::FormMain *m_ActualForm;

     // Caching some data for speed improvements
     mutable QHash<QString, bool> m_ReadableForms;
};

}  // End XmlForms

#endif  //  XMLFORMIO_H
