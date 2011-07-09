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
#ifndef VERSIONNUMBER_H
#define VERSIONNUMBER_H

#include <utils/global_exporter.h>

#include <QString>


namespace Utils {

class UTILS_EXPORT VersionNumber
{
public:
    VersionNumber(const QString &versionNumber);

    int majorNumber() const {return m_Major;}
    int minorNumber() const {return m_Minor;}
    int debugNumber() const {return m_Debug;}

    bool isAlpha() const {return m_IsAlpha;}
    int alphaNumber() const {return m_Alpha;}

    bool isBeta() const {return m_IsBeta;}
    int betaNumber() const {return m_Beta;}

    bool isRC() const {return m_IsRC;}
    int rcNumber() const {return m_RC;}

    bool operator>(const VersionNumber &b) const;
    bool operator<(const VersionNumber &b) const;

    bool operator==(const VersionNumber &b) const;
    bool operator!=(const VersionNumber &b) const;

    void warn() const;

private:
    QString m_Version;
    int m_Major, m_Minor, m_Debug, m_Alpha, m_Beta, m_RC;
    bool m_IsAlpha, m_IsBeta, m_IsRC;
};

}  // End namespace Utils

#endif // VERSIONNUMBER_H
