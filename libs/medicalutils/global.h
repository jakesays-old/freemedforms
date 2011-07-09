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
#ifndef MEDICALUTILS_GLOBAL_H
#define MEDICALUTILS_GLOBAL_H

#include <medicalutils/medical_exporter.h>
#include <medicalutils/aggir/girscore.h>

QT_BEGIN_NAMESPACE
class QDateTime;
class QDate;
QT_END_NAMESPACE

/**
 * \file global.h
 * \author Eric MAEKER <eric.maeker@gmail.com>
 * \version 0.0.3
 * \date 17 Dec 2009
*/

namespace MedicalUtils {

MEDICALUTILS_EXPORT QString readableAge(const QDate &DOB);
MEDICALUTILS_EXPORT int ageYears(const QDate &DOB);

MEDICALUTILS_EXPORT double clearanceCreatinin(const int ageYears, const int weightKg, const double creatMlMin, const bool isMale);

} // MedicalUtils

#endif // MEDICALUTILS_GLOBAL_H
