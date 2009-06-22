/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   Copyright (C) 2008-2009 by Eric MAEKER                                *
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
 ***************************************************************************/

/**
  \class tkUserPasswordDialog
  \brief Dialog to use when user want to change its password. You need to know the actual password of the user.
    Dialog is QDialog::Accepted only if all is good (old password correctly written and new password verification
    passed). You can verify this state of validity using canGetNewPassword() before
    retreiving the new crypted password with cryptedPassword().

    cryptedPassword() returns an empty string if you call it when an error occured.
  \ingroup usertoolkit widget_usertoolkit usermanager
*/

#include "tkUserPasswordDialog.h"
#include "tkUserPasswordDialog_p.h"

// include toolkit headers
#include <tkGlobal.h>
#include <tkLineEditEchoSwitcher.h>

// include usertoolkit headers
#include <tkUserGlobal.h>

// include Qt headers
#include <QMessageBox>

tkUserPasswordDialog::tkUserPasswordDialog( const QString & actualCryptedPassword, QWidget * parent ) :
    QDialog(parent)
{
   Q_ASSERT_X( !actualCryptedPassword.isEmpty(), "tkUserPasswordDialog", "actualCryptedPassword cannot be empty." );
   if ( actualCryptedPassword.isEmpty() )
       return;
   d = new tkUserPasswordDialogPrivate(this);
   d->m_ActualPass = actualCryptedPassword;
}

bool tkUserPasswordDialog::canGetNewPassword()
{
    return d->m_AllIsGood;
}

QString tkUserPasswordDialog::cryptedPassword()
{
    if ( d->m_AllIsGood )
        return d->m_CryptedNewPass;
    return QString::null;
}

tkUserPasswordDialogPrivate::tkUserPasswordDialogPrivate( QDialog * parent ) :
    QObject(parent)
{
   setupUi(parent);
   newPass->toogleEchoMode();
   newControl->toogleEchoMode();
   old->toogleEchoMode();
   m_Parent = parent;
   m_AllIsGood = false;
   // connect buttons
   connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

void tkUserPasswordDialogPrivate::accept()
{
    const QString & cryptedNewPass = tkUserGlobal::crypt( newPass->lineEdit()->text() );
    const QString & oldPass = tkUserGlobal::crypt( old->lineEdit()->text() );
    if ( ( oldPass == m_ActualPass ) &&
         ( newPass->lineEdit()->text() == newControl->lineEdit()->text() ) ) {
        m_AllIsGood = true;
        m_CryptedNewPass = cryptedNewPass;
        m_Parent->accept();
    } else {
        m_AllIsGood = false;
        QMessageBox mb( m_Parent );
        mb.setIcon( QMessageBox::Critical );
        mb.setWindowTitle( m_Parent->windowTitle() );
        mb.setText( tr( "Password can not be change." ) );
        mb.setStandardButtons(QMessageBox::Cancel);
        mb.setDefaultButton(QMessageBox::Cancel);
        if ( oldPass != m_ActualPass )
            mb.setInformativeText( tr( "The old password is not correct. Please retry with the correct password." ) );
        else
            mb.setInformativeText( tr( "Wrong password confirmation." ) );
        mb.exec();
        m_Parent->reject();
    }
}
