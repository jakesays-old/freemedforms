/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2010 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "mainwindow.h"

#include <translationutils/constanttranslations.h>
#include <utils/log.h>
#include <utils/global.h>
#include <utils/updatechecker.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/constants.h>
#include <coreplugin/translators.h>
#include <coreplugin/itheme.h>
#include <coreplugin/ipatient.h>
#include <coreplugin/iuser.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/actionmanager/mainwindowactions.h>
#include <coreplugin/actionmanager/mainwindowactionhandler.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/contextmanager/contextmanager.h>
#include <coreplugin/dialogs/plugindialog.h>
#include <coreplugin/dialogs/settingsdialog.h>
#include <coreplugin/dialogs/helpdialog.h>

#include <coreplugin/commandlineparser.h>

#include <accountplugin/constants.h>
#include <accountplugin/accountview.h>

#include <accountreceiptsplugin/receipts.h>
#include <accountreceiptsplugin/receiptviewer.h>

#include <extensionsystem/pluginerrorview.h>
#include <extensionsystem/pluginview.h>
#include <extensionsystem/pluginmanager.h>

#include "ui_mainwindow.h"

#include <QTextEdit>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDockWidget>

using namespace MainWin;
using namespace MainWin::Internal;
using namespace Trans::ConstantTranslations;

// Getting the Core instances
static inline Utils::UpdateChecker *updateChecker() { return Core::ICore::instance()->updateChecker(); }
static inline Core::CommandLine *commandLine() { return Core::ICore::instance()->commandLine(); }
static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }
static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline Core::ActionManager *actionManager() { return Core::ICore::instance()->actionManager(); }
static inline Core::ContextManager *contextManager() { return Core::ICore::instance()->contextManager(); }
static inline Core::IPatient *patient() { return Core::ICore::instance()->patient(); }
static inline Core::IUser *user() { return Core::ICore::instance()->user(); }
static inline Core::FileManager *fileManager() { return Core::ICore::instance()->fileManager(); }

// SplashScreen Messagers
static inline void messageSplash(const QString &s) {Core::ICore::instance()->messageSplashScreen(s); }
static inline void finishSplash(QMainWindow *w) {Core::ICore::instance()->finishSplashScreen(w); }

namespace MainWin {
namespace Internal {
    static const char* const  SETTINGS_COUNTDOWN = "applicationCountDown";
} // namespace Internal
} // namespace Core


//--------------------------------------------------------------------------------------------------------
//--------------------------------------- Constructor / Destructor ---------------------------------------
//--------------------------------------------------------------------------------------------------------
MainWindow::MainWindow( QWidget * parent ) :
        Core::IMainWindow(parent)
{
    setObjectName("MainWindow");
//    setWindowIcon(theme()->icon(Core::Constants::ICONFREEDIAMS));
    messageSplash(tr("Creating Main Window"));
}

bool MainWindow::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    // create menus
//    createGeneralMenu();
    createFileMenu();
    Core::ActionContainer *fmenu = actionManager()->actionContainer(Core::Constants::M_FILE);
//    Core::ActionContainer *fmenu = actionManager()->actionContainer(Core::Constants::M_GENERAL);
    connect(fmenu->menu(), SIGNAL(aboutToShow()),this, SLOT(aboutToShowRecentFiles()));
//    Core::ActionContainer *pmenu = actionManager()->actionContainer(Core::Constants::MENUBAR);
//    pmenu->appendGroup(DrugsWidget::Constants::G_PLUGINS_MODES);
//    pmenu->appendGroup(DrugsWidget::Constants::G_PLUGINS_SEARCH);
//    pmenu->appendGroup(DrugsWidget::Constants::G_PLUGINS_DRUGS);
//    pmenu->setTranslations(DrugsWidget::Constants::DRUGSMENU_TEXT);
//    createTemplatesMenu();
    createConfigurationMenu();
    createHelpMenu();

    Core::MainWindowActions actions;
    actions.setFileActions(
            Core::MainWindowActions::A_FileNew  |
            Core::MainWindowActions::A_FileOpen |
            Core::MainWindowActions::A_FileSave |
            Core::MainWindowActions::A_FileSaveAs |
            Core::MainWindowActions::A_FilePrintPreview |
            Core::MainWindowActions::A_FileQuit);
    actions.setConfigurationActions(
            Core::MainWindowActions::A_AppPreferences |
            Core::MainWindowActions::A_LangageChange //|
//            Core::MainWindowActions::A_ConfigureMedinTux
            );
    actions.setHelpActions(
            Core::MainWindowActions::A_AppAbout |
            Core::MainWindowActions::A_PluginsAbout |
            Core::MainWindowActions::A_AppHelp |
            Core::MainWindowActions::A_DebugDialog |
            Core::MainWindowActions::A_CheckUpdate //|
//            Core::MainWindowActions::A_QtAbout
            );
//    actions.setTemplatesActions( Core::MainWindowActions::A_Templates_New );
    actions.createEditActions(false);
    createActions(actions);

    connectFileActions();
    connectConfigurationActions();
    connectHelpActions();

    actionManager()->retranslateMenusAndActions();

    contextManager()->updateContext();

    return true;
}

void MainWindow::extensionsInitialized()
{
    // Update countdown to dosage transmission
    int count = settings()->value(Internal::SETTINGS_COUNTDOWN,0).toInt();
    ++count;
//    if ((count==30) || (commandLine()->value(Core::CommandLine::CL_TransmitDosage).toBool())) {
//        messageSplash(tr("Transmitting posologies..."));
//        settings()->setValue(Internal::SETTINGS_COUNTDOWN,0);
//        transmitDosage();
//    } else {
        settings()->setValue(Internal::SETTINGS_COUNTDOWN,count);
//    }

        // Creating MainWindow interface
        m_ui = new Internal::Ui::MainWindow();
        m_ui->setupUi(this);
        setWindowTitle(qApp->applicationName() + " - " + qApp->applicationVersion());

    // Disable some actions when starting as medintux plugin
    if (commandLine()->value(Core::CommandLine::CL_MedinTux).toBool()) {
//        this->aNew->setEnabled(false);
//        this->aSave->setEnabled(false);
//        this->aMedinTux->setEnabled(false);
    }

    // If needed read exchange file
    const QString &exfile = commandLine()->value(Core::CommandLine::CL_ExchangeFile).toString();
    if (!exfile.isEmpty()) {
        messageSplash(tr("Reading exchange file..."));
//        if (commandLine()->value(Core::CommandLine::CL_MedinTux).toBool()) {
//            Utils::Log::addMessage(this, tr("Reading a MedinTux exchange file."));
//            QString tmp = Utils::readTextFile(exfile, Utils::DontWarnUser);
//            Utils::Log::addMessage(this, "Content of the exchange file : " + tmp);
//            if (tmp.contains(DrugsDB::Constants::ENCODEDHTML_FREEDIAMSTAG)) {
//                int begin = tmp.indexOf(DrugsDB::Constants::ENCODEDHTML_FREEDIAMSTAG) + QString(DrugsDB::Constants::ENCODEDHTML_FREEDIAMSTAG).length();
//                int end = tmp.indexOf("\"", begin);
//                QString encoded = tmp.mid( begin, end - begin );
//                DrugsDB::DrugsIO::instance()->prescriptionFromXml(drugModel(), QByteArray::fromBase64(encoded.toAscii()));
//            } else if (tmp.contains("DrugsInteractionsEncodedPrescription:")) {
//                /** \todo Manage wrong file encoding */
//                int begin = tmp.indexOf("DrugsInteractionsEncodedPrescription:") + QString("DrugsInteractionsEncodedPrescription:").length();
//                int end = tmp.indexOf("\"", begin);
//                QString encoded = tmp.mid( begin, end - begin );
//                DrugsDB::DrugsIO::instance()->prescriptionFromXml(drugModel(), QByteArray::fromBase64(encoded.toAscii()));
//            }
//        } else {
//            QString extras;
//            DrugsDB::DrugsIO::loadPrescription(drugModel(), exfile, extras);
//            patient()->fromXml(extras);
//        }
    }

    raise();

    // Start the update checker
    if (updateChecker()->needsUpdateChecking(settings()->getQSettings())) {
        messageSplash(tkTr(Trans::Constants::CHECKING_UPDATES));
        Utils::Log::addMessage(this, tkTr(Trans::Constants::CHECKING_UPDATES));
        statusBar()->addWidget(new QLabel(tkTr(Trans::Constants::CHECKING_UPDATES), this));
        statusBar()->addWidget(updateChecker()->progressBar(this),1);
        connect(updateChecker(), SIGNAL(updateFound()), this, SLOT(updateFound()));
        connect(updateChecker(), SIGNAL(done(bool)), this, SLOT(updateCheckerEnd()));
        updateChecker()->check(Utils::Constants::FREEDIAMS_UPDATE_URL);
        settings()->setValue(Utils::Constants::S_LAST_CHECKUPDATE, QDate::currentDate());
    }


    // Here we set the UI according to the commandline parser
    if (commandLine()->value(Core::CommandLine::CL_ReceiptsCreator).toBool()) {
        setCentralWidget(new ReceiptsGUI(this));
    //    setCentralWidget(new receiptviewer(this));
        qDebug() << __FILE__ << QString::number(__LINE__) << " receiptGUI initialized";
    } else {
        setCentralWidget(new Account::AccountView(this));
    }

    connect(Core::ICore::instance()->user(), SIGNAL(userChanged()), this, SLOT(userChanged()));
    userChanged();

    createDockWindows();
    finishSplash(this);
    readSettings();

    setWindowIcon(theme()->icon(Core::Constants::ICONFREEACCOUNT));
    show();
}

MainWindow::~MainWindow()
{
}

/**
  \brief Refresh the ui data refering to the patient
  \sa Core::Internal::CoreImpl::instance()->patient(), diPatient
*/
void MainWindow::refreshPatient()
{
}

/**
  \brief Close the main window and the application
  \todo Add  ICoreListener
*/
void MainWindow::closeEvent( QCloseEvent *event )
{
    Utils::Log::addMessage(this, "Closing MainWindow");
    Core::ICore::instance()->requestSaveSettings();

    //    const QList<ICoreListener *> listeners =
    //        ExtensionSystem::PluginManager::instance()->getObjects<Core::ICoreListener>();
    //    foreach (Core::ICoreListener *listener, listeners) {
    //        if (!listener->coreAboutToClose()) {
    //            event->ignore();
    //            return;
    //        }
    //    }

    // Save exchange file
//    QString exfile = commandLine()->value(Core::CommandLine::CL_ExchangeFile).toString();
//    if ((!exfile.isEmpty()) && (!QFile(exfile).exists())) {
//        Utils::Log::addError(this,tkTr(Trans::Constants::FILE_1_DOESNOT_EXISTS).arg(exfile));
//    } else if ((!exfile.isEmpty()) && (QFile(exfile).exists())) {
//        Utils::Log::addMessage(this, QString("Exchange File : %1 ").arg(exfile));
//        Utils::Log::addMessage(this, QString("Running as MedinTux plug : %1 ").arg(commandLine()->value(Core::CommandLine::CL_MedinTux).toString()));
//        // if is a medintux plugins --> save prescription to exchange file
//        if (commandLine()->value(Core::CommandLine::CL_MedinTux).toBool()) {
//            QString tmp = DrugsDB::DrugsIO::instance()->prescriptionToHtml(drugModel());
//            tmp.replace("font-weight:bold;", "font-weight:600;");
//            Utils::saveStringToFile(Utils::toHtmlAccent(tmp) , exfile, Utils::DontWarnUser);
//        } else {
//            savePrescription(exfile);
//        }
//    }

    Core::ICore::instance()->coreIsAboutToClose();
    writeSettings();
    event->accept();
}

/** \brief Manages language changes : retranslate Ui and ActionManager. */
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type()==QEvent::LanguageChange) {
//        m_ui->retranslateUi(this);
        actionManager()->retranslateMenusAndActions();
        refreshPatient();
    }
}


/** \brief Populate recent files menu */
void MainWindow::aboutToShowRecentFiles()
{
    Core::ActionContainer *aci = actionManager()->actionContainer(Core::Constants::M_FILE_RECENTFILES);
    aci->menu()->clear();

    bool hasRecentFiles = false;
    foreach (const QString &fileName, fileManager()->recentFiles()) {
        hasRecentFiles = true;
        QAction *action = aci->menu()->addAction(fileName);
        action->setData(fileName);
        connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }
    aci->menu()->setEnabled(hasRecentFiles);
}

/** \brief Opens a recent file. This solt must be called by a recent files' menu's action. */
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;
    QString fileName = action->data().toString();
    if (!fileName.isEmpty()) {
        readFile(fileName);
    }
}

void MainWindow::userChanged()
{
    setWindowTitle(qApp->applicationName() + " - " + qApp->applicationVersion() + " // " + user()->value(Core::IUser::Uuid).toString());
}

void MainWindow::updateCheckerEnd()
{
    delete statusBar();
}

/** \brief Reads main window's settings */
void MainWindow::readSettings()
{
    settings()->restoreState(this, Account::Constants::S_STATEPREFIX);
    fileManager()->getRecentFilesFromSettings();
}

/** \brief Writes main window's settings */
void MainWindow::writeSettings()
{
    settings()->saveState(this, Account::Constants::S_STATEPREFIX);
    fileManager()->saveRecentFiles();
    settings()->sync();
}

/** \obsolete */
void MainWindow::createStatusBar()
{
    statusBar()->showMessage( tkTr(Trans::Constants::READY), 2000 );
}

bool MainWindow::newFile()
{
//    if (drugModel()->drugsList().count()) {
//        bool yes = Utils::yesNoMessageBox(
//                tr("Save actual prescription ?"),
//                tr("The actual prescription is not empty. Do you want to save it before creating a new one ?"));
//        if (yes) {
//            saveFile();
//        }
//    }
//    patient()->clear();
//    refreshPatient();
//    drugModel()->clearDrugsList();
    return true;
}

/**
  \brief Open the preferences dialog
  \sa mfDrugsPreferences
*/
bool MainWindow::applicationPreferences()
{
    Core::SettingsDialog dlg(this);
    dlg.exec();
    return true;
}

/** \brief Change the font of the viewing widget */
void MainWindow::changeFontTo( const QFont &font )
{
//    m_ui->m_CentralWidget->changeFontTo(font);
//    m_ui->patientName->setFont(font);
}


/**
  \brief Prints the prescription using the header, footer and watermark.
  \sa tkPrinter
*/
bool MainWindow::print()
{
//    return m_ui->m_CentralWidget->printPrescription();
    return true;
}

bool MainWindow::printPreview()
{
//    m_ui->m_CentralWidget->printPreview();
    return true;
}

/** \brief Runs the MedinTux configurator */
bool MainWindow::configureMedintux()
{
//    Internal::configureMedinTux();
    return true;
}

bool MainWindow::saveAsFile()
{
    return savePrescription();
}

bool MainWindow::saveFile()
{
    return savePrescription();
}

/**
  \brief Saves a prescription. If fileName is empty, user is ask about a file name.
  \sa openPrescription()
  \sa DrugsIO
*/
bool MainWindow::savePrescription(const QString &fileName)
{
//    QString xmlExtra = patient()->toXml();
//    if (commandLine()->value(Core::CommandLine::CL_EMR_Name).isValid()) {
//        xmlExtra.append(QString("<EMR name=\"%1\"").arg(commandLine()->value(Core::CommandLine::CL_EMR_Name).toString()));
//        if (commandLine()->value(Core::CommandLine::CL_EMR_Name).isValid()) {
//            xmlExtra.append(QString(" uid=\"%1\"").arg(commandLine()->value(Core::CommandLine::CL_EMR_Uid).toString()));
//        }
//        xmlExtra.append("/>");
//    }
//    qWarning() << xmlExtra;
//    return DrugsDB::DrugsIO::savePrescription(drugModel(), xmlExtra, fileName);
    return true;
}

/**
  \brief Opens a prescription saved using savePrescription().
  \sa savePrescription()
  \sa DrugsIO
*/
bool MainWindow::openFile()
{
//    QString f = QFileDialog::getOpenFileName(this,
//                                             tkTr(Trans::Constants::OPEN_FILE),
//                                             QDir::homePath(),
//                                             tkTr(Core::Constants::FREEDIAMS_FILEFILTER) );
//    if (f.isEmpty())
//        return false;
//    //    QString f = "/Users/eric/prescription.di";
//    readFile(f);
//    fileManager()->setCurrentFile(f);
//    fileManager()->addToRecentFiles(f);
    return true;
}

void MainWindow::readFile(const QString &file)
{
//    QString datas;
//    if (drugModel()->rowCount() > 0) {
//        int r = Utils::withButtonsMessageBox(
//                tr("Opening a prescription : merge or replace ?"),
//                tr("There is a prescription inside editor, do you to replace it or to add the opened prescription ?"),
//                QString(), QStringList() << tr("Replace prescription") << tr("Add to prescription"),
//                tr("Open a prescription") + " - " + qApp->applicationName());
//        if (r == 0) {
//            DrugsDB::DrugsIO::loadPrescription(drugModel(), file, datas, DrugsDB::DrugsIO::ReplacePrescription);
//        } else if (r==1) {
//            DrugsDB::DrugsIO::loadPrescription(drugModel(), file, datas, DrugsDB::DrugsIO::AppendPrescription);
//        }
//    } else {
//        DrugsDB::DrugsIO::loadPrescription(drugModel(), file, datas, DrugsDB::DrugsIO::ReplacePrescription);
//    }
////    qWarning() << datas;
//    patient()->fromXml(datas);
//    refreshPatient();
}

void MainWindow::createDockWindows()
{
//    QDockWidget *dock = m_TemplatesDock = new QDockWidget(tkTr(Trans::Constants::TEMPLATES), this);
//    dock->setObjectName("templatesDock");
//    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//    dock->setWidget(new Templates::TemplatesView(dock));
//    addDockWidget(Qt::RightDockWidgetArea, dock);
//    QMenu *menu = actionManager()->actionContainer(Core::Constants::M_TEMPLATES)->menu();
//    menu->addAction(dock->toggleViewAction());
}


///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_patientName_textChanged(const QString &text)
//{
//    patient()->setValue(Core::Patient::Name, text);
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_patientSurname_textChanged(const QString &text)
//{
//    patient()->setValue(Core::Patient::Surname, text);
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_sexCombo_currentIndexChanged(const QString &text)
//{
//    patient()->setValue(Core::Patient::Gender, text);
//    refreshPatient();
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_patientWeight_valueChanged(const QString &text)
//{
//    patient()->setValue(Core::Patient::Weight, text);
//    refreshPatient();
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_patientSize_valueChanged(const QString & text)
//{
//    patient()->setValue(Core::Patient::Height, text);
//    refreshPatient();
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_patientClCr_valueChanged(const QString & text)
//{
//    patient()->setValue(Core::Patient::CreatinClearance, text);
//    refreshPatient();
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_patientCreatinin_valueChanged(const QString & text)
//{
//    patient()->setValue(Core::Patient::Creatinine, text);
//    refreshPatient();
//}
//
///** \brief Always keep uptodate patient's datas */
//void MainWindow::on_listOfAllergies_textChanged(const QString &text)
//{
//    /** \todo manage allergies */
//    patient()->setValue(Core::Patient::DrugsAtcAllergies, text);
//    refreshPatient();
//}
