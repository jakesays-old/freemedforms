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
/**
  \class Agenda::UserCalendarModel
  Model for the management of user calendars.\n
  Use the Agenda::AgendaCore::userCalendarModel() to create/get models.
*/

#include "usercalendarmodel.h"
#include "usercalendar.h"
#include "agendabase.h"
#include "constants.h"

#include <coreplugin/icore.h>
#include <coreplugin/iuser.h>
#include <coreplugin/itheme.h>

#include <utils/log.h>
#include <translationutils/constanttranslations.h>

using namespace Agenda;
using namespace Internal;
using namespace Trans::ConstantTranslations;

static inline Core::ITheme *theme() {return Core::ICore::instance()->theme();}
static inline Core::IUser *user() {return Core::ICore::instance()->user();}
//static inline Core::IPatient *patient() {return Core::ICore::instance()->patient();}
static inline Agenda::Internal::AgendaBase *base() {return Agenda::Internal::AgendaBase::instance();}

namespace Agenda {
namespace Internal {
class UserCalendarModelPrivate
{
public:
    void getUserCalendars()
    {
        qDeleteAll(m_UserCalendars);
        m_UserCalendars.clear();
        m_UserCalendars = base()->getUserCalendars(m_UserUid);
    }

public:
    QString m_UserUid;
    QList<UserCalendar*> m_UserCalendars, m_RemovedCalendars;
};
} // End namespace Internal
} // End namespace Agenda


/** Use the Agenda::AgendaCore::userCalendarModel() to create new Agenda::UserCalendarModel \sa Agenda::AgendaCore. */
UserCalendarModel::UserCalendarModel(const QString &userUid, QObject *parent) :
    QAbstractTableModel(parent),
    d(new UserCalendarModelPrivate)
{
    if (userUid.isEmpty())
        d->m_UserUid = user()->uuid();
    else
        d->m_UserUid = userUid;
    d->getUserCalendars();
}

UserCalendarModel::~UserCalendarModel()
{
    if (d)
        delete d;
    d = 0;
}

int UserCalendarModel::rowCount(const QModelIndex &parent) const
{
    return d->m_UserCalendars.count();
}

int UserCalendarModel::columnCount(const QModelIndex &parent) const
{
    return ColumnCount;
}

QVariant UserCalendarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= d->m_UserCalendars.count())
        return QVariant();

    const UserCalendar *u = d->m_UserCalendars.at(index.row());
    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case Uid: return u->data(Constants::Db_CalId);
        case Label: return u->data(UserCalendar::Label);
        case ExtraLabel:
        {
            if (u->isDelegated()) {
                return QString("[%1] %2")
                        .arg(u->data(UserCalendar::UserOwnerFullName).toString())
                        .arg(u->data(UserCalendar::Label).toString());
            }
            return u->data(UserCalendar::Label);
        }
        case Description: return u->data(UserCalendar::Description);
        case Type: return u->data(UserCalendar::Type);
        case Status: return u->data(UserCalendar::Status);
        case IsDefault: return u->data(UserCalendar::IsDefault);
        case IsPrivate: return u->data(UserCalendar::IsPrivate);
        case Password: return u->data(UserCalendar::Password);
        case LocationUid: return u->data(UserCalendar::LocationUid);
        case DefaultDuration: return u->data(UserCalendar::DefaultDuration);
        }
    } else if (role==Qt::ToolTipRole) {
        switch (index.column()) {
        case Label: return u->data(UserCalendar::Label);
        }
    } else if (role==Qt::FontRole) {
        if (u->isDelegated()) {
            QFont italic;
            italic.setItalic(true);
            return italic;
        } else if (u->isDefault()) {
            QFont bold;
            bold.setBold(true);
            return bold;
        }
    } else if (role==Qt::DecorationRole) {
        if (!u->data(UserCalendar::AbsPathIcon).isNull()) {
            return theme()->icon(u->data(UserCalendar::AbsPathIcon).toString());
        }
    }

    return QVariant();
}

bool UserCalendarModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.row() < 0 || index.row() >= d->m_UserCalendars.count())
        return false;

    UserCalendar *u = d->m_UserCalendars.at(index.row());

    if (role==Qt::EditRole) {
        switch (index.column()) {
        case Label: return u->setData(UserCalendar::Label, value); break;
        case Description: return u->setData(UserCalendar::Description, value); break;
        case Type: return u->setData(UserCalendar::Type, value); break;
        case Status: return u->setData(UserCalendar::Status, value); break;
        case IsDefault: return u->setData(UserCalendar::IsDefault, value); break;
        case IsPrivate: return u->setData(UserCalendar::IsPrivate, value); break;
        case Password: return u->setData(UserCalendar::Password, value); break;
        case LocationUid: return u->setData(UserCalendar::LocationUid, value); break;
        case DefaultDuration: return u->setData(UserCalendar::DefaultDuration, value); break;
        }
    }

    return true;
}

/** Create new Agenda::UserCalendar by inserting new model rows. */
bool UserCalendarModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row+count);
    for(int i = 0 ; i < count; ++i) {
        UserCalendar *u = new UserCalendar();
        u->setData(UserCalendar::Label, tr("New calendar"));
        u->setData(UserCalendar::UserOwnerUid, d->m_UserUid);
        u->setData(Constants::Db_IsValid, 1);
        u->setData(Constants::Db_UserCalId, -1);
        for(int i=1; i < 8; ++i) {
            DayAvailability av;
            av.addTimeRange(QTime(06,00,00), QTime(20,00,00));
            av.setWeekDay(i);
            u->addAvailabilities(av);
        }
        d->m_UserCalendars.insert(row+i, u);
    }
    endInsertRows();
    return true;
}

/** Remove Agenda::UserCalendar by removing model rows. */
bool UserCalendarModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row+count);
    for(int i = 0 ; i < count; ++i) {
        UserCalendar *u = d->m_UserCalendars.at(row);
        u->setData(Constants::Db_IsValid, 0);
        d->m_RemovedCalendars.append(u);
        d->m_UserCalendars.removeAt(row);
    }
    endRemoveRows();
    return true;
}

/** Return the Agenda::UserCalendar at the index \e row. Pointer must not be deleted. */
UserCalendar *UserCalendarModel::userCalendarAt(const int row) const
{
    if (row < 0 || row >= d->m_UserCalendars.count())
        return 0;
    return d->m_UserCalendars.at(row);
}

/** Return the default Agenda::UserCalendar or the first Agenda::UserCalendar or 0 if no Agenda::UserCalendar are available. The pointer must not be deleted. */
UserCalendar *UserCalendarModel::defaultUserCalendar() const
{
    for(int i = 0; i < d->m_UserCalendars.count(); ++i) {
        UserCalendar *u = d->m_UserCalendars.at(i);
        if (u->data(UserCalendar::IsDefault).toBool()) {
            return u;
        }
    }
    if (d->m_UserCalendars.count())
        return d->m_UserCalendars.at(0);
    return 0;
}

/** Return the QModelIndex for the default Agenda::UserCalendar or the first Agenda::UserCalendar or 0 if no Agenda::UserCalendar are available. The pointer must not be deleted. */
QModelIndex UserCalendarModel::defaultUserCalendarModelIndex() const
{
    for(int i = 0; i < d->m_UserCalendars.count(); ++i) {
        UserCalendar *u = d->m_UserCalendars.at(i);
        if (u->data(UserCalendar::IsDefault).toBool()) {
            return index(i, 0);
        }
    }
    if (d->m_UserCalendars.count())
        return index(0, 0);
    return QModelIndex();
}

/** Return the Agenda::DayAvailabilityModel corresponding to the calendar at the specified \e index . You have to delete the returned pointer. */
DayAvailabilityModel *UserCalendarModel::availabilityModel(const QModelIndex &index, QObject *parent) const
{
    if (!index.isValid())
        return 0;

    if (index.row() < 0 || index.row() >= d->m_UserCalendars.count())
        return false;

    UserCalendar *u = d->m_UserCalendars.at(index.row());

    DayAvailabilityModel *model = new DayAvailabilityModel(parent);
    model->setUserCalendar(u);
    return model;
}

/** Set the full list of user delegates to UserCalendars. */
void UserCalendarModel::setPeopleList(const int row, const QList<Calendar::People> &peoples)
{
    if (row < 0 || row >= d->m_UserCalendars.count())
        return;
    UserCalendar *u = d->m_UserCalendars.at(row);
    u->setPeopleList(peoples);
}

/** Add user delegates to UserCalendars. The only allowed type of Calendar::People is Calendar::People::PeopleUserDelegate. */
void UserCalendarModel::addPeople(const int row, const Calendar::People &people)
{
    if (row < 0 || row >= d->m_UserCalendars.count())
        return;
    UserCalendar *u = d->m_UserCalendars.at(row);
    u->addPeople(people);
}

/** Remove one UserDelegate to the UserCalendar. \sa  addPeople() */
void UserCalendarModel::removePeople(const int row, const QString &uid)
{
    if (row < 0 || row >= d->m_UserCalendars.count())
        return;
    UserCalendar *u = d->m_UserCalendars.at(row);
    u->removePeople(uid);
}

/** Submit all modifications to the database. */
bool UserCalendarModel::submit()
{
    bool ok = true;
    for(int i = 0; i < d->m_UserCalendars.count(); ++i) {
        if (!base()->saveUserCalendar(d->m_UserCalendars.at(i)))
                ok = false;
    }
    for(int i = 0; i < d->m_RemovedCalendars.count(); ++i) {
        if (!base()->saveUserCalendar(d->m_RemovedCalendars.at(i)))
                ok = false;
    }
    return ok;
}

/** Revert all modifications. */
void UserCalendarModel::revert()
{
    d->getUserCalendars();
    reset();
}


///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////      DayAvailabilityModel     ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
namespace Agenda {
namespace Internal {
class DayAvailabilityModelPrivate
{
public:
    DayAvailabilityModelPrivate() : m_UserCalendar(0) {}

public:
    UserCalendar *m_UserCalendar;
};
}
}

DayAvailabilityModel::DayAvailabilityModel(QObject *parent) :
        QStandardItemModel(parent),
        d(new Internal::DayAvailabilityModelPrivate)
{}

DayAvailabilityModel::~DayAvailabilityModel()
{
    if (d) {
        delete d;
        d = 0;
    }
}

void DayAvailabilityModel::setUserCalendar(UserCalendar *calendar)
{
    d->m_UserCalendar = calendar;
    QFont bold;
    bold.setBold(true);
    // Create on item foreach week of day
    QVector<QStandardItem *> days;
    for(int i = 1; i < 8; ++i) {
        QStandardItem *day = new QStandardItem(QDate::longDayName(i));
        day->setFont(bold);
        days << day;
        // Add availabilities to items
        const QVector<DayAvailability> &avail = calendar->availabilities(i);
        for(int j = 0; j < avail.count(); ++j) {
            for(int k = 0; k < avail.at(j).timeRangeCount(); ++k) {
                TimeRange range = avail.at(j).timeRange(k);
//                QStandardItem *time1 = new QStandardItem(QString("%1").arg(range.from.toString()));
//                QStandardItem *time2 = new QStandardItem(QString("%1").arg(range.to.toString()));
//                day->appendRow(time1);
//                day->appendRow(time2);
                QStandardItem *time = new QStandardItem(tkTr(Trans::Constants::FROM_1_TO_2).arg(range.from.toString()).arg(range.to.toString()));
                time->setToolTip(time->text());
                day->appendRow(time);
            }
        }
        if (day->rowCount())
            invisibleRootItem()->appendRow(day);
    }
    if (!invisibleRootItem()->rowCount()) {
        QStandardItem *item = new QStandardItem(tkTr(Trans::Constants::NO_AVAILABILITY));
        invisibleRootItem()->appendRow(item);
    }
}

void DayAvailabilityModel::addAvailability(const DayAvailability &availability)
{
    Q_ASSERT(d->m_UserCalendar);
    d->m_UserCalendar->addAvailabilities(availability);
}

bool DayAvailabilityModel::insertRows(int row, int count, const QModelIndex &parent)
{
    /** \todo code here */
    qWarning() << Q_FUNC_INFO << row << count << parent;
    return true;
}

bool DayAvailabilityModel::removeRows(int row, int count, const QModelIndex &parent)
{
    /** \todo code here */
    qWarning() << Q_FUNC_INFO << row << count << parent;
    return true;
}

bool DayAvailabilityModel::submit()
{
    /** \todo code here */
    return true;
}
