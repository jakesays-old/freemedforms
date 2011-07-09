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
  \class Patients::IdentityWidget
  \brief Presents the identity of a patient.
*/

#include "identitywidget.h"
#include "patientmodel.h"
#include "patientbase.h"
#include "constants_db.h"

#include "ui_identitywidget.h"
#include "ui_identityviewer.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/constants_tokensandsettings.h>
#include <coreplugin/ipatient.h>

#include <utils/global.h>
#include <translationutils/constanttranslations.h>

#include <QDataWidgetMapper>
#include <QDir>
#include <QFileDialog>


#include <QDebug>

using namespace Patients;
using namespace Trans::ConstantTranslations;

static inline Core::ISettings *settings() {return Core::ICore::instance()->settings();}
static inline Patients::Internal::PatientBase *patientBase() {return Patients::Internal::PatientBase::instance();}

/**
  \todo Users can add pages in the identity widget using the XMLForm --> create a <Form> named \e Identity
  \todo Create a viewUi for the readonly mode (more compact)
*/

namespace Patients {
namespace Internal {
class IdentityWidgetPrivate
{
public:
    IdentityWidgetPrivate(IdentityWidget *parent, IdentityWidget::EditMode mode) :
            editUi(0), viewUi(0), m_Mapper(0), m_EditMode(mode), q(parent)
    {
        if (mode==IdentityWidget::ReadOnlyMode) {
            viewUi = new Ui::IdentityViewer;
            viewUi->setupUi(q);
        } else {
            editUi = new Ui::IdentityWidget;
            editUi->setupUi(q);
            editUi->genderCombo->addItems(genders());
            editUi->titleCombo->addItems(titles());
//            editUi->dob->setDisplayFormat(QLocale().dateFormat(QLocale::LongFormat));
            q->connect(editUi->photoButton, SIGNAL(clicked()), q, SLOT(photoButton_clicked()));
        }
    }

    ~IdentityWidgetPrivate()
    {
        if (editUi) {
            delete editUi;
            editUi = 0;
        }
        if (viewUi) {
            delete viewUi;
            viewUi = 0;
        }
        if (m_Mapper) {
            delete m_Mapper;
            m_Mapper = 0;
        }
    }

    void createMapper()
    {
        if (m_EditMode == IdentityWidget::ReadWriteMode) {
            if (m_Mapper) {
                delete m_Mapper;
                m_Mapper = 0;
            }
            m_Mapper = new QDataWidgetMapper(q);
            m_Mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
            m_Mapper->setModel(m_PatientModel);
            m_Mapper->addMapping(editUi->birthName, Core::IPatient::BirthName, "text");
            m_Mapper->addMapping(editUi->secondName, Core::IPatient::SecondName, "text");
            m_Mapper->addMapping(editUi->firstname, Core::IPatient::Firstname, "text");
            m_Mapper->addMapping(editUi->genderCombo, Core::IPatient::GenderIndex, "currentIndex");
            m_Mapper->addMapping(editUi->titleCombo, Core::IPatient::TitleIndex, "currentIndex");
            m_Mapper->addMapping(editUi->dob, Core::IPatient::DateOfBirth);
            m_Mapper->addMapping(editUi->street, Core::IPatient::Street, "plainText");
            m_Mapper->addMapping(editUi->city, Core::IPatient::City, "text");
            m_Mapper->addMapping(editUi->zipcode, Core::IPatient::ZipCode, "text");
            m_Mapper->addMapping(editUi->country, Core::IPatient::Country, "text");
            m_Mapper->toFirst();
        }
    }
public:
    Ui::IdentityWidget *editUi;
    Ui::IdentityViewer *viewUi;
    QDataWidgetMapper *m_Mapper;
    Patients::PatientModel *m_PatientModel;
    IdentityWidget::EditMode m_EditMode;

private:
    IdentityWidget *q;
};
}  // End namespace Internal
}  // End namespace Patients

/**
  \brief Create a Identity viewer with the specific \e mode of edition.
  You must specify the PatientModel to use
  \sa IdentityWidget::EditMode, IdentityWidget::setCurrentPatientModel
*/
IdentityWidget::IdentityWidget(QWidget *parent, EditMode mode) :
    QWidget(parent),
    d(new Internal::IdentityWidgetPrivate(this, mode))
{
}

IdentityWidget::~IdentityWidget()
{
    delete d;
}

/** \brief Define the model to use. */
void IdentityWidget::setCurrentPatientModel(Patients::PatientModel *model)
{
    d->m_PatientModel = model;
    d->createMapper();
}

/** \brief Return the actual PatientModel or 0 if it was not set. */
Patients::PatientModel *IdentityWidget::patientModel() const
{
    return d->m_PatientModel;
}

/** \brief Change the current identity to the model index \e patientIndex. */
void IdentityWidget::setCurrentIndex(const QModelIndex &patientIndex)
{
    QPixmap photo = d->m_PatientModel->index(patientIndex.row(), Core::IPatient::Photo).data().value<QPixmap>();
    if (d->m_Mapper) {
        d->m_Mapper->setCurrentModelIndex(patientIndex);
        d->editUi->photoButton->setIcon(photo);
    } else {
        d->viewUi->name->clear();
        d->viewUi->photoLabel->clear();
        d->viewUi->ageDOB->clear();
        d->viewUi->sex->clear();
        d->viewUi->fullAdress1->clear();
        d->viewUi->fullAdress2->clear();
        QString name = d->m_PatientModel->index(patientIndex.row(), Core::IPatient::FullName).data().toString();
        const QString &title = d->m_PatientModel->index(patientIndex.row(), Core::IPatient::Title).data().toString();
        if (!title.isEmpty())
            name.prepend(title + " ");
        d->viewUi->name->setText(name);
        d->viewUi->photoLabel->setPixmap(photo);
        const QString &age = d->m_PatientModel->index(patientIndex.row(), Core::IPatient::Age).data().toString();
        const QString &dob = d->m_PatientModel->index(patientIndex.row(), Core::IPatient::DateOfBirth).data().toDate().toString(QLocale().dateFormat(QLocale::LongFormat));
        if (!age.isEmpty() && !dob.isEmpty())
            d->viewUi->ageDOB->setText(age + "; " + tr("born on") + " " + dob);
        const QIcon &icon = d->m_PatientModel->index(patientIndex.row(), Core::IPatient::IconizedGender).data().value<QIcon>();
        d->viewUi->sex->setPixmap(icon.pixmap(QSize(64,64)));
        d->viewUi->fullAdress1->setText(d->m_PatientModel->index(patientIndex.row(), Core::IPatient::Street).data().toString());
        d->viewUi->fullAdress2->setText(d->m_PatientModel->index(patientIndex.row(), Core::IPatient::City).data().toString() + " " +
                                        d->m_PatientModel->index(patientIndex.row(), Core::IPatient::ZipCode).data().toString() + " " +
                                        d->m_PatientModel->index(patientIndex.row(), Core::IPatient::Country).data().toString());
    }
}

/** \brief Test the validity of the "actually showning" identity. */
bool IdentityWidget::isIdentityValid() const
{
    if (d->editUi->birthName->text().isEmpty()) {
        Utils::warningMessageBox(tr("You must specify a birthname."),
                                 tr("You can not create a patient without a birthname"),
                                 "", tr("No birthname"));
        return false;
    }
    if (d->editUi->firstname->text().isEmpty()) {
        Utils::warningMessageBox(tr("You must specify a firstname."),
                                 tr("You can not create a patient without a firstname"),
                                 "", tr("No firstname"));
        return false;
    }
    if (d->editUi->dob->date().isNull()) {
        Utils::warningMessageBox(tr("You must specify a date of birth."),
                                 tr("You can not create a patient without a date of birth"),
                                 "", tr("No date of birth"));
        return false;
    }
    if (d->editUi->genderCombo->currentIndex() < 0) {
        Utils::warningMessageBox(tr("You must specify a gender."),
                                 tr("You can not create a patient without a gender"),
                                 "", tr("No gender"));
        return false;
    }
    return true;
}

/** \brief Avoid duplicates. */
bool IdentityWidget::isIdentityAlreadyInDatabase() const
{
    // check database for doublons
    QString where = QString("`%1`='%2' AND ").arg(patientBase()->fieldName(Constants::Table_IDENT, Constants::IDENTITY_NAME), d->editUi->birthName->text());
    if (!d->editUi->secondName->text().isEmpty())
        where += QString("`%1`='%2' AND ").arg(patientBase()->fieldName(Constants::Table_IDENT, Constants::IDENTITY_SECONDNAME), d->editUi->secondName->text());
    where += QString("`%1`='%2'").arg(patientBase()->fieldName(Constants::Table_IDENT, Constants::IDENTITY_FIRSTNAME), d->editUi->firstname->text());
    return (patientBase()->count(Constants::Table_IDENT, Constants::IDENTITY_NAME, where)>0);
}

/** \brief Identity has been modified bu te user ? */
bool IdentityWidget::isModified() const
{
    if (d->m_EditMode==ReadOnlyMode)
        return false;
    return true;
}

/** \brief Submit the Identity to the model and the database. */
bool IdentityWidget::submit()
{
    if ((d->m_EditMode == ReadWriteMode) && (d->m_Mapper))
            return d->m_Mapper->submit();
    return false;
}

void IdentityWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        if (d->editUi)
            d->editUi->retranslateUi(this);
        break;
    default:
        break;
    }
}

/** \brief Patient's Photograph selector. */
void IdentityWidget::photoButton_clicked()
{
    if (d->m_EditMode != ReadWriteMode)
        return;

    // if a photo is already loaded --> ask user what to do
    // show openfiledialog
    QString file;
    file = QFileDialog::getOpenFileName(this, tr("Choose a photo"),
                                        QDir::homePath(),
                                        "Image (*.png *.jpg *.gif *.tiff)");
    if (file.isEmpty())
        return;

    // load pixmap
    QPixmap photo(file);
    if (photo.isNull())
        return;

    // resize pixmap
    photo = photo.scaled(QSize(50,50), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // change button pixmap
    QIcon icon(photo);
    d->editUi->photoButton->setIcon(icon);

    // save to DB
    d->m_PatientModel->setData(d->m_PatientModel->index(d->m_Mapper->currentIndex(), Core::IPatient::Photo), photo);
}

