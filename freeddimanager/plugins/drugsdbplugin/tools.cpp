/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2015 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "tools.h"
#include <drugsdbplugin/drugdatabasedescription.h>

#include <drugsbaseplugin/drugbaseessentials.h>
#include <drugsbaseplugin/constants_databaseschema.h>

#include <utils/global.h>
#include <utils/log.h>

#include <quazip/quazip/quazip.h>
#include <quazip/quazip/quazipfile.h>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QStringList>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTime>
#include <QVariant>
#include <QProgressDialog>
#include <QAbstractTableModel>
#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

namespace DrugsDb {
namespace Tools {

QString getBlock(const QString &content, const int posStart, int &posEnd, const QString &delimiter)
{
    // find first delimiter starting at pos posStart
    int begin = content.indexOf( delimiter, posStart );
    int end = content.indexOf( delimiter, begin + delimiter.length() + 1 );

    // modify posEnd for next block process
    if (begin == -1) {
        posEnd = posStart;
        return QString::null;
    } else {
        posEnd = end;
    }

    // return block
    QString tmp = content.mid( begin, end - begin );
    tmp.replace( "\r" , "" );
    tmp.replace( "\n" , "" );
    return tmp;
}

QString getBlock(const QString &content, const int posStart, int &posEnd, const QRegExp &delimiter)
{
    // find first delimiter starting at pos posStart
    int begin = content.indexOf( delimiter, posStart );
    int end = content.indexOf( delimiter, begin + 4 );

    // modify posEnd for next block process
    if (begin == -1) {
        posEnd = posStart;
        return QString::null;
    } else {
        posEnd = end;
    }

    // return block
    QString tmp = content.mid( begin, end - begin );
    tmp.replace( "\r" , "" );
    //      tmp.replace( "\n" , "" );
    return tmp;
}

bool executeProcess(const QString &proc)
{
    QProcess process;
    process.start(proc, QIODevice::ReadOnly);

    LOG_FOR("Tools", QString("Executing process: %1").arg(proc));

    if (!process.waitForStarted())
        LOG_ERROR_FOR("Tools", QString("Process %1 can not start").arg(proc.left(20)));

    if (!process.waitForFinished(100000))
        LOG_ERROR_FOR("Tools", QString("Process %1 does not end").arg(proc.left(20)));

    QString error = process.readAllStandardError();
    if (!error.isEmpty()) {
        LOG_ERROR_FOR("Tools", QString("ERROR: %1").arg(proc));
        LOG_ERROR_FOR("Tools", QString("ERROR: %1").arg(error));
        return false;
    }
    LOG_FOR("Tools", QString("Process done: %1, output: %2").arg(proc.left(20)).arg(QString(process.readAllStandardOutput())));
    return true;
}

bool connectDatabase(const QString &connection, const QString &fileName)
{
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        LOG_ERROR_FOR("Tools", QString("ERROR: SQLite driver is not available"));
        return false;
    }
    QSqlDatabase DB;
    // test drugs connection
    if (QSqlDatabase::contains(connection)) {
        DB = QSqlDatabase::database(connection);
    }  else {
        DB = QSqlDatabase::addDatabase("QSQLITE" , connection);
        // FIXME: why is there a macbundle path here?
        if (QFileInfo(fileName).isRelative())
            DB.setDatabaseName(QDir::cleanPath(qApp->applicationDirPath() + "/../../../" + fileName)); // MacBundle
        else
            DB.setDatabaseName(fileName);

        if (!DB.open()) {
            LOG_ERROR_FOR("Tools", QString("ERROR: %1 // %2").arg(DB.lastError().text()).arg(fileName));
            return false;
        } else {
            LOG_FOR("Tools", QString("Connection to database created: %1 %2")
                    .arg(DB.connectionName(), DB.databaseName()));
        }
    }
    return true;
}

bool signDatabase(const QString &connectionName)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) {
        if (!db.open()) {
            return false;
        }
    }

    QHash<QString, QString> tables;
    foreach(const QString &table, db.tables()) {
        QString req = QString("SELECT count(*) FROM %1;").arg(table);
        QSqlQuery query(db);
        if (query.exec(req)) {
            if (query.next())
                tables.insert(table, query.value(0).toString());
        }
    }
    if (tables.count() != db.tables().count())
        return false;

    QFileInfo info(db.databaseName());
    QString tag = info.fileName() + "(";
    tag += QString::number(info.size()) + "," + info.created().toString(Qt::ISODate) + ")@";
    foreach(const QString &table, tables.keys()) {
        tag += table + ":" + tables.value(table) + "/";
    }
    tag.chop(1);
    tag += "\n";

    QString fileName = QFileInfo(db.databaseName()).absolutePath() + "/check.db";
    QString content = QString(QByteArray::fromBase64(Utils::readTextFile(fileName, Utils::DontWarnUser).toUtf8()));
    QStringList linesToKeep;
    foreach(const QString &line, content.split("\n", QString::SkipEmptyParts)) {
        if (line.startsWith(QFileInfo(db.databaseName()).fileName() + "@")) {
            continue;
        }
        linesToKeep << line;
    }
    linesToKeep << tag;
    content.clear();
    content = linesToKeep.join("\n");

    Utils::saveStringToFile(content.toUtf8().toBase64(), fileName, Utils::Overwrite, Utils::DontWarnUser);

    return true;
}

/**
 * Add translated labels \e trLabels into the drugs database \e database,
 * using the \e masterLid primkey.
 * If the \e masterLib is set to -1, the function will find the first
 * available primkey and will return it.
 * \note This member does not create transaction, does not commit or rollback.
 */
int addLabels(DrugsDB::Internal::DrugBaseEssentials *database, const int masterLid, const QMultiHash<QString, QVariant> &trLabels)
{
    if (!database)
        return -1;
    QSqlDatabase db = database->database();
    if (!db.isOpen()) {
        if (!db.open()) {
            LOG_ERROR_FOR("Tools","Unable to connection database");
            return -1;
        }
    }
    QSqlQuery query(db);
    QString req;
    int mid = masterLid;
    if (mid == -1) {
        // get new master_lid
        mid = database->max(DrugsDB::Constants::Table_LABELSLINK, DrugsDB::Constants::LABELSLINK_MASTERLID).toInt();
        ++mid;
    }

    // insert all translated labels
    foreach(const QString &lang, trLabels.uniqueKeys()) {
        foreach(const QVariant &value, trLabels.values(lang)) {
            QString t = value.toString();

            // Check is couple already exists
            // TODO: code here: code buggy */
//            req = QString("SELECT LID FROM LABELS WHERE (LANG='%1' AND LABEL=\"%2\")")
//                  .arg(lang)
//                  .arg(t);
//            if (query.exec(req)) {
//                if (query.next()) {
//                    int lid = query.value(0).toInt();
//                    query.finish();

//                    req = QString("INSERT INTO `LABELS_LINK` (MASTER_LID, LID) VALUES "
//                                  "(%1  ,%2)")
//                            .arg(mid)
//                            .arg(lid)
//                            ;
//                    if (!query.exec(req)) {
//                        LOG_QUERY_ERROR_FOR("Drugs", query);
//                        return false;
//                    }
//                    query.finish();
//                    return mid;
//                }
//            } else {
//                LOG_QUERY_ERROR_FOR("Drugs", query);
//                return -1;
//            }
//            query.finish();

//            req = QString("INSERT INTO `LABELS` (LID,LANG,LABEL) VALUES (NULL,'%1','%2')")
//                    .arg(lang).arg(t.replace("'","''"));
            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_LABELS));
            query.bindValue(DrugsDB::Constants::LABELS_LID, QVariant());
            query.bindValue(DrugsDB::Constants::LABELS_LANG, lang);
            query.bindValue(DrugsDB::Constants::LABELS_LABEL, t);
            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR("Tools", query);
                return -1;
            }
            int id = query.lastInsertId().toInt();
            query.finish();

//            req = QString("INSERT INTO `LABELS_LINK` (MASTER_LID, LID) VALUES "
//                          "(%1  ,%2)")
//                    .arg(mid)
//                    .arg(id)
//                    ;
            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_LABELSLINK));
            query.bindValue(DrugsDB::Constants::LABELSLINK_LID, id);
            query.bindValue(DrugsDB::Constants::LABELSLINK_MASTERLID, mid);
            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR("Drugs", query);
                return -1;
            }
            query.finish();
        }
    }
    return mid;
}

/**
 * Create a specific ATC code using the \e code and the translated labels \e trLabels.
 * \e trLabels key represent the langage ("fr", "en", "de"...) while the
 * value represent the label itself. \n
 * You can force the database id for this ATC with \e forceAtcId. \n
 * If you do not need the drug interaction engine to warn ATC duplication
 * in prescription you can set \e warnDuplicates to \e false, otherwise set it to \e true.\n
 * \note This member does not create transaction, does not commit or rollback.
 */
bool createAtc(DrugsDB::Internal::DrugBaseEssentials *database, const QString &code, const QMultiHash<QString, QVariant> &trLabels, const int forceAtcId, const bool warnDuplicates)
{
    QSqlDatabase db = database->database();
    if (!db.isOpen())
        return false;
    QSqlQuery query(db);
    int id = 0;
    query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_ATC));
    query.bindValue(DrugsDB::Constants::ATC_ID, QVariant());
    query.bindValue(DrugsDB::Constants::ATC_CODE, code);
    query.bindValue(DrugsDB::Constants::ATC_WARNDUPLICATES, warnDuplicates);
    if (forceAtcId != -1)
        query.bindValue(DrugsDB::Constants::ATC_ID, forceAtcId);
    if (query.exec()) {
        id = query.lastInsertId().toInt();
        if (forceAtcId != -1 && forceAtcId != id) {
            LOG_ERROR_FOR("Tools", QString("Wrong ATC_ID Db=%1 / Asked=%2").arg(id).arg(forceAtcId));
            query.finish();
            return false;
        }
    } else {
        LOG_QUERY_ERROR_FOR("Tools", query);
    }
    query.finish();

    // Create labels
    int masterLid = Tools::addLabels(database, -1, trLabels);
    if (masterLid == -1) {
        LOG_ERROR_FOR("Tools", "No MasterLid");
        query.finish();
        return false;
    }

    // Create ATC_LABELS link
    query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_ATC_LABELS));
    query.bindValue(DrugsDB::Constants::ATC_LABELS_ATCID, id);
    query.bindValue(DrugsDB::Constants::ATC_LABELS_MASTERLID, masterLid);
    if (!query.exec()) {
        LOG_QUERY_ERROR_FOR("Tools", query);
        query.finish();
        return false;
    }
    query.finish();
    return true;
}

/**
 * Add the interaction using:
 * @param database: the drug database
 * @param atc1: the ATC codes of the first interactor
 * @param atc2: the ATC codes of the second interactor
 * @param type: coded type of interaction
 * @param risk: translated risk labels
 * @param management: translated management labels
 * \note This member does not create transaction, does not commit or rollback.
 */
bool addInteraction(DrugsDB::Internal::DrugBaseEssentials *database, const QStringList &atc1, const QStringList &atc2, const QString &type, const QMultiHash<QString, QVariant> &risk, const QMultiHash<QString, QVariant> &management)
{
    QSqlDatabase db = database->database();
    if (!db.isOpen())
        return false;
    QSqlQuery query(db);
    QString req;
    int iak_id = -1;
    QList<int> ia_ids;

    foreach(const QString &a1, atc1) {
        foreach(const QString &a2, atc2) {
//            req = QString("INSERT INTO INTERACTIONS (ATC_ID1, ATC_ID2) VALUES (%1, %2);")
//                    .arg(QString("(SELECT ATC_ID FROM ATC WHERE CODE=\"%1\")").arg(a1))
//                    .arg(QString("(SELECT ATC_ID FROM ATC WHERE CODE=\"%1\")").arg(a2));
            int atcId1 = -1;
            int atcId2 = -1;
            QHash<int, QString> w;
            w.insert(DrugsDB::Constants::ATC_CODE, QString("='%1'").arg(a1));
            req = database->select(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID, w);
            if (query.exec(req)) {
                atcId1 = query.lastInsertId().toInt();
            } else {
                LOG_QUERY_ERROR_FOR("Tools", query);
                return false;
            }
            w.clear();
            w.insert(DrugsDB::Constants::ATC_CODE, QString("='%1'").arg(a2));
            req = database->select(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID, w);
            if (query.exec(req)) {
                atcId2 = query.lastInsertId().toInt();
            } else {
                LOG_QUERY_ERROR_FOR("Tools", query);
                return false;
            }

            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_INTERACTIONS));
            query.bindValue(DrugsDB::Constants::INTERACTIONS_IAID, QVariant());
            query.bindValue(DrugsDB::Constants::INTERACTIONS_ATC_ID1, atcId1);
            query.bindValue(DrugsDB::Constants::INTERACTIONS_ATC_ID2, atcId2);
            if (query.exec()) {
                ia_ids << query.lastInsertId().toInt();
            } else {
                LOG_QUERY_ERROR_FOR("Tools", query);
                return false;
            }
            query.finish();
        }
    }
    // Check errors
    if (!ia_ids.count()) {
        LOG_ERROR_FOR("Tools", QString("Interaction not added: %1   //  %2").arg(atc1.join(",")).arg(atc2.join(",")));
        return false;
    }
    // Add labels
    int riskMasterLid = addLabels(database, -1, risk);
    int manMasterLid = addLabels(database, -1, management);
    if (riskMasterLid==-1 || manMasterLid==-1)
        return false;

    // Add IAK
    // TODO: add bibliography */
//    req = QString("INSERT INTO IAKNOWLEDGE (IAKID,TYPE,RISK_MASTER_LID,MAN_MASTER_LID) VALUES "
//                  "(NULL, \"%1\", %2, %3)")
//            .arg(type)
//            .arg(riskMasterLid)
//            .arg(manMasterLid);
    query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_IAKNOWLEDGE));
    query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_IAKID, QVariant());
    query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_TYPE, type);
    query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_RISK_MASTERLID, riskMasterLid);
    query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_BIB_MASTERID, QVariant());
    query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_MANAGEMENT_MASTERLID, manMasterLid);
    query.bindValue(DrugsDB::Constants::IAKNOWLEDGE_WWW, QVariant());
    if (query.exec()) {
        iak_id = query.lastInsertId().toInt();
    } else {
        LOG_QUERY_ERROR_FOR("Tools", query);
        return false;
    }
    query.finish();
    if (iak_id==-1)
        return false;

    // Add to IA_IAK
    foreach(const int ia, ia_ids) {
//        req = QString("INSERT INTO IA_IAK (IAID,IAKID) VALUES (%1,%2)")
//              .arg(ia)
//              .arg(iak_id)
//              ;
        query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_IA_IAK));
        query.bindValue(DrugsDB::Constants::IA_IAK_IAID, ia);
        query.bindValue(DrugsDB::Constants::IA_IAK_IAKID, iak_id);
        if (!query.exec()) {
            LOG_QUERY_ERROR_FOR("Tools", query);
            return false;
        }
        query.finish();
    }

    return true;
}

/**
 * Add a bibliographic reference to a drugs database \e database.
 * \note This member does not create transaction, does not commit or rollback.
 */
int addBibliography(DrugsDB::Internal::DrugBaseEssentials *database, const QString &type, const QString &link, const QString &reference, const QString &abstract, const QString &explain, const QString &xml)
{
    QSqlDatabase db = database->database();
    if (!db.isOpen()) {
        if (!db.open()) {
            return false;
        }
    }
    QString req;
    QSqlQuery query(db);

    // Bibliography exists ?
    int bib_id = -1;
//    req = QString("SELECT BIB_ID FROM BIBLIOGRAPHY WHERE LINK=\"%1\"").arg(link);
    QHash<int, QString> w;
    w.insert(DrugsDB::Constants::BIB_LINK, QString("=%1'").arg(link));
    req = database->select(DrugsDB::Constants::Table_BIB, DrugsDB::Constants::BIB_BIBID, w);
    if (query.exec(req)) {
        if (query.next()) {
            bib_id = query.value(0).toInt();
        } else {
            // Create the bib and retrieve the bib_id
//            QString t = type;
//            QString l = link;
//            QString r = reference;
//            QString a = abstract;
//            QString e = explain;
//            req = QString("INSERT INTO BIBLIOGRAPHY "
//                          "(BIB_ID,TYPE,LINK,TEXTUAL_REFERENCE,ABSTRACT,EXPLANATION) VALUES"
//                          "(NULL, '%1', '%2', '%3', '%4', '%5')")
//                    .arg(t.replace("'","''"))
//                    .arg(l.replace("'","''"))
//                    .arg(r.replace("'","''"))
//                    .arg(a.replace("'","''"))
//                    .arg(e.replace("'","''"));
            query.finish();
            // TODO: add bibliography XML here ? */
            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_BIB));
            query.bindValue(DrugsDB::Constants::BIB_BIBID, QVariant());
            query.bindValue(DrugsDB::Constants::BIB_TYPE, type);
            query.bindValue(DrugsDB::Constants::BIB_LINK, link);
            query.bindValue(DrugsDB::Constants::BIB_TEXTREF, reference);
            query.bindValue(DrugsDB::Constants::BIB_ABSTRACT, abstract);
            query.bindValue(DrugsDB::Constants::BIB_EXPLAIN, explain);
            query.bindValue(DrugsDB::Constants::BIB_XML, xml);
            if (query.exec()) {
                bib_id = query.lastInsertId().toInt();
            } else {
                LOG_QUERY_ERROR_FOR("Tools", query);
                return false;
            }
            query.finish();
        }
    } else {
        LOG_QUERY_ERROR_FOR("Tools", query);
        return false;
    }
    query.finish();
    return bib_id;
}

/**
 * Save the ATC Id <-> Molecule link to the drugs database
 * @param database: output drugs database
 * @param mol_atc: Molecule Id to ATC Id
 * @param sid: recorded SourceId of the drugs database collection
 * \note This member does not create transaction, does not commit or rollback.
*/
bool addComponentAtcLinks(DrugsDB::Internal::DrugBaseEssentials *database, const QMultiHash<int, int> &mol_atc, const int sid)
{
    QSqlDatabase db = database->database();
    if (!db.isOpen()) {
        if (!db.open()) {
            return false;
        }
    }

    // Clear existing links
    QHash<int, QString> w;
    w.insert(DrugsDB::Constants::LK_ATC_SID, QString("='%1'").arg(sid));
    database->executeSQL(database->prepareDeleteQuery(DrugsDB::Constants::Table_LK_MOL_ATC, w), db);

    // Save to links to drugs database
    QSqlQuery query(db);
    foreach(int mol, mol_atc.uniqueKeys()) {
        QList<int> atcCodesSaved;
        foreach(int atc, mol_atc.values(mol)) {
            if (atcCodesSaved.contains(atc))
                continue;
            atcCodesSaved.append(atc);
            query.prepare(database->prepareInsertQuery(DrugsDB::Constants::Table_LK_MOL_ATC));
            query.bindValue(DrugsDB::Constants::LK_MID, mol);
            query.bindValue(DrugsDB::Constants::LK_ATC_ID, atc);
            query.bindValue(DrugsDB::Constants::LK_ATC_SID, sid);
            if (!query.exec()) {
                LOG_QUERY_ERROR_FOR("Tools", query);
                query.finish();
                return false;
            }
            query.finish();
        }
    }
    return true;
}

/**
 * Return the ATC ids associated with the \e label,
 * searching in the drugs database \e database
 * \note This member does not create transaction, does not commit or rollback.
 */
QVector<int> getAtcIdsFromLabel(DrugsDB::Internal::DrugBaseEssentials *database, const QString &label)
{
    QVector<int> ret;
    QSqlDatabase db = database->database();
    if (!db.isOpen()) {
        if (!db.open()) {
            return ret;
        }
    }
    Utils::FieldList get;
    get << Utils::Field(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID);
    Utils::JoinList joins;
    joins << Utils::Join(DrugsDB::Constants::Table_ATC_LABELS, DrugsDB::Constants::ATC_LABELS_ATCID, DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID)
          << Utils::Join(DrugsDB::Constants::Table_LABELSLINK, DrugsDB::Constants::LABELSLINK_MASTERLID, DrugsDB::Constants::Table_ATC_LABELS, DrugsDB::Constants::ATC_LABELS_MASTERLID)
          << Utils::Join(DrugsDB::Constants::Table_LABELS, DrugsDB::Constants::LABELS_LID, DrugsDB::Constants::Table_LABELSLINK, DrugsDB::Constants::LABELSLINK_LID);
    Utils::FieldList cond;
    cond << Utils::Field(DrugsDB::Constants::Table_LABELS, DrugsDB::Constants::LABELS_LABEL, QString("='%1'").arg(label));
    QSqlQuery query(db);
    if (query.exec(database->select(get,joins,cond))) {
        while (query.next()) {
            ret << query.value(0).toInt();
        }
    } else {
        LOG_QUERY_ERROR_FOR("Tools", query);
    }
    return ret;
}

/**
 * Return the ATC ids associated with the \e code,
 * searching in the drugs database \e database
 * \note This member does not create transaction, does not commit or rollback.
 */
QVector<int> getAtcIdsFromCode(DrugsDB::Internal::DrugBaseEssentials *database, const QString &code)
{
    QVector<int> ret;
    QString req;
    QSqlDatabase db = database->database();
    if (!db.isOpen()) {
        if (!db.open()) {
            return ret;
        }
    }
    QSqlQuery query(db);
    QHash<int, QString> w;
    w.insert(DrugsDB::Constants::ATC_CODE, QString("LIKE '%1'").arg(code));
    req = database->select(DrugsDB::Constants::Table_ATC, DrugsDB::Constants::ATC_ID, w);
    if (query.exec(req)) {
        while (query.next()) {
            ret << query.value(0).toInt();
        }
    } else {
        LOG_QUERY_ERROR_FOR("Tools", query);
    }
    return ret;
}

} // end namespace Tools
} // end namespace DrugsDb
