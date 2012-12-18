/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main developers : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
/**
 * \class Identity::Internal::IsDirtyDataWidgetMapper
 *
 * Workflow:\n
 * The data mapper keeps the model original value when you set its current index,
 * then you can use the isDirty() method to make the comparison between cached original
 * values and the widgets current value. \n
 * Note that when the model is submitted, you have to refresh the cache with onModelSubmitted(). \n
 * This make no sense to use this data mapper with a submit policy (setSubmitPolicy())
 * different from QDataWidgetMapper::ManualSubmit.
*/

/**
 * \class Identity::IdentityEditorWidget
 * \brief Identity editor.
 * This widget allow user to view & edit the identity of a patient.
 * You can define a model, the mapping and the index to use,
 * or just keep the identity always sync with the Core::IPatient current
 * patient. \n
 *
 * Available widgets and view adaptation: \n
 * This editor contains:
 * - names
 * - firstnames
 * - date of birth
 * - photo
 * - full address with
 *    - street
 *    - city
 *    - state/province
 *    - country
 * - full login and password
 * If you don't want the whole information, you can be defined the widget to
 * show/hide using the setAvailableWidgets().
*/

#include "identityeditorwidget.h"

#include "ui_identityeditorwidget.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/constants_tokensandsettings.h>
#include <coreplugin/ipatient.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/iphotoprovider.h>
#include <patientbaseplugin/constants_settings.h>

#include <zipcodesplugin/zipcodescompleters.h>

#include <utils/log.h>
#include <utils/global.h>
#include <utils/widgets/uppercasevalidator.h>
#include <extensionsystem/pluginmanager.h>
#include <translationutils/constants.h>

#include <QDir>
#include <QFileDialog>
#include <QDateEdit>
#include <QMenu>

#include <QDebug>

using namespace Identity;
using namespace Internal;
using namespace Trans::ConstantTranslations;

static inline Core::ISettings *settings() {return Core::ICore::instance()->settings();}
static inline Core::ITheme *theme() {return Core::ICore::instance()->theme();}
static inline ExtensionSystem::PluginManager *pluginManager() {return ExtensionSystem::PluginManager::instance();}
static inline Core::IPatient *patient() {return Core::ICore::instance()->patient();}
//static inline Patients::PatientCore *patientCore() {return Patients::PatientCore::instance();}
//static inline Patients::Internal::PatientBase *patientBase() {return Patients::Internal::PatientBase::instance();}

//TODO: Users can add pages in the identity widget using the XMLForm --> create a <Form> named \e Identity

namespace {
const char * const XML_NAME1    = "n1";
const char * const XML_NAME2    = "n2";
const char * const XML_NAME3    = "n3";
const char * const XML_NAME4    = "n4";
const char * const XML_FIRSTNAME= "first";
const char * const XML_TITLE    = "tt";
const char * const XML_GENDER   = "gdr";
const char * const XML_LANG     = "i18";
const char * const XML_DOB      = "dob";
const char * const XML_DOD      = "dod";
const char * const XML_PHOTO    = "pix";
const char * const XML_STREET   = "str";
const char * const XML_CITY     = "city";
const char * const XML_PROVINCE = "prov";
const char * const XML_COUNTRY  = "ctry";
const char * const XML_ZIPCODE  = "zc";
const char * const XML_LOGIN    = "log";
const char * const XML_PASSWORD = "pwd";
} // namespace anonymous

namespace Identity {
namespace Internal {
IsDirtyDataWidgetMapper::IsDirtyDataWidgetMapper(QObject *parent) :
    QDataWidgetMapper(parent)
{
    //TODO: extract this class into Utils?
}

/** Use this method each time the model gets correctly submitted */
void IsDirtyDataWidgetMapper::onModelSubmitted()
{
    refreshCache();
}

/** Return true if the current values of the mapped widget differs from the original model values */
bool IsDirtyDataWidgetMapper::isDirty() const
{
    Q_ASSERT(orientation() == Qt::Horizontal);
    Q_ASSERT(rootIndex() == QModelIndex());

    // cycle through all widgets the mapper supports
    for(int i = 0; i < model()->columnCount(); i++) {
        QWidget *mapWidget = mappedWidgetAt(i);
        if (mapWidget) {
            const QVariant &current = mapWidget->property(mappedPropertyName(mapWidget));
            const QVariant &orig = _original.value(mapWidget);

            // Special case of null original variant
            if (orig.isNull() && current.toString().isEmpty())
                continue;
            if (current != orig) {
//                qWarning() << patient()->enumToString(Core::IPatient::PatientDataRepresentation(i))
//                           << "orig" << orig
//                           << "current" << current;
                return true;
            }
        }
    }
    return false;
}

/** Overload method (creates the internal cache) */
void IsDirtyDataWidgetMapper::setCurrentIndex(int index)
{
    QDataWidgetMapper::setCurrentIndex(index);
    refreshCache();
}

void IsDirtyDataWidgetMapper::refreshCache()
{
    Q_ASSERT(orientation() == Qt::Horizontal);
    Q_ASSERT(rootIndex() == QModelIndex());
    _original.clear();
    // cycle through all widgets the mapper supports
    for(int i = 0; i < model()->columnCount(); i++) {
        QWidget *mapWidget = mappedWidgetAt(i);
        if (mapWidget) {
            _original.insert(mapWidget, model()->data(model()->index(currentIndex(), i)));
        }
    }
}

class IdentityEditorWidgetPrivate
{
public:
    IdentityEditorWidgetPrivate(IdentityEditorWidget *parent) :
        ui(0),
        m_Mapper(0),
        m_initialized(false),
        m_hasRealPhoto(false),
        m_xmlOnly(false),
        m_availaibleSet(false),
        q(parent)
    {
    }

    ~IdentityEditorWidgetPrivate()
    {
        if (m_Mapper) {
            delete m_Mapper;
            m_Mapper = 0;
        }
        if (ui) {
            delete ui;
            ui = 0;
        }
    }

    void setupUi()
    {
        ui = new Ui::IdentityWidget;
        ui->setupUi(q);
        ui->dob->setDateIcon(theme()->iconFullPath(Core::Constants::ICONDATE));
        ui->dob->setClearIcon(theme()->iconFullPath(Core::Constants::ICONCLEARLINEEDIT));

        ui->zipcodesWidget->initialize(ZipCodes::ZipCodesWidget::GridLayout);

        ui->genderCombo->addItems(genders());
        ui->titleCombo->addItems(titles());

        ui->language->setFlagsIconPath(settings()->path(Core::ISettings::SmallPixmapPath));
        ui->language->setTranslationsPath(settings()->path(Core::ISettings::TranslationsPath));
        ui->language->setCurrentLanguage(QLocale().language());

        ui->login->setIcon(theme()->icon(Core::Constants::ICONEYES));
        ui->password->setIcon(theme()->icon(Core::Constants::ICONEYES));
        ui->password2->setIcon(theme()->icon(Core::Constants::ICONEYES));
        ui->login->setEchoMode(QLineEdit::Normal);
        ui->password->setEchoMode(QLineEdit::Password);
        ui->password2->setEchoMode(QLineEdit::Password);

        Utils::UpperCaseValidator *upperVal = new Utils::UpperCaseValidator(q);
        ui->birthName->setValidator(upperVal);
        ui->secondName->setValidator(upperVal);

        Utils::CapitalizationValidator *capVal = new Utils::CapitalizationValidator(q);
        ui->firstname->setValidator(capVal);

        QObject::connect(ui->photoButton, SIGNAL(clicked()), q, SLOT(photoButton_clicked()));
        //            QObject::connect(ui->genderCombo, SIGNAL(currentIndexChanged(int)), q, SLOT(updateGenderImage()));

        QList<Core::IPhotoProvider *> photoProviderList = pluginManager()->getObjects<Core::IPhotoProvider>();

        if (!photoProviderList.isEmpty()) {
            // sort the PhotoProviders by their priority property - this is done by the IPhotoProvider::operator< and qSort()
            qSort(photoProviderList);

            QAction *photoAction;
            foreach(Core::IPhotoProvider *provider, photoProviderList) {
                //: which IPhotoProvider to get picture from: from URL, from Webcam, from ...
                photoAction = new QAction(provider->displayText(), q);
                QObject::connect(photoAction, SIGNAL(triggered()), provider, SLOT(startReceivingPhoto()));
                QObject::connect(provider, SIGNAL(photoReady(QPixmap)), ui->photoButton, SLOT(setPixmap(QPixmap)));
                photoAction->setData(provider->id());
                ui->photoButton->addAction(photoAction);
            }
            updateDefaultPhotoAction();

        } else {
            LOG_ERROR_FOR(q, "No photoProvider");
            // buggy: the photo saving does not work ATM!
            //                if (ui->photoButton->pixmap().isNull())
            //                    ui->photoButton->setDisabled(true);
        }
        QObject::connect(ui->genderCombo, SIGNAL(currentIndexChanged(int)), q, SLOT(updateGenderImage(int)));
        QObject::connect(ui->photoButton->deletePhotoAction(), SIGNAL(triggered()), q, SLOT(updateGenderImage()));
    }

    void connectPropertiesNotifier()
    {
        QObject::connect(ui->birthName, SIGNAL(textChanged(QString)), q, SIGNAL(birthNameChanged(QString)));
        QObject::connect(ui->secondName, SIGNAL(textChanged(QString)), q, SIGNAL(secondNameChanged(QString)));
        QObject::connect(ui->firstname, SIGNAL(textChanged(QString)), q, SIGNAL(firstNameChanged(QString)));
        QObject::connect(ui->dob, SIGNAL(dateChanged(QDate)), q, SIGNAL(dateOfBirthChanged(QDate)));
        QObject::connect(ui->genderCombo, SIGNAL(currentIndexChanged(int)), q, SIGNAL(genderIndexChanged(int)));
        QObject::connect(ui->genderCombo, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(genderChanged(QString)));
//        QObject::connect(ui->titleCombo, SIGNAL(currentIndexChanged(int)), q, SIGNAL(titleIndexChanged(int)));
        QObject::connect(ui->titleCombo, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(titleChanged(QString)));
//        QObject::connect(ui->photoButton, SIGNAL(), q, SIGNAL(birthNameChanged(QString)));
    }


    void updateDefaultPhotoAction()
    {
        QString defaultId = settings()->value(Patients::Constants::S_DEFAULTPHOTOSOURCE).toString();
        foreach(QAction *action, ui->photoButton->actions()) {
            if (action->data().toString() == defaultId)
                ui->photoButton->setDefaultAction(action);
        }
    }

    // Create the mapper over the Core::IPatient
    void createGenericMapper()
    {
        if (m_Mapper) {
            delete m_Mapper;
            m_Mapper = 0;
        }
        m_Mapper = new IsDirtyDataWidgetMapper(q);
        m_Mapper->setSubmitPolicy(IsDirtyDataWidgetMapper::ManualSubmit);
        m_Mapper->setModel(patient());
        addMapperMapping();
    }

    // Create the mapper over a specified model. No-auto mapping
    void createModelMapper(QAbstractItemModel *model)
    {
        if (m_xmlOnly) {
            if (m_Mapper) {
                delete m_Mapper;
                m_Mapper = 0;
            }
            m_Model = 0;
            return;
        }
        m_xmlOnly = false;
        if (m_Mapper) {
            delete m_Mapper;
            m_Mapper = 0;
        }
        m_Mapper = new IsDirtyDataWidgetMapper(q);
        m_Mapper->setSubmitPolicy(IsDirtyDataWidgetMapper::ManualSubmit);
        m_Mapper->setModel(model);
        m_Model = model;
    }

    // Add mapping to the mapper
    void addMapperMapping()
    {
        m_Mapper->addMapping(ui->birthName, Core::IPatient::BirthName, "text");
        m_Mapper->addMapping(ui->secondName, Core::IPatient::SecondName, "text");
        m_Mapper->addMapping(ui->firstname, Core::IPatient::Firstname, "text");
        m_Mapper->addMapping(ui->genderCombo, Core::IPatient::GenderIndex, "currentIndex");
        m_Mapper->addMapping(ui->titleCombo, Core::IPatient::TitleIndex, "currentIndex");
        m_Mapper->addMapping(ui->dob, Core::IPatient::DateOfBirth, "date");

        ui->zipcodesWidget->addMapping(m_Mapper, Core::IPatient::Street, ZipCodes::ZipCodesWidget::StreetPlainText);
        ui->zipcodesWidget->addMapping(m_Mapper, Core::IPatient::City, ZipCodes::ZipCodesWidget::CityPlainText);
        ui->zipcodesWidget->addMapping(m_Mapper, Core::IPatient::ZipCode, ZipCodes::ZipCodesWidget::ZipcodePlainText);
        ui->zipcodesWidget->addMapping(m_Mapper, Core::IPatient::StateProvince, ZipCodes::ZipCodesWidget::StateProvincePlainText);
        ui->zipcodesWidget->addMapping(m_Mapper, Core::IPatient::Country, ZipCodes::ZipCodesWidget::CountryIso);
    }

    // Get the corresponding QWidget pointer
    QWidget *getWidget(IdentityEditorWidget::AvailableWidget widget)
    {
        switch (widget) {
        case IdentityEditorWidget::TitleIndex: return ui->titleCombo;
        case IdentityEditorWidget::BirthName: return ui->birthName;
        case IdentityEditorWidget::SecondName: return ui->secondName;
        case IdentityEditorWidget::FirstName: return ui->firstname;
        case IdentityEditorWidget::GenderIndex:
        case IdentityEditorWidget::Gender: return ui->genderCombo;
        case IdentityEditorWidget::Language_QLocale: return ui->language;
        case IdentityEditorWidget::DateOfBirth: return ui->dob;
        case IdentityEditorWidget::DateOfDeath: return 0; //TODO: ui->dod;
        case IdentityEditorWidget::Photo: return ui->photoButton;
        default: break;
        }
        return 0;
    }

    QByteArray getWidgetPropertyForMapper(IdentityEditorWidget::AvailableWidget widget)
    {
        switch (widget) {
        case IdentityEditorWidget::TitleIndex:
        case IdentityEditorWidget::Gender:
        case IdentityEditorWidget::GenderIndex:
            return "currentIndex";
        case IdentityEditorWidget::BirthName:
        case IdentityEditorWidget::SecondName:
        case IdentityEditorWidget::FirstName:
            return "text";
        case IdentityEditorWidget::DateOfBirth:
        case IdentityEditorWidget::DateOfDeath:
            return "date";
//        case IdentityEditorWidget::Photo: return "pixmap";
        case IdentityEditorWidget::Language_QLocale:
            return "currentLanguage";
        default: break;
        }
        return "";
    }

    bool fromXml(const QString &xml)
    {
        if (!m_xmlOnly)
            return false;
        m_lastXml = xml;

        // Read XML
        QHash<QString, QString> tags;
        if (!Utils::readXml(xml, "Identity", tags))
            return false;
        ui->birthName->setText(tags.value(::XML_NAME1));
        ui->secondName->setText(tags.value(::XML_NAME2));
        ui->firstname->setText(tags.value(::XML_FIRSTNAME));
        ui->titleCombo->setCurrentIndex(ui->titleCombo->findText(tags.value(::XML_TITLE)));
        int id = -1;
        const QString &g = tags.value(::XML_GENDER);
        if (g == "M")
            id = 0;
        else if (g == "F")
            id = 1;
        if (g == "H")
            id = 2;
        ui->genderCombo->setCurrentIndex(id);
        ui->language->setCurrentIsoLanguage(tags.value(::XML_LANG));
        ui->dob->setDate(QDate::fromString(tags.value(::XML_DOB), Qt::ISODate));
        //ui->dod->setText(tags.value(::XML_DOD));
        ui->photoButton->setPixmap(Utils::pixmapFromBase64(tags.value(::XML_PHOTO).toAscii()));

        ui->zipcodesWidget->setStreet(tags.value(::XML_STREET));
        ui->zipcodesWidget->setCountry(tags.value(::XML_COUNTRY));
        ui->zipcodesWidget->setCity(tags.value(::XML_CITY));
        ui->zipcodesWidget->setZipCode(tags.value(::XML_ZIPCODE));
        ui->zipcodesWidget->setStateProvince(tags.value(::XML_PROVINCE));
        return true;
    }

    QString toXml()
    {
        QHash<QString, QString> tags;
        tags.insert(::XML_NAME1, ui->birthName->text());
        tags.insert(::XML_NAME2, ui->secondName->text());
//        tags.insert(::XML_NAME3, ui->);
//        tags.insert(::XML_NAME4, ui->);
        tags.insert(::XML_FIRSTNAME, ui->firstname->text());
        tags.insert(::XML_TITLE, ui->titleCombo->currentText());
        switch (ui->genderCombo->currentIndex()) {
        case 0: //Male
            tags.insert(::XML_GENDER, "M");
            break;
        case 1: //Female
            tags.insert(::XML_GENDER, "F");
            break;
        case 2: //Herma
            tags.insert(::XML_GENDER, "H");
            break;
        }
        tags.insert(::XML_LANG, ui->language->currentLanguageIsoName());
        tags.insert(::XML_DOB, ui->dob->date().toString(Qt::ISODate));
        //tags.insert(::XML_DOD, ui->dod->date().toString(Qt::ISODate));
        tags.insert(::XML_PHOTO, Utils::pixmapToBase64(ui->photoButton->pixmap()));
        tags.insert(::XML_STREET, ui->zipcodesWidget->street());
        tags.insert(::XML_CITY, ui->zipcodesWidget->city());
        tags.insert(::XML_PROVINCE, ui->zipcodesWidget->stateProvince());
        tags.insert(::XML_COUNTRY, ui->zipcodesWidget->countryIso());
        tags.insert(::XML_ZIPCODE, ui->zipcodesWidget->zipCode());
        tags.insert(::XML_LOGIN, Utils::crypt(ui->login->text()));
        tags.insert(::XML_PASSWORD, Utils::crypt(ui->password->text()));
        return Utils::createXml("Identity", tags, 2);
    }

    void retranslate()
    {
        if (ui) {
            ui->retranslateUi(q);
            ui->birthName->setPlaceholderText(QApplication::translate("IdentityEditorWidget", "Birth Name"));
            ui->secondName->setPlaceholderText(QApplication::translate("IdentityEditorWidget", "Second Name"));
            ui->firstname->setPlaceholderText(QApplication::translate("IdentityEditorWidget", "Firstname"));
        }
    }

public:
    Ui::IdentityWidget *ui;
    IsDirtyDataWidgetMapper *m_Mapper;
    QAbstractItemModel *m_Model;
    QPixmap m_Photo;
    bool m_initialized, m_hasRealPhoto, m_xmlOnly, m_availaibleSet;
    QString m_lastXml;

private:
    QAction *m_deletePhotoAction;
    IdentityEditorWidget *q;
};

}  // namespace Internal
}  // namespace Identity

/**
 * \brief Create an Identity viewer with the specific \e mode of edition.
 * By default, the view is connected to the Core::IPatient but
 * you can specify the QAbstractItemModel to use, or use this view with
 * an unique XML in/out. \n
 * You must firstly initialize the object with initialize().
 * \sa setModel(), setXmlInOut()
*/
IdentityEditorWidget::IdentityEditorWidget(QWidget *parent) :
    QWidget(parent),
    d(new Internal::IdentityEditorWidgetPrivate(this))
{
    setObjectName("Patient::IdentityEditorWidget");
    d->setupUi();
    d->connectPropertiesNotifier();

    d->retranslate();
}

IdentityEditorWidget::~IdentityEditorWidget()
{
    if (d)
        delete d;
    d = 0;
}

/** Initialize the view with the default Core::IPatient model */
bool IdentityEditorWidget::initialize()
{
    if (d->m_initialized)
        return true;
    d->createGenericMapper();
    d->m_Mapper->toFirst();
    updateGenderImage();
    connect(patient(), SIGNAL(currentPatientChanged()), this, SLOT(onCurrentPatientChanged()));
    d->m_initialized = true;
    return true;
}

/**
 * Define the widgets to include in the view . You must initialize() the widget before
 * defining the view's content.\n
 * \note: You can only set the widgets once.
 */
void IdentityEditorWidget::setAvailableWidgets(AvailableWidgets widgets)
{
    if (d->m_availaibleSet)
        return;
    if (!d->ui)
        return;
    d->ui->titleCombo->setEnabled(widgets & TitleIndex);
    d->ui->genderCombo->setEnabled(widgets & Gender || widgets & GenderIndex);
    d->ui->birthName->setEnabled(widgets & BirthName);
    d->ui->secondName->setEnabled(widgets & SecondName);
    d->ui->firstname->setEnabled(widgets & FirstName);
    d->ui->language->setEnabled(widgets & Language_QLocale);
    d->ui->dob->setEnabled(widgets & DateOfBirth);
    //d->ui->dod->setEnabled(widgets & DateOfDeath);
    d->ui->photoButton->setEnabled(widgets & Photo);

//    d->ui->titleLabel->setVisible(d->ui->titleCombo->isEnabled());
    d->ui->titleCombo->setVisible(d->ui->titleCombo->isEnabled());
    d->ui->birthName->setVisible(d->ui->birthName->isEnabled());
//    d->ui->birthNameLabel->setVisible(d->ui->birthName->isEnabled());
    d->ui->secondName->setVisible(d->ui->secondName->isEnabled());
    d->ui->secondNameLabel->setVisible(d->ui->secondName->isEnabled());
    d->ui->firstname->setVisible(d->ui->firstname->isEnabled());
    d->ui->firstnameLabel->setVisible(d->ui->firstname->isEnabled());
    d->ui->genderCombo->setVisible(d->ui->genderCombo->isEnabled());
//    d->ui->genderLabel->setVisible(d->ui->genderCombo->isEnabled());
    d->ui->language->setVisible(d->ui->language->isEnabled());
    d->ui->languageLabel->setVisible(d->ui->language->isEnabled());
    d->ui->dob->setVisible(d->ui->dob->isEnabled());
    d->ui->dobLabel->setVisible(d->ui->dob->isEnabled());
    //d->ui->dod->setVisible(d->ui->dod->isEnabled());
    //d->ui->dodLabel->setVisible(d->ui->dod->isEnabled());
    d->ui->photoButton->setVisible(d->ui->photoButton->isEnabled());

    bool showAddress = (widgets & Street)
            || (widgets & City)
            || (widgets & Zipcode)
            || (widgets & Province)
            || (widgets & Country_TwoCharIso)
            || (widgets & Country_QLocale);
    d->ui->zipcodesWidget->setEnabled(showAddress);

    bool showLog = (widgets & Extra_Login)
            || (widgets & Extra_Password)
            || (widgets & Extra_ConfirmPassword);
    d->ui->loginGroup->setVisible(showLog);
    d->ui->password2->setVisible(widgets & Extra_ConfirmPassword);
    d->ui->login->setEnabled(widgets & Extra_Login);
    d->ui->password->setEnabled(widgets & Extra_Password);
    d->ui->password2->setEnabled(widgets & Extra_ConfirmPassword);

    d->ui->zipcodesWidget->setVisible(d->ui->zipcodesWidget->isEnabled());
    d->ui->login->setVisible(d->ui->login->isEnabled());
    d->ui->password->setVisible(d->ui->password->isEnabled());
    d->ui->password2->setVisible(d->ui->password2->isEnabled());
    d->m_availaibleSet = true;
}

/** Set/unset the view in read-only mode */
void IdentityEditorWidget::setReadOnly(bool readOnly)
{
    d->ui->birthName->setReadOnly(readOnly);
    d->ui->secondName->setReadOnly(readOnly);
    d->ui->firstname->setReadOnly(readOnly);
    d->ui->dob->setReadOnly(readOnly);

    d->ui->genderCombo->setEnabled(readOnly);
    d->ui->titleCombo->setEnabled(readOnly);
    d->ui->language->setEnabled(readOnly);
    d->ui->photoButton->setEnabled(readOnly);

    d->ui->zipcodesWidget->setReadOnly(readOnly);

    d->ui->login->setReadOnly(readOnly);
    d->ui->password->setReadOnly(readOnly);
    d->ui->password2->setReadOnly(readOnly);
}

/**
 * Clear the view. If you are using a model or the Core::IPatient,
 * you don't need to clear the view.
 */
void IdentityEditorWidget::clear()
{
    d->ui->titleCombo->setCurrentIndex(-1);
    d->ui->genderCombo->setCurrentIndex(-1);
    d->ui->language->setCurrentLanguage(QLocale().language());
    d->ui->birthName->clear();
    d->ui->secondName->clear();
    d->ui->firstname->clear();
    d->ui->dob->clear();
    d->ui->photoButton->clearPixmap();
    d->ui->zipcodesWidget->clear();
    d->ui->login->clear();
    d->ui->password->clear();
    d->ui->password2->clear();
//    d->ui->dod->clear();
}

/**
 * If you don't want to use the identity editor over the Core::IPatient
 * (which represents the current patient), you can set your own QAbstractItemModel.
 * The mapper is auto-selecting the first row of the model.\n
 * Use the setCurrentIndex() to set the current index of the current editing index.\n
 * \note The model should be a QTableAbstractItemModel. There are no auto-mapping.
 * \sa setCurrentIndex(), addMapping()
 */
void IdentityEditorWidget::setModel(QAbstractItemModel *model)
{
    d->createModelMapper(model);
//    d->m_Mapper->toFirst();
    updateGenderImage();
}

/**
 * If you want to use your own QAbstractItemModel, you can, once registered with
 * setModel(), define the mappings. Use the AvailableWidget to define the widget and
 * the \e modelIndex to define the corresponding model index. \n
 * Return \e true is the mapping was created, otherwise return \e false.
 */
bool IdentityEditorWidget::addMapping(AvailableWidget widget, int modelIndex)
{
    Q_ASSERT(d->m_Model);
    Q_ASSERT(d->m_Mapper);
    if (!d->m_Model)
        return false;
    QWidget *w = d->getWidget(widget);
    if (w) {
        // widget is directly accessible from the ui
        d->m_Mapper->addMapping(w, modelIndex, d->getWidgetPropertyForMapper(widget));
    } else {
        // widget is not directly accessible from the ui
        if (widget == Street ||
                widget == City ||
                widget == Zipcode ||
                widget == Province ||
                widget == Country_QLocale ||
                widget == Country_TwoCharIso) {
            // Use the zipcodesWidget
            ZipCodes::ZipCodesWidget::Mapping zipMapping;
            switch (widget) {
            case Street: zipMapping = ZipCodes::ZipCodesWidget::StreetPlainText; break;
            case City: zipMapping = ZipCodes::ZipCodesWidget::CityPlainText; break;
            case Zipcode: zipMapping = ZipCodes::ZipCodesWidget::ZipcodePlainText; break;
            case Province: zipMapping = ZipCodes::ZipCodesWidget::StateProvincePlainText; break;
            case Country_TwoCharIso: zipMapping = ZipCodes::ZipCodesWidget::CountryIso; break;
            case Country_QLocale: zipMapping = ZipCodes::ZipCodesWidget::CountryLocale; break;
            default: break;
            }
            d->ui->zipcodesWidget->addMapping(d->m_Mapper, modelIndex, zipMapping);
        } else {
            // Error
            return false;
        }
    }
    return true;
}

/**
 * Set the widget to work without any model. The input and output
 * can be defined with fromXml(), toXml(). The submit() method
 * is inhibited.
 */
void IdentityEditorWidget::setXmlInOut(bool xmlonly)
{
    d->m_xmlOnly = xmlonly;
    d->createModelMapper(0);
    updateGenderImage();
}

/**
 * Return true if the widget works only with an XML input/output
 * \sa setXmlInOut()
*/
bool IdentityEditorWidget::isXmlInOut() const
{
    return d->m_xmlOnly;
}

/**
 * Transform the current data to an XML content.
 * \sa fromXml(), setXmlInOut()
 */
QString IdentityEditorWidget::toXml() const
{
    return d->toXml();
}

/**
 * If you don't want to use the identity editor over the Core::IPatient
 * (which represents the current patient), you can set your own Patients::PatientModel.
 * Use the setModel() and setCurrentIndex() to set the current index of
 * the current editing index.
 * \sa setModel(), initiliaze()
 */
void IdentityEditorWidget::setCurrentIndex(const QModelIndex &modelIndex)
{
//    qWarning() << modelIndex << (modelIndex.model() == d->m_Mapper->model());
    if (modelIndex.model() == d->m_Mapper->model())
        d->m_Mapper->setCurrentIndex(modelIndex.row());
}

/**
 * Test the validity of the "actually shown" identity. The default implementation
 * test the content of the firstname, birthname, gender & DOB.
 * When subclassing, if you return false, the object can not submit to the model.
 */
bool IdentityEditorWidget::isIdentityValid() const
{
    if (d->ui->birthName->text().isEmpty()) {
        Utils::warningMessageBox(tr("You must specify a birthname."),
                                 tr("You can not create a patient without a birthname"),
                                 "", tr("No birthname"));
        d->ui->birthName->setFocus();
        return false;
    }
    if (d->ui->firstname->text().isEmpty()) {
        Utils::warningMessageBox(tr("You must specify a first name."),
                                 tr("You can not create a patient without a first name"),
                                 "", tr("No firstname"));
        d->ui->firstname->setFocus();
        return false;
    }
    if (d->ui->dob->date().isNull()) {
        Utils::warningMessageBox(tr("You must specify a date of birth."),
                                 tr("You can not create a patient without a date of birth"),
                                 "", tr("No date of birth"));
        d->ui->dob->setFocus();
        return false;
    }
    if (d->ui->genderCombo->currentIndex() == -1) {
        Utils::warningMessageBox(tr("You must specify a gender."),
                                 tr("You can not create a patient without a gender"),
                                 "", tr("No gender"));
        d->ui->genderCombo->setFocus();
        return false;
    }
    return true;
}

/** \brief Identity has been modified by the user? */
bool IdentityEditorWidget::isModified() const
{
    if (d->m_xmlOnly)
        return d->m_lastXml != d->toXml();

    return d->m_Mapper->isDirty();
}

/** Return the current editing value */
QString IdentityEditorWidget::currentTitle() const
{
    return d->ui->titleCombo->currentText();
}

/** Return the current editing value */
QString IdentityEditorWidget::currentBirthName() const
{
    return d->ui->birthName->text();
}

/** Return the current editing value */
QString IdentityEditorWidget::currentSecondName() const
{
    return d->ui->secondName->text();
}

/** Return the current editing value */
QString IdentityEditorWidget::currentFirstName() const
{
    return d->ui->firstname->text();
}

/** Return the current editing value */
QString IdentityEditorWidget::currentGender() const
{
    int genderIndex = -1;
    genderIndex = d->ui->genderCombo->currentIndex();

    if (IN_RANGE_STRICT_MAX(genderIndex, 0, Trans::ConstantTranslations::genders().count()))
        return Trans::ConstantTranslations::genders()[genderIndex];

    return QString();
}

/** Return the current editing value */
int IdentityEditorWidget::currentGenderIndex() const
{
    return d->ui->genderCombo->currentIndex();
}

/** Return the current editing value */
QDate IdentityEditorWidget::currentDateOfBirth() const
{
    return d->ui->dob->date();
}

/** Return the current editing value */
QString IdentityEditorWidget::currentLanguage() const
{
    return d->ui->language->currentLanguageName();
}

/*!
 * \brief Returns current widget photo of patient
 *
 * \returns QPixmap current widget photo of patient.
 * If patient has no photo in the current widget (this function does NOT query the database!),
 * it returns a QPixmap()
 */
QPixmap IdentityEditorWidget::currentPhoto() const
{
    QPixmap photo;    
    photo = hasPhoto() ? d->ui->photoButton->pixmap() : QPixmap();
    return photo;
}

/**
 * Return \e true if the identity photo was populated with a user pixmap.
 * Return \e false, if the photo is populated with the default gender pixmap.
 */
bool IdentityEditorWidget::hasPhoto() const
{
    return (!d->ui->photoButton->pixmap().isNull());
}

/**
 * Submit the Identity to the model (Core::IPatient or your QAbstractItemModel).
 * If you set the XML in/out only (setXmlInOut()), does nothing and return true.
 * \sa setModel(), setXmlInOut()
 */
bool IdentityEditorWidget::submit()
{
    if (d->m_xmlOnly)
        return true;
    if (d->m_Mapper) {
        if (!d->m_Mapper->submit())
            return false;
        d->m_Mapper->onModelSubmitted();
    }
    return true;
}

/** Force refreshing the photo */
void IdentityEditorWidget::updateGenderImage()
{
    updateGenderImage(d->ui->genderCombo->currentIndex());
}

/**
 * \internal
 * Connected to the gender ui combobox, update the gender pixmap.
 */
void IdentityEditorWidget::updateGenderImage(int genderIndex)
{
    d->ui->photoButton->setGenderImage(genderIndex);
}

/**
 * Set the current data from an XML content.
 * \sa toXml(), setXmlInOut()
 */
bool IdentityEditorWidget::fromXml(const QString &xml)
{
    clear();
    return d->fromXml(xml);
}

void IdentityEditorWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d->retranslate();
        break;
    default:
        break;
    }
    QWidget::changeEvent(e);
}

/**
 * \internal
 * Triggers the default action of the photo button.
 */
void IdentityEditorWidget::photoButton_clicked()
{
    QAction *action = d->ui->photoButton->defaultAction();
    if (action)
        action->trigger();
}

/** Force UI to update with the new current patient data */
void IdentityEditorWidget::onCurrentPatientChanged()
{
    // With XML editing we do not follow the Core::IPatient
    if (d->m_xmlOnly)
        return;
    d->m_Mapper->setCurrentModelIndex(QModelIndex());
    d->m_Mapper->setCurrentModelIndex(patient()->currentPatientIndex());
    updateGenderImage();
}